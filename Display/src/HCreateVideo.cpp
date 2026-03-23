#include "HCreateVideo.h"
#include "ui_HCreateVideo.h"

HCreateVideo::HCreateVideo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HCreateVideo)
{
    ui->setupUi(this);

    //Create video
    connect(ui->_createVideo,SIGNAL(pressed()),this,SIGNAL(VideoCreationRequired()));

    //Video FPS
    ui->_nb_frames->setRange(5,10000,"frames");
    ui->_nb_frames->setValue(100);
    connect(ui->_nb_frames,SIGNAL(valueEdited(double)),this,SIGNAL(newVideo_nb_Frames(double)));


    //Video framerate
    ui->_framerate->setRange(1,30,"fps");
    ui->_framerate->setValue(5);
    connect(ui->_framerate,SIGNAL(valueEdited(double)),this,SIGNAL(newVideoFramerate(double)));
}

HCreateVideo::~HCreateVideo()
{
    delete ui;
}
