//
// Created by wangzk on 2025/2/16.
//

#ifndef DECODE_H
#define DECODE_H

#include <string>
#include <thread>
#include <queue>

#include "video_decode.h"
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

#include <libswscale/swscale.h>
#include "bm_cv.h"

#include "bmcv_api_ext.h"
namespace CV186AH {

    class BMFrame : public kk::Frame<bm_image> {


    public:
        bm_image * data_ptr() override {
            return data_ptr_;
        };

        BMFrame(bm_image *data_ptr): Frame<bm_image>(data_ptr) {}

        virtual ~BMFrame() {
            bm_image_destroy(data_ptr_);
        };


    };

    constexpr int EXTRA_FRAME_BUFFER_NUM = 5;

    class Decode : public kk::VDecode<bm_image> {
    public:
        Decode();

        ~Decode() override;

        int start(const std::string& input) override;

        kk::Frame<bm_image>* grab() override;

        int stop() override;
    private:

        int init_ffm_stream(const std::string& input);
        int init_ffm_format(const std::string& input);
        int init_ffm_codec();

        void background_decode();
        AVFrame* grab_ffm_frame();
        AVFrame* flush_ffm_frame();

        int ffm_decode_frame(bool& got_frame);

        AVFrame *frame_;
        AVPacket* av_pkt_;
        AVFormatContext *av_fmt_ctx_;
        AVCodec *av_codec_;
        AVCodecContext *av_codec_ctx_;
        AVCodecParameters *av_codec_para_;
        AVStream* av_video_stream_;

        std::thread decode_thread_;


        bool is_hardaccel_ = true;


        bool quit_flag_ = false;

        bool is_stream_;
        int width_;
        int height_;
        int coded_width_;
        int coded_height_;
        int pix_fmt_;
        int video_stream_idx_;

        int refcount_;
        // "101" for compressed and "0" for linear
        int output_format_ = 101;


        bm_handle_t bm_handle_;
        std::queue<bm_image*> frame_buffer_;
        int capacity_ = 5;

        std::mutex mutex_;



    };
}




#endif //DECODE_H
