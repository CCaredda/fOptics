#include "FFmpegVideoReader.h"
#include <QDebug>

FFmpegVideoReader::~FFmpegVideoReader()
{
    close();
}

bool FFmpegVideoReader::open(const std::string& path, float fps)
{
    _fps = av_d2q((double)fps, 100000);

    _formatCtx = avformat_alloc_context();
    if (avformat_open_input(&_formatCtx, path.c_str(), nullptr, nullptr) < 0) {
        qWarning() << "[open] Could not open file:" << path.c_str();
        return false;
    }

    if (avformat_find_stream_info(_formatCtx, nullptr) < 0) {
        qWarning() << "[open] Could not find stream info";
        return false;
    }

    _streamIndex = -1;
    for (unsigned int i = 0; i < _formatCtx->nb_streams; i++) {
        if (_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            _streamIndex = i;
            break;
        }
    }

    if (_streamIndex < 0) {
        qWarning() << "[open] No video stream found";
        return false;
    }

    _timeBase = _formatCtx->streams[_streamIndex]->time_base;

    const AVCodec* codec = avcodec_find_decoder(_formatCtx->streams[_streamIndex]->codecpar->codec_id);
    if (!codec) {
        qWarning() << "[open] Codec not found";
        return false;
    }

    _codecCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(_codecCtx, _formatCtx->streams[_streamIndex]->codecpar);

    if (avcodec_open2(_codecCtx, codec, nullptr) < 0) {
        qWarning() << "[open] Could not open codec";
        return false;
    }

    _packet = av_packet_alloc();
    _frame  = av_frame_alloc();

    _isOpen = true;

    qDebug() << "[open] Opened:" << path.c_str()
             << "fps=" << av_q2d(_fps)
             << "timeBase=" << _timeBase.num << "/" << _timeBase.den
             << "size=" << _codecCtx->width << "x" << _codecCtx->height;

    return true;
}
void FFmpegVideoReader::close()
{
    if (_frame)     { av_frame_free(&_frame);    _frame     = nullptr; }
    if (_packet)    { av_packet_free(&_packet);  _packet    = nullptr; }
    if (_codecCtx)  { avcodec_free_context(&_codecCtx); _codecCtx = nullptr; }
    if (_formatCtx) { avformat_close_input(&_formatCtx); _formatCtx = nullptr; }
    _stream      = nullptr;
    _streamIndex = -1;
    _isOpen      = false;
}

bool FFmpegVideoReader::readFrame(int frameIndex, cv::Mat& outMat)
{
    if (!_isOpen) return false;

    int64_t targetPts = av_rescale_q(frameIndex,
                                     av_inv_q(_fps),
                                     _timeBase);

    av_seek_frame(_formatCtx, _streamIndex, targetPts, AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(_codecCtx);

    while (av_read_frame(_formatCtx, _packet) >= 0)
    {
        if (_packet->stream_index != _streamIndex) {
            av_packet_unref(_packet);
            continue;
        }

        if (avcodec_send_packet(_codecCtx, _packet) < 0) {
            av_packet_unref(_packet);
            continue;
        }

        while (avcodec_receive_frame(_codecCtx, _frame) == 0)
        {
            int64_t pts = _frame->best_effort_timestamp;
            int currentFrame = (int)av_rescale_q(pts, _timeBase,
                                                  av_inv_q(_fps));

            if (currentFrame >= frameIndex)
            {
                outMat = AVFrameToMat(_frame);
                av_frame_unref(_frame);
                av_packet_unref(_packet);
                return !outMat.empty();
            }

            av_frame_unref(_frame);
        }

        av_packet_unref(_packet);
    }

    return false;
}


bool FFmpegVideoReader::readFrame_debug(int frameIndex, cv::Mat& outMat)
{
    if (!_isOpen) {
        qWarning() << "[readFrame] Reader is not open";
        return false;
    }

    if (_fps.num == 0 || _fps.den == 0) {
        qWarning() << "[readFrame] Invalid FPS:" << _fps.num << "/" << _fps.den;
        return false;
    }

    int64_t targetPts = av_rescale_q(frameIndex,
                                     av_inv_q(_fps),
                                     _timeBase);

    qDebug() << "[readFrame] Seeking to frame" << frameIndex
             << "targetPts=" << targetPts
             << "timeBase=" << _timeBase.num << "/" << _timeBase.den
             << "fps=" << _fps.num << "/" << _fps.den;

    int seekRet = av_seek_frame(_formatCtx, _streamIndex,
                                targetPts, AVSEEK_FLAG_BACKWARD);
    if (seekRet < 0) {
        qWarning() << "[readFrame] Seek failed, ret=" << seekRet;
        return false;
    }

    avcodec_flush_buffers(_codecCtx);

    int packetCount = 0;
    int frameCount  = 0;

    while (av_read_frame(_formatCtx, _packet) >= 0)
    {
        if (_packet->stream_index != _streamIndex) {
            av_packet_unref(_packet);
            continue;
        }

        packetCount++;

        int sendRet = avcodec_send_packet(_codecCtx, _packet);
        if (sendRet < 0) {
            qWarning() << "[readFrame] avcodec_send_packet failed ret=" << sendRet;
            av_packet_unref(_packet);
            continue;
        }

        int recvRet;
        while ((recvRet = avcodec_receive_frame(_codecCtx, _frame)) == 0)
        {
            frameCount++;

            int64_t pts = _frame->best_effort_timestamp;

            // Fallback if best_effort_timestamp is invalid
            if (pts == AV_NOPTS_VALUE)
                pts = _frame->pts;
            if (pts == AV_NOPTS_VALUE)
                pts = _frame->pkt_dts;

            int currentFrame = (int)av_rescale_q(pts, _timeBase, av_inv_q(_fps));

            qDebug() << "[readFrame] decoded frame" << frameCount
                     << "pts=" << pts
                     << "currentFrame=" << currentFrame
                     << "target=" << frameIndex;

            if (currentFrame >= frameIndex)
            {
                outMat = AVFrameToMat(_frame);
                av_frame_unref(_frame);
                av_packet_unref(_packet);

                if (outMat.empty()) {
                    qWarning() << "[readFrame] AVFrameToMat returned empty mat";
                    return false;
                }

                qDebug() << "[readFrame] Success, frame" << currentFrame
                         << "size:" << outMat.cols << "x" << outMat.rows;
                return true;
            }

            av_frame_unref(_frame);
        }

        if (recvRet != AVERROR(EAGAIN) && recvRet != AVERROR_EOF) {
            qWarning() << "[readFrame] avcodec_receive_frame error ret=" << recvRet;
        }

        av_packet_unref(_packet);

        // Safety limit — avoid infinite loop
        if (packetCount > 500) {
            qWarning() << "[readFrame] Exceeded packet limit, giving up";
            break;
        }
    }

    qWarning() << "[readFrame] Frame not found. packets read=" << packetCount
               << "frames decoded=" << frameCount;
    return false;
}


bool FFmpegVideoReader::readRange(int startFrame, int endFrame,
                                  std::function<void(const cv::Mat&, int)> callback)
{
    if (!_isOpen) return false;

    int warmupFrames = 30;
    int seekFrame    = std::max(0, startFrame - warmupFrames);

    double targetSeconds = (double)seekFrame / av_q2d(_fps);
    int64_t targetPts    = (int64_t)(targetSeconds / av_q2d(_timeBase));

    av_seek_frame(_formatCtx, _streamIndex, targetPts, AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(_codecCtx);

    int  currentFrame   = -1; // will be calibrated on first decoded frame
    bool calibrated     = false;
    int  count          = 0;
    bool done           = false;

    auto processFrame = [&]() {
        // Calibrate ONCE using PTS of the very first decoded frame
        if (!calibrated) {
            int64_t pts = _frame->best_effort_timestamp;
            if (pts == AV_NOPTS_VALUE) pts = _frame->pts;
            if (pts == AV_NOPTS_VALUE) return; // skip until we have a valid PTS

            // Use PTS only here, just to anchor the counter
            currentFrame = (int)std::round(pts * av_q2d(_timeBase) * av_q2d(_fps));
            calibrated   = true;
        }

        if (currentFrame > endFrame) {
            done = true;
            return;
        }

        if (currentFrame >= startFrame) {
            cv::Mat mat = AVFrameToMat(_frame);
            if (!mat.empty()) {
                callback(mat, currentFrame);
                count++;
            }
        }

        currentFrame++; // pure increment after calibration — no more PTS math
    };

    while (!done && av_read_frame(_formatCtx, _packet) >= 0)
    {
        if (_packet->stream_index != _streamIndex) {
            av_packet_unref(_packet);
            continue;
        }

        avcodec_send_packet(_codecCtx, _packet);

        while (avcodec_receive_frame(_codecCtx, _frame) == 0) {
            processFrame();
            av_frame_unref(_frame);
        }

        av_packet_unref(_packet);
    }

    // Flush decoder
    if (!done) {
        avcodec_send_packet(_codecCtx, nullptr);
        while (avcodec_receive_frame(_codecCtx, _frame) == 0) {
            processFrame();
            av_frame_unref(_frame);
        }
    }

    qDebug() << "[readRange] delivered:" << count
             << "expected:" << (endFrame - startFrame + 1)
             << "calibrated start frame:" << (calibrated ? currentFrame - count - (startFrame > 0 ? warmupFrames : 0) : -1);

    return count > 0;
}


int FFmpegVideoReader::totalFrames() const
{
    if (!_isOpen) return 0;
    return (int)_stream->nb_frames;
}

double FFmpegVideoReader::fps() const
{
    if (!_isOpen || _fps.den == 0) return 0.0;
    return av_q2d(_fps);
}

cv::Mat FFmpegVideoReader::AVFrameToMat(AVFrame* frame)
{
    int w = frame->width, h = frame->height;
    cv::Mat mat(h, w, CV_8UC3);

    SwsContext* sws = sws_getContext(
        w, h, (AVPixelFormat)frame->format,
        w, h, AV_PIX_FMT_BGR24,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (!sws) return cv::Mat();

    uint8_t* dst[1]     = { mat.data };
    int      dstStep[1] = { (int)mat.step };
    sws_scale(sws, frame->data, frame->linesize, 0, h, dst, dstStep);
    sws_freeContext(sws);

    return mat;
}