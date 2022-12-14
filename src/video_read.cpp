//
// Created by Egor Shapovalov on 12.12.2022.
//

#include "video_read.h"
extern "C" {
#include <libavdevice/avdevice.h>
};

bool video_reader_open_file(VideoReaderState* state, const char* filename) {

    avdevice_register_all();

    auto &avFormatContext = state->avFormatContext;
    auto &avCodecContext = state->avCodecContext;
    auto &timeBase = state->timeBase;
    auto &avFrame = state->avFrame;
    auto &videoStreamIndex = state->videoStreamIndex;
    auto &avPacket = state->avPacket;
    auto &swsContext = state->swsContext;
    auto &width = state->width;
    auto &height = state->height;


    avFormatContext = avformat_alloc_context();
    if (!avFormatContext)
    {
        printf("Couldn't create AVFormarContext\n");
        return false;
    }
    const AVInputFormat* avInputFormat = av_find_input_format("avfoundation");

    if (!avInputFormat){
        printf("couldn't find AVFoundation input format to get webcam\n");
        return false;
    }

    AVDictionary* options = NULL;
    av_dict_set(&options, "framerate", "30", 0);
    av_dict_set(&options, "video_size", "1280x720", 0);
    //av_dict_set(&options, "pixel_fmt:0", "bgr0", 0);

    if(avformat_open_input(&avFormatContext, "default:none", avInputFormat, &options) != 0) {
        printf("Couldn't open video\n");
        return false;
    }

    //find valid video stream
    videoStreamIndex = -1;
    AVCodecParameters* avCodecParameters = NULL;
    const AVCodec* avCodec = NULL;
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        avCodecParameters = avFormatContext->streams[i]->codecpar;
        avCodec = avcodec_find_decoder(avCodecParameters->codec_id);
        if (!avCodec)
            continue;
        if (avCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            width = avCodecParameters->width ;
            height = avCodecParameters->height ;
            videoStreamIndex = i;
            timeBase = avFormatContext->streams[i]->time_base;
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
    return true;
}


bool video_reader_read_frame(VideoReaderState* state, uint8_t* frameBuffer, int64_t* pts) {
   auto &avFormatContext = state->avFormatContext;
   auto &avCodecContext = state->avCodecContext;
   auto &avFrame = state->avFrame;
   auto &videoStreamIndex = state->videoStreamIndex;
   auto &avPacket = state->avPacket;
   auto &swsContext = state->swsContext;
   auto &width = state->width;
   auto &height = state->height;

    int  response;
    while (av_read_frame(avFormatContext, avPacket) >=0) {
        if (avPacket->stream_index != videoStreamIndex) {
            av_packet_unref(avPacket);
            continue;
        }
        response = avcodec_send_packet(avCodecContext, avPacket);
        if (response < 0){
            printf("Failed to decode paket: %s\n", av_err2str(response));
            return false;
        }
        response = avcodec_receive_frame(avCodecContext, avFrame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {{
            av_packet_unref(avPacket);
            continue;
        }
        } else if (response < 0) {
            printf("Failed ot decode paket: %sn\n", av_err2str(response));
            return false;
        }
        av_packet_unref(avPacket);

        break;
    }

    *pts = avFrame->pts;

    printf(
        "Frame %c (%d) pts %d dts %d key_frame %d [coded_picture_number %d, display_picture_number %d]\n",
        av_get_picture_type_char(avFrame->pict_type),
        avCodecContext->frame_number,
        avFrame->pts,
        avFrame->pkt_dts,
        avFrame->key_frame,
        avFrame->coded_picture_number,
        avFrame->display_picture_number
    );


    swsContext = sws_getContext(width ,height ,
                                avCodecContext->pix_fmt,
                                width ,height,
                                AV_PIX_FMT_RGB0,
                                SWS_BILINEAR,
                                NULL,
                                NULL,
                                NULL);
    if (!swsContext) {
        printf("couldn't initialize swsScaler\n ");
        return false;
    }
    uint8_t* dest[8] = {frameBuffer, NULL, NULL, NULL};
    int destLinesize[4] = {width * 4, 0, 0, 0};
    sws_scale(swsContext, avFrame->data, avFrame->linesize, 0, height,dest, destLinesize );


    return true;
}


void video_reader_close(VideoReaderState *state) {
    sws_freeContext(state->swsContext);
    avformat_close_input(&state->avFormatContext);
    avformat_free_context(state->avFormatContext);
    av_frame_free(&state->avFrame);
    av_packet_free(&state->avPacket);
    avcodec_free_context(&state->avCodecContext);
}