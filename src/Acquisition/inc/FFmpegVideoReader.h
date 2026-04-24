#ifndef FFMPEGVIDEOREADER_H
#define FFMPEGVIDEOREADER_H

#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <functional>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class FFmpegVideoReader
{
public:
    FFmpegVideoReader() = default;
    ~FFmpegVideoReader();

    bool open(const std::string& path, float fps);
    void close();

    // Read a single frame by index
    bool readFrame(int frameIndex, cv::Mat& outMat);
    bool readFrame_debug(int frameIndex, cv::Mat& outMat);

    // Read a range of frames sequentially
    bool readRange(int startFrame, int endFrame,
                   std::function<void(const cv::Mat&, int)> callback);

    int    totalFrames() const;
    double fps() const;
    bool   isOpen() const { return _isOpen; }

private:
    cv::Mat AVFrameToMat(AVFrame* frame);

    AVFormatContext* _formatCtx  = nullptr;
    AVCodecContext*  _codecCtx   = nullptr;
    AVStream*        _stream     = nullptr;
    AVFrame*         _frame      = nullptr;
    AVPacket*        _packet     = nullptr;
    AVRational       _timeBase   = {0, 1};
    AVRational       _fps        = {0, 1};
    int              _streamIndex = -1;
    bool             _isOpen     = false;
};

#endif // FFMPEGVIDEOREADER_H
