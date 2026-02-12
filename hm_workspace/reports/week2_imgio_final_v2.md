# 2주차 실행 보고서: SD Image I/O

## 실행 환경
- **날짜**: 2026-02-12
- **보드**: ZCU104
- **SD 카드**: [용량/제조사 기입]
- **이미지 파일**: 
  - `img_01.bmp` ~ `img_10.bmp` (224x224, 24bpp RGB)
  - `bmp_24.bmp` (최종 검증용)

## 실행 결과

### 10장 연속 테스트
- **상태**: PASS (10/10)
- **로그**:
  ```
  --- ZCU104 Baremetal Image Loader ---
  [SD] SUCCESS: SD Card mounted
  
  --- 10-image sequential test ---
  
  [Test] 0:/img_01.bmp
  PASS: 0:/img_01.bmp (224x224, 150582 bytes)
  
  [Test] 0:/img_02.bmp
  PASS: 0:/img_02.bmp (224x224, 150582 bytes)
  
  ... (중략) ...
  
  [Test] 0:/img_10.bmp
  PASS: 0:/img_10.bmp (224x224, 150582 bytes)
  
  PASS: 10/10 images
  ```

### 최종 검증 (bmp_24.bmp)
- **파일명**: `0:/bmp_24.bmp`
- **상태**: PASS
- **로그**:
  ```
  --- Final test: bmp_24.bmp ---
  
  [Test] 0:/bmp_24.bmp
  PASS: 0:/bmp_24.bmp (224x224, 150582 bytes)
  
  ALL PASS: 10-image test + bmp_24.bmp final test
  ```

## 메모리 설정 확인
- **Linker Script**: `sd_img_loader/src/lscript.ld`
- **Stack Size**: 128KB (`0x20000`)
- **Heap Size**: 2MB (`0x200000`)
- **Global Buffer**:
  - `file_buffer`: ~6MB (BSS 섹션)
  - `image_buffer`: ~6MB (BSS 섹션)
- **동작 확인**: 대용량 버퍼 사용 시에도 스택 오버플로우 없이 정상 동작함.

## 검증 완료 항목
- [x] SD 카드 마운트 (f_mount)
- [x] 파일 존재 확인 (f_stat)
- [x] 파일 읽기 및 캐시 플러시 (f_read + Xil_DCacheFlushRange)
- [x] BMP 헤더 파싱 및 유효성 검사
- [x] 픽셀 데이터 추출 (BGR -> RGB 변환)
- [x] 연속 10회 안정성 테스트
- [x] 최종 샘플 이미지 검증

## 다음 단계 (3주차) 준비
- MobileNetV1 가중치 파일 (`mobilenet_v1_weights.bin`) 준비
- ImageNet 라벨 파일 (`imagenet_labels.txt`) 준비
- FPGA 비트스트림(`mobilenet_wrapper.bit`) 로드 기능 통합 준비
