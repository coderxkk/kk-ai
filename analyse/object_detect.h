//
// Created by wangzk on 2025/2/16.
//

#ifndef DETECT_H
#define DETECT_H

#include <iostream>
#include "video_decode.h"

namespace kk {
    template<typename T>
    class ObjectDetect<T> {
        virtual ~ObjectDetect() = default;

        ObjectDetect() = default;

    public:
        virtual void detect(Frame<T>* frame);
    private:
        virtual void preprocess(Frame<T>* frame);
        virtual void infer();
        virtual void postprocess();
    };
}





#endif //DETECT_H
