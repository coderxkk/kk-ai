//
// Created by wangzk on 2025/2/16.
//

#ifndef DETECT_H
#define DETECT_H

#include <iostream>
#include "video_decode.h"

namespace kk {
    class ObjectDetect {
    public:
        virtual void detect(Frame* frame);
    private:

        virtual void preprocess(Frame* frame);
        virtual void infer();
        virtual void postprocess();
    };
}





#endif //DETECT_H
