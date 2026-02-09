# Copyright 2017 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# python version: 3.13.11
# TensorFlow version: 2.20.0
# TensorFlow 1.x -> 2.x
# 
# tf-slim 라이브러리의 batch_norm 함수가 최신 텐서플로우에서 더 이상 지원하지 않는
# 오래된 방식을 사용 했었음.
#
# 기존의 파일을 새로운 환경 (python 3.13.11, tensorflow 2.20.0)에 알맞게 재구성.
# support : gemini-cli
#
# =============================================================================

import argparse
import os
import tensorflow as tf

from mobilenet_v1 import MobileNetV1

# 데이터셋 파이프라인
def create_dataset(dataset_dir, batch_size, image_size, is_traning):
    """
    tf.data를 사용해 데이터셋 입력 파이프라인 생성
    
    참고 : 이 함수는 실제 ImageNet TFRecord를 파싱하는 대신,
    전체 구조를 보여주기 위한 'dummy' 데이터셋 생성.
    실제 데이터셋에 맞게 이 부분을 수정.
    """
    # 더미 데이터 생성 (실제로는 이부분을 파일 경로로 대체)
    # file_paths = tf.io.gfile.glob(os.path.join(dataset_dir, '*.tfrecord'))
    dummy_images = tf.random.uniform(shape=[1000, image_size, image_size, 3], minval=0, maxval=255)
    dummy_labels = tf.random.uniform(shape=[1000], minval=0, maxval=999, dtype=tf.in32)

    dataset = tf.data.Dataset.from_tensor_slices((dummy_images, dummy_labels))

    def preprocess_image(imgae, label):
        # 이미지 전처리 함수
        # Keras의 내장 전처리 레이어를 사용하거나 직접 구현 가능
        # 여기서는 간단한 정규화만 수행
        image = tf.cast(image, tf.float32) / 255.0

        # 실제 MobileNet 전처리는 입력 범위를 [-1, 1]로 조정
        # image = (image - 0.5) * 2.0
        return image, label
    
    # 데이터셋에 전처리 함수 적용
    # num_parallel_calls는 여러 CPU 코어를 활용해 전처리 속도를 높임.
    dataset = dataset.map(preprocess_image, num_parallel_calls=tf.data.AUTOTUNE)

    if is_traning:
        # 학습 데이터셋의 경우, 데이터를 섞고 반복
        dataset = dataset.shuffle(buffer_size=1000)
        dataset = dataset.repeat()

    # 데이터를 batch 단위로 묶음.
    dataset = dataset.batch(batch_size)

    # prfetch는 모델이 현재 배치를 학습하는 동안 CPU가 다음 배치를 미리 준비하도록 하여
    # GPU가 쉬는 시간을 최소화하고 성능을 최적화.
    dataset = dataset.prefetch(buffer_size=tf.data.AUTOTUNE)

    return dataset

def main(args):

    # 메인 학습 함수
    # 1. 데이터셋 생성
    print("데이터셋 생성 시작")
    train_dataset = create_dataset(args.dataset_dir, args.batch_size, args.image_size, is_traning=True)
    # val_dataset = create_dataset(args.dataset_dir, args.batch_size, args.image_size, is_traning=False)
    # 필요 시 검증 데이터셋도 생성

    # 2. 모델 생성
    print("MobileNetV1 모델 생성")
    model = MobileNetV1(
        input_shape=(args.imge_size, args.image_size, 3),
        num_classes=args.num_classes,
        depth_multiplier=args.depth_multiplier,
        weight_decay=args.weight_decay
    )

    # model.summary()로 모델 구조 쉽게 확인
    model.summary()

    # 3. 학습 파라미터 설정
    initial_laerning_rate = 0.045
    lr_schedule = tf.keras.optimizers.schedules.ExponentialDecay(
        initial_laerning_rate,
        decay_steps=args.decay_steps,
        decay_rate=0.94,
        staircase=True
    )

    optimizer = tf.keras.optimizers.RMSprop(learning_rate=lr_schedule, momentum=0.9)

    # 4. 모델 컴파일
    print("모델 컴파일")
    model.compile(
        optimizer=optimizer,
        loss = 'sparse_categorical_crossentropy',   # 레이블이 one-hot 인코딩이 아닌 정수 형태일 때 사용
        metrics=['accuracy']
    )

    # 5. Keras 콜백 설정
    callbacks = [
        # TensorBoard 로그 저장할 때 Callback
        tf.keras.callbacks.TensorBoard(
            log_dir=args.checkpoint_dir,
            histogram_freq=1                        # 1 epoch마다 히스토그램 기록
        ),
        # 가장 좋은 성능의 모델 체크포인트를 저장할 Callback
        # 'val_accuracy'를 모니터링해 가장 높을 때만 모델 저장
        tf.keras.callbacks.ModelCheckpoint(
            filepath=os.path.join(args.checkpoint_dir, 'best_model.keras'),
            monitor='accuracy',     # 또는 'val_accuracy'
            save_best_only=True
        )
    ]

    # (option) 미세 조정을 위해 체크포인트 로드
    if args.fine_tune_checkpoint:
        print(f"체크포인트를 로드합니다 : {args.fine_tune_checkpoint}")
        model.load_weights(args.fine_tune_checkpoint)

    # 6. 모델 학습
    print("모델 학습 시작")
    model.fit(
        train_dataset,
        epochs=args.epochs,
        steps_per_epoch=args.steps_per_epoch,
        callbacks=callbacks
        # validation_data=val_dataset, # 검증 데이터셋이 있을 경우
        # validation_step=...
    )

    print("모델 학습 완료")

if __name__ == '__main__':
    # 커멘드라인 인자 파싱
    parser = argparse.ArgumentParser(description='MobileNetV1 Training Script')
    parser.add_argument('--dataset_dir', type=str, required=True, help='데이터셋이 위치한 디렉토리')
    parser.add_argument('--checkpoint_dir', type=str, default='./logs', help='체크포인트와 로그를 저장 디렉토리')
    parser.add_argument('--fine_tune_checkpoint', type=str, default=None, help='미세 조정을 시작할 체크포인트 경로')
    parser.add_argument('--batch_size', type=int, default=64, help='배치 크기')
    parser.add_argument('--image_size', type=int, default=224, help='입력 이미지 크기')
    parser.add_argument('--epochs', type=int, default=100, help='총 학습 에포크 수')
    parser.add_argument('--steps_per_epoch', type=int, default=1000, help='한 에포크당 스텝 수')
    parser.add_argument('--depth_multiplier', type=float, default=1.0, help='MobileNet 너비 배율')
    parser.add_argument('--weight_decay', type=float, default=0.00004, help='가중치 감쇠(L2 정규화) 값')
    parser.add_argument('--decay_steps', type=int, default=10000, help='학습률 감소 스템 수')
    
    
    args = parser.parse_args()

    # 체크포이트 디렉토리가 없으면 생성
    if not os.path.exists(args.checkpoint_dir):
        os.makedirs(args.chekcpoint_dir)

    main(args)

