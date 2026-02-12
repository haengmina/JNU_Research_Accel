import numpy as np
from PIL import Image
import os

# 이미지가 저장될 경로 (SD 카드에 바로 저장하거나, 복사하세요)
output_path = "D:/Lab/Project/FPGA accel/workspace/hm_workspace/assets"
os.makedirs(output_path, exist_ok=True)

for i in range(1, 11):
    # MobileNet 표준 입력 크기: 224x224, 3채널(RGB)
    img_array = np.random.randint(0, 256, (224, 224, 3), dtype=np.uint8)
    img = Image.fromarray(img_array, 'RGB')
    filename = f"img_{i:02d}.bmp"
    img.save(os.path.join(output_path, filename))
    print(f"Generated: {filename}")