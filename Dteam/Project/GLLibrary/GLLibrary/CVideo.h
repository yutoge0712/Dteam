#pragma once
#include "GL.h"
#include "CImage.h"
#include "CSound.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <inttypes.h>
}

#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "swresample.lib")

class CVideoTextrue : public CTexture {
    struct VideoReaderState {
        // Public things for other parts of the program to read from
        int width, height;
        AVRational time_base;

        // Private internal state
        AVFormatContext* av_format_ctx;
        AVCodecContext* video_codec_ctx;
        AVCodecContext* audio_codec_ctx;
        int video_stream_index;
        int audio_stream_index;
        AVFrame* av_frame;
        AVPacket* av_packet;
        SwsContext* sws_scaler_ctx;
        int frame_number;
        SwrContext* swr_ctx;
    };
    uint8_t* m_swr_buf = 0;
    int m_swr_buf_len = 0;
    float m_time;
    float m_speed_scale;
    std::string m_file_name;
    CSoundBase* m_sound;
    VideoReaderState m_state;
    bool video_reader_open(const char* filename);
    bool video_reader_read_frame(uint8_t* frame_buffer, int64_t* pts);
    bool video_decode_audio();
    bool video_reader_seek_frame(int64_t ts);
    void video_reader_close();

public:
    CVideoTextrue(const char* filename);
    void RenderFrame();
    void MapTexture();
    void Release();
    void Play();
    void Stop();

    friend class CVideo;
};

class CVideo : private CImage {
    CVideoTextrue* mp_video_texture;
public:
    CVideo(const char* filename);
    void Draw();
    void Play();
    void Stop();
    bool isPlay() {
        return mp_video_texture->m_speed_scale == 0 ? false : true;
    }
    void SetSize(int w, int h) {
        CImage::SetSize(w, h);
    }
    void SetPos(int x, int y) {
        CImage::SetPos(x, y);
    }
    void SetPos(CVector2D pos) {
        CImage::SetPos(pos);
    }
};

