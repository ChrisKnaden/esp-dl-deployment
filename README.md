# Fork Changes
This deployment is tested for ESP32S3.

## Setup
1. You can go through the `model_deployment.ipynb` to generate an `.onnx` and `.espdl` file. They are automatically placed at the right folders. To use it, make sure that the model, in this case `yolo11n.pt`, is in the folder `models/coco_detect/models/`.
   1. For now, the pipeline is optimized for deployment of a `yolo11n.pt` model
   2. If you want to use a different model, you need to change the conversion to the `.onnx`as in `torch.onnx.export(...)`
2. Copy an image of size 640x640 named bikes.jpg into `examples/yolo11_detect/main/`. You can use `model_deployment.ipynb` to compress the image.
3. Ensure ESP-DL Environment is correctly setup and you are in your virtual environment to deploy the program to your ESP.
4. Move into `examples/yolo11_detect/` to build the program. Use the following command to build the program:
 

    idf.py fullclean build flash monitor


## Tips
If you just quickly want to test a `.espdl` model, put it in the following path. Just overwrite the existing model but keep the same name. Make sure to store the old model, if you need it later.

    models/coco_detect/models/s3/coco_detect_yolo11n_s8_v1.espdl

Unless you just want to override an existing model with your new model and use the same name as the old model, you can also add models for "long term use". To add a new model inside the coco_detect you need to change the following files. Take a look at `CUSTOM_YOLO11N` for reference.
- `models/coco_detect/CMakeLists.txt`
- `models/coco_detect/Kconfig`
- `models/coco_detect/coco_detect.cpp`
- `models/coco_detect/coco_detect.hpp`
- `models/coco_detect/models/s3/<YOUR-MODEL>.espdl`

The threshold of the model inside coco_detect can be changed in this file `models/coco_detect/coco_detect.cpp`. There the first parameter after `m_model` represents the threshold value, in this case 10%:

    new dl::detect::yolo11PostProcessor(m_model, 0.10, 0.7, 10, {{8, 8, 4, 4}, {16, 16, 8, 8}, {32, 32, 16, 16}});

## Bugs before build
- if the build is crashing, it might be due to a too big image. Reduce the size with the help of the `model_deployment.ipynb` or some other software. The yolo11n is trained on images of size 640x640. Smaller resolutions also work.
- make sure you are in your esp-idf virtual environment with Python 3.10
- sometimes you have to set the IDF_TARGET again:


    unset IDF_TARGET
    idf.py set-target esp32s3



---
# ESP-DL [[中文]](./README_cn.md)

[![Documentation Status](./docs/_static/doc_latest.svg)](https://docs.espressif.com/projects/esp-dl/en/latest/index.html)  [![Component Registry](https://components.espressif.com/components/espressif/esp-dl/badge.svg)](https://components.espressif.com/components/espressif/esp-dl)

ESP-DL is a lightweight and efficient neural network inference framework designed specifically for ESP series chips. With ESP-DL, you can easily and quickly develop AI applications using Espressif's System on Chips (SoCs).

## Overview

ESP-DL offers APIs to load, debug, and run AI models. The framework is easy to use and can be seamlessly integrated with other Espressif SDKs. ESP-PPQ serves as the quantization tool for ESP-DL, capable of quantizing models from ONNX, Pytorch, and TensorFlow, and exporting them into the ESP-DL standard model format.

- **ESP-DL Standard Model Format**: This format is similar to ONNX but uses FlatBuffers instead of Protobuf, making it more lightweight and supporting zero-copy deserialization, with a file extension of `.espdl`.
- **Efficient Operator Implementation**: ESP-DL efficiently implements common AI operators such as Conv, Gemm, Add, and Mul. [**The list of supported operators**](./operator_support_state.md).
- **Static Memory Planner**: The memory planner automatically allocates different layers to the optimal memory location based on the user-specified internal RAM size, ensuring efficient overall running speed while minimizing memory usage.
- **Dual Core Scheduling**: Automatic dual-core scheduling allows computationally intensive operators to fully utilize the dual-core computing power. Currently, Conv2D and DepthwiseConv2D support dual-core scheduling.
- **8bit LUT Activation**: All activation functions except for ReLU and PReLU are implemented using an 8-bit LUT (Look Up Table) method in ESP-DL to accelerate inference. You can use any activation function, and their computational complexity remains the same.

## News
- [2025/04/30] We released a new [esp-detection](https://github.com/espressif/esp-detection)​​ project and the `​​ESPDet-Pico`​​ model, which can easily train and deploy object detection models. [espdet_pico_224_224_cat​​ espdl model](./models/cat_detect/) and [example](./examples/cat_detect/) is a cat detection model trained by ​esp-detection​​. Feel free to try it and share your feedback!   
- [2025/02/18] We supported yolo11n [espdl model](https://github.com/espressif/esp-dl/tree/master/models/coco_detect) and [example](https://github.com/espressif/esp-dl/tree/master/examples/yolo11_detect).
- [2025/01/09] We updated the schema of espdl model and released ESP-DL v3.1.0. Note: previous models can be load by new schema, but new model is not compatible with previous version. 
- [2024/12/20] We released ESP-DL v3.0.0.

## Getting Started

### Software Requirements

- **ESP-IDF**  

ESP-DL runs based on ESP-IDF. For detailed instructions on how to get ESP-IDF, please see [ESP-IDF Programming Guide](https://idf.espressif.com).

> Please use [ESP-IDF](https://github.com/espressif/esp-idf) `release/v5.3` or above.


- **ESP-PPQ**

ESP-PPQ is a quantization tool based on ppq. If you want to quantize your own model, please install esp-ppq using the following command:
```
pip uninstall ppq
pip install git+https://github.com/espressif/esp-ppq.git
```
Use esp-ppq with docker:
```
docker build -t esp-ppq:your_tag https://github.com/espressif/esp-ppq.git
```

### Model Quantization

First, please refer to the [ESP-DL Operator Support State](./operator_support_state.md) to ensure that the operators in your model are already supported.  

ESP-PPQ can directly read ONNX models for quantization. Pytorch and TensorFlow need to be converted to ONNX models first, so make sure your model can be converted to ONNX models. For more details about quantization, please refer to

[how to quantize model](https://docs.espressif.com/projects/esp-dl/en/latest/tutorials/how_to_quantize_model.html)  
[how to quantize MobileNetV2](https://docs.espressif.com/projects/esp-dl/en/latest/tutorials/how_to_deploy_mobilenetv2.html#model-quantization)  
[how to quantize YOLO11n](https://docs.espressif.com/projects/esp-dl/en/latest/tutorials/how_to_deploy_yolo11n.html#model-quantization)  


### Model Deployment
ESP-DL provides a series of API to quickly load and run models.  A typical example is as follows:

[how to load test profile model](https://docs.espressif.com/projects/esp-dl/en/latest/tutorials/how_to_load_test_profile_model.html)  
[how to run model](https://docs.espressif.com/projects/esp-dl/en/latest/tutorials/how_to_run_model.html)  


## Support Models

[Pedestrian Detection](./models/pedestrian_detect/)     
[Human Face Detection](./models/human_face_detect/)     
[Human Face Recognition](./models/human_face_recognition/)     
[Imagenet Classification (MobileNetV2)](./models/imagenet_cls/)    
[COCO Detection (YOLO11n)](./models/coco_detect/)    
[CAT Detection (​​ESPDet-Pico)](./models/cat_detect/) 

## Support Operators

If you encounter unsupported operators, please point them out in the [issues](https://github.com/espressif/esp-dl/issues), and we will support them as soon as possible. Contributions to this ESP-DL are also welcomed, please refer to [Creating a New Module (Operator)](https://docs.espressif.com/projects/esp-dl/en/latest/tutorials/how_to_add_a_new_module%28operator%29.html) for more details.

[ESP-DL Operator Support State](./operator_support_state.md)
