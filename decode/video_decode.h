//
// Created by wangzk on 2025/2/16.
//

#ifndef FF_VIDEO_DECODE_H
#define FF_VIDEO_DECODE_H
#include <cstddef>
#include <string>

namespace kk {
    template<typename T>
    class Frame {
    public:
        explicit Frame(T *data_ptr)
            : data_ptr_(data_ptr) {
        }

        virtual ~Frame() = default;
        virtual T* data_ptr() = 0;

        T* data_ptr_;
    };

    template<typename T>
    class VDecode {
    public:
        VDecode() = default;

        virtual ~VDecode() = default;

        virtual int start(const std::string& input);
        virtual Frame<T>* grab() = 0;
        virtual int stop();
    };

}



#endif //FF_VIDEO_DECODE_H
