import importlib.metadata
import sys
import torch
import executorch.backends.arm.quantizer.quantization_annotator
import tosa_serializer

assert sys.version_info[:2] == (3, 12), 'Use uv venv --python 3.12'
assert importlib.metadata.version('executorch') == '1.4.1'
assert torch.__version__.split('+')[0] == '2.13.0'
assert importlib.metadata.version('flatbuffers') == '24.3.25'
print('Python 3.12 / ExecuTorch 1.4.1 / torch 2.13.0 / TOSA imports OK')
