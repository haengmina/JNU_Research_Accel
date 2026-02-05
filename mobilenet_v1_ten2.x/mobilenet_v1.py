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
# 일반 Conv, DepthSepConv 모듈 정의
# MobileNet_V1 모듈 정의
# 
# =============================================================================


import tensorflow as tf

def _conv_block(inputs, filters, alpha, kernel=(3,3), strides=(1,1), weight_decay=0.00004):
    """표준 컨볼루션 레이어 블록(Conv2D -> BN -> ReLU6) """
    # 1. 채널(필터) 수 조정 : 너비 배율(alpha) 적용해 최종 필터 수 결정
    # alpha=1.0이면, 원본 필터 수 그대로 사용, alpha=0.5이면 절반으로 줄임
    filters = int(filters * alpha)

    # 2. 컨볼루션 레이어
    #    - filters : 위에서 계산된 필터 수
    #    - kernel : 커널 크기 (기본값 (3,3)
    #    - padding='same' : 입력과 동일한 공간 크기의 출력 생성. 필요한 패딩 자동 추가.
    #    - use_bias=False : BatchNormalization을 사용하면 Conv2D에서 bias는 불필요해서 비활성화.
    #    - strides : 컨볼루션 보폭. (2,2)면 출력 크기가 절반으로 줄어듦.
    #    - kernel_regularizer : L2 정규화 적용해 가중치가 너무 커지는 것을 방지하고 과적합을 줄임.
    #    - name : 레이어 이름 지정.
    x = tf.keras.layers.Conv2D(
        filters,
        kernel,
        padding='same',
        use_bias=False,
        strides=strides,
        kernel_regularizer=tf.keras.regularizers.l2(weight_decay),
        name='conv1'
    )(inputs)       # (inputs)는 Keras 함수형 API의 연결 방식

    # 3. 배치 정규화 레이어
    #    - 각 배치마다 출력의 평균을 0, 분산을 1로 정규화해 학습 안정성과 속도 향상.
    x = tf.keras.layers.BatchNormalization(name='conv1_bn')(x)

    # 4. ReLU6 활성화 함수
    #    - ReLU(x) = max(0, x)와 비슷하지만 출력값을 최대 6으로 제한. (max(0, min(x, 6))).
    #    - 낮은 연산 정밀도 환경(e.g., 모바일 장치)에서 더 나은 성능, 안정성 제공.
    return tf.keras.layers.ReLU(6.0, name='conv1_relu6')(x)

def _depthwise_conv_block(inputs, pointwise_conv_filters, alpha, strides=(1,1), 
                          block_id=1, weight_decay=0.00004):
    """Depthwise Separable Convolution 블록 (Depthwise Conv -> BN -> ReLU6 -> Pointwise Conv -> BN -> ReLU6) """
    
    # 1. Pointwise Convolution 필터 수 계산 : 너비 배율(alpha) 적용
    pointwise_conv_filters = int(pointwise_conv_filters * alpha)

    # 2. Depthwise Convolution
    #   - 각 입력 채널에 대해 독립적으로 하나의 필터를 적용해 공간적 특징 추출.
    #   - (3,3) : 커널 크기
    #   - padding='same' : 입력과 동일한 공간 크기의 출력 생성.
    #   - strides : 컨볼루션 보폭. (2,2)면 출력 크기가 절반으로 줄어듦.
    #   - use_bias=False : BatchNormalization 사용 시 bias는 불필요.
    #   - depthwise_regularizer : Depthwise kernel에 L2 정규화 적용.
    x = tf.keras.layers.DepthwiseConv2D(
        (3,3),
        padding='same',
        strides=strides,
        use_bias=False,
        depthwise_regularizer=tf.keras.regularizers.l2(weight_decay),
        name=f'conv_dw_{block_id}'          # Depthwise임을 나타내는 이름
    )(inputs)

    # 3. BatchNormalization & ReLU6 : Depthwise Conv 결과에 정규화와 활성화 함수 적용
    x = tf.keras.layers.BatchNormalization(name=f'conv_dw_{block_id}_bn')(x)
    x = tf.keras.layers.ReLU(6.0, name=f'conv_dw_{block_id}_relu6')(x)

    # 4. Pointwise Convolution
    #   - (1,1) 커널 사용해 Depthwise 출력의 채널들을 선형 결합.
    #   - pointwise_conv_filters : 출력 채널 수 제어.
    #   - strides=(1,1) : 공간적 크기 유지.
    #   - kernel_regularizer : L2 정규화 적용.
    x = tf.keras.layers.Conv2D(
        pointwise_conv_filters,
        (1,1),
        padding='same',
        use_bias=False,
        strides=(1,1),
        kernel_regularizer=tf.keras.regularizers.l2(weight_decay),
        name=f'conv_pw_{block_id}'
    )(x)                                    # Depthwise 결과가 Pointwise의 입력.

    # 5. BatchNormalization & ReLU6 : Pointwise Conv 결과에 정규화와 활성화 함수 적용
    x = tf.keras.layers.BatchNormalization(name=f'conv_pw_{block_id}_bn')(x)
    return tf.keras.layers.ReLU(6.0, name=f'conv_pw_{block_id}_relu6')(x)

def MobileNetV1(
        inputs_shape=(224, 224, 3),
        num_classes=1000,
        depth_multiplier=1.0,
        dropout_rate=0.001,
        weight_decay=0.00004,
        include_top=True,
        input_tensor=None
):
    """Keras API를 사용해 MobileNet V1 모델 인스턴스화 """
    # ... (파라미터 검증) ...

    # 1. 모델의 입력 레이어 정의
    #   - 모든 Keras 모델은 Input 레이어로 시작
    #   - input_shape : 모델이 기대하는 입력 데이터의 형태 (배치 크기 제외).
    if input_tensor is None:
        img_inputs = tf.keras.layers.Input(shape=inputs_shape)
    else:
        # 다른 모델의 출력을 입력으로 사용하는 등, 외부 텐서를 입력으로 쓸 수 있게 함.
        if not tf.keras.backend.is_keras_tensor(input_tensor):
            img_inputs = tf.keras.layers.Input(tensor=input_tensor, shape=inputs_shape)
        else:
            img_inputs = input_tensor
    # 모델의 "입구". 데이터가 들어오는 시작점 역할.
    
    # 2. MobileNet V1 아키텍처
    # 한 블록의 출력(x)가 다음 블록의 입력으로 들어감.
    x = _conv_block(img_inputs, 32, depth_multiplier, strides=(2,2), weight_decay=weight_decay)
    x = _depthwise_conv_block(x, 64, depth_multiplier, block_id=1, weight_decay=weight_decay)
    x = _depthwise_conv_block(x, 128, depth_multiplier, strides=(2,2), block_id=2, weight_decay=weight_decay)
    x = _depthwise_conv_block(x, 128, depth_multiplier, block_id=3, weight_decay=weight_decay)
    x = _depthwise_conv_block(x, 256, depth_multiplier, strides=(2,2), block_id=4, weight_decay=weight_decay)
    x = _depthwise_conv_block(x, 256, depth_multiplier, block_id=5, weight_decay=weight_decay)
    x = _depthwise_conv_block(x, 512, depth_multiplier, strides=(2,2), block_id=6, weight_decay=weight_decay)
    x = _depthwise_conv_block(x, 512, depth_multiplier, block_id=7, weight_decay=weight_decay)
    x = _depthwise_conv_block(x, 512, depth_multiplier, block_id=8, weight_decay=weight_decay)
    x = _depthwise_conv_block(x, 512, depth_multiplier, block_id=9, weight_decay=weight_decay)
    x = _depthwise_conv_block(x, 512, depth_multiplier, block_id=10, weight_decay=weight_decay)
    x = _depthwise_conv_block(x, 512, depth_multiplier, block_id=11, weight_decay=weight_decay)    
    x = _depthwise_conv_block(x, 1024, depth_multiplier, strides=(2,2), block_id=12, weight_decay=weight_decay)
    x = _depthwise_conv_block(x, 1024, depth_multiplier, block_id=13, weight_decay=weight_decay)
    
    # 3. 분류기 Head 추가
    # include_top=True일 경우, 분류를 위한 (head) 부분을 추가.
    if include_top:
        # 1. GlobalAveragePooling2D:
        #   - feature map의 공간 차원(H, W)를 평균내어 채널(C)만 남김.
        #   - e.g., [B, 7, 7, 1024] -> [B, 1024] 형태의 벡터로 변환.
        x = tf.keras.layers.GlobalAveragePooling2D(name='global_avg_pool')(x)
        
        # 2. Dropout : 과적합 방지를 위해 일부 뉴런을 비활성화.
        x = tf.keras.layers.Dropout(dropout_rate, name='dropout')(x)
        
        # 3. Dense (완전 연결 계층) :
        #   - 최종 분류를 위해 Dense 레이어 사용. num_classes 만큼의 노드로 연결.
        #   - activation='softmax' : 각 클래스에 대한 확률 출력.
        x = tf.keras.layers.Dense(num_classes, activation='softmax', name='predictions')(x)
    # 특징 벡터(x)를 받아, GlobalAveragePooling2D로 1차원 벡터로 만들고, Dropout으로 정규화를 거친 뒤, 
    # Dense 레이어를 통해 최종적으로 num_classes 개수만큼의 클래스별 확률 출력.


    # 4. 모델 인스턴스화
    # 모델 생성 : 정의된 입력(img_input)과 최종 출력(x)를 연결해 Keras 모델을 만듦.
    model = tf.keras.Model(img_inputs, x, name=f'mobilenet_v1_{depth_multiplier}')
    
    return model
    # img_input으로 시작해서, 여러 레이어를 거쳐 최종적으로 x를 출력하는 모델이라는 것을 Keras에 알려줌.


# 너비 배율(depth multiplier)에 따른 모델 함수 정의
# functools.partial 대신 간단한 람다(lambda) 함수 사용해 가독성 높임
mobilenet_v1_075 = lambda **kwargs: MobileNetV1(depth_multiplier=0.75, **kwargs)
mobilenet_v1_050 = lambda **kwargs: MobileNetV1(depth_multiplier=0.50, **kwargs)
mobilenet_v1_025 = lambda **kwargs: MobileNetV1(depth_multiplier=0.25, **kwargs)