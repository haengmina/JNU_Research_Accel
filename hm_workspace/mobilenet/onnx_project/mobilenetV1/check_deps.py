import tensorflow as tf
import tf2onnx
import onnxruntime
import logging

# Suppress TensorFlow logging that might be verbose
tf.get_logger().setLevel(logging.ERROR)

print(f"TensorFlow version: {tf.__version__}")
print(f"tf2onnx version: {tf2onnx.__version__}")
print(f"ONNX Runtime version: {onnxruntime.__version__}")


try:
    # Attempt to use a basic TensorFlow operation to ensure it's functional
    _ = tf.constant(1)
    # Attempt to convert a simple graph with tf2onnx
    # (This is just a basic import check, not a full conversion test)
    # For a full test, we'd need a more complex graph.
    print("Successfully imported TensorFlow and tf2onnx.")

    # Check if ONNX Runtime can be initialized
    _ = onnxruntime.InferenceSession("null.onnx", providers=["CPUExecutionProvider"])
    print("Successfully initialized ONNX Runtime (basic check).")

except Exception as e:
    print(f"An error occurred: {e}")
    print("There might still be a dependency conflict or issue.")

print("Dependency check complete.")
