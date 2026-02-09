import tensorflow.compat.v1 as tf
tf.disable_eager_execution()
import tf_slim as slim
import tf2onnx
import os
import tarfile

# mobilenet_v1.py 파일이 있는 경로를 sys.path에 추가하여 import 가능하도록 함
# 이 스크립트와 mobilenet_v1.py가 같은 디렉토리에 있다고 가정
# 만약 다른 경로에 있다면 이 부분을 수정해야 합니다.
# from mobilenet_v1 import mobilenet_v1, mobilenet_v1_arg_scope
# 현재 작업 디렉토리가 /home/mini/jnu/lab/ml/mobilenetV1 이고,
# mobilenet_v1.py는 project_gemini/python_model/mobilenet_v1.py 에 있으므로,
# 해당 경로를 sys.path에 추가해줍니다.
import sys
sys.path.append(os.path.join(os.getcwd(), 'project_gemini', 'python_model'))

from mobilenet_v1 import mobilenet_v1, mobilenet_v1_arg_scope

# 1. MobileNetV1 모델 그래프 정의
# 입력 플레이스홀더를 정의합니다. MobileNetV1의 기본 입력 크기는 224x224입니다.
inputs = tf.placeholder(tf.float32, [None, 224, 224, 3], name="input_tensor")

# arg_scope를 사용해 MobileNetV1 모델을 생성합니다.
# is_training=False로 설정하여 추론 모드로 그래프를 구축합니다.
with slim.arg_scope(mobilenet_v1_arg_scope(is_training=False)):
    logits, end_points = mobilenet_v1(inputs, num_classes=1001, is_training=False)
    # num_classes는 MobileNetV1이 학습된 ImageNet 클래스 수 (1000개 + 배경 1개 = 1001)

# 2. 학습된 가중치(체크포인트) 로드 준비
# 저장된 체크포인트 파일 경로.
# 예시: 'project_gemini/python_model/PreTrainedModels/mobilenet_v1_1.0_224_quant.tgz'
# 해당 파일은 tgz 압축 파일이므로, 압축을 해제하고 .ckpt 파일을 찾아야 합니다.

# 압축 해제할 디렉토리 설정
pretrained_models_dir = 'project_gemini/python_model/PreTrainedModels'
extracted_model_path = os.path.join(pretrained_models_dir, 'mobilenet_v1_1.0_224.ckpt')
tar_file_path = os.path.join(pretrained_models_dir, 'mobilenet_v1_1.0_224.tgz')

# .tgz 파일 압축 해제
if not os.path.exists(extracted_model_path + '.data-00000-of-00001') and \
   not os.path.exists(extracted_model_path + '.index') and \
   os.path.exists(tar_file_path):
    print(f"Extracting {tar_file_path}...")
    with tarfile.open(tar_file_path, "r:gz") as tar:
        tar.extractall(path=pretrained_models_dir)
    print("Extraction complete.")
else:
    print(f"Model {extracted_model_path} already extracted or .tgz not found.")

# 체크포인트 파일 경로 설정 (압축 해제된 .ckpt 파일)
checkpoint_path = extracted_model_path

# 가중치를 복원하기 위한 Saver 객체
# 'MobilenetV1' 스코프에 있는 변수만 저장된 체크포인트라고 가정
variables_to_restore = slim.get_variables_to_restore(include=['MobilenetV1'])
saver = tf.train.Saver(variables_to_restore)

# 3. ONNX로 변환
onnx_output_path = "mobilenet_v1.onnx"

with tf.Session() as sess:
    # 모든 변수 초기화
    sess.run(tf.global_variables_initializer())
    
    # 체크포인트로부터 가중치 로드
    print(f"Restoring weights from {checkpoint_path}...")
    saver.restore(sess, checkpoint_path)
    print("Weights restored.")
    
    # 변환할 출력 노드의 이름을 지정합니다.
    # 'Predictions'는 `mobilenet_v1` 함수의 `end_points`에 추가된 최종 출력입니다.
    output_node_names = [end_points['Predictions'].op.name]
    
    # tf2onnx를 사용하여 그래프를 ONNX 모델로 변환합니다.
    print(f"Converting TensorFlow model to ONNX format...")
    onnx_graph = tf2onnx.tfonnx.process_tf_graph(
        sess.graph,
        input_names=[inputs.op.name + ':0'], # 플레이스홀더의 이름을 정확히 지정
        output_names=[name + ':0' for name in output_node_names], # 출력 노드의 이름을 정확히 지정
        opset=13 # ONNX Opset 버전을 지정합니다. 호환성을 위해 보통 최신 버전을 사용합니다.
    )
    
    model_proto = onnx_graph.make_model("mobilenet_v1_classification_model")
    
    # 4. ONNX 파일로 저장
    with open(onnx_output_path, "wb") as f:
        f.write(model_proto.SerializeToString())
        
print(f"ONNX 모델이 '{onnx_output_path}' 파일로 성공적으로 저장되었습니다.")
print("이제 이 ONNX 모델을 C++ 환경에서 ONNX Runtime으로 로드하여 사용할 수 있습니다.")
