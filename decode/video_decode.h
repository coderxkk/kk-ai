//
// Created by wangzk on 2025/2/16.
//

#ifndef FF_VIDEO_DECODE_H
#define FF_VIDEO_DECODE_H
#include <cstddef>
#include <string>

namespace kk {
    class Frame {

    };

    class VDecode {
    public:
        VDecode() = default;

        virtual ~VDecode() = default;

        virtual int start(const std::string& input);
        virtual Frame* grab();
        virtual int stop();
    };

}



#endif //FF_VIDEO_DECODE_H
