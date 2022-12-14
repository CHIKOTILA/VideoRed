#ifndef video_reader_hpp
#define video_reader_hpp


extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <inttypes.h>
}


struct VideoReaderState {
    //public things for other parts of the program to read
    int width, height;
    uint8_t* frameBuffer;

    //private internal state
    AVFormatContext* avFormatContext;
    int videoStreamIndex;
    AVCodecContext* avCodecContext;
    AVFrame* avFrame;
    AVPacket* avPacket;
    SwsContext* swsContext;
};

bool video_reader_open_file(VideoReaderState* state, const char* filename);
bool video_reader_read_frame(VideoReaderState* state, uint8_t* frameBuffer );
void video_reader_close(VideoReaderState *state);

#endif