# `dinh/zcu104-a53-mobilenetv1-baseline/tools/` 코드 리뷰 보고서

이 보고서는 `zcu104-a53-mobilenetv1-baseline` 프로젝트의 `tools/` 폴더에 포함된 Python 스크립트들에 대한 상세한 코드 리뷰 결과를 담고 있습니다. 이 폴더에는 합성 숫자 데이터셋 생성 및 모델 학습(`synth_digits_train.py`)과 학습된 가중치를 펌웨어용 바이너리로 내보내는 스크립트(`export_bins.py`)가 포함되어 있습니다.

---

# `synth_digits_train.py` 코드 리뷰 보고서

이 스크립트는 32x32 RGB 합성 숫자 이미지를 생성하고, 이를 사용하여 간단한 Depthwise Separable CNN 모델을 학습시키는 역할을 합니다.

## 1. 헤더 및 의존성

```python
import os, math, random
import numpy as np
from PIL import Image, ImageDraw, ImageFont
import torch
import torch.nn as nn
import torch.nn.functional as F
from torch.utils.data import Dataset, DataLoader
```

*   **표준 라이브러리**: `os`, `math`, `random`은 파일 시스템 작업, 수학 연산, 난수 생성에 사용됩니다.
*   **NumPy**: 배열 연산 및 이미지 데이터 처리에 사용됩니다.
*   **Pillow (PIL)**: 합성 숫자 이미지 렌더링에 사용됩니다. `ImageDraw`와 `ImageFont`를 통해 텍스트를 이미지로 그립니다.
*   **PyTorch**: 신경망 모델 정의, 학습, 데이터 로딩에 사용됩니다.

## 2. 양자화 파라미터

```python
IN_SCALE = 0.02
IN_ZP    = 128
```

*   **역할**: 이 파라미터들은 펌웨어에서 사용하는 양자화 스케일과 영점(zero-point)과 일치하도록 설정되어 있습니다.
*   **중요성**: 학습 시 입력 데이터를 펌웨어에서 역양자화(dequantize)하는 방식과 동일하게 전처리함으로써, 학습된 모델이 실제 하드웨어에서 동일하게 동작하도록 보장합니다.
*   **공식**: `real_value = IN_SCALE * (uint8_value - IN_ZP)` → `0.02 * (pixel - 128)`

## 3. 모델 정의: `TinyDigitDSCNN`

```python
class TinyDigitDSCNN(nn.Module):
    def __init__(self, cout=10):
        super().__init__()
        # Depthwise 3x3 (groups=3), Cin=3 -> Cout_dw=3
        self.dw = nn.Conv2d(3, 3, kernel_size=3, padding=1, groups=3, bias=False)
        # Pointwise 1x1: 3 -> 10 classes
        self.pw = nn.Conv2d(3, cout, kernel_size=1, bias=False)

    def forward(self, x):
        x = self.dw(x)                 # DW
        x = torch.clamp(x, 0.0, 6.0)   # ReLU6 clamp
        x = self.pw(x)                 # PW
        x = x.mean(dim=(2, 3))         # Global Average Pool (H,W)
        return x                        # logits [N,10]
```

### 3.1 모델 아키텍처 분석

*   **Depthwise Convolution (`self.dw`)**: 
    *   `groups=3`으로 설정하여 각 입력 채널(RGB)에 대해 독립적인 3x3 필터를 적용합니다.
    *   `padding=1`로 'Same' 패딩을 구현하여 출력 크기가 입력과 동일하게 유지됩니다.
    *   `bias=False`로 편향을 제거하여 양자화된 펌웨어 구현과 일치시킵니다.

*   **ReLU6 활성화 (`torch.clamp`)**: 
    *   `torch.clamp(x, 0.0, 6.0)`은 `max(0, min(6, x))` 형태의 ReLU6를 구현합니다.
    *   MobileNet에서 사용되는 활성화 함수로, 양자화 친화적입니다.

*   **Pointwise Convolution (`self.pw`)**: 
    *   1x1 커널을 사용하여 3개의 채널을 10개의 출력 클래스로 매핑합니다.
    *   채널 간 정보를 결합하는 역할을 합니다.

*   **Global Average Pooling (`x.mean(dim=(2, 3))`)**: 
    *   공간 차원(H, W)에 대해 평균을 계산하여 `[N, 10]` 형태의 로짓(logits)을 생성합니다.
    *   Fully Connected 레이어 없이 분류를 수행하여 파라미터 수를 최소화합니다.

### 3.2 펌웨어 매칭

이 모델 구조는 `ref_kernels.c`에 구현된 `dwconv3x3_nhwc_u8`, `pwconv1x1_nhwc_u8`, `avgpool_global_nhwc_u8` 함수들과 정확히 일치하도록 설계되었습니다.

## 4. 합성 데이터셋: `SyntheticDigits`

### 4.1 폰트 로딩

```python
FONT_PATHS = [
    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
]
def load_font(size):
    for p in FONT_PATHS:
        try:
            return ImageFont.truetype(p, size=size)
        except Exception:
            pass
    try:
        return ImageFont.load_default()
    except Exception:
        return None
```

*   **폴백 메커니즘**: 여러 폰트 경로를 시도하고, 실패 시 기본 폰트로 폴백합니다.
*   **크로스 플랫폼 고려**: Linux 시스템의 일반적인 폰트 경로를 사용합니다.

### 4.2 숫자 이미지 렌더링

```python
def render_digit_32x32(d: int) -> np.ndarray:
    W, H = 32, 32
    size = random.randint(22, 30)          # 폰트 크기 변동
    font = load_font(size)
    img = Image.new('L', (W, H), color=0)  # 그레이스케일, 검은 배경
    draw = ImageDraw.Draw(img)
    text = str(d)
    stroke = random.randint(0, 2)          # 테두리 두께 변동
    # ... 텍스트 중앙 배치 + 약간의 랜덤 오프셋 ...
    img = img.rotate(random.uniform(-15, 15), ...)  # 회전 증강
    # ... 노이즈 추가 ...
    rgb = np.stack([arr, arr, arr], axis=0)  # [3, H, W]
    return rgb
```

*   **데이터 증강(Data Augmentation)**:
    *   **폰트 크기 변동**: 22~30 픽셀 범위
    *   **위치 변동**: 중앙에서 ±4 픽셀
    *   **회전**: -15° ~ +15° 범위
    *   **가우시안 노이즈**: σ=8.0
    *   **테두리(stroke) 변동**: 0~2 픽셀

*   **출력 형식**: `[3, 32, 32]` 형태의 RGB 이미지 (채널 우선, CHW 형식)

### 4.3 Dataset 클래스

```python
class SyntheticDigits(Dataset):
    def __init__(self, n_per_class: int, seed: int = 0):
        self.W, self.H = 32, 32
        self.npc = n_per_class
        self.N = n_per_class * 10
        self.rng = random.Random(seed)

    def __len__(self): return self.N

    def __getitem__(self, idx):
        d = idx % 10                        # 클래스 레이블
        rgb = render_digit_32x32(d)         # [3,32,32], uint8
        rgb_u8 = torch.from_numpy(rgb.copy())
        rgb_f  = rgb_u8.float()
        x_real = IN_SCALE * (rgb_f - IN_ZP) # 펌웨어 역양자화 시뮬레이션
        y = torch.tensor(d, dtype=torch.long)
        return x_real, y
```

*   **균등 분포**: 각 클래스(0-9)당 동일한 수의 샘플을 생성합니다.
*   **온라인 생성**: `__getitem__` 호출 시마다 새로운 이미지를 생성하여 무한한 변동성을 제공합니다.
*   **양자화 시뮬레이션**: `IN_SCALE * (rgb_f - IN_ZP)` 공식으로 펌웨어에서의 역양자화 과정을 시뮬레이션합니다.

## 5. 학습 루틴

```python
def train(epochs=5, batch=128, n_per_class_train=800, n_per_class_val=200, lr=1e-3, device=None):
    if device is None:
        device = "cuda" if torch.cuda.is_available() else "cpu"
    
    train_set = SyntheticDigits(n_per_class=n_per_class_train, seed=123)
    val_set   = SyntheticDigits(n_per_class=n_per_class_val,   seed=456)
    
    train_loader = DataLoader(train_set, batch_size=batch, shuffle=True, num_workers=2, pin_memory=True)
    val_loader   = DataLoader(val_set,   batch_size=batch, shuffle=False, num_workers=2, pin_memory=True)
    
    model = TinyDigitDSCNN(cout=10).to(device)
    opt = torch.optim.Adam(model.parameters(), lr=lr)
    
    for ep in range(1, epochs+1):
        # ... 학습 루프 ...
        # ... 검증 루프 ...
        print(f"Epoch {ep}: train loss={tr_loss:.4f}  val acc={val_acc:.2f}%")
    
    os.makedirs("artifacts", exist_ok=True)
    torch.save(model.state_dict(), "artifacts/digits_tiny_dsconv.pt")
```

### 5.1 학습 설정 분석

| 하이퍼파라미터 | 기본값 | 설명 |
|----------------|--------|------|
| epochs | 5 | 학습 에포크 수 |
| batch | 128 | 배치 크기 |
| n_per_class_train | 800 | 클래스당 학습 샘플 수 (총 8,000개) |
| n_per_class_val | 200 | 클래스당 검증 샘플 수 (총 2,000개) |
| lr | 1e-3 | Adam 옵티마이저 학습률 |

### 5.2 학습 파이프라인

1.  **손실 함수**: `F.cross_entropy` (소프트맥스 + 크로스 엔트로피)
2.  **옵티마이저**: Adam
3.  **검증 지표**: Top-1 정확도
4.  **체크포인트 저장**: `artifacts/digits_tiny_dsconv.pt`

---

# `export_bins.py` 코드 리뷰 보고서

이 스크립트는 학습된 PyTorch 모델의 가중치를 양자화하고, 펌웨어에서 사용할 수 있는 바이너리 형식으로 내보내는 역할을 합니다.

## 1. 양자화 파라미터

```python
W_ZP = 128  # weight zero-point (single for both layers)

def compute_single_w_scale(dw_w, pw_w):
    max_abs = max(dw_w.abs().max().item(), pw_w.abs().max().item())
    return max_abs / 127.0 if max_abs > 1e-12 else 0.01
```

*   **단일 스케일**: Depthwise와 Pointwise 가중치 모두에 대해 동일한 스케일을 사용합니다. 이는 펌웨어 API의 단순화를 위한 설계 결정입니다.
*   **대칭 양자화**: `w_zp = 128`을 중심으로 하는 대칭 양자화를 사용합니다.
*   **스케일 계산**: 두 레이어의 가중치 중 최대 절대값을 기준으로 스케일을 계산하여 오버플로우를 방지합니다.

## 2. 양자화 함수

```python
def quant_u8(w, w_scale, w_zp=W_ZP):
    q = np.round(w / w_scale).astype(np.int32) + w_zp
    q = np.clip(q, 0, 255).astype(np.uint8)
    return q
```

*   **양자화 공식**: `q = round(w / w_scale) + w_zp`
*   **클램핑**: 결과를 `uint8` 범위(0-255)로 제한합니다.
*   **반올림**: `np.round`를 사용하여 가장 가까운 정수로 반올림합니다.

## 3. 메인 내보내기 로직

```python
def main():
    state = torch.load("artifacts/digits_tiny_dsconv.pt", map_location="cpu")
    model = TinyDigitDSCNN(cout=10)
    model.load_state_dict(state)
    model.eval()

    # 가중치 추출
    dw = model.dw.weight.detach().cpu()  # [3,1,3,3]
    pw = model.pw.weight.detach().cpu()  # [10,3,1,1]
    dw_w = dw.squeeze(1)                 # [3,3,3]
    pw_w = pw.squeeze(2).squeeze(2)      # [10,3]

    w_scale = compute_single_w_scale(dw_w, pw_w)
    
    # 양자화
    dw_q = quant_u8(dw_w.numpy(), w_scale, W_ZP)
    pw_q = quant_u8(pw_w.numpy(), w_scale, W_ZP)
```

### 3.1 가중치 형태 변환

| 레이어 | PyTorch 형태 | 변환 후 형태 | 설명 |
|--------|--------------|--------------|------|
| Depthwise | `[3, 1, 3, 3]` | `[3, 3, 3]` | 채널별 3x3 커널 |
| Pointwise | `[10, 3, 1, 1]` | `[10, 3]` | 출력×입력 채널 |

## 4. 펌웨어 레이아웃 변환

### 4.1 Depthwise 가중치 레이아웃

```python
# DW: c*9 + (ky+1)*3 + (kx+1)
dw_flat = []
for c in range(3):
    for ky in (-1, 0, 1):
        for kx in (-1, 0, 1):
            dw_flat.append(int(dw_q[c, ky+1, kx+1]))
```

*   **인덱싱 공식**: `c*9 + (ky+1)*3 + (kx+1)`
*   **메모리 레이아웃**: 채널 우선, 그 다음 커널 y, 커널 x 순서
*   **펌웨어 매칭**: `ref_kernels.c`의 `k3x3[c*9+(ky+1)*3+(kx+1)]` 접근 패턴과 일치

### 4.2 Pointwise 가중치 레이아웃

```python
# PW: co*Cin + ci
pw_flat = []
for co in range(10):
    for ci in range(3):
        pw_flat.append(int(pw_q[co, ci]))
```

*   **인덱싱 공식**: `co*Cin + ci`
*   **메모리 레이아웃**: 출력 채널 우선, 입력 채널 순서
*   **펌웨어 매칭**: `ref_kernels.c`의 `k1x1[co*Cin]` 접근 패턴과 일치

## 5. 출력 파일 생성

```python
os.makedirs("artifacts", exist_ok=True)
with open("artifacts/dw3x3_c3.bin", "wb") as f: f.write(bytearray(dw_flat))
with open("artifacts/pw1x1_c10x3.bin", "wb") as f: f.write(bytearray(pw_flat))
with open("artifacts/labels.txt", "w", encoding="utf-8") as f:
    for d in range(10): f.write(f"{d}\n")
```

| 파일명 | 크기 | 설명 |
|--------|------|------|
| `dw3x3_c3.bin` | 27 bytes | 3채널 × 9(3×3) = 27 가중치 |
| `pw1x1_c10x3.bin` | 30 bytes | 10출력 × 3입력 = 30 가중치 |
| `labels.txt` | ~20 bytes | 0-9 숫자 레이블 |
| `sample_32x32.bmp` | ~3KB | 테스트용 BMP 샘플 |

## 6. 무결성 검증

```python
import hashlib
for fn in ["dw3x3_c3.bin", "pw1x1_c10x3.bin", "labels.txt", "sample_32x32.bmp"]:
    p = os.path.join("artifacts", fn)
    with open(p, "rb") as f: data = f.read()
    sha = hashlib.sha256(data).hexdigest()
    print(f"{fn:>20}  size={len(data):4d}  sha256={sha}")
```

*   **SHA-256 해시**: 각 출력 파일의 무결성을 검증하기 위한 해시를 출력합니다.
*   **크기 확인**: 파일 크기가 예상과 일치하는지 확인합니다.

---

# 코드 품질 및 최적화 고려사항

## 장점

1.  **펌웨어 일관성**: 학습 스크립트와 내보내기 스크립트 모두 펌웨어의 양자화 파라미터 및 메모리 레이아웃과 정확히 일치하도록 설계되었습니다.

2.  **데이터 증강**: 합성 데이터셋은 다양한 증강 기법(크기, 위치, 회전, 노이즈)을 적용하여 모델의 일반화 능력을 향상시킵니다.

3.  **모듈화**: 학습(`synth_digits_train.py`)과 내보내기(`export_bins.py`)가 분리되어 있어 각각 독립적으로 실행할 수 있습니다.

4.  **무결성 검증**: SHA-256 해시를 통해 생성된 바이너리 파일의 무결성을 검증할 수 있습니다.

## 개선 제안

### 1. 재현성 강화

```python
# 현재 코드
def render_digit_32x32(d: int) -> np.ndarray:
    size = random.randint(22, 30)
    # ...
```

*   **문제점**: `render_digit_32x32` 함수 내에서 전역 `random` 모듈을 사용하므로 재현성이 보장되지 않습니다.
*   **개선 방안**: `Dataset` 클래스의 `self.rng`를 함수에 전달하여 일관된 난수 생성을 보장합니다.

```python
# 개선된 코드
def render_digit_32x32(d: int, rng: random.Random) -> np.ndarray:
    size = rng.randint(22, 30)
    # ...
```

### 2. 크로스 플랫폼 폰트 지원

```python
# 현재 코드
FONT_PATHS = [
    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
]
```

*   **문제점**: Linux 경로만 지원하므로 Windows/macOS에서 폰트 로딩이 실패할 수 있습니다.
*   **개선 방안**: 플랫폼별 폰트 경로를 추가합니다.

```python
import sys
FONT_PATHS = [
    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",  # Linux
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "C:/Windows/Fonts/arial.ttf",                             # Windows
    "/System/Library/Fonts/Helvetica.ttc",                    # macOS
]
```

### 3. 양자화 정확도 검증

*   **현재 상태**: 양자화된 가중치와 원본 float 가중치 간의 오차를 검증하는 코드가 없습니다.
*   **개선 방안**: 양자화 전후의 가중치를 비교하고, 역양자화 후 오차를 출력하는 검증 코드를 추가합니다.

```python
# 양자화 오차 검증
dw_reconstructed = (dw_q.astype(np.float32) - W_ZP) * w_scale
mse = np.mean((dw_w.numpy() - dw_reconstructed) ** 2)
print(f"DW quantization MSE: {mse:.6f}")
```

### 4. 모델 추론 테스트 추가

*   **현재 상태**: 학습 후 검증 정확도만 출력하고, 양자화된 가중치로 실제 추론을 테스트하지 않습니다.
*   **개선 방안**: `export_bins.py`에 양자화된 가중치를 사용한 간단한 추론 테스트를 추가합니다.

---

# 결론

`tools/` 폴더의 Python 스크립트들은 ZCU104 플랫폼에서 실행될 양자화된 MobileNet 스타일 모델을 학습하고 내보내는 완전한 파이프라인을 제공합니다.

*   **`synth_digits_train.py`**: 합성 숫자 데이터셋을 생성하고, 펌웨어와 정확히 일치하는 Depthwise Separable CNN 모델을 학습시킵니다. 데이터 증강과 양자화 시뮬레이션을 통해 실제 하드웨어에서의 성능을 예측할 수 있도록 합니다.

*   **`export_bins.py`**: 학습된 모델의 가중치를 양자화하고, 펌웨어의 메모리 레이아웃에 맞게 바이너리 파일로 내보냅니다. SHA-256 해시를 통한 무결성 검증도 포함되어 있습니다.

이 스크립트들은 `vitis_src/` 폴더의 C/C++ 펌웨어 코드와 긴밀하게 연동되도록 설계되어 있으며, 학습-양자화-배포의 전체 워크플로우를 효과적으로 지원합니다.
