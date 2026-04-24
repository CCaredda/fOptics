#include "AloadDatas.h"
#include <QMessageBox>

AloadDatas::AloadDatas(QObject *parent) : QThread(parent)
{
    //mode video reading (Qt, opencv, ffmpeg)
    _M_mode_video_reading = "ffmpeg";
    //moveToThread(this);

    //Frame per second
    _M_FPS=0;

    _M_tot_frames_per_video.clear();
    _M_tot_frames=0;
    _M_start_frame=0;
    _M_last_frame=0;
    _M_video_name="";
    _M_video_dir="";
    _M_mode_video=false;

    _M_frame_idx_loaded = false;
    _M_img_files.clear();

    _M_video_files.clear();
    _M_idx_video.clear();
    _M_first_frames_video.clear();
    _M_last_frames_video.clear();
    _M_video_id = 0;


    _M_camera_name = "";
    _M_Light_source_type = "";


    //Connect video processor
    connect(&_M_video_processor,SIGNAL(newImageAcquired(_Processed_img)),this,SIGNAL(newImageAcquired(_Processed_img)));
    connect(&_M_video_processor,SIGNAL(newImgSend(int,int)),this,SIGNAL(newImgSend(int,int)));
    connect(&_M_video_processor,SIGNAL(allFramesProcessed()),this,SLOT(LoadDatas()));
}


AloadDatas::~AloadDatas()
{
    if (isRunning())
    {
        requestInterruption();
        wait();
    }
}


// Load all datas
void AloadDatas::LoadDatas()
{
    this->start();
}

//Protected function (run the code in a different thread)
void AloadDatas::run()
{
    if(_M_mode_video)
    {
        if(_M_mode_video_reading=="Qt")
            _LoadDatasFromVideoFile_Qt();
        if(_M_mode_video_reading=="opencv")
            _LoadDatasFromVideoFile_Opencv();
        if(_M_mode_video_reading=="ffmpeg")
            _LoadDatasFromVideoFile_ffmpeg();
    }
    else
        _LoadDatasFromImgFiles();

}

//read img from directory containing img files (.tiff or .png)
void AloadDatas::_LoadDatasFromImgFiles()
{
    QString ext = _M_img_files[0].mid(_M_img_files[0].size()-4,4);
    QStringList result;

    int nbFrame = _M_tot_frames;

    _Processed_img img;
    for(int current_frame=_M_start_frame;current_frame<_M_last_frame;current_frame++)
    {

        // filename
        result.clear();
        result          = _M_img_files.filter(QString::number(current_frame)+ext);
        QString path    = _M_video_dir+result[0];

        img.img         = imread(path.toStdString());
        img.thread_id   = current_frame-_M_start_frame;


        emit newImageAcquired(img);
        emit newImgSend(current_frame-_M_start_frame,nbFrame);
    }

}


//read img from directory containing a video file
void AloadDatas::_LoadDatasFromVideoFile_Qt()
{
    qDebug()<<"[AloadDatas::_LoadDatasFromVideoFile_Qt] video ID: "<< _M_video_id;
    qDebug()<<"[AloadDatas::_LoadDatasFromVideoFile_Qt] nb videos: "<< _M_idx_video.size();

    if (_M_first_frames_video.empty())
        return;

    if (_M_video_id >= _M_idx_video.size())
        return;

    qDebug()<<"[AloadDatas::_LoadDatasFromVideoFile_Qt] ask video reading";


    qDebug()<<"[AloadDatas::_LoadDatasFromVideoFile_Qt] Load video mode Qt";
    _M_video_processor.init_frame_count();
    _M_video_processor.loadVideo(_M_video_dir + _M_video_files[_M_idx_video[_M_video_id]]);
    _M_video_processor.set_Start_pos_ms(qint64((_M_first_frames_video[_M_video_id]/_M_FPS)*1000));
    _M_video_processor.set_Start_frame(_M_first_frames_video[_M_video_id]);
    _M_video_processor.set_Last_frame(_M_last_frames_video[_M_video_id]);
    _M_video_processor.startReadingFrames();

    _M_video_id++;

}

void AloadDatas::_LoadDatasFromVideoFile_ffmpeg()
{
    qDebug()<<"[AloadDatas::_LoadDatasFromVideoFile_ffmpeg] video ID: "<< _M_video_id;
    qDebug()<<"[AloadDatas::_LoadDatasFromVideoFile_ffmpeg] nb videos: "<< _M_idx_video.size();

    if (_M_first_frames_video.empty())
        return;

    if (_M_video_id >= _M_idx_video.size())
        return;

    qDebug()<<"[AloadDatas::_LoadDatasFromVideoFile_ffmpeg] ask video reading";

    int id_frame = 0;
    for(int id_video = 0;id_video<_M_idx_video.size();id_video++)
    {

        QString video_file = _M_video_dir + _M_video_files[_M_idx_video[id_video]];

        int startFrame = _M_first_frames_video[id_video];
        int endFrame   = _M_last_frames_video[id_video];

        qDebug()<<"[AloadDatas::_LoadDatasFromVideoFile_ffmpeg] tot_frames: "<<_M_tot_frames;
        qDebug()<<"[AloadDatas::_LoadDatasFromVideoFile_ffmpeg] startFrame: "<<startFrame;
        qDebug()<<"[AloadDatas::_LoadDatasFromVideoFile_ffmpeg] endFrame: "<<endFrame;
        qDebug()<<"[AloadDatas::_LoadDatasFromVideoFile_ffmpeg] endFrame-startFrame: "<<endFrame-startFrame;





        cv::Mat frame;

        FFmpegVideoReader reader;
        reader.open(video_file.toStdString(),(float)_M_FPS);
        qDebug()<<"[AloadDatas::_LoadDatasFromVideoFile_ffmpeg] reader is open "<<reader.isOpen();



        std::map<int, int> frameCount; // frameIndex -> how many times received


        reader.readRange(startFrame, endFrame,
                         [this, &id_frame, &frameCount](const cv::Mat& mat, int index)
                         {
                             frameCount[index]++;

                             _Processed_img process_img;
                             process_img.img = mat.clone();
                             process_img.thread_id = id_frame;

                             emit newImageAcquired(process_img);
                             emit newImgSend(id_frame,_M_tot_frames);
                             id_frame++;
                         }
                         );

        reader.close();


        int duplicates = 0;
        for (auto& [idx, cnt] : frameCount) {
            if (cnt > 1) {
                qDebug() << "Duplicate frame index:" << idx << "received" << cnt << "times";
                duplicates += cnt - 1;
            }
        }
        qDebug() << "Total duplicates:" << duplicates;
        qDebug() << "Unique frames received:" << frameCount.size();

        qDebug()<<"AloadDatas::_LoadDatasFromVideoFile_ffmpeg video "<<id_video<<" "<<id_frame<<"/"<<_M_tot_frames;
    }
}

void AloadDatas::_LoadDatasFromVideoFile_Opencv()
{
    qDebug()<<"[AloadDatas::_LoadDatasFromVideoFile_Opencv] video ID: "<< _M_video_id;
    qDebug()<<"[AloadDatas::_LoadDatasFromVideoFile_Opencv] nb videos: "<< _M_idx_video.size();

    if (_M_first_frames_video.empty())
        return;

    if (_M_video_id >= _M_idx_video.size())
        return;

    qDebug()<<"[AloadDatas::_LoadDatasFromVideoFile_Opencv] ask video reading";

    int id_frame = 0;
    for(int id_video = 0;id_video<_M_idx_video.size();id_video++)
    {


        cv::VideoCapture cap((_M_video_dir + _M_video_files[_M_idx_video[id_video]]).toStdString(), preferredBackendForFile(_M_video_dir + _M_video_files[_M_idx_video[id_video]])); // On Windows, FFmpeg backend is often the best if available
        if (!cap.isOpened()) {
            qDebug()<<"[AloadDatas::_LoadDatasFromVideoFile_Opencv] Error: cannot open video";
            return;
        }

        int startFrame = _M_first_frames_video[id_video];
        int endFrame   = _M_last_frames_video[id_video];

        cv::Mat frame;

        cap.set(cv::CAP_PROP_POS_FRAMES, startFrame); // seek ONCE

        for (int i = startFrame; i <= endFrame; ++i)
        {
            cap.set(cv::CAP_PROP_POS_FRAMES, i);
            // int currentPos = (int)cap.get(cv::CAP_PROP_POS_FRAMES);

            // // Re-seek only if drifted
            // if (currentPos != i) {
            //     qDebug() << "Drift detected, re-seeking to" << i;
            //     cap.set(cv::CAP_PROP_POS_FRAMES, i);
            // }

            if (!cap.read(frame) || frame.empty()) break;

            _Processed_img process_img;
            process_img.img = frame;
            process_img.thread_id = id_frame;

            emit newImageAcquired(process_img);
            emit newImgSend(id_frame,_M_tot_frames);
            id_frame++;
        }

        cap.release();
    }

    qDebug()<<"[_LoadDatasFromVideoFile_Opencv] frame idx "<<id_frame<<" tot frames "<<_M_tot_frames;

}

//Set video path
void AloadDatas::setVideoPath(QString v)
{
    //Get info from path
    QFileInfo fi(v);
    QDir currentDir(fi.dir());


    //Set video name
    _M_video_name   = v;
    _M_video_dir    = currentDir.path()+"/";


    //Check if acquisition info file have all required entries
    QStringList acquisition_info_entries = check_acquisition_info_file(_M_video_dir);
    if(!acquisition_info_entries.isEmpty())
    {
        QMessageBox::critical(nullptr, "Missing entries in Acquisition_info.txt", "The following parameters are missing:\n" + acquisition_info_entries.join("\n"));
        QCoreApplication::quit();
        return;
    }

    //Get file extension to determine if it's a video or a image file
    QString ext = v.mid(v.size()-4,4);

    //define mode video
    _M_mode_video   = (ext==".avi") || (ext==".mpg") || (ext==".mp4") || (ext==".MOV") || (ext==".mkv")  ? true : false;


    //Camera name
    _M_camera_name = LoadCameraName(_M_video_dir);

    //Light source type
    _M_Light_source_type = LoadLightSourceType(_M_video_dir);



    //request video informations
    _getVideoInformations();


    //Request first img
    onRequestNewImage();
}

//Request first img
Mat AloadDatas::RequestFirstImg()
{

    int video_id = 0;
    Mat dst;

    if(_M_mode_video)
    {
        if (_M_first_frames_video.empty())
            return dst;

        if(_M_mode_video_reading == "opencv")
            dst = _RequestFirstImg_opencv(_M_video_dir + _M_video_files[_M_idx_video[video_id]]);
        if(_M_mode_video_reading == "Qt")
            dst = _RequestFirstImg_Qt(_M_video_dir + _M_video_files[_M_idx_video[video_id]]);
        if(_M_mode_video_reading == "ffmpeg")
            dst = _RequestFirstImg_ffmpeg(_M_video_dir + _M_video_files[_M_idx_video[video_id]]);
    }
    //Img files mode
    else
    {
        //Get the extension of the image
        QString ext = _M_video_name.mid(_M_video_name.size()-4,4);

        QStringList result ;

        //Find id of first img
        result  = _M_img_files.filter(QString::number(_M_start_frame)+ext);

        if(result.empty())
        {
            _M_is_video_loaded=false;
            emit newMessage("Problem loading img file");
            return dst;
        }

        QString path = _M_video_dir+result[0];
        _M_is_video_loaded=true;


        try
        {

            dst = imread(path.toStdString());

        }
        catch( cv::Exception)
        {
            _M_is_video_loaded=false;
            qDebug()<<"exception catch";
            emit newMessage("Problem loading img file");
        }
    }

    return dst;

}

Mat AloadDatas::_RequestFirstImg_Qt(QString video)
{
    return _RequestFirstImg_opencv(video);
}

Mat AloadDatas::_RequestFirstImg_ffmpeg(QString video)
{
    Mat dst;
    FFmpegVideoReader reader;

    if (!reader.open(video.toStdString(),(float)_M_FPS))
        return dst;

    qDebug()<<"[AloadDatas::_LoadDatasFromVideoFile_ffmpeg] reader is open "<<reader.isOpen();

    reader.readFrame(_M_first_frames_video[0],dst);
    return dst;
}

Mat AloadDatas::_RequestFirstImg_opencv(QString video)
{
    Mat dst;
    try
    {
        VideoCapture capture;
        capture.release();
        capture.open(video.toStdString(),preferredBackendForFile(video));
        qDebug()<<"[AloadDatas::RequestFirstImg] read video "<<video;

        if (!capture.isOpened())
        {
            qDebug()<<"[AloadDatas::RequestFirstImg] cannot open video";
            emit newMessage("Error during loading video");
            _M_is_video_loaded=false;
            return dst;
        }


        /// THIS IS NOT WORKING!!
        //set the video player at the frame pos: _M_start_frame
        capture.set(CAP_PROP_POS_FRAMES,_M_first_frames_video[0]);
        qDebug()<<"[AloadDatas::RequestFirstImg] get frame "<<_M_first_frames_video[0];
        capture >> dst;

        //            qDebug()<<"[AloadDatas::RequestFirstImg] First img size "<<dst.rows<<" "<<dst.cols;
        //            qDebug()<<"[AloadDatas::RequestFirstImg] Total nb of images "<<capture.get(CAP_PROP_FRAME_COUNT);
        //            qDebug()<<"[AloadDatas::RequestFirstImg] frame pos "<<capture.get(CAP_PROP_POS_FRAMES);



        // //// DO THIS INSTEAD

        // qDebug()<<"[AloadDatas::RequestFirstImg] id of the first frame to read "<<_M_first_frames_video[video_id];
        // //Read all images until the first frame is reached
        // for(int i=0;i<_M_first_frames_video[video_id]+1;i++)
        //     capture >> dst;




        capture.release();
        _M_is_video_loaded=true;
    }
    catch( cv::Exception)
    {
        _M_is_video_loaded=false;
        qDebug()<<"exception catch";
        emit newMessage("Codec Problem, Rewrite the video with the correct codec");
    }

}


void AloadDatas::onRequestNewImage()
{
    Mat dst = RequestFirstImg();
    if(!dst.empty())
        emit requestPreProcessing(dst);

}






//Get video informations
void AloadDatas::_getVideoInformations()
{
    qDebug()<<"[AloadDatas::_getVideoInformations]";

    if(_M_mode_video)
    {
        QDir currentDir(_M_video_dir);
        QStringList fileName ;
        QString ext = _M_video_name.mid(_M_video_name.size()-4,4);

        _M_video_files.clear();
        fileName.append("*"+ext);
        _M_video_files = currentDir.entryList(QStringList(fileName),
                                              QDir::Files | QDir::NoSymLinks);

        _M_idx_video.clear();
        _M_idx_video.push_back(0);

        try
        {
            qDebug()<<"[AloadDatas::_getVideoInformations] Video name: "<<_M_video_name;
            VideoCapture capture;
            capture.open(_M_video_name.toStdString(), preferredBackendForFile(_M_video_name));
            if (!capture.isOpened())
            {
                qDebug()<<"[AloadDatas::_getVideoInformations] error when loading video";
                emit newMessage("Error when loading video");
                _M_is_video_loaded=false;
                emit newStatut(false);
                return;
            }

            //Video can be correctly openned
            _M_is_video_loaded  =true;

            //get tot number of frames
            _M_tot_frames_per_video = LoadTotalFrameCountInfo(_M_video_dir);
            _M_tot_frames = _M_tot_frames_per_video[0];

            qDebug()<<"[AloadDatas::_getVideoInformations] _M_tot_frames_per_video"<<_M_tot_frames_per_video;

        }
        catch( cv::Exception)
        {
            _M_is_video_loaded=false;
            qDebug()<<"exception catch";
            emit newMessage("Codec Problem, Rewrite the video with the correct codec");
        }
    }
    else
    {
        QDir currentDir(_M_video_dir);
        QStringList fileName ;
        QString ext = _M_video_name.mid(_M_video_name.size()-4,4);

        _M_img_files.clear();
        fileName.append("*"+ext);
        _M_img_files = currentDir.entryList(QStringList(fileName),
                                            QDir::Files | QDir::NoSymLinks);

        _M_tot_frames   = _M_img_files.size();

    }

    //Load frame rate info
    _M_FPS          = LoadFrameRateInfo(_M_video_dir);

    //Load process Time
    QVector<int> process_id;
    LoadTimeProcessInfo(_M_video_dir,process_id,_M_tot_frames);

    //Load analysis type
    QVector<int>  analysis_choice= LoadAnalysisType(_M_video_dir);
    emit requestAnalysisChoose(analysis_choice);


    qDebug()<<"[AloadDatas::_getVideoInformations] FPS: "<<_M_FPS;
    qDebug()<<"[AloadDatas::_getVideoInformations] tot frames: "<<_M_tot_frames;
    qDebug()<<"[AloadDatas::_getVideoInformations] tot frames per video: "<<_M_tot_frames_per_video;
    qDebug()<<"[AloadDatas::_getVideoInformations] analysis choice: "<<analysis_choice;

    emit newStatut(true);
    emit newFrameRate((double)_M_FPS);
    //    if(!process_id.empty())
    //        emit newProcessid(process_id,_M_FPS);
    emit newProcessid(process_id,_M_FPS);
}


//Request start and last acquisition frames depending on the analysis
//-1: no analysis
//0 : task-based
//1: resting state
//2: Impulsion
void AloadDatas::getStartEndAnalysisFrames(int analysis)
{

    if(_M_frame_idx_loaded)
        return;
    _M_frame_idx_loaded = true;
    LoadStartEndAnalysisFrames(_M_video_dir,analysis,_M_start_frame,_M_last_frame);

    qDebug()<<"[AloadDatas::getStartEndAnalysisFrames] analysis start frame "<<_M_start_frame;
    qDebug()<<"[AloadDatas::getStartEndAnalysisFrames] analysis end frame "<<_M_last_frame;
    emit newFirstLastAnalysisFrames(_M_start_frame,_M_last_frame);

    //Calculate tot frame
    _M_tot_frames = _M_last_frame - _M_start_frame;
    _M_video_processor.set_Total_frame_number(_M_tot_frames);

    //Init video ID
    _M_video_id = 0;


    _M_idx_video.clear();
    _M_first_frames_video.clear();
    _M_last_frames_video.clear();

    if(_M_mode_video)
    {
        qDebug()<<"[AloadDatas::getStartEndAnalysisFrames] video files: "<<_M_video_files;

        //If there are more than one video
        if(_M_video_files.size()>1)
        {
            //Calculate cumulative sum of total number of frames in the video
            QVector<int> cumsum_tot_frame;
            int sum = 0;
            for(int i=0; i<_M_tot_frames_per_video.size();i++)
            {
                sum += _M_tot_frames_per_video[i];
                cumsum_tot_frame.push_back(sum);
            }

            qDebug()<<"[AloadDatas::getStartEndAnalysisFrames] _M_tot_frames_per_video "<< _M_tot_frames_per_video;
            qDebug()<<"[AloadDatas::getStartEndAnalysisFrames] cumsum_tot_frame "<< cumsum_tot_frame;


            //Check the video id for the starting frame
            int start_video_id = 0;
            for(int i=0; i<cumsum_tot_frame.size();i++)
            {
                if(_M_start_frame<=cumsum_tot_frame[i])
                {
                    start_video_id = i;
                    break;
                }
            }

            qDebug()<<"Start frame "<<_M_start_frame;
            qDebug()<<"Start frame video id: "<<start_video_id<<" video name"<<_M_video_files[start_video_id];

            //Check the video id for the last frame
            int last_video_id = 0;
            for(int i=0; i<cumsum_tot_frame.size();i++)
            {
                if(_M_last_frame<=cumsum_tot_frame[i])
                {
                    last_video_id = i;
                    break;
                }
            }

            qDebug()<<"Last frame "<<_M_last_frame;
            qDebug()<<"Last frame video id: "<<last_video_id<<" video name"<<_M_video_files[last_video_id];


            //set index of video to load
            _M_tot_frames_per_video.clear();
            if(start_video_id == last_video_id)
            {
                //Set id of the video
                _M_idx_video.push_back(start_video_id);

                //Set total number, first and last frame of the video
                _M_first_frames_video.push_back(_M_start_frame);
                _M_last_frames_video.push_back(_M_last_frame);
                _M_tot_frames_per_video.push_back(_M_last_frame - _M_start_frame);
            }
            else
            {
                //If the acquisition is split into two videos
                _M_idx_video.push_back(start_video_id);
                _M_idx_video.push_back(last_video_id);

                //Id of the first video
                _M_first_frames_video.push_back(_M_start_frame);
                _M_last_frames_video.push_back(_M_tot_frames_per_video[start_video_id]-1);
                //_M_tot_frames += _M_tot_frames_per_video[start_video_id]-1 - _M_start_frame;

                //ID of the second video
                _M_first_frames_video.push_back(0);
                _M_last_frames_video.push_back(_M_last_frame - _M_tot_frames_per_video[start_video_id]-1);
            }
        }
        else
        {
            //If there is only one video in the directory
            _M_idx_video.push_back(0);
            _M_first_frames_video.push_back(_M_start_frame);
            _M_last_frames_video.push_back(_M_last_frame);
            _M_tot_frames_per_video.clear();
            _M_tot_frames_per_video.push_back(_M_last_frame - _M_start_frame);

            _M_tot_frames = _M_last_frame - _M_start_frame;
        }
    }
    else
    {
        _M_tot_frames = _M_last_frame - _M_start_frame;
    }


    qDebug()<<"IDX video"<<_M_idx_video;
    qDebug()<<"First frames per video"<<_M_first_frames_video;
    qDebug()<<"Last frames per video"<<_M_last_frames_video;
    qDebug()<<"tot frames: "<<_M_tot_frames;


    //Get first first and display it
    onRequestNewImage();
}

