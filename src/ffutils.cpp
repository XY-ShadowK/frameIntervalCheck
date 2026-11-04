#include <algorithm>
#include <cstdint>
#include <functional>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
}

#include "include/ffutils.hpp"

const long double E_6 = 1e6;
const int64_t defaultAverageFPS = 30LL;
const size_t defaultNumFrame = 1000;

int64_t getRoughNumFrame(const AVStream* const videoStream, const int64_t& duration) {
    if(videoStream->nb_frames > 0)
        return videoStream->nb_frames;
    long double fpsAverage = av_q2d(videoStream->avg_frame_rate);
    if(duration <= 0)
        return 0;
    if(fpsAverage <= 0)
        return duration * defaultAverageFPS;
    return (int64_t)(fpsAverage * (long double)duration / E_6 + 0.5L);
}

bool getptsFrame(AVCodecContext* codecContext, AVFrame* frame, std::vector<int64_t>& ptsArray) {
    bool isLossPTS = false;
    int64_t pts = 0;
    while(avcodec_receive_frame(codecContext, frame) == 0) {
        pts = (frame->pts != AV_NOPTS_VALUE) ? frame->pts : frame->best_effort_timestamp;
        if(pts != AV_NOPTS_VALUE)
            ptsArray.emplace_back(pts);
        else {
            isLossPTS = true;
            break;
        }
        av_frame_unref(frame);
    }
    av_frame_unref(frame);
    return !isLossPTS;
}

bool getPTSArrayContainer(AVFormatContext* formatContext, const int& indexVideoStream, std::vector<int64_t>& ptsArray
, const bool isVerbose = false) {
    if(isVerbose)
        fprintf(stdout, "try scan container.\n");
    bool isLossPTS = false;
    AVPacket* avPacket = av_packet_alloc();
    while (av_read_frame(formatContext, avPacket) >= 0) {
        if(avPacket->stream_index != indexVideoStream) {
            av_packet_unref(avPacket);
            continue;
        }
        if(avPacket->pts != AV_NOPTS_VALUE)
            ptsArray.emplace_back(avPacket->pts);
        else {
            isLossPTS = true;
            break;
        }
        av_packet_unref(avPacket);
    }
    av_packet_unref(avPacket);

    if(isVerbose && isLossPTS)
        fprintf(stderr, "container has lost pts.\n");
    return !isLossPTS;
}

bool getPTSArrayDecodeNV(const AVStream* const videoStream, AVFormatContext* formatContext, const int& indexVideoStream, std::vector<int64_t>& ptsArray
, const bool isVerbose = false) {
    if(isVerbose)
        fprintf(stdout, "try decode by nvdec.\n");
    bool isLossPTS = false;
    AVPacket* avPacket = av_packet_alloc();
    const AVCodec* codec = avcodec_find_decoder(videoStream->codecpar->codec_id);
    if(!codec)
        return false;

    AVPixelFormat nvPixelFormat = AV_PIX_FMT_NONE;
    int indexCodecHWConfig = 0;
    const AVCodecHWConfig* codecHWConfig;
    while(codecHWConfig = avcodec_get_hw_config(codec, indexCodecHWConfig)) {
        if(codecHWConfig->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX && codecHWConfig->device_type == AV_HWDEVICE_TYPE_CUDA) {
            nvPixelFormat = codecHWConfig->pix_fmt;
            break;
        }
        indexCodecHWConfig ++;
    }

    static std::function<AVPixelFormat(AVCodecContext*, const AVPixelFormat* const)> getFormatNVTemp;
    getFormatNVTemp = [&nvPixelFormat](AVCodecContext* _codecContext, const AVPixelFormat* const pixelFormats) {
        for(int i = 0;pixelFormats[i] != AV_PIX_FMT_NONE;i ++)
            if(pixelFormats[i] == nvPixelFormat)
                return nvPixelFormat;
        return AV_PIX_FMT_NONE;
    };
    static auto getFormatNV = [](AVCodecContext* _codecContext, const AVPixelFormat* const pixelFormats) {
        return getFormatNVTemp(_codecContext, pixelFormats);
    };

    AVBufferRef* cudaDeviceContext = nullptr;
    AVCodecContext* codecContext;
    if(av_hwdevice_ctx_create(&cudaDeviceContext, AV_HWDEVICE_TYPE_CUDA, nullptr, nullptr, 0) < 0
    || !(codecContext = avcodec_alloc_context3(codec))) {
        av_buffer_unref(&cudaDeviceContext);
        return false;
    }
    if(avcodec_parameters_to_context(codecContext, videoStream->codecpar) < 0) {
        avcodec_free_context(&codecContext);
        av_buffer_unref(&cudaDeviceContext);
        return false;
    }
    codecContext->hw_device_ctx = av_buffer_ref(cudaDeviceContext);
    codecContext->get_format = getFormatNV;
    if(avcodec_open2(codecContext, codec, nullptr)) {
        avcodec_free_context(&codecContext);
        av_buffer_unref(&cudaDeviceContext);
        return false;
    }
    AVFrame* frame = av_frame_alloc();

    while (av_read_frame(formatContext, avPacket) >= 0) {
        if(avPacket->stream_index != indexVideoStream) {
            av_packet_unref(avPacket);
            continue;
        }
        if(avcodec_send_packet(codecContext, avPacket) < 0
        || !getptsFrame(codecContext, frame, ptsArray)) {
            isLossPTS = true;
            break;
        }
        av_packet_unref(avPacket);
    }
    av_packet_unref(avPacket);
    av_packet_free(&avPacket);
    if(avcodec_send_packet(codecContext, nullptr) < 0
    || !getptsFrame(codecContext, frame, ptsArray))
        isLossPTS = true;

    av_frame_free(&frame);
    avcodec_free_context(&codecContext);
    av_buffer_unref(&cudaDeviceContext);

    if(isVerbose && isLossPTS)
        fprintf(stderr, "nvdec decoding has lost pts.\n");
    return !isLossPTS;
}

bool getPTSArrayDecodeSW(const AVStream* const videoStream, AVFormatContext* formatContext, const int& indexVideoStream, std::vector<int64_t>& ptsArray
, const bool isVerbose = false) {
    if(isVerbose)
        fprintf(stdout, "try decode by software.\n");
    bool isLossPTS = false;
    AVPacket* avPacket = av_packet_alloc();
    const AVCodec* codec = avcodec_find_decoder(videoStream->codecpar->codec_id);
    if(!codec)
        return false;
    AVCodecContext* codecContext = avcodec_alloc_context3(codec);
    if(!codecContext
    || avcodec_parameters_to_context(codecContext, videoStream->codecpar) < 0
    || avcodec_open2(codecContext, codec, nullptr) < 0) {
        avcodec_free_context(&codecContext);
        return false;
    }
    AVFrame* frame = av_frame_alloc();

    while (av_read_frame(formatContext, avPacket) >= 0) {
        if(avPacket->stream_index != indexVideoStream) {
            av_packet_unref(avPacket);
            continue;
        }
        if(avcodec_send_packet(codecContext, avPacket) < 0
        || !getptsFrame(codecContext, frame, ptsArray)) {
            isLossPTS = true;
            break;
        }
        av_packet_unref(avPacket);
    }
    av_packet_unref(avPacket);
    av_packet_free(&avPacket);
    if(avcodec_send_packet(codecContext, avPacket) < 0
    || !getptsFrame(codecContext, frame, ptsArray))
        isLossPTS = true;

    av_frame_free(&frame);
    avcodec_free_context(&codecContext);

    if(isVerbose && isLossPTS)
        fprintf(stderr, "software decoding has lost pts.\n");
    return !isLossPTS;
}

bool getPTSArrayDecode(const AVStream* const videoStream, AVFormatContext* formatContext, const int& indexVideoStream, std::vector<int64_t>& ptsArray
, const int opt = (int)PTS_GET_WAY::SOFTWARE_DECODE
, const bool isVerbose = false) {
    if(opt & (int)PTS_GET_WAY::HARDWARE_DECODE)
        return getPTSArrayDecodeNV(videoStream, formatContext, indexVideoStream, ptsArray, isVerbose);
    if(opt & (int)PTS_GET_WAY::SOFTWARE_DECODE)
        return getPTSArrayDecodeSW(videoStream, formatContext, indexVideoStream, ptsArray, isVerbose);
    return false;
}

// RVO?
bool getPTSArray(std::vector<int64_t>& ptsArray, const std::string& filename
, const int opt
, const bool isVerbose) {
    AVFormatContext* formatContext = nullptr;
    int indexVideoStream;
    if(avformat_open_input(&formatContext, filename.c_str(), nullptr, nullptr) < 0)
        return false;
    if(avformat_find_stream_info(formatContext, nullptr) < 0
    || (indexVideoStream = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0)) < 0) {
        avformat_close_input(&formatContext);
        return false;
    }
    AVStream* videoStream = formatContext->streams[indexVideoStream];

    int64_t numFrame = getRoughNumFrame(videoStream, formatContext->duration);
    if(numFrame)
        ptsArray.reserve(numFrame);
    else
        ptsArray.reserve(defaultNumFrame);

    if (!(opt & (int)PTS_GET_WAY::CONTAINER)
    || !getPTSArrayContainer(formatContext, indexVideoStream, ptsArray, isVerbose)) {
        ptsArray.clear();
        if(av_seek_frame(formatContext, -1, 0, AVSEEK_FLAG_BACKWARD) >= 0)
            if(!(opt & ((int)PTS_GET_WAY::SOFTWARE_DECODE | (int)PTS_GET_WAY::HARDWARE_DECODE))
            || !getPTSArrayDecode(videoStream, formatContext, indexVideoStream, ptsArray, opt, isVerbose))
                ptsArray.clear();
    }
    
    avformat_close_input(&formatContext);

    if(ptsArray.empty())
        return false;

    if(!std::is_sorted(ptsArray.begin(), ptsArray.end()))
        std::sort(ptsArray.begin(), ptsArray.end());
    return true;
}

std::pair<int, int> getTimeBase(const std::string& filename) {
    AVFormatContext* formatContext = nullptr;
    int indexVideoStream;
    if(avformat_open_input(&formatContext, filename.c_str(), nullptr, nullptr) < 0)
        return {0, 0};
    if(avformat_find_stream_info(formatContext, nullptr) < 0
    || (indexVideoStream = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0)) < 0) {
        avformat_close_input(&formatContext);
        return {0, 0};
    }
    AVRational timeBase = formatContext->streams[indexVideoStream]->time_base;
    avformat_close_input(&formatContext);
    return {timeBase.num, timeBase.den};
}