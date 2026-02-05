# A53 베어메탈 양자화 숫자 인식기 (A53 Bare‑Metal Quantized Digit Recognizer)

SD 카드에서 **32×32 RGB BMP**를 로드하고, 양자화된 **MobileNet 스타일** 경로(Depthwise 3×3 + ReLU6 → Pointwise 1x1 → Global Average Pool → Softmax)를 실행하여 **0–9** 숫자에 대한 상위 K개 예측 결과를 출력하는 최소한의 베어메탈 Cortex‑A53 데모입니다.


## 프로젝트 계보 (Project Lineage)

이 프로젝트는 다음 리포지토리의 SD 카드 이미지 I/O 및 베어메탈 스캐폴딩을 기반으로 확장되었습니다:
- **zcu104-baremetal-imgio** — https://github.com/vandinhtranengg/zcu104-baremetal-imgio

이전 리포지토리에서 제공하는 SD 파일 처리 및 BMP 로드 패턴을 재사용하며, 양자화된 MobileNet 스타일 파이프라인(DW3×3 → ReLU6 → PW1×1 → GAP → Softmax)과 숫자 분류(0–9) 기능을 추가하여 확장했습니다.


## 주요 기능 (Features)
- 참조용 NHWC uint8 커널: `dwconv3x3_nhwc_u8`, `pwconv1x1_nhwc_u8`, `avgpool_global_nhwc_u8`, `softmax_u8`
- 단일 가중치 스케일 `w_scale` 및 `w_zp=128` 사용
- SD 자산: 가중치(`dw3x3_c3.bin`, `pw1x1_c10x3.bin`), `labels.txt`, BMP 샘플(`digit_0.bmp`..`digit_9.bmp`)


## 디렉토리 구조 (Directory)
- `vitis_src/` — 베어메탈 소스 코드 (`mobilenet_bm.cpp`, `ref_kernels.*`)
- `tools/` — 훈련 및 내보내기 스크립트 (합성 숫자 데이터 사용)
- `assets/` — 양자화된 가중치, 라벨 및 샘플 이미지
- `docs/` — 심층 분석 및 퀵스타트 문서


## 빌드 (펌웨어)
> Xilinx/AMD Vitis IDE와 Standalone A53 BSP가 필요합니다.
> 새 애플리케이션 프로젝트를 생성하고 `vitis_src/` 소스를 추가한 뒤, FatFs (`xilffs`) 및 타이머 (`xtime_l`) 라이브러리를 링크하십시오.


## SD 카드 레이아웃 (SD Card Layout)
```c
0:/assets/dw3x3_c3.bin        (27 bytes)
0:/assets/pw1x1_c10x3.bin     (30 bytes)
0:/assets/labels.txt          (10 lines: 0..9)
0:/assets/samples             (sample image folder)
```


## 사전 빌드된 하드웨어 플랫폼 (XSA)

이 리포지토리는 사전 빌드된 하드웨어 플랫폼을 포함하고 있습니다:
- **`standalone_zynq_core.xsa`** — Vivado에서 내보낸 하드웨어(Zynq UltraScale+ MPSoC) 파일로, Vitis에서 Standalone A53 도메인을 생성하는 데 적합합니다.

이 XSA 파일을 사용하여 플랫폼과 도메인(A53 Standalone)을 생성한 뒤, Vivado를 열지 않고도 펌웨어 소스를 가져와 베어메탈 앱을 빌드할 수 있습니다.


## 시작하기 (Getting Started)

1. Vitis/SDK에서 베어메탈 앱을 빌드합니다 (A53 Standalone).
2. SD 카드의 `0:/assets/` 경로에 자산 파일들을 복사합니다:
   - `dw3x3_c3.bin` (27 B), `pw1x1_c10x3.bin` (30 B), `labels.txt` (10 lines).
   - BMP 테스트 이미지: `digit_0.bmp`..`digit_9.bmp`.
3. 보드에서 실행합니다; UART를 통해 타이밍 정보와 Top-K 예측 결과가 출력됩니다.

문제 해결 (Troubleshooting):
- 타이밍이 `DWConv: ms`와 같이 숫자 없이 출력된다면, float 출력 대신 정수형 **ms/µs** 출력을 사용하도록 변경해 보세요.
- 크기 불일치 오류가 발생하면 파일 이름과 크기를 확인하세요:
  - DW = `Cin*3*3` = 27 바이트; PW = `Cout*Cin` = 30 바이트; labels = 10 줄.


---

## 🚀 다음 단계 (What to do next)

### 1) 실제 MobileNetV1 가중치로 교체 (양자화된 INT8)
### 2) MobileNetV1 구조와 더 비슷하게 블록(DW + PW) 추가
### 3) 다음 연산자를 위한 가속기 HLS IP 구축:
- DepthwiseConv3×3 연산자
```c
void dwconv3x3_nhwc_u8(const tensor_u8_nhwc_t *in,const uint8_t *k3x3,const int32_t *bias,
                       float w_scale,int w_zp,tensor_u8_nhwc_t *out,int apply_relu6){
  int H=in->H,W=in->W,C=in->C;
  for(int y=0;y<H;++y)for(int x=0;x<W;++x)for(int c=0;c<C;++c){
    float acc=(bias? (float)bias[c]*in->scale*w_scale : 0.0f);
    for(int ky=-1;ky<=1;++ky){int iy=y+ky; if(iy<0||iy>=H) continue;
      for(int kx=-1;kx<=1;++kx){int ix=x+kx; if(ix<0||ix>=W) continue;
        uint8_t q_in=in->data[(iy*W+ix)*C+c];
        uint8_t q_k=k3x3[c*9+(ky+1)*3+(kx+1)];
        acc+=deq(q_in,in->scale,in->zp)*deq(q_k,w_scale,w_zp);
      }}
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
- PWConv 1×1 연산자
```c
void pwconv1x1_nhwc_u8(const tensor_u8_nhwc_t *in,const uint8_t *k1x1,const int32_t *bias,
                       float w_scale,int w_zp,tensor_u8_nhwc_t *out){
  int H=in->H,W=in->W,Cin=in->C,Cout=out->C;
  for(int y=0;y<H;++y)for(int x=0;x<W;++x){
    const uint8_t* vin=&in->data[(y*W+x)*Cin];
    for(int co=0;co<Cout;++co){
      float acc=(bias? (float)bias[co]*in->scale*w_scale:0.0f);
      const uint8_t* wrow=&k1x1[co*Cin];
      for(int ci=0;ci<Cin;++ci){ acc+=deq(vin[ci],in->scale,in->zp)*deq(wrow[ci],w_scale,w_zp); }
      out->data[(y*W+x)*Cout+co]=req(acc,out->scale,out->zp);
    }
  }
}
```

### 시스템 아키텍처 제안 (System Architecture Suggestion)

두 개의 독립적인 HLS IP가 **AXI4-Stream** 데이터 인터페이스와 **AXI4-Lite** 제어 인터페이스를 사용:
```c
MM2S (AXI DMA)   →   DW3x3 IP   →   PW1x1 IP   →   S2MM (AXI DMA)
                   (AXI-Lite)    (AXIS-Lite)
```

- **MM2S — Memory-Mapped to Stream**
  - **방향:** DDR → AXI4-Stream
  - **목적:** 메모리에서 데이터를 읽어 스트리밍 인터페이스로 전송 (가속기로 입력 데이터 공급).
  - **데이터 경로:**
    `DDR (AXI4-MM) → AXI DMA (MM2S) → AXI4-Stream → Accelerator`

- **S2MM — Stream to Memory-Mapped**
  - **방향:** AXI4-Stream → DDR
  - **목적:** 스트리밍 데이터를 받아 메모리에 다시 기록 (가속기의 결과를 수집).
  - **데이터 경로:**
    `Accelerator → AXI4-Stream → AXI DMA (S2MM) → DDR (AXI4-MM)`

- **IP당 AXI4-Lite 레지스터**를 제어/상태 확인용으로 사용.

- **독립(Standalone) 모드:** 디버깅 시 각 IP를 MM2S/S2MM과 개별적으로 실행 가능  
  (예: `DW → DDR`, 그 다음 `DDR → PW`).

- **체인(Chained) 모드:** 안정화되면 `DW`의 `M_AXIS`를 `PW`의 `S_AXIS`에 직접 연결하여 중간 DDR 트래픽 제거.

---
### 알림: Depthwise (DW) 및 Pointwise (PW) 컴퓨팅 특성

- **DW (Depthwise Convolution)**
  - **바이트당 연산량이 낮음** → 컴퓨팅 바운드보다는 대역폭(Bandwidth) 바운드.
  - **채널 병렬성 극대화**, **라인 버퍼 사용**, 그리고 **DW→PW 퓨전**을 적용하여 불필요한 DRAM 왕복을 줄이는 것이 최선.

- **PW (1×1 Pointwise Convolution)**
  - **GEMM**처럼 동작하며 일반적으로 **실행 시간과 파라미터 크기 모두를 지배**함.
  - **공격적인 타일링(tiling)**과 **온칩 데이터 재사용**을 통해 최적화하며, 가능한 경우 가중치와 활성화 타일을 모두 **BRAM**에 유지.

---

## 구현을 위한 추천 HLS 스켈레톤 (Suggested HLS Skeleton)
- **PW (1×1)에 시스톨릭 어레이(systolic array) 배치** 및 타일링된 **GEMM**으로 처리: M = H·W, K = Cin, N = Cout.
- **DW (3×3)는 스트리밍 스텐실(streaming stencil)**로 처리하며 라인 버퍼와 채널 병렬성 활용.
- 모든 산술 연산은 **INT8 × INT8 → INT32 누적**, 그 후 **u8로 재양자화**.
- DW 출력을 PW 시스톨릭 어레이로 직접 스트리밍하는 **DW→PW 퓨전 탑(top)**을 제공하여 **DRAM 트래픽 방지**.

### 0) 공통 타입 & 양자화 헬퍼 — accel_common.hpp
```c

// 공통 타입 (모든 블록에서 사용)
using axis_cvec_t = Axis<DW_BITS>;   // DW_BITS = CHAN_ALIGN * 8
using q24_t      = ap_int<32>;       // Q24 재양자화 승수 (requant multiplier)

```

### 1) DW 3×3 스트리밍 스텐실 — dw3x3_stream.hpp
- **AXIS 입력 (u8 NHWC 벡터)** → **라인 버퍼** + 채널당 **3×3 윈도우** → **INT32 MAC** → **Q24 재양자화** → **ReLU6 클램프** → **AXIS 출력 (u8 NHWC 벡터)**.
- **채널 병렬성**: 매 사이클마다 P_C 채널을 병렬 처리.
- PW에 직접 공급하도록 설계됨 (DW→PW 퓨전).
```c
void dw3x3_stream(
  hls::stream<axis_cvec_t> &s_in,     // AXIS(u8, NHWC, 비트당 CHAN_ALIGN)
  hls::stream<axis_cvec_t> &s_out,    // AXIS(u8, 동일 폭)
  const ap_uint<8>* w_dw,             // m_axi: [C * 9], 레이아웃 c*9 + ky*3 + kx
  int H, int W, int C, int stride,    // stride = 1 (스켈레톤)
  int in_zp, int w_zp, int out_zp,    // 양자화 영점(zero-points)
  q24_t alpha_q,                      // Q24: (in_s * w_s / out_s)
  ap_uint<8> relu6_qmax               // out_zp + round(6/out_s)
);
//-------------의사코드(Pseudocode)---------------------
for y in [0..H-1]:
  for x in [0..W-1]:
    for cb in [0..(C/CHAN_ALIGN)-1]:      // 채널 블록 인덱스
      (u8[CHAN_ALIGN]) pix_pack = AXIS_read(s_in)          // 한 비트(beat)
      // 1) 라인 버퍼: 현재 팩을 행 링(row ring, y % 3)의 컬럼 x에 기록
      LB[(y % 3)][x][:] = pix_pack[:]

      // 2) 레인(lane)당 3×3 윈도우 (경계는 제로 패딩)
      for pc in 0..CHAN_ALIGN-1:
        WIN[pc] = gather_3x3_from_LB(LB, y, x, pc)

      // 3) 현재 채널 블록에 대한 DW 가중치 로드 (레인당 9 탭)
      WDWC[CHAN_ALIGN][3][3] = load_from(w_dw, c_base=cb*CHAN_ALIGN)

      // 4) 레인별 연산: INT8xINT8→INT32, Q24 재양자화, ReLU6 클램프
      for pc in 0..CHAN_ALIGN-1:
        acc = sum_{ky,kx}( (WIN[pc][ky][kx] - in_zp) * (WDWC[pc][ky][kx] - w_zp) )
        r   = (acc * alpha_q) >> 24
        q   = out_zp + r
        q   = clamp(q, lo=out_zp, hi=relu6_qmax)
        out_pack[pc] = saturate_u8(q)

      // 5) 한 AXIS 비트 팩 & 쓰기; TLAST는 마지막 (y,x,cb)에서만 어설션
      AXIS_write(s_out, out_pack, last = (y==H-1 && x==W-1 && cb==C/CHAN_ALIGN-1))
```

### 2) PW 1×1 시스톨릭 (GEMM) — pw1x1_systolic.hpp
- DW 출력 스트림(NHWC u8) 또는 외부 스트림을 소비하며 PW를 GEMM으로 처리:
- A (M×K) × B (K×N) → C (M×N), 여기서 M = H·W, K = Cin, N = Cout.
- 온칩 A_tile (Tk) 및 B_tile (Tk×Tn)을 사용한 타일링; PE 메시(mesh)가 활성화 벡터당 Tn 출력을 계산.
- 웨이트-스테이셔너리(weight-stationary) 방식 예시 (활성화를 스트리밍하는 동안 가중치를 로컬 타일에 유지).
```c
void pw1x1_systolic(
  hls::stream<axis_cvec_t> &s_in,     // AXIS(u8, NHWC)
  hls::stream<axis_cvec_t> &s_out,    // AXIS(u8, NHWC)
  const ap_uint<8>* w_pw,             // m_axi: [Cout * Cin], row-major co*Cin + ci
  int H, int W, int Cin, int Cout,
  int in_zp, int w_zp, int out_zp,    // 양자화 영점
  q24_t alpha_q                       // Q24: (in_s * w_s / out_s)
)
//-------------의사코드(Pseudocode)---------------------
M = H * W
for m in [0..M-1]:                         // 픽셀 인덱스
  // 1) 스트림에서 Cin 활성화를 A_buf로 읽음 (CHAN_ALIGN 크기 비트 단위)
  for k0 in [0..Cin-1 step CHAN_ALIGN]:
    beat = AXIS_read(s_in)
    A_buf[k0 : k0+CHAN_ALIGN] = unpack(beat)

  // 2) Tn 타일 단위로 Cout 출력 생성 (예: 8 또는 16)
  for n0 in [0..Cout-1 step Tn]:
    ACC[Tn] = {0}

    // 3) Tk 청크로 K-리덕션 (초기엔 Tk = CHAN_ALIGN 권장)
    for k0 in [0..Cin-1 step Tk]:
      A_tile[Tk]       = A_buf[k0 : k0+Tk]
      B_tile[Tn][Tk]   = load_from(w_pw, rows = n0..n0+Tn-1, cols = k0..k0+Tk-1)

      // 4) 웨이트-스테이셔너리 MAC
      for tn in 0..Tn-1:
        for tk in 0..Tk-1:
          ACC[tn] += (A_tile[tk] - in_zp) * (B_tile[tn][tk] - w_zp)

    // 5) 재양자화 & Tn 결과 방출 (한 AXIS 비트)
    out_pack[0..Tn-1] = quantize_u8(ACC, out_zp, alpha_q)  // (acc*alpha_q)>>24 + out_zp
    AXIS_write(s_out, out_pack, last = (n0+Tn>=Cout && m==M-1))
```

### 3) DW→PW 퓨전 탑 (스트리밍) — dw_pw_fused_top.cpp
- 데이터 흐름 파이프라인 구축: 중간 DRAM 없이 DW 스트림 → PW 시스톨릭.
- AXI‑Lite 인수를 통해 모든 양자화 파라미터를 일관되게 설정.
```c
void dw_pw_fused_top(
  hls::stream<axis_cvec_t> &s_in,
  hls::stream<axis_cvec_t> &s_out,
  const ap_uint<8>* w_dw,             // DW 가중치
  const ap_uint<8>* w_pw,             // PW 가중치
  int H, int W, int Cin, int Cout, int stride,
  // DW 양자화
  int dw_in_zp, int dw_w_zp, int dw_out_zp, q24_t dw_alpha_q, ap_uint<8> dw_relu6_qmax,
  // PW 양자화 (참고: pw_in_zp == dw_out_zp)
  int pw_in_zp, int pw_w_zp, int pw_out_zp, q24_t pw_alpha_q
);
//-------------의사코드(Pseudocode)---------------------
create FIFO s_dw2pw (depth ~ 32–128)

DATAFLOW {
  // 1단계: DW 3×3 (NHWC u8 벡터를 밖으로 스트리밍)
  dw3x3_stream(s_in, s_dw2pw, w_dw, H, W, Cin, stride,
               dw_in_zp, dw_w_zp, dw_out_zp, dw_alpha_q, dw_relu6_qmax)

  // 2단계: PW 1×1 GEMM (DW 스트림을 직접 소비)
  pw1x1_systolic(s_dw2pw, s_out, w_pw, H, W, Cin, Cout,
                 pw_in_zp /*= dw_out_zp*/, pw_w_zp, pw_out_zp, pw_alpha_q)
}
```
