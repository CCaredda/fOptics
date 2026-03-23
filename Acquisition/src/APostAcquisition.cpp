#include "APostAcquisition.h"

APostAcquisition::APostAcquisition(QObject *parent) : QThread(parent)
{
    //moveToThread(this);

    //Init result directory
    _M_result_directory = "/home/results/";


    //initial img
    _M_initial_img = Mat(0,0,CV_8UC3);

    //init flags
    _M_apply_Pre_ROI        = false;
    _M_acquisition_is_over  = false;
    _M_spatial_sampling     = 1.0;
    _M_visualization_spectral_band = 0;

    //init Rect ROI
    _M_Pre_ROI_Rect.x = 0;
    _M_Pre_ROI_Rect.y = 0;
    _M_Pre_ROI_Rect.width = 0;
    _M_Pre_ROI_Rect.height =0;

    // //Get RAM size
    // qDebug()<<"Appdir: "<<QString(QCoreApplication::applicationDirPath());
    // QProcess p;
    // p.start(QString(QCoreApplication::applicationDirPath())+"/../share/script/get_Ram_size.sh");
    // p.waitForFinished();
    // QString output(p.readAllStandardOutput());
    // int pos = output.lastIndexOf(QChar('\\'));
    // int Ram_size = (output.left(pos)).toInt();
    // qDebug()<<" Ram_size: "<<Ram_size;

    // //Define max data amount that can be stored in RAM depending on the RAM size
    // if(Ram_size>40*1e6)
    // {
    //     qDebug()<<"Do not apply underspampling";
    //     _M_max_data_amount = 100000;
    // }
    // else
    // {
    //     qDebug()<<"Apply underspampling";
    //     _M_max_data_amount = 50000;
    // }

    _M_max_data_amount = 50000;

    qDebug()<<"[APostAcquisition] Max data amount: "<<_M_max_data_amount;
    _M_img_buffer.clear();


}

APostAcquisition::~APostAcquisition()
{
    if (isRunning())
    {
        requestInterruption();
        wait();
    }
}


void APostAcquisition::on_New_Maximum_number_Analyzed_pixels(double v)
{
    _M_max_data_amount = int(v);
    qDebug()<<"[APostAcquisition] Max data amount: "<<_M_max_data_amount;

    if( ((_M_Pre_ROI_Rect.x == 0) && (_M_Pre_ROI_Rect.y ==0) && (_M_Pre_ROI_Rect.width == 0) && (_M_Pre_ROI_Rect.height == 0)) )
        return;

    _M_spatial_sampling = sqrt(_M_Pre_ROI_Rect.width*_M_Pre_ROI_Rect.height/_M_max_data_amount);
    _M_spatial_sampling = (_M_spatial_sampling<1)? 1 : _M_spatial_sampling;
}

//Set Pre ROI
void APostAcquisition::_setROI(Rect roi)
{
    _M_Pre_ROI_Rect = roi;

    qDebug()<<"[APostAcquisition::_setROI(Rect roi)] Rect ("<<_M_Pre_ROI_Rect.x<<";"<<_M_Pre_ROI_Rect.y<<";"<<_M_Pre_ROI_Rect.width<<";"<<_M_Pre_ROI_Rect.height<<")";

    //Find spatial sampling to respect the maximum amount of data


    _M_spatial_sampling = sqrt(_M_Pre_ROI_Rect.width*_M_Pre_ROI_Rect.height/_M_max_data_amount);
    _M_spatial_sampling = (_M_spatial_sampling<1)? 1 : _M_spatial_sampling;

    qDebug()<<"[APostAcquisition] Max data amount: "<<_M_max_data_amount;
    qDebug()<<"[APostAcquisition] Spatial sampling: "<<_M_spatial_sampling;

    //Write Pre ROI and spatial sampling
    WriteInfos();
}


void APostAcquisition::_setROI(Rect roi,double spatial_sampling)
{
    _M_Pre_ROI_Rect = roi;

    qDebug()<<"[APostAcquisition::_setROI(Rect roi,double spatial_sampling)] Rect ("<<_M_Pre_ROI_Rect.x<<";"<<_M_Pre_ROI_Rect.y<<";"<<_M_Pre_ROI_Rect.width<<";"<<_M_Pre_ROI_Rect.height<<")";

    //Set spatial sampling
    _M_spatial_sampling = spatial_sampling;
    qDebug()<<"[APostAcquisition] Spatial sampling: "<<_M_spatial_sampling;


}

//Write Pre ROI and spatial sampling
void APostAcquisition::WriteInfos()
{
    //Write spatial sampling info
    QDir dir(QString(_M_result_directory)+"contours/");


    //Create saving dir
    if(!dir.exists())
        dir.mkdir(QString(_M_result_directory)+"contours/");

    QFile file(QString(_M_result_directory)+"contours/spatial_sampling.txt");

    //Remove file if exists
    if(!file.exists())
    {
        if ( file.open(QIODevice::ReadWrite) )
        {
            QTextStream stream( &file );
            stream <<_M_spatial_sampling;
        }
    }

    file.close();

    //Rect ROI
    file.setFileName(QString(_M_result_directory)+"contours/Rect_ROI.txt");
    if(!file.exists())
    {
        //Write Pre ROI info
        QVector<Point> pre_ROI;
        pre_ROI.push_back(_M_Pre_ROI_Rect.tl());
        pre_ROI.push_back(_M_Pre_ROI_Rect.br());

        WritePointVector(QString(_M_result_directory)+"contours/Rect_ROI.txt",pre_ROI);
    }

    //Maximum data amount
    file.setFileName(QString(_M_result_directory)+"contours/Maximum_data_amout.txt");
    if(!file.exists())
    {
        if ( file.open(QIODevice::ReadWrite) )
        {
            QTextStream stream( &file );
            stream <<_M_max_data_amount;
        }
    }
}


void APostAcquisition::run()
{
    while(_M_img_buffer.size()>0)
    {

        _Processed_img img;

        QMutexLocker locker(&_M_mutex);
         _M_img_buffer[0].img.copyTo(img.img);
         img.thread_id = _M_img_buffer[0].thread_id;
         locker.unlock();




        _ProcessPreProcessing(img);

        locker.relock();
        _M_img_buffer.erase(_M_img_buffer.begin());
        locker.unlock();
    }
}

//Request pre processing for a single img
void APostAcquisition::requestPreProcessing(Mat img)
{
    if (_M_Pre_ROI_Rect.x<=0)
        qDebug()<<"Roi.x<=0";
    if (_M_Pre_ROI_Rect.y<=0)
        qDebug()<<"Roi.y<=0";

    if (_M_Pre_ROI_Rect.width<=0)
        qDebug()<<"Roi.width<=0";
    if (_M_Pre_ROI_Rect.height<=0)
        qDebug()<<"Roi.height<=0";

    if (_M_Pre_ROI_Rect.x+_M_Pre_ROI_Rect.width>=img.cols)
    {
        qDebug()<<"img.cols: "<<img.cols;
        qDebug()<<"Roi.x+ROI.width>=cols";
    }
    if (_M_Pre_ROI_Rect.y+_M_Pre_ROI_Rect.height>=img.rows)
    {
        qDebug()<<"Roi.y+ROI.height>=rows";
        qDebug()<<"Roi.y: "<<_M_Pre_ROI_Rect.y;
        qDebug()<<"Roi.height: "<<_M_Pre_ROI_Rect.height;

        qDebug()<<"img.rows: "<<img.rows;
        qDebug()<<"Roi.y+ROI.height: "<<_M_Pre_ROI_Rect.y+_M_Pre_ROI_Rect.height;
    }


    //Apply ROI RECT
    if(_M_apply_Pre_ROI)
        img=img(_M_Pre_ROI_Rect);


    //Spatial sampling
    if(_M_spatial_sampling>1)
    {
        double fx_y = (double)(1/_M_spatial_sampling);
        resize(img,img,Size(),fx_y,fx_y,CV_INTER_AREA);
    }


    if(!_M_apply_Pre_ROI && _M_spatial_sampling==1.0)
    {
        qDebug()<<"[APostAcquisition] emit initial img size ("<<img.rows<<";"<<img.cols<<")";
        img.copyTo(_M_initial_img);
        emit newInitialImg(img);
    }


    _M_img_size = img.size();
    img.copyTo(_M_pre_processed_img);

    emit newDisplayImage(img);
}


void APostAcquisition::_ProcessPreProcessing(_Processed_img img)
{
    //Apply ROI RECT
    if(_M_apply_Pre_ROI)
    {
        img.img=img.img(_M_Pre_ROI_Rect);
    }

    //Spatial sampling
    if(_M_spatial_sampling>1)
    {
        double fx_y = (double)(1/_M_spatial_sampling);
        resize(img.img,img.img,Size(),fx_y,fx_y,CV_INTER_AREA);
    }
    //emit results
    emit newPreProcessedImg(img);

}


void APostAcquisition::newImage(_Processed_img img)
{
    QMutexLocker locker(&_M_mutex);
    _M_img_buffer.push_back(img);
    locker.unlock();

    if(img.thread_id==0 || !this->isRunning())
    {
        this->start();
    }
}

void  APostAcquisition::newImage(Mat in_img,int frame_id)
{
    _Processed_img img;
    img.img = in_img;
    img.thread_id = frame_id;

    QMutexLocker locker(&_M_mutex);
    _M_img_buffer.push_back(img);
    locker.unlock();

    if(img.thread_id==0 || !this->isRunning())
    {
        this->start();
    }
}




//Set Pre-ROI rect
void APostAcquisition::setRectROI(QPoint p1,QPoint p2)
{
    qDebug()<<"[APostAcquisition::setRectROI] start";
    Point P1(p1.x(),p1.y());
    Point P2(p2.x(),p2.y());

    //Set ROI
    Rect rect = Rect(P1,P2);
    _setROI(rect);

    if( ((rect.x ==0) && (rect.y ==0) && (rect.width == _M_img_size.width) && (rect.height == _M_img_size.height)) )
        _applyRoi(false);
    else
        _applyRoi(true);

//    _applyRoi((rect & cv::Rect(0, 0, _M_img_size.width, _M_img_size.height)) == rect);
//    _applyRoi( ((rect.x ==0) && (rect.y ==0) && (rect.width == _M_img_size.width) && (rect.height == _M_img_size.height)) );

    //update display
    requestPreProcessing(_M_initial_img);

    qDebug()<<"[APostAcquisition::setRectROI] end";
}

void APostAcquisition::setRectROI(QPoint p1,QPoint p2,double spatial_sampling)
{
    Point P1(p1.x(),p1.y());
    Point P2(p2.x(),p2.y());

    //Check if P1==P2==Point(0,0)
    if(P1.x==P2.x && P1.x==0 && P1.y==P2.y && P1.y==0)
        _applyRoi(false);
    else
    {
        Rect rect = Rect(P1,P2);

        _setROI(rect,spatial_sampling);
//        _applyRoi((rect & cv::Rect(0, 0, _M_img_size.width, _M_img_size.height)) == rect);
//        _applyRoi( ((rect.x ==0) && (rect.y ==0) && (rect.width == _M_img_size.width) && (rect.height == _M_img_size.height)) );
        if( ((rect.x ==0) && (rect.y ==0) && (rect.width == _M_img_size.width) && (rect.height == _M_img_size.height)) )
            _applyRoi(false);
        else
            _applyRoi(true);
    }

    //update display
    requestPreProcessing(_M_initial_img);
}

void APostAcquisition::_applyRoi(bool v)
{
    qDebug()<<"APostAcquisition::_applyRoi: "<<v;


    _M_apply_Pre_ROI = v;
    if(!_M_apply_Pre_ROI)
        _M_spatial_sampling = 1.0;
}

//Init Pre-ROI
void APostAcquisition::InitROI()
{
    _applyRoi(false);

    //update display
    requestPreProcessing(_M_initial_img);
}
