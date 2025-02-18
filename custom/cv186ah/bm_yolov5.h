//
// Created by wangzk on 2025/2/18.
//

#ifndef BM_YOLOV5_H
#define BM_YOLOV5_H
#include <object_detect.h>
#include "bmcv_api_ext.h"


namespace kk {
    class BMYolov5 : public ObjectDetect<bm_image>{


    public:
        BMYolov5(const std::string& model_path);
        void detect(Frame<bm_image> *frame) override;

    private:
        void preprocess(Frame<bm_image> *frame) override;

        void infer() override;

        void postprocess() override;

        ~BMYolov5() override;
    };
}




#endif //BM_YOLOV5_H
