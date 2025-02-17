//
// Created by wangzk on 2025/2/17.
//

#include "bm_cv.h"
#include "Util/logger.h"

#define FF_ALIGN(x,a) (((x)+(a)-1)&~((a)-1))


int map_avformat_to_bmformat(int avformat) {
    int format;
    switch (avformat) {
        case AV_PIX_FMT_RGB24:
            format = FORMAT_RGB_PACKED;
        break;
        case AV_PIX_FMT_BGR24:
            format = FORMAT_BGR_PACKED;
        break;
        case AV_PIX_FMT_YUV420P:
        case AV_PIX_FMT_YUVJ420P:
            format = FORMAT_YUV420P;
        break;
        case AV_PIX_FMT_YUV422P:
        case AV_PIX_FMT_YUVJ422P:
            format = FORMAT_YUV422P;
        break;
        case AV_PIX_FMT_YUV444P:
        case AV_PIX_FMT_YUVJ444P:
            format = FORMAT_YUV444P;
        break;
        case AV_PIX_FMT_NV12:
            format = FORMAT_NV12;
        break;
        case AV_PIX_FMT_NV16:
            format = FORMAT_NV16;
        break;
        case AV_PIX_FMT_GRAY8:
            format = FORMAT_GRAY;
        break;
        case AV_PIX_FMT_GBRP:
            format = FORMAT_RGBP_SEPARATE;
        break;
        default:
            ErrorL << "unsupported av_pix_format " << avformat;
        return -1;
    }

    return format;
}

int get_bm_parameters(AVFrame* in, int& plane, int& data_four_denominator, int& data_five_denominator, int& data_six_denominator) {
    switch (in->format) {
        case AV_PIX_FMT_RGB24:
        case AV_PIX_FMT_BGR24:
            plane = 1;
            data_four_denominator = 1;
            data_five_denominator = -1;
            data_six_denominator = -1;
            break;
        case AV_PIX_FMT_GRAY8:
            plane = 1;
            data_four_denominator = -1;
            data_five_denominator = -1;
            data_six_denominator = -1;
            break;
        case AV_PIX_FMT_YUV420P:
        case AV_PIX_FMT_YUVJ420P:
            plane = 3;
            data_four_denominator = -1;
            data_five_denominator = 2;
            data_six_denominator = 2;
            break;
        case AV_PIX_FMT_NV12:
            plane = 2;
            data_four_denominator = -1;
            data_five_denominator = 1;
            data_six_denominator = -1;
            break;
        case AV_PIX_FMT_YUV422P:
        case AV_PIX_FMT_YUVJ422P:
            plane = 3;
            data_four_denominator = -1;
            data_five_denominator = 1;
            data_six_denominator = 1;
            break;
        // case AV_PIX_FMT_YUV440P:
        // case AV_PIX_FMT_YUVJ440P:
        //     plane = 3;
        //     data_four_denominator = -1;
        //     data_five_denominator = 1;
        //     data_six_denominator = 4;
        //     break;
        case AV_PIX_FMT_NV16:
            plane = 2;
            data_four_denominator = -1;
            data_five_denominator = 2;
            data_six_denominator = -1;
            break;
        case AV_PIX_FMT_YUV444P:
        case AV_PIX_FMT_YUVJ444P:
        case AV_PIX_FMT_GBRP:
            plane = 3;
            data_four_denominator = -1;
            data_five_denominator = 1;
            data_six_denominator = 1;
            break;
        default:
            ErrorL << "unsupported format, only yuv420,yuvj420,yuv422,yuvj422,yuv444,yuvj444,nv12,nv16,gray,rgb_packed,bgr_packed supported";
            return -1;
    }
    return 0;
}


#define QUEUE_MAX_SIZE 5
#define USEING_MEM_HEAP2 4
#define USEING_MEM_HEAP1 2

bm_status_t avframe_to_bm_image(bm_handle_t& handle, AVFrame* in, bm_image* out, bool is_jpeg, bool data_on_device_mem, int coded_width, int coded_height) {
    int plane = 0;
    int data_four_denominator = -1;
    int data_five_denominator = -1;
    int data_six_denominator = -1;
    static int mem_flags = USEING_MEM_HEAP2;

    int ret = get_bm_parameters(in, plane, data_four_denominator, data_five_denominator, data_six_denominator);
    if (ret < 0) {
        return BM_ERR_PARAM;
    }

    if (in->channel_layout == 101) { /* COMPRESSED NV12 FORMAT */
        if ((0 == in->height) || (0 == in->width) || (0 == in->linesize[4]) || (0 == in->linesize[5]) ||
            (0 == in->linesize[6]) || (0 == in->linesize[7]) || (0 == in->data[4]) || (0 == in->data[5]) ||
            (0 == in->data[6]) || (0 == in->data[7])) {
            ErrorL << "bm_image_from_frame: get yuv failed!!";
            return BM_ERR_PARAM;
        }
        bm_image cmp_bmimg;
        bm_image_create(handle, coded_height, coded_width, FORMAT_COMPRESSED, DATA_TYPE_EXT_1N_BYTE, &cmp_bmimg);

        bm_device_mem_t input_addr[4];
        int size = in->height * in->linesize[4];
        input_addr[0] = bm_mem_from_device((unsigned long long)in->data[6], size);
        size = (in->height / 2) * in->linesize[5];
        input_addr[1] = bm_mem_from_device((unsigned long long)in->data[4], size);
        size = in->linesize[6];
        input_addr[2] = bm_mem_from_device((unsigned long long)in->data[7], size);
        size = in->linesize[7];
        input_addr[3] = bm_mem_from_device((unsigned long long)in->data[5], size);
        bm_image_attach(cmp_bmimg, input_addr);
        bm_image_create(handle, in->height, in->width, FORMAT_YUV420P, DATA_TYPE_EXT_1N_BYTE, out);
        if (mem_flags == USEING_MEM_HEAP2 && bm_image_alloc_dev_mem_heap_mask(*out, mem_flags) != BM_SUCCESS) {
            mem_flags = USEING_MEM_HEAP1;
        }
        if (mem_flags == USEING_MEM_HEAP1 && bm_image_alloc_dev_mem_heap_mask(*out, mem_flags) != BM_SUCCESS) {
            ErrorL << "bmcv allocate mem failed!!!";
        }

        bmcv_rect_t crop_rect = {0, 0, (unsigned)in->width, (unsigned)in->height};
        bmcv_image_vpp_convert(handle, 1, cmp_bmimg, out, &crop_rect);
        bm_image_destroy(&cmp_bmimg);
    } else {
        int stride[3];
        bm_image_format_ext bm_format;
        bm_device_mem_t input_addr[3] = {0};

        data_on_device_mem ? stride[0] = in->linesize[4] : stride[0] = in->linesize[0];

        if (plane > 1) {
            data_on_device_mem ? stride[1] = in->linesize[5] : stride[1] = in->linesize[1];
        }
        if (plane > 2) {
            data_on_device_mem ? stride[2] = in->linesize[6] : stride[2] = in->linesize[2];
        }
        bm_image tmp;
        bm_format = (bm_image_format_ext)map_avformat_to_bmformat(in->format);
        bm_image_create(handle, in->height, in->width, bm_format, DATA_TYPE_EXT_1N_BYTE, &tmp, stride);
        bm_image_create(handle, in->height, in->width, FORMAT_BGR_PACKED, DATA_TYPE_EXT_1N_BYTE, out);
        if (mem_flags == USEING_MEM_HEAP2 && bm_image_alloc_dev_mem_heap_mask(*out, mem_flags) != BM_SUCCESS) {
            mem_flags = USEING_MEM_HEAP1;
        }
        if (mem_flags == USEING_MEM_HEAP1 && bm_image_alloc_dev_mem_heap_mask(*out, mem_flags) != BM_SUCCESS) {
            ErrorL << "bmcv allocate mem failed!!!";
        }

        int size = in->height * stride[0];
        if (data_four_denominator != -1) {
            size = in->height * stride[0] * 3;
        }
        if (data_on_device_mem) {
            input_addr[0] = bm_mem_from_device((unsigned long long)in->data[4], size);
        } else {
            bm_malloc_device_byte(handle, &input_addr[0], size);
            bm_memcpy_s2d_partial(handle, input_addr[0], in->data[0], size);
        }

        if (data_five_denominator != -1) {
            size = FF_ALIGN(in->height,2) * stride[1] / data_five_denominator;
            if (data_on_device_mem) {
                input_addr[1] = bm_mem_from_device((unsigned long long)in->data[5], size);
            } else {
                bm_malloc_device_byte(handle, &input_addr[1], size);
                bm_memcpy_s2d_partial(handle, input_addr[1], in->data[1], size);
            }
        }

        if (data_six_denominator != -1) {
            size = FF_ALIGN(in->height,2) * stride[2] / data_six_denominator;
            if (data_on_device_mem) {
                input_addr[2] = bm_mem_from_device((unsigned long long)in->data[6], size);
            } else {
                bm_malloc_device_byte(handle, &input_addr[2], size);
                bm_memcpy_s2d_partial(handle, input_addr[2], in->data[2], size);
            }
        }

        bm_image_attach(tmp, input_addr);
        if (is_jpeg) {
            csc_type_t csc_type = CSC_YPbPr2RGB_BT601;
            bmcv_image_vpp_csc_matrix_convert(handle, 1, tmp, out, csc_type, NULL, BMCV_INTER_NEAREST, NULL);
        } else {
            bmcv_rect_t crop_rect = {0, 0, (unsigned)in->width, (unsigned)in->height};
            bmcv_image_vpp_convert(handle, 1, tmp, out, &crop_rect);
        }
        bm_image_destroy(&tmp);

        if (!data_on_device_mem) {
            bm_free_device(handle, input_addr[0]);
            if (data_five_denominator != -1)
                bm_free_device(handle, input_addr[1]);
            if (data_six_denominator != -1)
                bm_free_device(handle, input_addr[2]);
        }
    }
    return BM_SUCCESS;
}

