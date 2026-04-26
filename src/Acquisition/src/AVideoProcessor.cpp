#include "AVideoProcessor.h"
#include <QImage>
#include <QDebug>

AVideoProcessor::AVideoProcessor(QObject *parent)
    : QObject(parent),
      _M_mediaPlayer(new QMediaPlayer(this)),
      _M_videoSink(new QVideoSink(this)),
      _M_frameCount(0),
      _M_video_framecount(0),
      _M_starting_frame_count(0),
      _M_last_frame(0),
      _M_start_frame(0),
      _M_tot_frames(0),
      _M_start_pos_ms(0)
{
    _M_mediaPlayer->setVideoSink(_M_videoSink);
    connect(_M_videoSink, &QVideoSink::videoFrameChanged, this, &AVideoProcessor::onVideoFrameChanged);

    //Set starting position
    connect(_M_mediaPlayer,&QMediaPlayer::mediaStatusChanged,this,[=](QMediaPlayer::MediaStatus status)
    {
        if(status==QMediaPlayer::MediaStatus::BufferedMedia)
        {
            _M_mediaPlayer->setPosition(_M_start_pos_ms);
        }
    });
}

AVideoProcessor::~AVideoProcessor()
{
    _M_mediaPlayer->stop();
}

void AVideoProcessor::stop_reading()
{
    _M_mediaPlayer->stop();
}

void AVideoProcessor::loadVideo(const QString &filePath)
{
    _M_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
    _M_mediaPlayer->stop();
}


void AVideoProcessor::startReadingFrames()
{
    _M_starting_frame_count = 0;
    _M_video_framecount = 0;
    _M_mediaPlayer->play();
}

void AVideoProcessor::onVideoFrameChanged(const QVideoFrame &frame)
{
    // qDebug()<<"[AVideoProcessor::onVideoFrameChanged] On new frame pos"<< _M_mediaPlayer->position();
    // qDebug() << "Frame received, valid:" << frame.isValid()
    //          << "size:" << frame.size()
    //          << "framecount:" << _M_video_framecount;

    if (!frame.isValid())
    {
        qDebug()<<"[AVideoProcessor::onVideoFrameChanged] invalid frame";
        _M_mediaPlayer->stop();
        return;
    }

    if (_M_video_framecount > _M_last_frame - _M_start_frame)
    {
        qDebug()<<"[AVideoProcessor::onVideoFrameChanged] frame count > last frame - first frame"<< _M_start_frame << _M_last_frame <<_M_last_frame - _M_start_frame;
        _M_mediaPlayer->stop();
        return;
    }

    // if (_M_starting_frame_count < _M_start_frame)
    // {
    //     qDebug()<<"[AVideoProcessor::onVideoFrameChanged] Skipping frames "<<_M_starting_frame_count<< " "<<_M_start_frame;
    //     _M_starting_frame_count++;
    //     return;
    // }

    QImage image = frame.toImage();
    if (image.isNull()) {
        qWarning() << "Failed to convert QVideoFrame to QImage.";
        return;
    }

    // Convert QImage to cv::Mat
    cv::Mat mat;
    switch (image.format()) {
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
            mat = cv::Mat(image.height(), image.width(),
                          CV_8UC4,
                          const_cast<uchar*>(image.bits()),
                          image.bytesPerLine()).clone();
            cv::cvtColor(mat, mat, cv::COLOR_BGRA2BGR);
            break;

        case QImage::Format_RGB888:
            mat = cv::Mat(image.height(), image.width(),
                          CV_8UC3,
                          const_cast<uchar*>(image.bits()),
                          image.bytesPerLine()).clone();
            cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
            break;

        case QImage::Format_RGBA8888_Premultiplied:
            mat = cv::Mat(image.height(), image.width(),
                          CV_8UC4,
                          const_cast<uchar*>(image.bits()),
                          image.bytesPerLine()).clone();
            cv::cvtColor(mat, mat, cv::COLOR_RGBA2BGR);
            break;

        case QImage::Format_Grayscale8:
            mat = cv::Mat(image.height(), image.width(),
                          CV_8UC1,
                          const_cast<uchar*>(image.bits()),
                          image.bytesPerLine()).clone();
            break;



        default:
            qWarning() << "Unsupported QImage format:" << image.format();
            return;
    }

    _Processed_img process_img;
    process_img.img = mat;
    process_img.thread_id = _M_frameCount;
    emit newImageAcquired(process_img);
    emit newImgSend(_M_frameCount,_M_tot_frames);

    _M_frameCount++;
    _M_video_framecount++;

    if (_M_video_framecount > _M_last_frame - _M_start_frame)
    {
        qDebug()<<"[AVideoProcessor::onVideoFrameChanged] _M_video_framecount > _M_last_frame - _M_start_frame";
        _M_starting_frame_count = 0;
        _M_video_framecount = 0;

        _M_mediaPlayer->stop();
        emit allFramesProcessed();
    }
}
