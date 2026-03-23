#include "PFiltering.h"

PFiltering::PFiltering(QObject *parent) : QThread(parent)
{
    //Init result directory
    _M_result_directory =  "";

    //moveToThread(this);


    //Init spectral bands idx with RGB bands (3 values)
    _M_spectral_bands_idx.clear();
    for(int i=0;i<3;i++)
        _M_spectral_bands_idx.push_back(i);

    //thread is running ?
    _M_in_thread = false;

    //Save non filtered signals
    _M_save_NonFilteredSignals = false;

    //Enable filtering  (low pass + cardiac filtering + data correction)
    _M_enable_filtering = true;

    //Enable low pass filtering
    _M_enable_low_pass_filtering = true;

    //Enable Data correction
    _M_enable_data_correction = true;

    //mode resting state
    _M_mode_resting_state = false;

    //points of interest
    _M_Point_Of_Interest.clear();

    _M_nb_spectral_channels = 3;

    //Frame rate
    _M_frame_rate = 30;


    //Filtering type
    _M_filtering_type = FFT_FILTERING;

    _M_nb_Learning_frames = 10;

    _M_tot_frames = 2700;


    //Filtering buffer
    _M_acquisition_is_over = false;
    _M_in_buffer_filtering.clear();
    _M_buffer_first_value.clear();
    _M_out_buffer_filtering.clear();
    _M_remain_values.clear();

    _M_img_buffer.clear();
    _M_output_id=0;
    _M_nb_Spatial_pixels=0;


    //Data extraction
    connect(&_M_data_extract,SIGNAL(newExtractedData(_Processed_img)),this,SLOT(onNewDataExtracted(_Processed_img)));
}


PFiltering::~PFiltering()
{
    if (isRunning())
    {
        requestInterruption();
        wait();
    }
}


//New spectral bands idx
void PFiltering::onnewWavelengthIDX(QVector<int> v)
{
    _M_spectral_bands_idx.clear();
    for(int i=0;i<v.size();i++)
        _M_spectral_bands_idx.push_back(v[i]);
}


// Request aquisition info
void PFiltering::onrequestAcquisitionInfo(bool v)
{
    //Info PFiltering

    //Info PFiltering
    QString msg = "Class PFiltering \n";
    msg += (_M_in_thread) ? "Thread is running" : "Thread is not running";
    msg+="\n ";
    msg+="Buffer size: "+QString::number(_M_img_buffer.size())+"\n ";
    msg+="Flag acquisition is over "+QString::number(_M_acquisition_is_over)+"\n \n";

    //Info Extract data
    msg+=_M_data_extract.requestAcquisitionInfo(v);

    emit newAcquisitionInfo(msg);
}

//nb of temporal element
int PFiltering::getNbOfTemporalElement()
{
    if(_M_ROI_row_data.empty())
        return 0;

    if(_M_ROI_row_data[0].empty())
        return 0;

    return _M_ROI_row_data[0][0].size();
}


//Get spectral vector
QVector<float> PFiltering::get_spectral_vector(int spatial_id)
{
    QVector<float> spectrum;

    for(int s=0;s<_M_ROI_row_data[spatial_id].size();s++)
    {
        spectrum.push_back(_M_ROI_row_data[spatial_id][s][0]);
    }

    return spectrum;
}

QVector<QVector<float> > PFiltering::get_spectral_vectors(int spatial_id)
{
    QVector<QVector<float> > spectrum;

    for(int t=0;t<_M_ROI_row_data[spatial_id][0].size();t++)
    {
        QVector<float> temp;
        for(int s=0;s<_M_nb_spectral_channels;s++)
            temp.push_back(_M_ROI_row_data[spatial_id][s][t]);

        spectrum.push_back(temp);
    }

    return spectrum;
}

/*********************************************************/
/*********************************************************/
/*********************RUN FUNCTION************************/
/*********************************************************/
/*********************************************************/


//RUN FUNCTION
void PFiltering::run()
{
    _M_in_thread = true;

    while(_M_img_buffer.size()>0)
    {

//            qDebug()<<"[PFiltering] run step 1";
        _Processed_img img;
        QMutexLocker lock(&_M_mutex);
        img.thread_id = _M_img_buffer[0].thread_id;
        _M_img_buffer[0].img.copyTo(img.img);
        lock.unlock();

        _StoreData(img);

        lock.relock();
        _M_img_buffer.remove(0);
        lock.unlock();



    }

    _M_in_thread = false;
}

/*********************************************************/
/*********************************************************/
/*********************INIT********************************/
/*********************************************************/
/*********************************************************/

//init buffer size
void PFiltering::init(int nbElements, int nbChannels, int nb_frames)
{
    _M_data_extract.init();

    _M_nb_spectral_channels = nbChannels;
    _M_acquisition_is_over  = false;
    _M_tot_frames           = nb_frames;
    _M_nb_Spatial_pixels    = nbElements;
    _M_data_extract.setTotFrames(nb_frames);


    //buffer
    _M_buffer_first_value.clear();
    _M_in_buffer_filtering.clear();
    _M_remain_values.clear();
    _M_img_buffer.clear();


    //Resize final Data and temp data used for RT filtering
    _M_ROI_row_data.clear();
    _M_ROI_row_data.resize(nbElements);

    #pragma omp parallel
    {
        #pragma omp for
        for(int px=0;px<nbElements;px++)
        {
            _M_ROI_row_data[px].resize(nbChannels);
            for(int channels=0;channels<nbChannels;channels++)
                _M_ROI_row_data[px][channels].resize(nb_frames);
        }
    }

    //Non filtered data
    _M_ROI_row_data_non_filtered.clear();
    if(_M_save_NonFilteredSignals)
    {
        _M_ROI_row_data_non_filtered.resize(nbElements);
        #pragma omp parallel
        {
            #pragma omp for
            for(int px=0;px<nbElements;px++)
            {
                _M_ROI_row_data_non_filtered[px].resize(nbChannels);
                for(int channels=0;channels<nbChannels;channels++)
                    _M_ROI_row_data_non_filtered[px][channels].resize(nb_frames);
            }
        }
    }


//    double data_weight = ((double)nbElements*(double)nbChannels*(double)_M_coeff_filter_FIR.size()-1*sizeof(float));
//    if(data_weight>MAX_data_WEIGHT_FILTERING_GB)
//    {
//      qDebug()<<"Too many datas > "<<MAX_data_WEIGHT_FILTERING_GB<<" GB ";
//      return false;
//    }

    //resize elements
    _M_in_buffer_filtering.resize(nbElements);
    _M_buffer_first_value.resize(nbElements);
    _M_out_buffer_filtering.resize(nbElements);
    _M_remain_values.resize(nbElements);

    for(int px=0;px<nbElements;px++)
    {
        _M_in_buffer_filtering[px].resize(nbChannels);
        _M_out_buffer_filtering[px].resize(nbChannels);
        _M_buffer_first_value[px].resize(nbChannels);
        _M_remain_values[px].resize(nbChannels);
    }


//    qDebug()<<"[PFiltering] data weight :"<<data_weight;
    _M_output_id=0;
}


/*********************************************************/
/*********************************************************/
/*********************Get Data****************************/
/*********************************************************/
/*********************************************************/


/** Get temporal vectors */
QVector<QVector<float> > PFiltering::getTemporalVector(int spatial_id)
{
//    return _M_ROI_row_data[spatial_id];

    if(_M_spectral_bands_idx.empty())
        return _M_ROI_row_data[spatial_id];

    QVector<QVector<float> > mixel;
    for(int i=0;i<_M_spectral_bands_idx.size();i++)
        mixel.push_back(_M_ROI_row_data[spatial_id][_M_spectral_bands_idx[i]]);

    return mixel;
}

/** Get non filtered temporal vectors */
QVector<QVector<float> > PFiltering::getNonFilteredTemporalVector(int spatial_id)
{
//    return _M_ROI_row_data_non_filtered[spatial_id];
    if(_M_spectral_bands_idx.empty())
        return _M_ROI_row_data_non_filtered[spatial_id];

    QVector<QVector<float> > mixel;
    for(int i=0;i<_M_spectral_bands_idx.size();i++)
        mixel.push_back(_M_ROI_row_data_non_filtered[spatial_id][_M_spectral_bands_idx[i]]);

    return mixel;
}


QVector<QVector<float> > PFiltering::getTemporalVector(int spatial_id,QVector<int> wavelength)
{
    if(wavelength.empty())
        return _M_ROI_row_data[spatial_id];

    QVector<QVector<float> > mixel;
    for(int i=0;i<wavelength.size();i++)
        mixel.push_back(_M_ROI_row_data[spatial_id][wavelength[i]]);

    return mixel;
}

QVector<QVector<float> > PFiltering::getTemporalVector(int spatial_id,QVector<int> wavelength,int t_start,int t_end)
{
    if(wavelength.empty())
        return _M_ROI_row_data[spatial_id];

    QVector<QVector<float> > mixel;
    for(int i=0;i<wavelength.size();i++)
    {
        QVector<float> temp;
        for(int t=t_start;t<t_end;t++)
            temp.push_back(_M_ROI_row_data[spatial_id][wavelength[i]][t]);

        mixel.push_back(temp);
    }

    return mixel;
}




//Get first value (for data redressing)
QVector<float> *PFiltering::getFirstValuePointer()
{
    return _M_buffer_first_value.data();
}

QVector<float> PFiltering::getFirstValues(int id)
{
    QMutexLocker lock(&_M_mutex);
    return _M_buffer_first_value[id];
}

//Set filtering type
void PFiltering::setFilteringType(int v)
{
    _M_filtering_type=v;
}

//Get mean signal value (for IIR filtering)
void PFiltering::_GetMeanSignalValue(_Processed_img &img)
{
    /****************************************************************/
    /**********************Handle empty img**************************/
    /****************************************************************/
    if(img.img.empty())
    {
        if(_M_empty_img_buffer.empty())
        {
            qDebug()<<"[PFiltering] _GetMeanSignalValue img "<<img.thread_id<<" empty";
            return;
        }
        img.img = _M_empty_img_buffer;
    }
    else
    {
        //Create buffer if the next img is empty
        img.img.copyTo(_M_empty_img_buffer);
    }


    #pragma omp parallel
    {
        #pragma omp for
        for(int k=0;k<_M_nb_spectral_channels;k++)
        {
            ushort *ptr = img.img.ptr<ushort>(k);
            for(int i=0;i<_M_nb_Spatial_pixels;i++)
            {
                _M_buffer_first_value[i][k] += ptr[i];
            }
        }
    }

    if(img.thread_id==_M_nb_Learning_frames-1)
    {
        #pragma omp parallel
        {
            #pragma omp for
            for(int k=0;k<_M_nb_spectral_channels;k++)
            {
                for(int i=0;i<_M_nb_Spatial_pixels;i++)
                {
                    _M_buffer_first_value[i][k] /=_M_nb_Learning_frames;
                }
            }
        }
    }
}

/*********************************************************/
/*********************************************************/
/*********************NEW FILTERING***********************/
/*********************************************************/
/*********************************************************/

//receive img from PANalyse
void PFiltering::new_Filtering(_Processed_img &mat)
{
    //Send img to other thread doing data extraction
    _M_data_extract.extractData(mat);
}

void PFiltering::new_Filtering(_Processed_img_HS &mat)
{
    //Send img to other thread doing data extraction
    _M_data_extract.extractData(mat);
}

/*********************************************************/
/*********************************************************/
/*********************LAUNCH FILTERING********************/
/*********************************************************/
/*********************************************************/

//on new data extracted from PDataExtracting
void PFiltering::onNewDataExtracted(_Processed_img img)
{
    QMutexLocker locker(&_M_mutex);
//    //TEMP
//    if(!img.img.empty())
//    {
//        if(img.img.cols>(int)(_M_nb_Spatial_pixels/2))
//        {
//            _Mycolor *ptr = img.img.ptr<_Mycolor>(0);
//            _M_test_signal[0].push_back((float)(ptr[(int)(_M_nb_Spatial_pixels/2)].r));
//            _M_test_signal[1].push_back((float)(ptr[(int)(_M_nb_Spatial_pixels/2)].g));
//            _M_test_signal[2].push_back((float)(ptr[(int)(_M_nb_Spatial_pixels/2)].b));
//        }
//    }
//    //END TEMP

    _M_img_buffer.push_back(img);
//    qDebug()<<"PFiltering::onNewDataExtracted Buffer size: "<<_M_img_buffer.size();
    locker.unlock();


    if(!_M_in_thread || !this->isRunning())
    {
        this->start();
    }

}

/*********************************************************/
/*********************************************************/
/*********************STORE DATA**************************/
/*********************************************************/
/*********************************************************/

//Store data
void PFiltering::_StoreData(_Processed_img img)
{

//    qDebug()<<"PFiltering::_StoreData";
    if(img.img.cols!=_M_nb_Spatial_pixels)
    {
        qDebug()<<"[PFiltering] _StoreData img received cols :"<<img.img.cols<<" expected : "<<_M_nb_Spatial_pixels;
        qDebug()<<"[PFiltering] _StoreData img received rows :"<<img.img.rows<<" expected : "<<_M_nb_spectral_channels;
        return;
    }

    #pragma omp parallel
    {
        #pragma omp for
        for(int k=0;k<_M_nb_spectral_channels;k++)
        {
            ushort *ptr = img.img.ptr<ushort>(k);
            for(int i=0;i<_M_nb_Spatial_pixels;i++)
            {
                _M_ROI_row_data[i][k][img.thread_id] = ptr[i];
                if(_M_save_NonFilteredSignals)
                    _M_ROI_row_data_non_filtered[i][k][img.thread_id] = ptr[i];
            }
        }
    }

//    //TEMP write non filtered signal
//    QFile file(QString(_M_result_directory)+"non_filtered.txt");
//    if(file.open(QIODevice::WriteOnly | QIODevice::Append))
//    {
//        QTextStream Qt( &file );
//        for(int k=0;k<_M_nb_spectral_channels;k++)
//        {
//            Qt<<_M_ROI_row_data[int(_M_ROI_row_data.size()/2)][k][img.thread_id]<<" ";
//        }
//        Qt<<endl;
//    }
//    file.close();
//    //END TEMP


    emit newProgressStatut("Store data",(100*img.thread_id)/(_M_tot_frames-1));

    //If the last frame is stored and filtering is enabled
    if(img.thread_id==_M_tot_frames-1)
    {
        //Process FFT filtering
        _Process_FFT_Filtering();

        _M_acquisition_is_over = true;
//        emit ProcessIsOver();
    }
}

/*********************************************************/
/*********************************************************/
/*********************Get data****************************/
/*********************************************************/
/*********************************************************/


//is Data valid
bool PFiltering::isRowDataValid()
{
    if(_M_ROI_row_data.empty())
        return false;
    if(_M_ROI_row_data[0].empty())
        return false;
    if(_M_ROI_row_data[0][0].empty())
        return false;

    return true;
}

//get last filtered id
int PFiltering::getLastFilteredId()
{
     return _M_output_id-1;
}

/*********************************************************/
/*********************************************************/
/****************Process FFT Filtering********************/
/*********************************************************/
/*********************************************************/

void PFiltering::_NormalizeData(Mat &inout)
{
    for(int k=0;k<_M_nb_spectral_channels;k++)
    {
        Scalar mean,stddev;
        meanStdDev(inout.col(k),mean,stddev);

        //Normalize (center data)
        for (int i = 0; i < _M_tot_frames; i++)
            inout.at<double>(i,k) = (inout.at<double>(i,k) - mean[0]);
//            inout.at<double>(i,k) = (inout.at<double>(i,k) - mean[0])/stddev[0];
    }
}

//Process FFT filtering
void PFiltering::_Process_FFT_Filtering()
{
    //Do not process whole data filtering operation if all conditions are not required
    if((!_M_enable_data_correction && !_M_enable_low_pass_filtering) || !_M_enable_filtering)
    {
        emit readyForNotRealTimeProcess();
        return;
    }

//    QElapsedTimer timer;
//    timer.start();

    //Axis
    QVector<float> x;
    x.clear();
    for(int i=0;i<_M_tot_frames;i++)
        x.push_back(i+1);

    /*************************************************
     ********** Define Low Pass Filter ***************
     *************************************************/
    QVector<float> filter_low_pass;
    if(_M_mode_resting_state)
        Blackman_Window(_M_frame_rate,Low_Pass_Cut_OFF_Frequency_Resting_State,filter_low_pass,_M_tot_frames);
    else
        Blackman_Window(_M_frame_rate,Low_Pass_Cut_OFF_Frequency,filter_low_pass,_M_tot_frames);

    /******************************************************************
     ****** Temporal Filtering (low Pass filtering)********************
     ******************************************************************/

    fftwf_complex  *in   = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*_M_tot_frames);
    fftwf_complex  *out  = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*_M_tot_frames);

    fftwf_plan p = fftwf_plan_dft_1d(_M_tot_frames,in,out,FFTW_FORWARD,FFTW_ESTIMATE);
    fftwf_plan q = fftwf_plan_dft_1d(_M_tot_frames,out,in,FFTW_BACKWARD,FFTW_ESTIMATE);



    //Filtering operations

    //If only data correction is required
    if(_M_enable_data_correction && !_M_enable_low_pass_filtering)
    {
        #pragma omp parallel
        {
            int nb_thread = omp_get_num_threads();
            #pragma omp for
            for(int id=0;id<_M_nb_Spatial_pixels;id++)
            {
                emit newProgressStatut("Data correction",(int)((100*id*nb_thread)/_M_nb_Spatial_pixels));

                for(int k=0;k<_M_nb_spectral_channels;k++)
                {
                    Apply_Data_Correction(x,_M_ROI_row_data[id][k]);
                }
            }
        }
    }

    //If data correction and data low pass filtering is required
    if(_M_enable_data_correction && _M_enable_filtering)
    {
        #pragma omp parallel
        {
            int nb_thread = omp_get_num_threads();
            #pragma omp for
            for(int id=0;id<_M_nb_Spatial_pixels;id++)
            {
                emit newProgressStatut("Filtering",(int)((100*id*nb_thread)/_M_nb_Spatial_pixels));

                for(int k=0;k<_M_nb_spectral_channels;k++)
                {
                    Apply_Filtering(x,_M_ROI_row_data[id][k],p,q,filter_low_pass);
                }
            }
        }
    }


    //If only low pass filtering is required
    if(!_M_enable_data_correction && _M_enable_filtering)
    {
        #pragma omp parallel
        {
            int nb_thread = omp_get_num_threads();
            #pragma omp for
            for(int id=0;id<_M_nb_Spatial_pixels;id++)
            {
                emit newProgressStatut("Filtering",(int)((100*id*nb_thread)/_M_nb_Spatial_pixels));

                for(int k=0;k<_M_nb_spectral_channels;k++)
                {
                    Apply_Low_Pass_Filtering(x,_M_ROI_row_data[id][k],p,q,filter_low_pass);
                }
            }
        }
    }




    //Destroy FFT plans and datas
    fftwf_destroy_plan(p);
    fftwf_destroy_plan(q);
    fftwf_free(in);
    fftwf_free(out);

    //Emit results
    emit readyForNotRealTimeProcess();



    //emit newCardiacMeanValues(_M_filtered_mean_cardiac_values);


//    qDebug()<<"FFT filtering elapsed time : "<<timer.elapsed()<<" ms";
}











/*********************************************************/
/*********************************************************/
/****************Redress data*****************************/
/*********************************************************/
/*********************************************************/

void PFiltering::_ProcessDataRedress()
{
    QVector<float> x1;


    x1.clear();

    for(int i=0;i<_M_tot_frames;i++)
        x1.push_back(i+1);


    //Redressement des données
    #pragma omp parallel
    {
        int nb_thread = omp_get_num_threads();
        #pragma omp for
        for(int id=0;id<_M_nb_Spatial_pixels;id++)
        {
            emit newProgressStatut("Redress data",(int)((100*id*nb_thread)/_M_nb_Spatial_pixels));

            for(int k=0;k<_M_nb_spectral_channels;k++)
            {
                vector<float> p1;
                if(!polyfit2(x1,_M_ROI_row_data[id][k],1,p1)==0)
                {
                    //Apply polyfit (maybe redress and
                    for(int i=0;i<_M_tot_frames;i++)
                    {
                        _M_ROI_row_data[id][k][i]    = _M_ROI_row_data[id][k][i]-(p1[1]*x1[i]) + _M_buffer_first_value[id][k];
                    }
                }
                else
                    qDebug()<<"Polyfit Error";

            }
        }
    }
}




