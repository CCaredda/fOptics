#include "PDataExtracting.h"

PDataExtracting::PDataExtracting(QObject *parent) : QThread(parent)
{
    //Init result directory
    _M_result_directory =  "/home/results/";

    _M_debug_level = false;
    //moveToThread(this);
    _M_correct_spectral_data = false;

    _M_buffer_img_HS.clear();
    _M_empty_img_buffer_HS.clear();
    _M_buffer_img.clear();
    _M_pixels_pos.clear();
    _M_acquisition_is_over  = false;
    _M_mode_RGB             = true;

    _M_in_thread = false;

    connect(this,SIGNAL(finished()),this,SLOT(onThreadFinished()));
}

PDataExtracting::~PDataExtracting()
{
    if (isRunning())
    {
        requestInterruption();
        wait();
    }
}

/** request acquisiton info */
QString PDataExtracting::requestAcquisitionInfo(bool v)
{
    _M_debug_level = v;
    //Info PFiltering
    QString msg = "Class PDataExtracting \n";
    msg += (this->isRunning()) ? "Thread is running" : "Thread is not running";
    msg+="\n ";

    msg+="RGB buffer size: "+QString::number(_M_buffer_img.size())+"\n ";
    msg+="HS buffer size: "+QString::number(_M_buffer_img_HS.size())+"\n ";
    msg+="Flag acquisition is over "+QString::number(_M_acquisition_is_over)+"\n ";
    msg+="In thread: "+QString::number(_M_in_thread);
    return msg;
}


// Inform that thread finished
void PDataExtracting::onThreadFinished()
{
    // qDebug()<<"PDataExtracting thread finished";
}

void PDataExtracting::init()
{
    _M_buffer_img_HS.clear();
    _M_empty_img_buffer_HS.clear();
    _M_buffer_img.clear();
    _M_acquisition_is_over  = false;
}


void PDataExtracting::run()
{
    _M_in_thread = true;

    if(_M_mode_RGB)
    {
        while(_M_buffer_img.size()>0)
        {
            if(_M_debug_level)
                qDebug()<<"PDataExtracting 1";

            _Processed_img img;
            if(_M_debug_level)
                qDebug()<<"PDataExtracting 2";

            QMutexLocker locker(&_M_mutex);
            _M_buffer_img[0].img.copyTo(img.img);
            img.thread_id=_M_buffer_img[0].thread_id;
            locker.unlock();
            _extractData_RGB(img);

            locker.relock();
            _M_buffer_img.erase(_M_buffer_img.begin());
            locker.unlock();

            if(_M_debug_level)
                qDebug()<<"PDataExtracting 3";

        }
    }
    else
    {
        while(_M_buffer_img_HS.size()>0)
        {
            _Processed_img_HS img;

            QMutexLocker locker(&_M_mutex);
            img = copyHyperCube(_M_buffer_img_HS[0]);
            locker.unlock();

            _extractData_HS(img);

            locker.relock();
            _M_buffer_img_HS.erase(_M_buffer_img_HS.begin());
            locker.unlock();

        }
    }
    _M_in_thread = false;


//     if(_M_mode_RGB)
//     {
//         while(!_M_acquisition_is_over)
//         {
//             if(_M_debug_level)
//                 qDebug()<<"PDataExtracting 1";
//             if(!_M_buffer_img.empty())
//             {
//                 _Processed_img img;
//                 if(_M_debug_level)
//                     qDebug()<<"PDataExtracting 2";

//                 QMutexLocker locker(&_M_mutex);
//                 _M_buffer_img[0].img.copyTo(img.img);
//                 img.thread_id=_M_buffer_img[0].thread_id;
//                 locker.unlock();
//                 _extractData_RGB(img);

//                 locker.relock();
//                 _M_buffer_img.erase(_M_buffer_img.begin());
//                 locker.unlock();

//                 if(_M_debug_level)
//                     qDebug()<<"PDataExtracting 3";
//             }
// //            else
// //                qDebug()<<"PDataExtracting::RGB buffer empty";
//         }
//     }
//     else
//     {
//         while(!_M_acquisition_is_over)
//         {
//             if(!_M_buffer_img_HS.empty())
//             {
//                 _Processed_img_HS img;

//                 QMutexLocker locker(&_M_mutex);
//                 img = copyHyperCube(_M_buffer_img_HS[0]);
//                 locker.unlock();

//                 _extractData_HS(img);

//                 locker.relock();
//                 _M_buffer_img_HS.erase(_M_buffer_img_HS.begin());
//                 locker.unlock();
//             }
//         }
//     }
//     qDebug()<<"PDataExtracting::stop thread";
//     _M_in_thread = false;
}


void PDataExtracting::extractData(_Processed_img img)
{
    QMutexLocker locker(&_M_mutex);
    _M_buffer_img.push_back(img);
//    qDebug()<<"PDataExtracting::extractData Buffer size: "<<_M_buffer_img.size();
    locker.unlock();


    if(!_M_in_thread || !this->isRunning())
    {
        _M_mode_RGB = true;
        this->start();
//        this->exec();
    }
}

void PDataExtracting::extractData(_Processed_img_HS img)
{
    QMutexLocker locker(&_M_mutex);
    _M_buffer_img_HS.push_back(img);
    locker.unlock();

    if(img.thread_id==0 || !_M_buffer_img_HS.empty() || !this->isRunning())
    {
        _M_mode_RGB = false;
        this->start();
    }
}

void PDataExtracting::_extractData_RGB(_Processed_img img)
{
    /****************************************************************/
    /**********************Handle empty img**************************/
    /****************************************************************/
    if(img.img.empty())
    {
        qDebug()<<"[PDataExtracting] _extractData img "<<img.thread_id<<" empty";
        if(_M_empty_img_buffer.empty())
        {
            emit newExtractedData(img);
            return;
        }
        img.img = _M_empty_img_buffer;
    }
    else
    {
        //Create buffer if the next img is empty
        img.img.copyTo(_M_empty_img_buffer);
    }
    _Processed_img temp;
    temp.thread_id = img.thread_id;
    temp.img = Mat::zeros(3,_M_pixels_pos.size(),CV_16UC1);

    for(int i=0;i<_M_pixels_pos.size();i++)
    {
        Vec3b *ptr = img.img.ptr<Vec3b>(_M_pixels_pos[i].y);
        for(int k=0;k<3;k++)
        {
            temp.img.ptr<ushort>(k)[i] =  static_cast<ushort>(ptr[_M_pixels_pos[i].x][2-k]);
        }
    }

    emit newExtractedData(temp);

    if(img.thread_id==_M_tot_frames-1)
    {
        _M_acquisition_is_over = true;
        qDebug()<<"[PDataExtracting::_extractData_RGB] acquisition is over";
    }
}

void PDataExtracting::_extractData_HS(_Processed_img_HS img)
{
    /****************************************************************/
    /**********************Handle empty img**************************/
    /****************************************************************/
    if(img.img.empty())
    {
        if(_M_empty_img_buffer_HS.empty())
        {
            qDebug()<<"[PDataExtracting] _extractData_HS img "<<img.thread_id<<" empty";
            return;
        }
        img.img = _M_empty_img_buffer_HS;
    }
    else
    {
        //Create buffer if the next img is empty
        _M_empty_img_buffer_HS.clear();

        Mat temp;
        for(unsigned int i=0;i<img.img.size();i++)
        {
            img.img[i].copyTo(temp);
            _M_empty_img_buffer_HS.push_back(temp);
        }
    }

//    _Processed_img temp;
//    temp.thread_id = img.thread_id;
//    temp.img = Mat::zeros(Spectral_bands,_M_pixels_pos.size(),CV_16UC1);

//    for(int k=0;k<Spectral_bands;k++)
//    {
//        for(int i=0;i<_M_pixels_pos.size();i++)
//        {
//            ushort *ptr = img.img[k].ptr<ushort>(_M_pixels_pos[i].y);
//            temp.img.ptr<ushort>(k)[i] = ptr[_M_pixels_pos[i].x];
//        }
//    }

    //Data extraction and spectral correction : Sc = M x S (Sc(25,1), M(25,25), S(25,1))
    // S : initial spectrim, Sc : corrected spectrum

    _Processed_img extracted_img;
    extracted_img.thread_id = img.thread_id;
    extracted_img.img       = Mat::zeros(Spectral_bands,_M_pixels_pos.size(),CV_16UC1);

    Mat S   = Mat::zeros(25,1,CV_64F);
    Mat Sc  = Mat::zeros(25,1,CV_64F);

    for(int i=0;i<_M_pixels_pos.size();i++)
    {
        //Get spectrum at point _M_pixels_pos[i]
        for(int k=0;k<Spectral_bands;k++)
        {
            S.ptr<double>(k)[0] = (double)(img.img[k].ptr<ushort>(_M_pixels_pos[i].y)[_M_pixels_pos[i].x]);
        }

        //Correct S spectrum
        Sc = _M_correct_spectral_data ? _M_Spectral_Corr_Matrix * S : S;

        //Store corrected spectrum
        for(int k=0;k<Spectral_bands;k++)
            extracted_img.img.ptr<ushort>(k)[i] = (ushort)(Sc.ptr<double>(k)[0]);
    }

    emit newExtractedData(extracted_img);

    if(img.thread_id==_M_tot_frames-1)
        _M_acquisition_is_over = true;


//    //TEMP write non filtered signal
//    int id = (int)(_M_pixels_pos.size()/2);

//    QFile file(QString(_M_result_directory)+"non_filtered_no_corrected.txt");
//    if(file.open(QIODevice::WriteOnly | QIODevice::Append))
//    {
//        QTextStream Qt( &file );
//        for(int k=0;k<Spectral_bands;k++)
//        {
//            Qt<<img.img[k].ptr<ushort>(_M_pixels_pos[id].y)[_M_pixels_pos[id].x]<<" ";
//        }
//        Qt<<endl;
//    }
//    file.close();

//    file.setFileName(QString(_M_result_directory)+"non_filtered_corrected.txt");
//    if(file.open(QIODevice::WriteOnly | QIODevice::Append))
//    {
//        QTextStream Qt( &file );
//        //Get spectrum at point _M_pixels_pos[i]
//        for(int k=0;k<Spectral_bands;k++)
//        {
//            S.ptr<double>(k)[0] = (double)(img.img[k].ptr<ushort>(_M_pixels_pos[id].y)[_M_pixels_pos[id].x]);
//        }

//        //Correct S spectrum
//        Sc = _M_Spectral_Corr_Matrix * S;

//        //Store corrected spectrum
//        for(int k=0;k<Spectral_bands;k++)
//            Qt<<Sc.ptr<double>(k)[0]<<" ";

//        Qt<<endl;
//    }
//    file.close();
//    //END TEMP
}
