//
// Created by wangzk on 2025/2/18.
//

#include "bm_yolov5.h"
namespace kk {
    BMYolov5::BMYolov5(const std::string &model_path) : ObjectDetect<bm_image>() {

    }

    void kk::BMYolov5::detect(Frame<bm_image> *frame) {
        ObjectDetect<bm_image>::detect(frame);
    }

    void kk::BMYolov5::preprocess(Frame<bm_image> *frame) {
    }

    void kk::BMYolov5::infer() {
    }

    void kk::BMYolov5::postprocess() {
    }

    kk::BMYolov5::~BMYolov5() {
    }

}

