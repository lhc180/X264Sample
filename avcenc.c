#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "x264.h"

#define YUV_WIDTH   352
#define YUV_HIGHT   288
#define Y_LENGTH    (YUV_WIDTH)*(YUV_HIGHT)
#define UV_LENGTH   ((YUV_WIDTH)>>1)*((YUV_HIGHT)>>1)
#define YUV_LENGTH  (YUV_WIDTH)*(YUV_HIGHT)*3/2

int main() {
    x264_param_t params;
    x264_t *enc = NULL;
    FILE *yuv_fp = NULL;
    FILE *out_fp = NULL;
    unsigned char *yuv_buf = NULL;
    int bitrate = 200000;
    
    if (x264_param_default_preset(&params, "superfast", "zerolatency")) {
        printf("Canot apply default x264 configuration");
        goto FAILED;
    }
    
	params.i_threads = 1;
	params.i_width = YUV_WIDTH;
	params.i_height = YUV_HIGHT;
	params.i_fps_num = 10;
	params.i_fps_den = 1;
	params.i_slice_max_size = 1400;
	params.i_level_idc = 13;
        
	params.b_repeat_headers = 1;
	params.b_annexb = 1;
    
	//these parameters must be set so that our stream is baseline
	params.analyse.b_transform_8x8 = 0;
	params.b_cabac = 0;
	params.i_cqm_preset = X264_CQM_FLAT;
	params.i_bframe = 0;
	params.analyse.i_weighted_pred = X264_WEIGHTP_NONE;
    
	bitrate = bitrate*0.92;
	params.rc.i_rc_method = X264_RC_ABR;
	params.rc.i_bitrate = (int)(bitrate/1000);
	params.rc.f_rate_tolerance = 0.1;
	params.rc.i_vbv_max_bitrate = (int) ((bitrate+10000/2)/1000);
	params.rc.i_vbv_buffer_size = params.rc.i_vbv_max_bitrate;
	params.rc.f_vbv_buffer_init = 0.5;
    
    //debug
    // params.i_log_level = X264_LOG_DEBUG;
    params.analyse.b_psnr = 1;

    enc = x264_encoder_open(&params);
    if (enc == NULL) {
        printf("Fail to create x264 encoder.");
        goto FAILED;
    }

    yuv_buf = malloc(params.i_width*params.i_height*3/2);
    if (yuv_buf == NULL) {
        printf("Fail to malloc yuv buffer.");
        goto FAILED;
    }
    
    yuv_fp = fopen("foreman_cif.yuv", "rb");
    if (yuv_fp == NULL) {
        printf("Fail to open yuv file.");
        goto FAILED;
    }

    out_fp = fopen("out.264", "wb");
    if (out_fp == NULL) {
        printf("Fail to open output H.264 raw file.");
        goto FAILED;
    }
    
    while (fread(yuv_buf, YUV_LENGTH, 1, yuv_fp)) {
        x264_picture_t xpic;
		x264_picture_t oxpic;
		x264_nal_t *xnals = NULL;
		int num_nals = 0;
        
        memset(&xpic, 0, sizeof(xpic));
        memset(&oxpic, 0, sizeof(oxpic));
        
        xpic.i_type = X264_TYPE_AUTO;
		xpic.i_qpplus1 = 0;
		xpic.param = NULL;
		xpic.img.i_csp = X264_CSP_I420;
		xpic.img.i_plane = 3;
		xpic.img.i_stride[0] = YUV_WIDTH;
		xpic.img.i_stride[1] = YUV_WIDTH/2;
		xpic.img.i_stride[2] = YUV_WIDTH/2;
		xpic.img.i_stride[3] = 0;
		xpic.img.plane[0] = yuv_buf;
		xpic.img.plane[1] = yuv_buf + Y_LENGTH;
		xpic.img.plane[2] = yuv_buf + Y_LENGTH + UV_LENGTH;
		xpic.img.plane[3] = 0;
        
        if (x264_encoder_encode(enc, &xnals, &num_nals, &xpic, &oxpic) >= 0) {
            int i;
            for (i = 0; i < num_nals; ++i) {
                fwrite(xnals[i].p_payload, xnals[i].i_payload, 1, out_fp);
            }
        }
    }
    
FAILED:
    if (out_fp != NULL) fclose(out_fp);
    if (yuv_fp != NULL) fclose(yuv_fp);
    if (enc != NULL) x264_encoder_close(enc);

    return 0;
}