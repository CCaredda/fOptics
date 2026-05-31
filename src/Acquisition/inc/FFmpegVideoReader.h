/**
 * @file FFmpegVideoReader.h
 *
 * @brief Real time motion compensation of RGB images.
 * This class serves as a wrapper providing direct access to the code developed by Michaël Sdika : https://doi.org/10.1016/j.media.2018.12.005.
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */


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

	/** Open video flux using ffmpeg */
    bool open(const std::string& path, float fps);
	
	/** Close video flux */
    void close();

	/** Request stop reading frames */
    void stop_reading() {_M_stop = true;}

    /** Read a single frame by index */
    bool readFrame(int frameIndex, cv::Mat& outMat);

    /** Read a range of frames sequentially */
    bool readRange(int startFrame, int endFrame,
                   std::function<void(const cv::Mat&, int)> callback);

    /** Get total number of frames in the video */
    int    totalFrames() const;

    /** Get video framerate in frame per second */
    double fps() const;

    /** Check if video is openned */
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

    bool             _M_stop     = false;
};

#endif // FFMPEGVIDEOREADER_H
