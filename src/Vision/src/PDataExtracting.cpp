#include "PDataExtracting.h"

PDataExtracting::PDataExtracting(QObject *parent) : QThread(parent)
{
    //Init result directory
    _M_result_directory =  "/home/results/";

    _M_debug_level = false;
    _M_buffer_img.clear();
    _M_pixels_pos.clear();
    _M_acquisition_is_over  = false;
    _M_in_thread = false;

    connect(this,SIGNAL(finished()),this,SLOT(onThreadFinished()));

    _M_stop = false;
    start(); //Start the thread

}

PDataExtracting::~PDataExtracting()
{
    if (isRunning()) // Only stop/wait if not already done in closeEvent()
    {
        QMutexLocker locker(&_M_mutex);
        _M_stop = true;
        _M_condition.wakeOne();
        // Must release lock before wait()
    }

    if (isRunning())
        wait();
}

void PDataExtracting::stop()
{
    qDebug()<<"PDataExtracting::stop";
    QMutexLocker locker(&_M_mutex);
    _M_stop = true;
    _M_condition.wakeOne(); // wake it so it can exit
}



// Inform that thread finished
void PDataExtracting::onThreadFinished()
{
    // qDebug()<<"PDataExtracting thread finished";
}

void PDataExtracting::init()
{
    _M_buffer_img.clear();
    _M_acquisition_is_over  = false;
}

void PDataExtracting::run()
{
    _M_in_thread = true;

    for (;;)
    {
        _Processed_img img;
        {
            QMutexLocker locker(&_M_mutex);

            // Sleep here until there is work or a stop request
            while (_M_buffer_img.isEmpty() && !_M_stop)
                _M_condition.wait(&_M_mutex);

            if (_M_stop)
                break;

            _Processed_img tmp = _M_buffer_img.dequeue();
            img.thread_id = tmp.thread_id;
            img.img = tmp.img.clone(); // deep copy — fully independent
        } // mutex unlocks here


        _extractData_RGB(img);

    }

    _M_in_thread = false;
}


void PDataExtracting::extractData(_Processed_img img)
{
    QMutexLocker locker(&_M_mutex);
    _M_buffer_img.enqueue(img); // push to back
    _M_condition.wakeOne(); // wake the sleeping thread
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
        _M_empty_img_buffer = (img.img).clone();
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
