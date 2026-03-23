#include "AloadDatas.h"
#include <QMessageBox>

AloadDatas::AloadDatas(QObject *parent) : QThread(parent)
{
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

    //Simulate real camera
    _M_simulate_real_camera = false;

    //RGB mode / Hyperspectral camera mode
    _M_mode_RGB = true;
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

    if(_M_mode_RGB)
    {
        if(_M_mode_video)
            _LoadDatasFromVideoFile_Qt();
        else
            _LoadDatasFromImgFiles();
    }
    else
    {
        if(!_M_mode_video)
        {
            _Load_HS_DatasFromImgFiles();
        }
    }
}

//read img from directory containing img files (.tiff or .png)
void AloadDatas::_LoadDatasFromImgFiles()
{
    QString ext = _M_img_files[0].mid(_M_img_files[0].size()-4,4);
    QStringList result;

    int nbFrame = _M_tot_frames;

    QElapsedTimer timer;
    int temporal_period = 1000/_M_FPS;

    // QVector<QVector<double> > signal;

    _Processed_img img;
    for(int current_frame=_M_start_frame;current_frame<_M_last_frame;current_frame++)
    {
        timer.start();
        // filename
        result.clear();
        result          = _M_img_files.filter(QString::number(current_frame)+ext);
        QString path    = _M_video_dir+result[0];

        img.img         = imread(path.toStdString());
        img.thread_id   = current_frame-_M_start_frame;

        //simulate the frame rate acquis (if real time mode)
        int elapsed_time    = timer.elapsed();
        if(_M_simulate_real_camera)
        {
            if(elapsed_time<temporal_period)
                msleep(temporal_period-elapsed_time);
        }


        // //TEMP
        // QVector<double> temp;
        // Vec3b px = img.img.at<Vec3b>(int(img.img.rows/2), int(img.img.cols/2));
        // temp.push_back(px[0]);
        // temp.push_back(px[1]);
        // temp.push_back(px[2]);
        // signal.push_back(temp);
        // //END TEMP

        emit newImageAcquired(img);
        emit newImgSend(current_frame-_M_start_frame,nbFrame);
    }

    // WriteTemporalVector("/home/caredda/temp/signal_LoadData.txt",signal);
}

void AloadDatas::_Load_HS_DatasFromImgFiles()
{
    qDebug()<<"AloadDatas::_Load_HS_DatasFromImgFiles 1";
    QString ext = _M_img_files[0].mid(_M_img_files[0].size()-4,4);
    QStringList result;


    int nbFrame         = _M_tot_frames;
    int temporal_period = 1000/_M_FPS;
    QElapsedTimer timer;
    _Processed_img img;
    for(int current_frame=_M_start_frame;current_frame<_M_last_frame;current_frame++)
    {
        timer.start();
        // filename
        result.clear();
        result  = _M_img_files.filter(QString::number(current_frame)+ext);
        QString path = _M_video_dir+result[0];

        #if (CV_VERSION_MAJOR >= 4)
            img.img         = imread(path.toStdString(),IMREAD_ANYDEPTH);

        #else
            img.img         = imread(path.toStdString(),CV_LOAD_IMAGE_ANYDEPTH);

        #endif

        img.thread_id   = current_frame-_M_start_frame;

        //simulate the frame rate acquis (if real time mode)
        int elapsed_time    = timer.elapsed();
        if(_M_simulate_real_camera)
        {
            if(elapsed_time<temporal_period)
                msleep(temporal_period-elapsed_time);
        }
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


    _M_video_processor.init_frame_count();


    _M_video_processor.loadVideo(_M_video_dir + _M_video_files[_M_idx_video[_M_video_id]]);

    _M_video_processor.set_Start_pos_ms(qint64((_M_first_frames_video[_M_video_id]/_M_FPS)*1000));
    _M_video_processor.set_Start_frame(_M_first_frames_video[_M_video_id]);
    _M_video_processor.set_Last_frame(_M_last_frames_video[_M_video_id]);

    _M_video_processor.startReadingFrames();

    _M_video_id++;

    // //Loop over video files
    // for(int video_id=0; video_id<_M_idx_video.size(); video_id++)
    // {
    //     _M_video_processor.loadVideo(_M_video_dir + _M_video_files[_M_idx_video[video_id]]);

    //     _M_video_processor.set_Start_frame(_M_first_frames_video[video_id]);
    //     _M_video_processor.set_Last_frame(_M_last_frames_video[video_id]);


    //     _M_video_processor.startReadingFrames();
    // }
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
    _M_mode_video   = (ext==".avi") || (ext==".mpg") || (ext==".mp4") || (ext==".MOV")  ? true : false;


    //Camera name
    _M_camera_name = LoadCameraName(_M_video_dir);

    //Light source type
    _M_Light_source_type = LoadLightSourceType(_M_video_dir);

    //Check if image have been acquired with a RGB camera or a hyperspectral camera
    // _M_mode_RGB = (fi.baseName().mid(0,3) == "RGB" || _M_mode_video) ? true : false;
    _M_mode_RGB = bool(LoadCameraType(_M_video_dir));


    emit newRGBCameraMode(_M_mode_RGB);





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
        try
        {
            VideoCapture capture;
            capture.release();
            capture.open((_M_video_dir + _M_video_files[_M_idx_video[video_id]]).toStdString());
            qDebug()<<"[AloadDatas::RequestFirstImg] read video "<<_M_video_dir + _M_video_files[_M_idx_video[0]];

            if (!capture.isOpened())
            {
                qDebug()<<"[AloadDatas::RequestFirstImg] cannot open video";
                emit newMessage("Error during loading video");
                _M_is_video_loaded=false;
                return dst;
            }


            /// THIS IS NOT WORKING!!
//            //set the video player at the frame pos: _M_start_frame
//            capture.set(CAP_PROP_POS_FRAMES,_M_first_frames_video[0]);
//            qDebug()<<"[AloadDatas::RequestFirstImg] get frame "<<_M_first_frames_video[0];
//            capture >> dst;

//            qDebug()<<"[AloadDatas::RequestFirstImg] First img size "<<dst.rows<<" "<<dst.cols;
//            qDebug()<<"[AloadDatas::RequestFirstImg] Total nb of images "<<capture.get(CAP_PROP_FRAME_COUNT);
//            qDebug()<<"[AloadDatas::RequestFirstImg] frame pos "<<capture.get(CAP_PROP_POS_FRAMES);



            //// DO THIS INSTEAD

            qDebug()<<"[AloadDatas::RequestFirstImg] id of the first frame to read "<<_M_first_frames_video[video_id];
            //Read all images until the first frame is reached
            for(int i=0;i<_M_first_frames_video[video_id]+1;i++)
                capture >> dst;


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
            if(_M_mode_RGB)
            {
                dst = imread(path.toStdString());
            }
            else
            {

            #if (CV_VERSION_MAJOR >= 4)
                dst = imread(path.toStdString(),IMREAD_ANYDEPTH);
            #else
                dst = imread(path.toStdString(),CV_LOAD_IMAGE_ANYDEPTH);

            #endif
            }
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
            capture.open(_M_video_name.toStdString());
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

