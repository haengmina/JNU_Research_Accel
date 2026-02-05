# `dinh/zybo_memcopy_accel_interrupt/src/GPIO-demo/` 코드 리뷰 보고서

이 보고서는 `zybo_memcopy_accel_interrupt` 프로젝트의 GPIO 데모 소스 코드에 대한 상세한 코드 리뷰 결과를 담고 있습니다. 이 폴더에는 Zybo Z7 보드용 GPIO 제어 코드(`zybo_GPIO.c`)와 ZedBoard용 GPIO 제어 코드(`zedboard_GPIO.c`)가 포함되어 있습니다.

---

# `zybo_GPIO.c` 코드 리뷰 보고서

이 파일은 Zybo Z7-10 보드에서 버튼 입력을 읽어 LED를 제어하는 간단한 GPIO 데모 애플리케이션입니다.

## 1. 헤더 및 상수 정의

```c
#include "xparameters.h"
#include "xil_printf.h"
#include "xgpio.h"
#include "xil_types.h"

// Get device IDs from xparameters.h
#define BTN_ID XPAR_AXI_GPIO_BUTTONS_DEVICE_ID
#define LED_ID XPAR_AXI_GPIO_LED_DEVICE_ID
#define BTN_CHANNEL 1
#define LED_CHANNEL 1
#define BTN_MASK 0b1111
#define LED_MASK 0b1111
```

### 1.1 헤더 분석

| 헤더 | 용도 |
|------|------|
| `xparameters.h` | Vivado에서 생성된 하드웨어 파라미터 정의 |
| `xil_printf.h` | 경량 printf 구현 |
| `xgpio.h` | Xilinx AXI GPIO 드라이버 |
| `xil_types.h` | Xilinx 데이터 타입 정의 (u8, u16, u32 등) |

### 1.2 상수 분석

| 상수 | 값 | 설명 |
|------|-----|------|
| `BTN_ID` | XPAR_AXI_GPIO_BUTTONS_DEVICE_ID | 버튼 GPIO 장치 ID |
| `LED_ID` | XPAR_AXI_GPIO_LED_DEVICE_ID | LED GPIO 장치 ID |
| `BTN_CHANNEL` | 1 | 버튼용 GPIO 채널 |
| `LED_CHANNEL` | 1 | LED용 GPIO 채널 |
| `BTN_MASK` | 0b1111 (4비트) | 4개 버튼 마스크 |
| `LED_MASK` | 0b1111 (4비트) | 4개 LED 마스크 |

*   **Zybo Z7-10 특성**: 4개의 푸시 버튼(BTN0-BTN3)과 4개의 LED(LD0-LD3)를 가집니다.

## 2. 메인 함수

```c
int main() {
    XGpio_Config *cfg_ptr;
    XGpio led_device, btn_device;
    u32 data;

    xil_printf("Entered function main\r\n");

    // Initialize LED Device
    cfg_ptr = XGpio_LookupConfig(LED_ID);
    XGpio_CfgInitialize(&led_device, cfg_ptr, cfg_ptr->BaseAddress);

    // Initialize Button Device
    cfg_ptr = XGpio_LookupConfig(BTN_ID);
    XGpio_CfgInitialize(&btn_device, cfg_ptr, cfg_ptr->BaseAddress);

    // Set Button Tristate (input)
    XGpio_SetDataDirection(&btn_device, BTN_CHANNEL, BTN_MASK);

    // Set Led Tristate (output)
    XGpio_SetDataDirection(&led_device, LED_CHANNEL, 0);

    while (1) {
        data = XGpio_DiscreteRead(&btn_device, BTN_CHANNEL);
        data &= BTN_MASK;
        if (data != 0) {
            data = LED_MASK;  // 버튼 눌리면 모든 LED 켜기
        } else {
            data = 0;          // 버튼 안 눌리면 모든 LED 끄기
        }
        XGpio_DiscreteWrite(&led_device, LED_CHANNEL, data);
    }
}
```

### 2.1 초기화 흐름

```
1. XGpio_LookupConfig() → 장치 설정 조회
2. XGpio_CfgInitialize() → 드라이버 인스턴스 초기화
3. XGpio_SetDataDirection() → 입/출력 방향 설정
```

### 2.2 데이터 방향 설정

```c
// 버튼: 입력 (마스크 비트 = 1)
XGpio_SetDataDirection(&btn_device, BTN_CHANNEL, BTN_MASK);

// LED: 출력 (마스크 비트 = 0)
XGpio_SetDataDirection(&led_device, LED_CHANNEL, 0);
```

*   **Tristate 레지스터**: 1 = 입력(high-impedance), 0 = 출력
*   **BTN_MASK = 0b1111**: 4개 핀 모두 입력
*   **LED = 0**: 4개 핀 모두 출력

### 2.3 메인 루프 로직

```
버튼 읽기 → 마스킹 → 조건 판단 → LED 쓰기
```

| 버튼 상태 | 동작 |
|-----------|------|
| 아무 버튼도 안 눌림 (data = 0) | 모든 LED 끔 |
| 하나 이상 버튼 눌림 (data ≠ 0) | 모든 LED 켬 |

*   **단순화된 로직**: 어떤 버튼이든 눌리면 모든 LED가 켜지는 이진 동작입니다.

---

# `zedboard_GPIO.c` 코드 리뷰 보고서

이 파일은 ZedBoard에서 딥 스위치 입력을 읽어 LED에 미러링하는 GPIO 데모 애플리케이션입니다.

## 1. 헤더 및 상수 정의

```c
#include "xparameters.h"
#include "xil_printf.h"
#include "xgpio.h"
#include "xil_types.h"

#define LED_DEV_ID      XPAR_AXI_GPIO_LED_DEVICE_ID
#define SW_DEV_ID       XPAR_AXI_GPIO_SW_DEVICE_ID
#define LED_CHANNEL     1
#define SW_CHANNEL      1
#define LED_MASK        0xFF
#define SW_MASK         0xFF
```

### 1.1 상수 분석

| 상수 | 값 | 설명 |
|------|-----|------|
| `LED_DEV_ID` | XPAR_AXI_GPIO_LED_DEVICE_ID | LED GPIO 장치 ID |
| `SW_DEV_ID` | XPAR_AXI_GPIO_SW_DEVICE_ID | 스위치 GPIO 장치 ID |
| `LED_CHANNEL` | 1 | LED용 GPIO 채널 |
| `SW_CHANNEL` | 1 | 스위치용 GPIO 채널 |
| `LED_MASK` | 0xFF (8비트) | 8개 LED 마스크 |
| `SW_MASK` | 0xFF (8비트) | 8개 스위치 마스크 |

*   **ZedBoard 특성**: 8개의 딥 스위치(SW0-SW7)와 8개의 LED(LD0-LD7)를 가집니다.

## 2. 메인 함수

```c
int main() {
    XGpio_Config *cfg_ptr;
    XGpio led_gpio, sw_gpio;
    u32 sw_val;

    xil_printf("ZedBoard: Switch LED mirror (separate AXI GPIOs)\r\n");

    // Initialize LED GPIO
    cfg_ptr = XGpio_LookupConfig(LED_DEV_ID);
    if (cfg_ptr == NULL) {
        xil_printf("ERROR: LED XGpio_LookupConfig failed\r\n");
        return XST_FAILURE;
    }
    if (XGpio_CfgInitialize(&led_gpio, cfg_ptr, cfg_ptr->BaseAddress) != XST_SUCCESS) {
        xil_printf("ERROR: LED XGpio_CfgInitialize failed\r\n");
        return XST_FAILURE;
    }

    // Initialize Switch GPIO
    cfg_ptr = XGpio_LookupConfig(SW_DEV_ID);
    if (cfg_ptr == NULL) {
        xil_printf("ERROR: SW XGpio_LookupConfig failed\r\n");
        return XST_FAILURE;
    }
    if (XGpio_CfgInitialize(&sw_gpio, cfg_ptr, cfg_ptr->BaseAddress) != XST_SUCCESS) {
        xil_printf("ERROR: SW XGpio_CfgInitialize failed\r\n");
        return XST_FAILURE;
    }

    // Set directions
    XGpio_SetDataDirection(&sw_gpio,  SW_CHANNEL, SW_MASK); // input
    XGpio_SetDataDirection(&led_gpio, LED_CHANNEL, 0x00);   // output

    while (1) {
        sw_val = XGpio_DiscreteRead(&sw_gpio, SW_CHANNEL) & SW_MASK;
        XGpio_DiscreteWrite(&led_gpio, LED_CHANNEL, sw_val & LED_MASK);
    }
}
```

### 2.1 Zybo 코드와의 차이점

| 항목 | Zybo | ZedBoard |
|------|------|----------|
| 입력 장치 | 버튼 (4개) | 딥 스위치 (8개) |
| 출력 장치 | LED (4개) | LED (8개) |
| 비트 수 | 4비트 | 8비트 |
| 오류 처리 | 없음 | 있음 |
| 동작 | 이진 (켜짐/꺼짐) | 미러링 |

### 2.2 오류 처리

```c
if (cfg_ptr == NULL) {
    xil_printf("ERROR: LED XGpio_LookupConfig failed\r\n");
    return XST_FAILURE;
}
if (XGpio_CfgInitialize(&led_gpio, cfg_ptr, cfg_ptr->BaseAddress) != XST_SUCCESS) {
    xil_printf("ERROR: LED XGpio_CfgInitialize failed\r\n");
    return XST_FAILURE;
}
```

*   **NULL 체크**: `XGpio_LookupConfig()` 반환값 검증
*   **초기화 결과 체크**: `XGpio_CfgInitialize()` 반환값 검증
*   **오류 메시지**: 구체적인 오류 위치 출력

### 2.3 스위치-LED 미러링 로직

```c
while (1) {
    sw_val = XGpio_DiscreteRead(&sw_gpio, SW_CHANNEL) & SW_MASK;
    XGpio_DiscreteWrite(&led_gpio, LED_CHANNEL, sw_val & LED_MASK);
}
```

*   **직접 미러링**: 스위치 상태가 그대로 LED에 반영됩니다.
*   **비트 대응**: SW0 → LD0, SW1 → LD1, ..., SW7 → LD7

---

# 코드 품질 및 최적화 고려사항

## 두 코드의 비교

| 항목 | zybo_GPIO.c | zedboard_GPIO.c |
|------|-------------|-----------------|
| 오류 처리 | 없음 | 완전함 |
| 출력 메시지 | 최소 | 상세 |
| 로직 복잡도 | 간단 (이진) | 직관적 (미러링) |
| 코드 품질 | 기본 | 더 나음 |

## 장점

1.  **단순성**: 두 코드 모두 GPIO 제어의 기본 패턴을 명확하게 보여줍니다.

2.  **표준 드라이버 사용**: Xilinx 표준 XGpio 드라이버를 사용하여 이식성과 유지보수성을 높입니다.

3.  **ZedBoard 코드의 오류 처리**: 초기화 실패 시 적절한 오류 메시지를 출력합니다.

## 개선 제안

### 1. Zybo 코드에 오류 처리 추가

```c
// 현재 코드
cfg_ptr = XGpio_LookupConfig(LED_ID);
XGpio_CfgInitialize(&led_device, cfg_ptr, cfg_ptr->BaseAddress);

// 개선된 코드
cfg_ptr = XGpio_LookupConfig(LED_ID);
if (cfg_ptr == NULL) {
    xil_printf("ERROR: LED XGpio_LookupConfig failed\r\n");
    return -1;
}
if (XGpio_CfgInitialize(&led_device, cfg_ptr, cfg_ptr->BaseAddress) != XST_SUCCESS) {
    xil_printf("ERROR: LED XGpio_CfgInitialize failed\r\n");
    return -1;
}
```

### 2. Zybo 코드의 로직 개선 (개별 버튼-LED 매핑)

```c
// 현재 코드: 이진 동작
if (data != 0) {
    data = LED_MASK;
} else {
    data = 0;
}

// 개선된 코드: 개별 버튼-LED 매핑
// BTN0 → LD0, BTN1 → LD1, BTN2 → LD2, BTN3 → LD3
XGpio_DiscreteWrite(&led_device, LED_CHANNEL, data);  // 직접 미러링
```

### 3. 디바운싱 추가

```c
// 버튼 디바운싱 (간단한 지연 방식)
#define DEBOUNCE_DELAY 100000

u32 read_debounced_buttons(XGpio *btn_device) {
    u32 first_read = XGpio_DiscreteRead(btn_device, BTN_CHANNEL) & BTN_MASK;
    for (volatile int i = 0; i < DEBOUNCE_DELAY; i++);
    u32 second_read = XGpio_DiscreteRead(btn_device, BTN_CHANNEL) & BTN_MASK;
    
    if (first_read == second_read)
        return first_read;
    else
        return 0;  // 불안정 상태
}
```

### 4. 인터럽트 기반 버튼 처리

```c
// 폴링 대신 인터럽트 사용
void btn_isr(void *CallbackRef) {
    u32 btn_state = XGpio_DiscreteRead(&btn_device, BTN_CHANNEL);
    // 처리 로직...
    XGpio_InterruptClear(&btn_device, BTN_CHANNEL);
}

// 설정
XGpio_InterruptEnable(&btn_device, BTN_CHANNEL);
XGpio_InterruptGlobalEnable(&btn_device);
```

### 5. 상태 변화 감지

```c
// 엣지 감지 (상태 변화만 처리)
u32 prev_data = 0;

while (1) {
    data = XGpio_DiscreteRead(&btn_device, BTN_CHANNEL) & BTN_MASK;
    
    if (data != prev_data) {
        // 상태가 변했을 때만 처리
        xil_printf("Button state changed: 0x%x -> 0x%x\r\n", prev_data, data);
        XGpio_DiscreteWrite(&led_device, LED_CHANNEL, data);
        prev_data = data;
    }
}
```

### 6. 통합 GPIO 드라이버 고려

```c
// 듀얼 채널 GPIO를 사용하면 하나의 AXI GPIO IP로 두 장치 제어 가능
// 채널 1: 입력 (버튼/스위치)
// 채널 2: 출력 (LED)
XGpio gpio;
XGpio_Initialize(&gpio, XPAR_AXI_GPIO_0_DEVICE_ID);
XGpio_SetDataDirection(&gpio, 1, 0xFF);  // 채널 1: 입력
XGpio_SetDataDirection(&gpio, 2, 0x00);  // 채널 2: 출력
```

---

# 결론

`GPIO-demo/` 폴더의 코드는 Xilinx FPGA 보드에서 GPIO를 제어하는 기본적인 패턴을 보여주는 입문용 데모입니다.

*   **`zybo_GPIO.c`**: Zybo Z7-10 보드의 4개 버튼과 4개 LED를 제어합니다. 간단한 이진 로직(버튼 눌림 → 모든 LED 켜짐)을 구현합니다. 오류 처리가 없어 프로덕션 코드로는 부적합하지만, 학습용으로는 충분합니다.

*   **`zedboard_GPIO.c`**: ZedBoard의 8개 딥 스위치와 8개 LED를 제어합니다. 스위치 상태를 LED에 직접 미러링하는 직관적인 로직을 구현합니다. 완전한 오류 처리가 포함되어 있어 더 견고합니다.

두 코드 모두 Xilinx 표준 XGpio 드라이버를 사용하여 GPIO 제어의 기본 패턴을 명확하게 보여주며, FPGA 입문자에게 좋은 시작점이 됩니다. 위의 개선 제안들을 적용하면 더욱 견고하고 기능적인 GPIO 애플리케이션으로 발전시킬 수 있습니다.
