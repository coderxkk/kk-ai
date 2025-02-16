//
// Created by wangzk on 2025/2/16.
//

#include <time.h>
#include "decode.h"
#include "Util/logger.h"




namespace CV186AH {
    Decode::Decode() {

        is_stream_ = false;
        width_ = 0;
        height_ = 0;
        pix_fmt_ = 0;

        video_stream_idx_ = -1;
        refcount_ = 1;

        av_pkt_ = av_packet_alloc();
        av_pkt_->data = NULL;
        av_pkt_->size = 0;

        bm_dev_request(&bm_handle_, 0);

        frame_ = av_frame_alloc();
    }

    Decode::~Decode() {
        quit_flag_ = true;
        decode_thread_.join();
    }


    void Decode::background_decode() {
        // todo(wzk) 帧队列维护
        // while (1) {
        //     while (frame_buffer_.size() == QUEUE_MAX_SIZE) {
        //         if (is_rtsp) {
        //             std::lock_guard<std::mutex> my_lock_guard(lock);
        //             bm_image* img = queue.front();
        //             bm_image_destroy(*img);
        //             delete img;
        //             img = nullptr;
        //             queue.pop();
        //         } else {
        //             usleep(2000);
        //         }
        //     }
        //
        //     bm_image* img = new bm_image;
        //     AVFrame* avframe = grabFrame();
        //     if (quit_flag)
        //     {
        //         delete img;
        //         img = nullptr;
        //         break;
        //     }
        //     coded_width  = video_dec_ctx->coded_width;
        //     coded_height = video_dec_ctx->coded_height;
        //     avframe_to_bm_image(*(this->handle), avframe, img, false, this->data_on_device_mem,
        //                         coded_width, coded_height);
        //
        //     std::lock_guard<std::mutex> my_lock_guard(lock);
        //     queue.push(img);
        // }
        // return NULL;

    }

    static int avcodec_decode_video2(AVCodecContext* dec_ctx, AVFrame *frame, int *got_picture, AVPacket* pkt)
    {
        int ret;
        *got_picture = 0;
        ret = avcodec_send_packet(dec_ctx, pkt);
        if (ret == AVERROR_EOF) {
            ret = 0;
        }
        else if (ret < 0) {
            char err[256] = {0};
            av_strerror(ret, err, sizeof(err));
            fprintf(stderr, "Error sending a packet for decoding, %s\n", err);
            return -1;
        }
        while (ret >= 0) {
            ret = avcodec_receive_frame(dec_ctx, frame);
            if (ret == AVERROR(EAGAIN)) {
                ret = 0;
                break;
            }else if (ret == AVERROR_EOF) {
                printf("File end!\n");
                avcodec_flush_buffers(dec_ctx);
                ret = 0;
                break;
            }
            else if (ret < 0) {
                fprintf(stderr, "Error during decoding\n");
                break;
            }
            *got_picture += 1;
            break;
        }
        if (*got_picture > 1) {
            printf("got picture %d\n", *got_picture);
        }
        return ret;
    }

    AVFrame* Decode::grab_ffm_frame() {
        int ret;
        int got_frame = 0;
        timeval tv1, tv2;
        toolkit::gettimeofday(&tv1, NULL);

        while (!quit_flag_) {
            av_packet_unref(av_pkt_);
            ret = av_read_frame(av_fmt_ctx_, av_pkt_);
            if (ret < 0) {
                // 如果1分钟内连接不上，跳出循环
                if (ret == AVERROR(EAGAIN)) {
                    toolkit::gettimeofday(&tv2, NULL);
                    if (((tv2.tv_sec - tv1.tv_sec) * 1000 + (tv2.tv_usec - tv1.tv_usec) / 1000) > 1000 * 60) {
                        WarnL << "av_read_frame failed ret(" + std::to_string(ret) + ") retry time >60s.";
                        break;
                    }
                    toolkit::usleep(10 * 1000);
                    continue;
                }
                AVFrame* flush_frame = flush_ffm_frame();
                if(flush_frame)
                    return flush_frame;
                ErrorL << "av_read_frame ret("+ std::to_string(ret) + ") maybe eof...";
                quit_flag_ = true;
                return nullptr;
            }

            if (av_pkt_->stream_index != video_stream_idx_) {
                continue;
            }

            if (!frame_) {
                ErrorL << "av_read_frame could not allocate frame";
                return nullptr;
            }

            if (refcount_) {
                av_frame_unref(frame_);
            }
            // todo(wzk)
        //     toolkit::gettimeofday(&tv1, NULL);
        //     ret = avcodec_decode_video2(video_dec_ctx, frame, &got_frame, &pkt);
        //     if (ret < 0) {
        //         av_log(video_dec_ctx, AV_LOG_ERROR, "Error decoding video frame (%d)\n", ret);
        //         continue;
        //     }
        //
        //     if (!got_frame) {
        //         continue;
        //     }
        //
        //     width = video_dec_ctx->width;
        //     height = video_dec_ctx->height;
        //     pix_fmt = video_dec_ctx->pix_fmt;
        //     if (frame->width != width || frame->height != height || frame->format != pix_fmt) {
        //         av_log(video_dec_ctx, AV_LOG_ERROR,
        //                "Error: Width, height and pixel format have to be "
        //                "constant in a rawvideo file, but the width, height or "
        //                "pixel format of the input video changed:\n"
        //                "old: width = %d, height = %d, format = %s\n"
        //                "new: width = %d, height = %d, format = %s\n",
        //                width, height, av_get_pix_fmt_name((AVPixelFormat)pix_fmt), frame->width, frame->height,
        //                av_get_pix_fmt_name((AVPixelFormat)frame->format));
        //         continue;
        //     }
        //
        //     break;
        }
        return frame_;
    }

    AVFrame * Decode::flush_ffm_frame() {
        av_frame_unref(frame_);
        int ret = avcodec_send_packet(av_codec_ctx_, nullptr);
        ret = avcodec_receive_frame(av_codec_ctx_, frame_);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF || ret < 0)
        {
            return nullptr;
        }
        return frame_;
    }

    int Decode::start(const std::string& input) {
        InfoL << "FFMPEG hardaccel: CV186AH ";
        if (strstr(input.data(), "rtsp://")) {
            is_stream_ = true;
        }

        int ret = init_ffm_stream(input);
        if (ret != 0) {
            return ret;
        }



        decode_thread_ = std::thread(&Decode::background_decode, this);

        width_ = av_codec_ctx_->width;
        height_ = av_codec_ctx_->height;
        pix_fmt_ = av_codec_ctx_->pix_fmt;

        InfoL << "Video: "<< input
        << " width: "<< width_
        << "height: " << height_
        << "pix_fmt: " << pix_fmt_
        << "video_stream_idx: " << video_stream_idx_
        << " Start!";


        return 0;
    }

    kk::Frame * Decode::grab() {
        return VDecode::grab();
    }

    int Decode::stop() {
        VDecode::stop();
    }

    int Decode::init_ffm_stream(const std::string& input) {
        int ret = init_ffm_format(input);
        if (ret != 0) return ret;
        ret = init_ffm_codec();
        return ret;
    }

    int Decode::init_ffm_format(const std::string& input) {
        AVDictionary* dict = NULL;
        av_dict_set(&dict, "rtsp_flags", "prefer_tcp", 0);
        int ret = avformat_open_input(&av_fmt_ctx_, input.data(), NULL, &dict);
        if (ret < 0) {
            ErrorL << "FFMPEG decode cannot open input: " << input ;
            return ret;
        }

        ret = avformat_find_stream_info(av_fmt_ctx_, NULL);
        if (ret < 0) {
            ErrorL << "FFMPEG decode cannot find stream information: " << input ;
            return ret;
        }
        // 使用sophon硬件加速接口
        ret = av_find_best_stream(av_fmt_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
        if (ret < 0) {
            ErrorL << "FFMPEG decode could not find " << av_get_media_type_string(AVMEDIA_TYPE_VIDEO) << " stream";
            return ret;
        }

        video_stream_idx_ = ret;
        av_video_stream_ = av_fmt_ctx_->streams[video_stream_idx_];

        if (av_video_stream_->codecpar->codec_id != AV_CODEC_ID_H264
            && av_video_stream_->codecpar->codec_id != AV_CODEC_ID_HEVC) {
            this->is_hardaccel_ = false;
        }

        av_dict_free(&dict);
        return 0;
    }

    int Decode::init_ffm_codec() {
        /* find decoder for the stream */
        av_codec_ = const_cast<AVCodec*>(avcodec_find_decoder(av_video_stream_->codecpar->codec_id));

        if (!av_codec_) {
            ErrorL << "FFMPEG decode could not find " << av_get_media_type_string(AVMEDIA_TYPE_VIDEO) << " codec";
            return AVERROR(EINVAL);
        }

        /* Allocate a codec context for the decoder */
        av_codec_ctx_ = avcodec_alloc_context3(av_codec_);
        if (!av_codec_ctx_) {
            ErrorL << "FFMPEG decode could not allocate the " << av_get_media_type_string(AVMEDIA_TYPE_VIDEO) << " codec context";
            return AVERROR(ENOMEM);
        }

        /* Copy codec parameters from input stream to output codec context */
        int ret = avcodec_parameters_to_context(av_codec_ctx_, av_video_stream_->codecpar);
        if (ret < 0) {
            ErrorL << "FFMPEG decode failed to copy " << av_get_media_type_string(AVMEDIA_TYPE_VIDEO) << " codec parameters to decoder context";
            return ret;
        }

        av_codec_para_ = av_video_stream_->codecpar;
        /* Init the decoders, with or without reference counting */
        AVDictionary* opts;
        av_dict_set(&opts, "refcounted_frames", refcount_ ? "1" : "0", 0);
        // 使用sophon硬件加速接口
        av_dict_set_int(&opts, "sophon_idx", bm_get_devid(bm_handle_), 0);
        av_dict_set_int(&opts, "extra_frame_buffer_num", EXTRA_FRAME_BUFFER_NUM, 0);  // if we use dma_buffer mode
        av_dict_set_int(&opts, "output_format", output_format_, 18);

        ret = avcodec_open2(av_codec_ctx_, av_codec_, &opts);
        if (ret < 0) {
            ErrorL << "FFMPEG decode failed to open " << av_get_media_type_string(AVMEDIA_TYPE_VIDEO) << " codec";
            return ret;
        }

        av_dict_free(&opts);
        return 0;
    }
}
