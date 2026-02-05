# `dinh/zcu104-a53-mobilenetv1-baseline/vitis_src/ref_kernels.c` 코드 리뷰 보고서

이 보고서는 `zcu104-a53-mobilenetv1-baseline` 프로젝트의 핵심 커널 함수들을 구현하고 있는 `ref_kernels.c` 파일에 대한 상세한 코드 리뷰 결과를 담고 있습니다. 이 파일은 양자화된 딥러닝 모델의 추론을 위한 기본적인 연산들, 즉 양자화/역양자화 헬퍼 함수, Depthwise Convolution, Pointwise Convolution, Global Average Pooling, Softmax 등을 포함하고 있습니다.

## 1. 헤더 및 유틸리티 함수: 양자화 변환의 기초

### 1.1 포함된 헤더

```c
#include "ref_kernels.h" // 텐서 구조체 등 정의
#include <math.h>        // expf, lrintf 등 수학 함수
#include <string.h>      // (이 파일에서는 직접 사용되지 않음, 일반적인 포함)
```

*   **`ref_kernels.h`**: 프로젝트의 텐서 구조체(`tensor_u8_nhwc_t`)와 같은 공통 정의를 포함합니다.
*   **`math.h`**: `expf` (float 지수 함수), `lrintf` (가장 가까운 정수로 반올림) 등 부동 소수점 연산을 위해 필요합니다.

### 1.2 양자화/역양자화 헬퍼 함수

이 두 함수는 양자화된 신경망에서 실수(float) 값과 양자화된 정수(uint8_t) 값 사이의 변환을 담당합니다.

#### `deq` (Dequantize - 역양자화)

```c
static inline float deq(uint8_t q, float s, int zp){ 
  return s*((int)q - zp); // real value = scale * (quantized_value - zero_point)
}
```

*   **역할**: `uint8_t` 타입의 양자화된 정수 `q`를 실제 `float` 값으로 변환합니다.
*   **원리**: 양자화된 값 `q`에서 영점 `zp`를 뺀 후, 스케일 `s`를 곱하여 `float` 실수 값으로 되돌립니다.
*   **`static inline`**: 이 함수가 이 파일 내에서만 사용되고 컴파일러가 가능한 한 인라인(inline)으로 처리하여 함수 호출 오버헤드를 줄이도록 지시합니다. 이는 임베디드 시스템에서 성능에 중요합니다.

#### `req` (Requantize - (재)양자화)

```c
static inline uint8_t req(float r, float s, int zp){
    int q = zp + (int)lrintf(r/s);
    if(q<0) q=0;
    else if(q>255) q=255;           // uint8_t 범위 (0-255)로 클램핑 (overflow/underflow 방지)
    return (uint8_t)q;
}
```

*   **역할**: `float` 실수 값 `r`을 `uint8_t` 타입의 양자화된 정수 값으로 변환합니다.
*   **원리**: 실수 `r`을 스케일 `s`로 나눈 후 반올림하고 영점 `zp`를 더합니다.
*   **클램핑**: 계산된 `q` 값이 `uint8_t`의 유효 범위(0-255)를 벗어나지 않도록 강제로 제한(클램핑)합니다. 이는 양자화된 값의 오버플로우나 언더플로우를 방지하는 중요한 단계입니다.

## 2. 컨볼루션 및 풀링 커널: 신경망의 주요 연산 구현

### 2.1 `dwconv3x3_nhwc_u8` (Depthwise Convolution 3x3)

```c
void dwconv3x3_nhwc_u8(const tensor_u8_nhwc_t *in,const uint8_t *k3x3,
                       const int32_t *bias,float w_scale,int w_zp,
                       tensor_u8_nhwc_t *out,int apply_relu6) {
  int H=in->H,W=in->W,C=in->C;

  for(int y=0;y<H;++y)
  for(int x=0;x<W;++x)
  for(int c=0;c<C;++c) {
    float acc = (bias ? (float)bias[c]*in->scale*w_scale : 0.0f);

    for(int ky=-1;ky<=1;++ky) {
      int iy=y+ky; 
      if(iy<0||iy>=H) continue;
      
      for(int kx=-1;kx<=1;++kx) {
        int ix=x+kx; 
        if(ix<0||ix>=W) continue;

        uint8_t q_in=in->data[(iy*W+ix)*C+c];
        uint8_t q_k=k3x3[c*9+(ky+1)*3+(kx+1)];
        acc+=deq(q_in,in->scale,in->zp)*deq(q_k,w_scale,w_zp);
      }
    }
    
    uint8_t q=req(acc,out->scale,out->zp);
    
    if(apply_relu6){
      int q6=out->zp+(int)lrintf(6.0f/out->scale);
      if(q>q6) q=(q6>255?255:(q6<0?0:(uint8_t)q6));
      if(q<out->zp) q=(uint8_t)out->zp;
    }
    out->data[(y*W+x)*C+c]=q;
  }
}
```

*   **역할**: MobileNet에서 Depthwise Convolution은 각 입력 채널에 대해 독립적인 3x3 필터(커널)를 적용합니다. 이를 통해 모델의 연산 비용을 크게 줄일 수 있습니다.
*   **NHWC 레이아웃**: 입력 `in->data`는 `(y * W + x) * C + c`와 같은 공식으로 접근됩니다. 이는 배치, 높이, 너비, 채널 순서로 데이터가 저장되어 있음을 의미합니다.
*   **패딩 처리**: 커널 오프셋 `ky`, `kx`가 -1부터 1까지 움직일 때 `if(iy<0||iy>=H)`와 `if(ix<0||ix>=W)`와 같은 조건으로 경계를 벗어나는 입력 픽셀은 연산에서 제외됩니다. 이는 'Valid' 패딩과 유사하게 동작하여, 출력 텐서의 크기가 입력 텐서와 동일한 'Same' 패딩 효과를 간접적으로 만듭니다. (0으로 패딩하는 일반적인 'Same' 패딩과는 다소 차이가 있습니다.)
*   **양자화 연산**: 입력 픽셀 값(`q_in`)과 커널 가중치(`q_k`) 모두 `deq` 함수로 실수화된 후 곱해지고 누적됩니다. 최종 결과는 `req` 함수로 다시 양자화됩니다.
*   **ReLU6 활성화**: 선택적으로 `max(0, min(6, x))` 형태의 ReLU6 활성화 함수가 적용됩니다.

### 2.2 `pwconv1x1_nhwc_u8` (Pointwise Convolution 1x1)

```c
void pwconv1x1_nhwc_u8(const tensor_u8_nhwc_t *in,const uint8_t *k1x1,const int32_t *bias,
                       float w_scale,int w_zp,tensor_u8_nhwc_t *out){
  int H=in->H,W=in->W,Cin=in->C,Cout=out->C;
  for(int y=0;y<H;++y)for(int x=0;x<W;++x){
    const uint8_t* vin=&in->data[(y*W+x)*Cin];
    for(int co=0;co<Cout;++co){
      float acc=(bias? (float)bias[co]*in->scale*w_scale:0.0f);
      const uint8_t* wrow=&k1x1[co*Cin];
      for(int ci=0;ci<Cin;++ci){
        acc+=deq(vin[ci],in->scale,in->zp)*deq(wrow[ci],w_scale,w_zp);
      }
      out->data[(y*W+x)*Cout+co]=req(acc,out->scale,out->zp);
    }
  }
}
```

*   **역할**: Depthwise Convolution 이후에 따라오는 Pointwise Convolution은 1x1 커널을 사용하여 입력 채널들을 선형적으로 결합하여 새로운 출력 채널들을 생성합니다. 이를 통해 모델의 채널 수를 효율적으로 변경합니다.
*   **동작**: 공간적 위치(`y`, `x`)는 그대로 유지하면서, 각 출력 채널(`co`)에 대해 모든 입력 채널(`ci`)의 가중합(내적)을 계산합니다.

### 2.3 `avgpool_global_nhwc_u8` (Global Average Pooling)

```c
void avgpool_global_nhwc_u8(const tensor_u8_nhwc_t *in,tensor_u8_nhwc_t *out){
  int H=in->H,W=in->W,C=in->C;
  for(int c=0;c<C;++c){
    float sum=0.0f; 
    for(int y=0;y<H;++y)for(int x=0;x<W;++x)
      sum+=deq(in->data[(y*W+x)*C+c],in->scale,in->zp);
    float mean=sum/(float)(H*W);
    out->data[c]=req(mean,out->scale,out->zp);
  }
}
```

*   **역할**: Global Average Pooling은 각 채널의 모든 공간(높이 x 너비) 차원에 걸쳐 평균값을 계산합니다. 이를 통해 각 채널당 하나의 값만을 남기며, 특징 맵의 크기를 줄이고 모델의 파라미터 수를 줄이는 데 사용됩니다.
*   **출력 형태**: 입력 텐서의 `H`와 `W` 차원이 1로 줄어들어 `1x1xC` 형태의 출력 텐서가 됩니다.

### 2.4 `softmax_u8` (Softmax 활성화 함수)

```c
void softmax_u8(const tensor_u8_nhwc_t *in,tensor_u8_nhwc_t *out){
  int C=in->C; float tmp[2048]; if(C>2048) return; float maxv=-1e30f;
  
  for(int c=0;c<C;++c){ 
    float r=deq(in->data[c],in->scale,in->zp);
    tmp[c]=r;                               
    if(r>maxv)maxv=r;                       
  }
  
  float sum=0.0f; 
  for(int c=0;c<C;++c){ 
    tmp[c]=expf(tmp[c]-maxv);               
    sum+=tmp[c];                            
  }
  
  for(int c=0;c<C;++c){ 
    float prob=tmp[c]/sum;                  
    out->data[c]=req(prob,out->scale,out->zp); 
  }
}
```

*   **역할**: 신경망의 마지막 레이어에서 주로 사용되어, 모델의 출력을 확률 분포로 변환합니다. 모든 출력값의 합은 1이 됩니다.
*   **수치 안정성**: `exp(x - max)` 형태로 계산하는 것은 `exp()` 함수의 입력값이 너무 커서 오버플로우가 발생하거나 너무 작아서 언더플로우가 발생하는 것을 방지하기 위한 표준적인 Softmax 구현 기법입니다.

## 3. 코드 품질 및 최적화 고려사항

*   **NHWC 데이터 레이아웃**: 모든 커널 함수가 NHWC(Batch, Height, Width, Channel) 순서로 데이터에 접근합니다. 이는 임베디드 시스템에서 특정 하드웨어 가속기(예: Xilinx DPU)와 호환성이 좋거나, 특정 상황에서 캐시 효율에 유리할 수 있습니다.
*   **`static inline` 사용**: `deq`와 `req` 함수에 `static inline` 키워드를 사용하여 함수 호출 오버헤드를 최소화합니다. 이는 임베디드 환경에서 성능에 중요한 요소입니다.
*   **`lrintf` 함수**: 부동 소수점을 가장 가까운 정수로 반올림하는 `lrintf` 함수를 사용하여 양자화 시 정확도를 높였습니다.
*   **명시적인 양자화 파라미터 전달**: `scale`과 `zp` 값이 각 함수 호출 시 명시적으로 전달되어, 유연성과 재사용성을 높였습니다.

### 3.1 개선 제안

1.  **Depthwise Conv의 경계 처리 최적화**:
    *   현재 `dwconv3x3_nhwc_u8`의 `ky`, `kx` 루프 내에서 경계 체크 (`if(iy<0||iy>=H) continue;`)가 매번 발생합니다. 이는 반복 횟수가 많아질수록 성능 저하를 일으킬 수 있습니다.
    *   **개선 방안**: 이미지의 가장자리 픽셀과 중앙 픽셀을 구분하여 별도의 루프나 함수로 처리하거나, 입력 텐서 자체에 제로 패딩을 미리 추가하여 경계 체크를 없앨 수 있습니다.

2.  **HLS (고수준 합성) 최적화 고려**:
    *   이 코드는 FPGA 기반 하드웨어 가속기(HLS IP) 개발의 참조 구현으로 사용될 수 있습니다. HLS 도구는 C/C++ 코드를 분석하여 하드웨어 회로를 생성합니다.
    *   **개선 방안**:
        *   **루프 파이프라이닝(Loop Pipelining)**: `for` 루프에 HLS 지시자(pragma)를 적용하여 루프의 반복들을 병렬로 처리하도록 유도할 수 있습니다.
        *   **배열 분할(Array Partitioning)**: `in->data`, `out->data`, `k3x3`과 같은 배열을 HLS 지시자로 분할하여 메모리 접근 병렬성을 높일 수 있습니다.

이 `ref_kernels.c` 파일은 ZCU104 보드의 Cortex-A53에서 양자화된 MobileNet 모델을 효율적으로 실행하기 위한 기본적인 연산들을 견고하게 구현하고 있습니다. 특히 양자화 처리, NHWC 데이터 레이아웃, 그리고 임베디드 환경에 최적화된 C 언어 사용이 주요 특징입니다.

---

# `dinh/zcu104-a53-mobilenetv1-baseline/vitis_src/mobilenet_bm.cpp` 코드 리뷰 보고서

이 보고서는 `zcu104-a53-mobilenetv1-baseline` 프로젝트의 베어메탈(bare-metal) 데모 애플리케이션 `mobilenet_bm.cpp` 파일에 대한 상세한 코드 리뷰 결과를 담고 있습니다. 이 파일은 ZCU104 보드의 Cortex-A53에서 MobileNetV1 추론을 실행하기 위한 전체 흐름을 담당하며, SD 카드에서 이미지 및 가중치를 로드하고, 전처리, 신경망 연산(Depthwise Conv, Pointwise Conv, Global Average Pooling, Softmax), 그리고 최종적으로 Top-5 추론 결과를 출력하는 과정을 포함합니다.

### 1. 개요 및 의존성 관리

#### 1.1 포함된 헤더

```cpp
#include "xil_printf.h" // Xilinx Bare-metal printf
#include "xil_cache.h"  // Xilinx Cache operations
#include "ff.h"            // FatFs (xilffs) for SD card
#include "xtime_l.h"       // Xilinx Time library
#include <cstdint>         // C++ standard integer types
#include <vector>          // std::vector
#include <string>          // std::string
#include <algorithm>       // std::min, std::sort
#include <cmath>           // std::lrintf (for lrintf in C++)
#include "ref_kernels.h"   // Custom kernel definitions (dwconv, pwconv, softmax etc.)
```

*   **Xilinx BSP 관련 헤더**: `xil_printf.h`, `xil_cache.h`, `ff.h`, `xtime_l.h`는 Zynq UltraScale+ MPSoC의 베어메탈 환경에서 하드웨어 기능을 제어하고 파일 시스템(FatFs) 및 시간 측정 기능을 사용하기 위해 필수적입니다.
*   **C++ 표준 라이브러리**: `cstdint`, `vector`, `string`, `algorithm`, `cmath`는 데이터 구조 관리, 문자열 처리, 정렬 및 수학적 연산을 위해 사용됩니다. 특히 `vector`는 동적 크기의 버퍼 관리에 유용하게 사용됩니다.
*   **`ref_kernels.h`**: 이전 `ref_kernels.c`에서 정의된 양자화된 컨볼루션, 풀링, 소프트맥스 등의 핵심 신경망 연산 함수들의 선언을 포함합니다. 이는 `mobilenet_bm.cpp`가 이러한 저수준 커널들을 호출하여 추론 파이프라인을 구축함을 나타냅니다.

#### 1.2 네임스페이스

```cpp
using namespace std;
```

*   `std` 네임스페이스를 전역으로 사용하여 `std::vector`, `std::string` 등의 접두사를 생략합니다. 베어메탈 애플리케이션에서는 일반적으로 문제가 되지 않으나, 대규모 프로젝트에서는 네임스페이스 충돌을 피하기 위해 특정 항목만 `using`하거나 `std::` 접두사를 사용하는 것이 권장됩니다.

### 2. 유틸리티 및 헬퍼 함수

#### 2.1 시간 측정 헬퍼 함수

```cpp
static inline void print_ms(XTime t0, XTime t1);
static inline u32 ms_from_counts(XTime t0, XTime t1);
static inline u32 us_from_counts(XTime t0, XTime t1);
static inline void print_ms_us(XTime t0, XTime t1);
```

*   **역할**: `XTime_GetTime()`으로 측정된 CPU 사이클(`XTime`) 간의 차이를 밀리초(ms) 또는 마이크로초(us)로 변환하고 `xil_printf`로 출력하는 함수들입니다.
*   **특징**: 부동 소수점 연산을 최대한 배제하고 정수 연산을 통해 시간을 계산합니다. 이는 베어메탈 환경에서 부동 소수점 유닛(FPU) 사용을 최소화하여 성능을 높이거나 컴파일러 옵션에 따라 바이너리 크기를 줄이는 데 도움이 될 수 있습니다.
*   **`static inline`**: 함수 호출 오버헤드를 줄여 시간 측정 자체의 영향을 최소화합니다.

#### 2.2 SD 카드 헬퍼 함수

```cpp
static FATFS fatfs;
static FIL   file;

static bool sd_mount();
static bool sd_read_all(const char* path, vector<uint8_t>& out);
```

*   **`FATFS`, `FIL`**: FatFs 파일 시스템 라이브러리의 전역 변수로, SD 카드 마운트 정보와 파일 핸들을 저장합니다.
*   **`sd_mount()`**: SD 카드를 "0:/" 경로로 마운트합니다. 실패 시 `xil_printf`로 오류 코드를 출력합니다.
*   **`sd_read_all()`**: 지정된 경로의 파일을 열어 모든 내용을 `std::vector<uint8_t>`에 로드합니다. 파일 크기를 미리 확인하고 벡터의 크기를 조정한 후 데이터를 읽어 효율성을 높였습니다.

#### 2.3 최소 BMP 로더 (`load_bmp_24_stream`)

```cpp
#pragma pack(push,1)
struct BMPFileHeader{ /* ... */ };
struct BMPInfoHeader{ /* ... */ };
#pragma pack(pop)

static bool load_bmp_24_stream(const char* path, int& W, int& H, vector<uint8_t>& rgb);
```

*   **역할**: SD 카드에서 24비트 비압축 BMP 파일을 읽어 RGB 픽셀 데이터로 변환합니다.
*   **구조체**: `BMPFileHeader`와 `BMPInfoHeader` 구조체는 BMP 파일 헤더를 파싱하는 데 사용됩니다. `pragma pack(push,1)`은 구조체 멤버들이 1바이트 경계로 정렬되도록 하여 파일 형식과의 일치성을 보장합니다.
*   **유효성 검사**: `bfType`, `biSize`, `biPlanes`, `biBitCount`, `biCompression` 필드를 검사하여 지원되지 않는 BMP 형식을 걸러냅니다.
*   **행 스캔 및 BGR->RGB 변환**: BMP 데이터는 보통 하단부터 위로(bottom-up) 저장되고 BGR 순서이므로, 이를 상단부터 아래로(top-down) 그리고 RGB 순서로 변환하는 로직이 포함되어 있습니다. `row_stride` 계산은 BMP 파일의 행 패딩을 올바르게 처리합니다.
*   **스트림 방식**: 파일을 스트림 방식으로 읽어 메모리 사용량을 최적화했습니다 (전체 BMP를 한 번에 메모리에 로드하지 않음).

#### 2.4 라벨 로더 (`load_labels`)

```cpp
static vector<string> load_labels(const char* path);
```

*   **역할**: SD 카드에서 텍스트 파일(`labels.txt`)을 읽어 각 라인을 `std::string`으로 파싱하여 `std::vector<string>`으로 반환합니다.
*   **줄바꿈 처리**: `
`과 `
` (CRLF) 줄바꿈을 모두 처리하여 윈도우/리눅스 파일 시스템 호환성을 확보합니다.

#### 2.5 Top-5 출력 (`print_top5`)

```cpp
static void print_top5(const std::vector<float>& probs, const std::vector<std::string>& labels);
```

*   **역할**: 신경망의 최종 출력 확률(`probs`)과 라벨(`labels`)을 기반으로 상위 5개의 예측 클래스를 정렬하여 출력합니다.
*   **정렬**: `std::sort`와 람다 함수를 사용하여 확률을 기준으로 내림차순 정렬합니다.
*   **출력 형식**: `xil_printf`를 사용하여 순위, 라벨 이름, 255 스케일 점수, 백분율을 출력합니다. `lrintf`를 사용하여 float 값을 정수로 변환합니다.

### 3. 메인 추론 흐름 (`main` 함수)

`main` 함수는 MobileNetV1 추론 파이프라인의 전체적인 오케스트레이션을 담당합니다.

```cpp
int main(){
    xil_printf("mobilenet_bm: bare-metal demo\r\n");
    if(!sd_mount()) return -1;

    // 1. 이미지 로드 및 전처리
    // ...
    if(!load_bmp_24_stream("0:/assets/samples/digit_1.bmp", W, H, rgb)){ /* ... */ }
    if(W!=32 || H!=32){ /* ... */ }

    // 2. 양자화 파라미터 정의
    const float in_scale = 0.02f;  const int in_zp = 128;
    const float w_scale  = 0.0019579321f;  const int w_zp = 128;
    const float out_scale = 0.02f; const int out_zp = 128;
    const float sm_scale = 1.0f/255.0f; const int sm_zp = 0;

    // 3. NHWC 버퍼 및 텐서 구조체 초기화
    vector<uint8_t> in_u8(W*H*Cin), dw_out(W*H*Cin), pw_out(W*H*Cout), sm_u8(Cout);
    // ...
    tensor_u8_nhwc_t tin={H,W,Cin,in_u8.data(), in_scale, in_zp};
    // ...

    // 4. 가중치 로드
    vector<uint8_t> dw_w, pw_w;
    if(!sd_read_all("0:/assets/dw3x3_c3.bin", dw_w)) return -1;
    if(!sd_read_all("0:/assets/pw1x1_c10x3.bin", pw_w)) return -1;

    // 5. 라벨 로드
    vector<string> labels = load_labels("0:/assets/labels.txt");

    // 6. 신경망 연산 실행 및 시간 측정
    // Depthwise Convolution
    XTime_GetTime(&t0);
    dwconv3x3_nhwc_u8(&tin, dw_w.data(), NULL, w_scale, w_zp, &tdw, 1);
    XTime_GetTime(&t1);
    xil_printf("DWConv: "); print_ms_us(t0, t1);

    // Pointwise Convolution
    XTime_GetTime(&t0);
    pwconv1x1_nhwc_u8(&tdw, pw_w.data(), NULL, w_scale, w_zp, &tpw);
    XTime_GetTime(&t1);
    xil_printf("PWConv: "); print_ms_us(t0, t1);

    // Global Average Pooling (수동 구현)
    XTime_GetTime(&t0);
    // ... manual avgpool implementation ...
    XTime_GetTime(&t1);
    xil_printf("AvgPool: "); print_ms_us(t0, t1);

    // Softmax
    XTime_GetTime(&t0);
    softmax_u8(&tsm_in, &tsm_out);
    XTime_GetTime(&t1);
    xil_printf("Softmax: "); print_ms_us(t0, t1);

    // 7. 결과 후처리 및 출력
    vector<float> probs(Cout);
    for(int c=0;c<Cout;++c) probs[c] = sm_scale * ((int)sm_u8[c] - sm_zp);
    print_top5(probs, labels);

    // 8. 메모리 사용량 및 종료
    xil_printf("RAM bytes: in=%lu dw_out=%lu pw_out=%lu sm=%lu\r\n", /* ... */);
    xil_printf("Done.\r\n");
    while(1){} // 무한 루프
    return 0;
}
```

*   **초기화**: SD 카드 마운트, BMP 이미지 로드 및 32x32 크기 검증, 이미지 데이터를 NHWC `uint8_t` 형식으로 전처리합니다.
*   **양자화 파라미터**: `in_scale`, `in_zp` 등 MobileNetV1 모델에 맞는 양자화 스케일과 영점 값들이 명시적으로 정의됩니다. 이는 양자화된 신경망의 정확한 연산을 위해 매우 중요합니다.
*   **텐서 버퍼 및 가중치**: 각 레이어의 입출력 버퍼(`std::vector<uint8_t>`)와 가중치(`dw_w`, `pw_w`)가 할당되고 SD 카드에서 로드됩니다. `tensor_u8_nhwc_t` 구조체를 사용하여 이러한 버퍼들에 대한 메타데이터(높이, 너비, 채널, 스케일, 영점)를 관리합니다.
*   **추론 단계**:
    *   **Depthwise Convolution**: `dwconv3x3_nhwc_u8` 함수를 호출하여 Depthwise Convolution을 수행합니다. `apply_relu6` 플래그가 1로 설정되어 ReLU6 활성화가 적용됩니다.
    *   **Pointwise Convolution**: `pwconv1x1_nhwc_u8` 함수를 호출하여 Pointwise Convolution을 수행합니다.
    *   **Global Average Pooling**: `ref_kernels.c`에 정의된 `avgpool_global_nhwc_u8` 함수 대신 `main` 함수 내에서 수동으로 구현되었습니다. 이는 아마도 HLS 가속기 통합을 위해 이 부분을 참조 구현으로 남겨두거나, 특정 요구사항에 의해 직접 구현한 것으로 보입니다. *개선 제안에서 이 부분을 다룰 것입니다.*
    *   **Softmax**: `softmax_u8` 함수를 호출하여 최종 확률 분포를 계산합니다.
*   **성능 측정**: 각 주요 신경망 연산 전후에 `XTime_GetTime()`을 호출하고 `print_ms_us` 함수를 사용하여 각 단계의 실행 시간을 측정하여 프로파일링 정보를 제공합니다.
*   **결과 출력**: Softmax 결과(`sm_u8`)를 `float` 확률(`probs`)로 변환한 후 `print_top5` 함수를 호출하여 최종 추론 결과를 사용자에게 보여줍니다.
*   **메모리 사용량**: 각 버퍼의 크기를 출력하여 대략적인 RAM 사용량을 파악할 수 있도록 합니다.

### 4. 코드 품질 및 최적화 고려사항

*   **베어메탈 환경 최적화**: `xil_printf`, `xil_cache`, `xtime_l` 등 Xilinx BSP 기능을 적극적으로 활용하여 베어메탈 환경에 최적화된 코드를 작성했습니다. 시간 측정 함수에서 부동 소수점을 배제한 점도 좋은 예입니다.
*   **모듈화**: SD 카드, BMP 로딩, 라벨 로딩, Top-5 출력 등 각각의 기능을 별도의 `static` 헬퍼 함수로 모듈화하여 코드의 가독성과 유지보수성을 높였습니다.
*   **정확한 데이터 형식**: `uint8_t` 및 `float` 간의 양자화/역양자화 변환을 `ref_kernels.h` 및 `ref_kernels.c`의 헬퍼 함수들을 통해 일관성 있게 처리합니다.
*   **오류 처리**: 파일 로딩 실패, BMP 형식 불일치, 가중치 크기 불일치 등 여러 잠재적 오류 상황에 대해 `xil_printf`로 메시지를 출력하고 프로그램을 종료하여 안정성을 확보합니다.
*   **NHWC 데이터 레이아웃**: `ref_kernels.c`와 일관되게 NHWC 데이터 레이아웃을 사용하여 MobileNet 모델의 데이터 처리에 적합합니다.

### 4.1 개선 제안

1.  **Global Average Pooling의 일관성**:
    *   `ref_kernels.c`에 `avgpool_global_nhwc_u8` 함수가 정의되어 있음에도 불구하고, `main` 함수 내에서 Global Average Pooling 로직이 수동으로 구현되었습니다 (225-233줄).
    *   **개선 방안**: 이미 정의된 `avgpool_global_nhwc_u8` 함수를 호출하여 코드의 일관성과 재사용성을 높이는 것이 좋습니다. 이는 코드 중복을 줄이고, 나중에 `avgpool_global_nhwc_u8` 함수가 최적화되거나 변경될 경우 `main` 함수 코드를 수정할 필요가 없도록 합니다. (코드 203-205줄에 주석 처리된 참조 코드가 있으나 사용되지 않고 있음)

2.  **캐시 관리**:
    *   코드에서 `xil_cache.h` 헤더를 포함하고 있지만, 데이터 버퍼(`in_u8`, `dw_out`, `pw_out`, `sm_u8`)에 대한 명시적인 캐시 무효화/플러시 작업이 보이지 않습니다. 특히 DMA 또는 하드웨어 가속기(예: DPU)가 메모리에 접근하는 경우, A53 캐시와 일관성을 유지하기 위해 적절한 캐시 관리가 필요합니다.
    *   **개선 방안**: 각 신경망 연산 전후에 입력 버퍼에 대한 `Xil_DCacheInvalidateRange` 또는 출력 버퍼에 대한 `Xil_DCacheFlushRange`와 같은 함수를 호출하여 데이터 일관성을 보장해야 합니다.

3.  **오류 메시지 상세화**:
    *   현재 오류 메시지들은 간단한 편입니다. 예를 들어, `BMP load failed`나 `read FH failed`는 어떤 유형의 실패인지 명확히 알려주지 않습니다.
    *   **개선 방안**: `errno`와 같은 시스템 오류 코드를 함께 출력하거나, 어떤 단계에서 어떤 문제(예: "파일 읽기 중 실패", "잘못된 BMP 헤더")가 발생했는지 더 구체적인 정보를 제공하여 디버깅을 용이하게 할 수 있습니다.

4.  **하드웨어 가속기(HLS/DPU) 통합 준비**:
    *   이 코드는 순수 소프트웨어 참조 구현으로 보입니다. ZCU104 플랫폼의 이점을 최대한 활용하려면 Xilinx DPU와 같은 하드웨어 가속기를 통합해야 합니다.
    *   **개선 방안**:
        *   DPU 드라이버 및 API 호출을 통합하여 `dwconv3x3_nhwc_u8` 및 `pwconv1x1_nhwc_u8`와 같은 연산들을 DPU로 오프로드합니다.
        *   DPU에서 지원하는 데이터 형식 및 메모리 관리 요구사항(예: CMA 버퍼 할당)에 맞춰 데이터 버퍼를 조정합니다.
        *   소프트웨어 구현과 하드웨어 가속 구현을 쉽게 전환할 수 있도록 추상화 계층을 추가하는 것을 고려합니다.

### 5. 결론

`mobilenet_bm.cpp` 파일은 ZCU104 베어메탈 환경에서 MobileNetV1 추론을 위한 견고하고 잘 구조화된 데모 애플리케이션입니다. SD 카드에서 자원을 로드하고, 이미지 전처리를 수행하며, `ref_kernels.c`에서 정의된 핵심 커널 함수들을 활용하여 신경망 추론 파이프라인을 구축합니다. 또한 각 단계의 성능을 측정하여 프로파일링 정보를 제공합니다. 위에 제시된 개선 제안들을 적용하면 코드의 견고성, 유지보수성, 그리고 ZCU104 플랫폼에서의 성능을 더욱 향상시킬 수 있을 것입니다. 특히 하드웨어 가속기 통합은 이 데모의 핵심적인 다음 단계가 될 것입니다.
