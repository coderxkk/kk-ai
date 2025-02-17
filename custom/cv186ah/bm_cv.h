//
// Created by wangzk on 2025/2/17.
//

#ifndef BM_CV_H
#define BM_CV_H
#include "bmcv_api_ext.h"

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

/**
 * @brief convert avframe pix format to bm_image pix format.
 * @return bm_image_format_ext.
 */
int map_avformat_to_bmformat(int avformat);

/**
 * @brief convert avformat to bm_image.
 */
bm_status_t avframe_to_bm_image(bm_handle_t &handle, AVFrame *in, bm_image *out, bool is_jpeg, bool data_on_device_mem, int coded_width=-1, int coded_height=-1);



#endif //BM_CV_H
