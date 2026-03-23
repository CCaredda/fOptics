/**
 * @file HCreateVideo.h
 *
 * @brief This class is the graphical interface used to send parameters used in the edition of videos.
 * Videos are created in the PAnalyse class.
 *
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef HCREATEVIDEO_H
#define HCREATEVIDEO_H

#include <QWidget>

namespace Ui {
class HCreateVideo;
}

class HCreateVideo : public QWidget
{
    Q_OBJECT

public:
    explicit HCreateVideo(QWidget *parent = 0);
    ~HCreateVideo();

signals:
    /** Request the creation of video of results */
    void VideoCreationRequired();
    /** Send the number of frames of the video */
    void newVideo_nb_Frames(double);
    /** Send video framerate */
    void newVideoFramerate(double);

private:
    Ui::HCreateVideo *ui;
};

#endif // HCREATEVIDEO_H
