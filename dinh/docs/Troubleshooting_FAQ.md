# 트러블슈팅 및 자주 묻는 질문 (FAQ)

> FPGA 개발 중 발생하는 일반적인 문제와 해결 방법

---

## 목차

1. [Vivado 문제](#1-vivado-문제)
2. [Vitis HLS 문제](#2-vitis-hls-문제)
3. [Vitis IDE 문제](#3-vitis-ide-문제)
4. [보드/하드웨어 문제](#4-보드하드웨어-문제)
5. [SD 카드/파일 시스템 문제](#5-sd-카드파일-시스템-문제)
6. [인터럽트 문제](#6-인터럽트-문제)
7. [성능 문제](#7-성능-문제)
8. [자주 묻는 질문 (FAQ)](#8-자주-묻는-질문-faq)

---

## 1. Vivado 문제

### 1.1 보드 파일을 찾을 수 없음

**증상:**
```
Board "zybo-z7-10" not found
```

**해결:**
1. Digilent 보드 파일 다운로드:
   ```bash
   git clone https://github.com/Digilent/vivado-boards.git
   ```
2. 보드 파일 복사:
   ```
   vivado-boards/new/board_files/* 
   → <Vivado_Install>/2022.1/data/boards/board_files/
   ```
3. Vivado 재시작

---

### 1.2 합성 오류: 리소스 초과

**증상:**
```
ERROR: [Place 30-640] Place Check: Utilization exceeds device capacity
BRAM: 120% (used) > 100% (available)
```

**해결:**
1. **HLS 최적화 조정:**
   ```cpp
   // ARRAY_PARTITION 타입 변경
   #pragma HLS ARRAY_PARTITION variable=buf cyclic factor=4  // complete 대신
   
   // 버퍼 크기 축소
   #define BUFFER_SIZE 16  // 32 대신
   ```

2. **리소스 공유 활성화:**
   ```cpp
   #pragma HLS ALLOCATION instances=mul limit=2
   ```

3. **더 큰 디바이스 사용:**
   - Zybo Z7-10 → Z7-20 (3배 리소스)

---

### 1.3 타이밍 미달성

**증상:**
```
CRITICAL WARNING: [Timing 38-282] WNS = -0.234 ns
```

**해결:**
1. **클럭 주파수 낮추기:**
   - 100MHz → 75MHz

2. **HLS에서 파이프라인 조정:**
   ```cpp
   #pragma HLS PIPELINE II=2  // II=1 대신
   ```

3. **레지스터 삽입:**
   ```cpp
   #pragma HLS LATENCY min=1 max=1
   ```

---

### 1.4 Block Design 연결 오류

**증상:**
```
ERROR: [BD 41-237] Cannot connect 'M_AXI' to 'S_AXI_HP0'
```

**해결:**
1. **클럭 도메인 확인:**
   - 모든 연결된 IP가 같은 클럭 사용

2. **데이터 폭 확인:**
   - HP 포트는 64-bit, GP 포트는 32-bit

3. **AXI Interconnect 추가:**
   - 직접 연결 대신 Interconnect 경유

---

## 2. Vitis HLS 문제

### 2.1 C Simulation 실패

**증상:**
```
Segmentation fault (core dumped)
```

**해결:**
1. **배열 범위 확인:**
   ```cpp
   // 잘못됨
   int arr[10];
   arr[10] = 0;  // 범위 초과
   
   // 올바름
   arr[9] = 0;
   ```

2. **포인터 NULL 체크:**
   ```cpp
   if (ptr == nullptr) return -1;
   ```

3. **테스트벤치 메모리 크기:**
   ```cpp
   #define TEST_SIZE 1024
   int* test_data = new int[TEST_SIZE];  // malloc 대신
   ```

---

### 2.2 합성 실패: II 목표 미달성

**증상:**
```
WARNING: [HLS 200-880] Loop 'loop' has II = 3, target II = 1
```

**원인 및 해결:**

| 원인 | 해결 |
|------|------|
| 메모리 의존성 | 배열 분할 |
| 연산 의존성 | 파이프라인 깊이 증가 |
| 리소스 부족 | 연산 병렬성 감소 |

```cpp
// 메모리 의존성 해결
int buf[N];
#pragma HLS ARRAY_PARTITION variable=buf complete

// 또는 의존성 명시
#pragma HLS DEPENDENCE variable=buf inter false
```

---

### 2.3 Co-Simulation 행 (Hang)

**증상:**
```
Co-simulation이 무한히 실행됨
```

**해결:**
1. **무한 루프 확인:**
   ```cpp
   // 위험
   while (condition) { ... }  // condition이 항상 true일 수 있음
   
   // 안전
   for (int i = 0; i < MAX_ITER; i++) {
       if (!condition) break;
   }
   ```

2. **스트림 읽기/쓰기 균형:**
   ```cpp
   // in과 out의 요소 수가 일치해야 함
   for (int i = 0; i < N; i++) {
       out.write(in.read());
   }
   ```

3. **depth 파라미터 증가:**
   ```cpp
   #pragma HLS INTERFACE m_axi port=data depth=4096  // depth 증가
   ```

---

### 2.4 Export RTL 실패

**증상:**
```
ERROR: [HLS 200-70] Export of IP failed
```

**해결:**
1. **합성 먼저 실행:**
   - C Synthesis가 성공해야 Export 가능

2. **인터페이스 일관성:**
   ```cpp
   // 모든 m_axi에 offset=slave 사용
   #pragma HLS INTERFACE m_axi port=a offset=slave bundle=A
   #pragma HLS INTERFACE m_axi port=b offset=slave bundle=B
   ```

3. **INTERFACE pragma 완전성:**
   ```cpp
   #pragma HLS INTERFACE s_axilite port=return bundle=CTRL  // 필수
   ```

---

## 3. Vitis IDE 문제

### 3.1 플랫폼 빌드 실패

**증상:**
```
ERROR: [XSCT 50-38] Unable to generate platform project
```

**해결:**
1. **XSA 파일 확인:**
   - Vivado에서 "Include bitstream" 옵션으로 다시 내보내기

2. **Vitis 버전 일치:**
   - Vivado와 Vitis 버전이 동일해야 함 (예: 둘 다 2022.1)

3. **워크스페이스 정리:**
   - 새 워크스페이스에서 다시 시작

---

### 3.2 링커 오류: undefined reference

**증상:**
```
undefined reference to `XGpio_Initialize'
```

**해결:**
1. **BSP 라이브러리 확인:**
   ```
   Platform → BSP Settings → Libraries
   └── 필요한 드라이버가 포함되어 있는지 확인
   ```

2. **플랫폼 재빌드:**
   ```
   Platform Project → Clean → Build
   Application Project → Clean → Build
   ```

3. **헤더 파일 포함:**
   ```c
   #include "xgpio.h"  // 누락된 헤더 추가
   ```

---

### 3.3 디버그 연결 실패

**증상:**
```
Error: Unable to connect to target. Please verify that
the JTAG connection is correct.
```

**해결:**
1. **부트 모드 확인:**
   - ZCU104: SW6 = 0000 (JTAG)
   - Zybo: JP5 제거 (JTAG)

2. **USB 연결 확인:**
   ```
   장치 관리자 → USB Serial Converter 확인
   ```

3. **드라이버 재설치:**
   ```
   <Vivado>/data/xicom/cable_drivers/
   └── install_drivers (관리자 권한)
   ```

4. **보드 재시작:**
   - 전원 OFF → 5초 대기 → 전원 ON

---

## 4. 보드/하드웨어 문제

### 4.1 보드 전원이 켜지지 않음

**체크리스트:**
- [ ] 전원 어댑터 연결 확인 (ZCU104: 12V)
- [ ] 전원 스위치 ON 확인
- [ ] USB 전원 충분 (Zybo: 1A 이상 권장)
- [ ] 전원 LED 점등 확인

---

### 4.2 UART 출력 없음

**증상:**
```
터미널에 아무것도 출력되지 않음
```

**해결:**
1. **COM 포트 확인:**
   - 보통 2개 중 높은 번호가 UART

2. **보드레이트 확인:**
   - 115200, 8-N-1

3. **UART 핀 설정 확인:**
   - Vivado에서 UART 활성화 여부

4. **코드 확인:**
   ```c
   // 초기화 필요 없음 (standalone BSP가 자동 초기화)
   xil_printf("Hello\r\n");  // \r\n 필수
   ```

---

### 4.3 애플리케이션이 즉시 크래시

**증상:**
```
실행 즉시 재부팅되거나 멈춤
```

**해결:**
1. **스택/힙 크기:**
   ```
   Generate Linker Script:
   ├── Heap: 4MB 이상
   └── Stack: 64KB 이상
   ```

2. **메모리 영역:**
   ```
   모든 섹션을 DDR (psu_ddr_0)로 매핑
   ```

3. **캐시 문제:**
   ```c
   Xil_DCacheDisable();  // 테스트용으로 캐시 비활성화
   ```

---

## 5. SD 카드/파일 시스템 문제

### 5.1 f_mount 실패

**에러 코드:**

| 코드 | 의미 | 해결 |
|------|------|------|
| 1 (FR_DISK_ERR) | 디스크 오류 | 다른 SD 카드 사용 |
| 3 (FR_NOT_READY) | 드라이브 준비 안됨 | 카드 삽입 확인 |
| 13 (FR_NO_FILESYSTEM) | 파일시스템 없음 | FAT32로 포맷 |

**디버깅:**
```c
FRESULT rc = f_mount(&fatfs, "0:/", 1);
xil_printf("f_mount: %d\r\n", rc);
```

---

### 5.2 파일을 찾을 수 없음

**증상:**
```
f_open: 4 (FR_NO_FILE)
```

**해결:**
1. **경로 확인:**
   ```c
   "0:/test.bmp"      // 올바름
   "test.bmp"         // 틀림 (드라이브 번호 필요)
   "0:\\test.bmp"     // 틀림 (백슬래시)
   ```

2. **대소문자:**
   - FAT32는 대소문자 구분 없음
   - 하지만 긴 파일명(LFN) 사용 시 `use_lfn=1` 필요

3. **파일 존재 확인:**
   ```c
   FILINFO fno;
   FRESULT rc = f_stat("0:/test.bmp", &fno);
   xil_printf("f_stat: %d, size: %lu\r\n", rc, fno.fsize);
   ```

---

### 5.3 파일 읽기 불완전

**증상:**
```
읽은 바이트 수가 예상보다 적음
```

**해결:**
```c
UINT bytes_read;
FRESULT rc = f_read(&file, buffer, size, &bytes_read);

// 확인
if (bytes_read != size) {
    xil_printf("Partial read: %u/%u\r\n", bytes_read, size);
}
```

---

## 6. 인터럽트 문제

### 6.1 인터럽트 핸들러 미호출

**체크리스트:**
- [ ] Vivado에서 인터럽트 연결 확인
- [ ] GIC 초기화 완료
- [ ] IP 인터럽트 활성화 (GIE, IER)
- [ ] 올바른 IRQ ID 사용

**디버깅 코드:**
```c
// GIC 초기화 확인
XScuGic_Config* cfg = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID);
if (cfg == NULL) {
    xil_printf("GIC config not found!\r\n");
    return -1;
}

// IP 인터럽트 상태 확인
uint32_t isr = Xil_In32(ACCEL_BASE + ISR_OFFSET);
xil_printf("ISR: 0x%08X\r\n", isr);
```

---

### 6.2 인터럽트 스톰 (무한 반복)

**증상:**
```
ISR이 무한히 호출됨
```

**원인:**
- AXI-Lite 쓰기가 비동기라서 ISR 클리어 전에 리턴

**해결:**
```c
void my_isr(void* ref) {
    // 인터럽트 클리어
    Xil_Out32(BASE + ISR_OFFSET, 0x1);
    
    // 중요: 더미 읽기로 AXI 쓰기 완료 보장
    (void)Xil_In32(BASE + ISR_OFFSET);
    
    done_flag = 1;
}
```

---

### 6.3 인터럽트 ID 찾기

**xparameters.h에서 확인:**
```c
#define XPAR_FABRIC_MEMCOPY_ACCEL_0_INTERRUPT_INTR  61
```

**또는 계산:**
```
Zynq-7000:  IRQ_F2P[0] = 61, [1] = 62, ...
ZynqMP:     IRQ_F2P[0] = 121, [1] = 122, ...
```

---

## 7. 성능 문제

### 7.1 예상보다 느린 가속기

**원인 및 해결:**

| 원인 | 진단 | 해결 |
|------|------|------|
| 캐시 미스 | 프로파일링 | 데이터 프리페치 |
| 버스트 비효율 | AXI 모니터 | 버스트 크기 증가 |
| 캐시 일관성 | Flush/Invalidate 누락 | 적절한 캐시 관리 |

---

### 7.2 CPU 오버헤드

**폴링 vs 인터럽트:**
```c
// 폴링 (CPU 100% 사용)
while (!(Xil_In32(BASE + CTRL) & 0x02));

// 인터럽트 + WFI (저전력)
while (!done_flag) {
    __asm__ volatile("wfi");
}
```

---

### 7.3 데이터 전송 병목

**측정:**
```c
XTime t0, t1;
XTime_GetTime(&t0);

// 가속기 실행

XTime_GetTime(&t1);
double time_sec = (double)(t1 - t0) / COUNTS_PER_SECOND;
double bandwidth_mbps = (data_bytes / 1e6) / time_sec;
xil_printf("Bandwidth: %d MB/s\r\n", (int)bandwidth_mbps);
```

**이론적 최대:**
- 32-bit @ 100MHz = 400 MB/s
- 64-bit @ 100MHz = 800 MB/s

---

## 8. 자주 묻는 질문 (FAQ)

### Q1: Vivado와 Vitis 버전이 달라도 되나요?

**A:** 아니요. 반드시 같은 버전을 사용해야 합니다.
- XSA 파일은 버전 종속적
- 호환성 문제 발생 가능

---

### Q2: 베어메탈과 Linux 중 어떤 것을 사용해야 하나요?

**A:** 목적에 따라 다릅니다.

| 베어메탈 | Linux |
|----------|-------|
| 최소 지연시간 | 파일시스템, 네트워크 |
| 결정론적 동작 | 사용자 인터페이스 |
| 단순한 시스템 | 복잡한 애플리케이션 |
| 빠른 부팅 | 동적 로딩 |

---

### Q3: HLS IP의 최대 클럭 주파수는?

**A:** 타겟 디바이스와 설계에 따라 다릅니다.
- Zybo Z7: 100-200 MHz
- ZCU104: 200-400 MHz
- 합성 리포트의 타이밍 결과 확인

---

### Q4: 왜 printf가 동작하지 않나요?

**A:** 베어메탈에서는 `xil_printf` 사용:
```c
// 틀림
printf("Hello %f\n", 3.14);  // float 미지원

// 올바름
xil_printf("Hello %d\r\n", 314);  // 정수만, \r\n 필수
```

---

### Q5: SD 카드 속도를 높이려면?

**A:**
1. Class 10 또는 UHS-I 카드 사용
2. 클러스터 크기 32KB로 포맷
3. 대용량 버퍼로 읽기 (작은 파일 여러 번 읽기 비효율)

---

### Q6: 가속기가 CPU보다 느린 이유는?

**A:** 가능한 원인:
1. 데이터 크기가 작아서 오버헤드가 지배적
2. 캐시 flush/invalidate 비용
3. 가속기 시작/종료 오버헤드
4. 최적화되지 않은 HLS 코드

일반적으로 데이터 크기가 수 KB 이상일 때 가속기가 유리합니다.

---

### Q7: JTAG 디버깅 중 변수 값이 이상합니다

**A:** 최적화 레벨 확인:
- `-O0`: 디버깅 최적 (느림)
- `-O2` 이상: 변수가 최적화되어 사라질 수 있음

디버깅용으로 `-O0` 사용 권장.

---

### Q8: HLS에서 malloc을 사용할 수 있나요?

**A:** 아니요. 동적 메모리 할당은 HLS에서 지원되지 않습니다.
```cpp
// 틀림
int* buf = (int*)malloc(N * sizeof(int));

// 올바름
int buf[N];  // 컴파일 타임에 크기 결정
```

---

### Q9: 여러 HLS IP를 하나의 디자인에서 사용할 수 있나요?

**A:** 네. 각 IP에 고유한 AXI 주소를 할당하면 됩니다.
- AXI Interconnect로 여러 IP 연결
- Address Editor에서 각 IP에 다른 주소 할당

---

### Q10: Co-simulation이 너무 오래 걸립니다

**A:** 
1. 테스트 데이터 크기 줄이기
2. `Optimizing Compile` 활성화
3. Dump Trace 비활성화 (필요 없으면)
4. 간단한 테스트 케이스로 시작

---

## 추가 리소스

### 공식 문서
- UG1399: Vitis HLS User Guide
- UG1137: Zynq UltraScale+ Software Developer Guide
- UG585: Zynq-7000 TRM

### 커뮤니티
- Xilinx Forums: https://support.xilinx.com
- Digilent Forums: https://forum.digilent.com
- Reddit r/FPGA

### 예제 저장소
- https://github.com/Xilinx/Vitis-Tutorials
- https://github.com/Xilinx/Vitis-HLS-Introductory-Examples

---

*문서 작성일: 2026-02-06*  
*관련 문서: FPGA_Learning_Guide.md, HLS_Development_Guide.md*
