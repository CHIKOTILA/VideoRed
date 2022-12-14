//
// Created by Egor Shapovalov on 08.12.2022.
//
#include "video_read.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <inttypes.h>
}

bool load_frame(const char* filename, int* width_out, int* height_out, unsigned char** data_out) {
    AVFormatContext* avFormatContext;
    AVCodecContext* avCodecContext;
    AVFrame* avFrame;
    AVPacket* avPacket;
    SwsContext* swsContext;

  //  auto &avFormatContext = state->avFormatContext;
  //  auto &avCodecContext = state->avCodecContext;
  //  auto &avFrame = state->avFrame;
  //  auto &videoStreamIndex = state->videoStreamIndex;
  //  auto &avPacket = state->avPacket;
  //  auto &swsContext = state->swsContext;
  //  auto &width = state->width;
  //  auto &height = state->height;

    //open the file
    avFormatContext = avFormatContext = avformat_alloc_context();
    if (!avFormatContext)
    {
        printf("Couldn't create AVFormarConext\n");
        return false;
    }

    if(avformat_open_input(&avFormatContext, filename, NULL, NULL ) != 0) {
        printf("Couldn't open video\n");
        return false;
    }
    //find valid video stream
    int videoStreamIndex = -1;
    AVCodecParameters* avCodecParameters = NULL;
    const AVCodec* avCodec = NULL;
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        avCodecParameters = avFormatContext->streams[i]->codecpar;
        avCodec = avcodec_find_decoder(avCodecParameters->codec_id);
        if (!avCodec)
            continue;
        if (avCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }
    if (videoStreamIndex == -1) {
        printf("couldn't find valid video stream inside file\n");
        return false;
    }
    // set up a codec context for the decoder

    avCodecContext = avcodec_alloc_context3(avCodec);
    if (!avCodecContext) {
        printf("Couldn't create AVCodecContext\n");
        return false;
    }
    if (avcodec_parameters_to_context(avCodecContext, avCodecParameters) < 0) {
        printf("couldn't initialize AVCodecContext\n ");
        return false;
    }
    if (avcodec_open2(avCodecContext, avCodec, NULL) < 0) {
        printf("couldn't open codec\n");
        return false;
    }

    avFrame = av_frame_alloc();
    if (!avFrame) {
        printf("couldn't allocate AVFrame\n");
        return false;
    }
    avPacket = av_packet_alloc();
    if (!avPacket) {
        printf("couldn't allocatr AVPaket\n");
        return false;
    }
    int  response;
    while (av_read_frame(avFormatContext, avPacket) >=0) {
        if (avPacket->stream_index != videoStreamIndex) {
            continue;
        }
        response = avcodec_send_packet(avCodecContext, avPacket);
        if (response < 0)
        {
            printf("Failed to decode paket: %s\n", av_err2str(response));
            return false;
        }
        response = avcodec_receive_frame(avCodecContext, avFrame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            continue;
        } else if (response < 0) {
            printf("Failed ot decode paket: %sn\n", av_err2str(response));
            return false;
        }
        av_packet_unref(avPacket);
        break;
    }
    uint8_t *data = new uint8_t [avFrame->width * avFrame->height * 4];
    swsContext = sws_getContext(avFrame->width,avFrame->height,
                                            avCodecContext->pix_fmt,
                                            avFrame->width,avFrame->height,
                                            AV_PIX_FMT_RGB0,
                                            SWS_BILINEAR,
                                            NULL,
                                            NULL,
                                            NULL);
    if (!swsContext) {
        printf("couldn't initialize swsScaler\n ");
        return false;
    }
    uint8_t* dest[8] = {data, NULL, NULL, NULL};
    int destLinesize[4] = {avFrame->width * 4, 0, 0, 0};
    sws_scale(swsContext, avFrame->data, avFrame->linesize, 0, avFrame->height,dest, destLinesize );
    sws_freeContext(swsContext);

    *width_out = avFrame->width;
    *height_out = avFrame->height;
    *data_out = data;

    avformat_close_input(&avFormatContext);
    avformat_free_context(avFormatContext);
    av_frame_free(&avFrame);
    av_packet_free(&avPacket);
    avcodec_free_context(&avCodecContext);
    return true;
}
