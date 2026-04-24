#include "PFiltering.h"

PFiltering::PFiltering(QObject *parent) : QThread(parent)
{
    //Init result directory
    _M_result_directory =  "/home/results/";

    //Init spectral bands idx with RGB bands (3 values)
    _M_spectral_bands_idx.clear();
    for(int i=0;i<3;i++)
        _M_spectral_bands_idx.push_back(i);

    //thread is running ?
    _M_in_thread = false;



    //Enable filtering  (low pass + cardiac filtering + data correction)
    _M_enable_filtering = true;

    //Enable low pass filtering
    _M_enable_low_pass_filtering = true;

    //Enable Data correction
    _M_enable_data_correction = true;


    _M_nb_spectral_channels = 3;

    //Frame rate
    _M_frame_rate = 30;



    _M_tot_frames = -1;


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


    //Thread
    _M_stop = false;
    _M_process_is_finished = false;
    connect(this, &QThread::finished,
            this, &PFiltering::onPreprocessFinished);
    start(); //Start the thread
}


PFiltering::~PFiltering()
{
    {
        QMutexLocker locker(&_M_mutex);
        _M_stop = true;
        _M_condition.wakeOne(); // wake the thread so it can see _M_stop and exit
    }

    wait(); // block here until the thread has fully finished
}

void PFiltering::stop()
{
    QMutexLocker locker(&_M_mutex);
    _M_stop = true;
    _M_condition.wakeOne(); // wake it so it can exit
}

void PFiltering::onPreprocessFinished()
{

    qDebug()<<"In PFiltering::onPreprocessFinished";
    if(_M_process_is_finished)
    {
        this->wait(); // block here until the thread has fully finished
        qDebug()<<"In PFiltering::onPreprocessFinished send readyForProcessing";
        emit readyForProcessing();
    }
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

    qDebug()<<"[PFiltering] run step 1";


    for(;;)
    {

        _Processed_img img;
        {
            QMutexLocker locker(&_M_mutex);

            // Sleep here until there is work or a stop request
            while (_M_img_buffer.isEmpty() && !_M_stop)
                _M_condition.wait(&_M_mutex);

            if (_M_stop)
                break;

            _Processed_img tmp = _M_img_buffer.dequeue();
            img.thread_id = tmp.thread_id;
            img.img = tmp.img.clone(); // deep copy — fully independent
        } // mutex unlocks here

        _StoreData(img);
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



QVector<QVector<float> > PFiltering::getTemporalVector_RT(int spatial_id,int data_length)
{
    QVector<QVector<float> > data;

    QMutexLocker lock(&_M_mutex);
//    for(int k=0;k<_M_nb_spectral_channels;k++)
//        data.push_back(_M_ROI_row_data[spatial_id][k].mid(0,data_length));

    for(int k=0;k<_M_nb_spectral_channels;k++)
    {
        QVector<float> temp;
        for(int i=0;i<data_length;i++)
        {
            temp.push_back(_M_ROI_row_data[spatial_id][k][i] + _M_buffer_first_value[spatial_id][k]);
        }
        data.push_back(temp);
    }

    lock.unlock();

    return data;
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

/*********************************************************/
/*********************************************************/
/*********************LAUNCH FILTERING********************/
/*********************************************************/
/*********************************************************/

//on new data extracted from PDataExtracting
void PFiltering::onNewDataExtracted(_Processed_img img)
{

    QMutexLocker locker(&_M_mutex);
    _M_img_buffer.enqueue(img); // push to back
     _M_condition.wakeOne(); // wake the sleeping thread
}

/*********************************************************/
/*********************************************************/
/*********************STORE DATA**************************/
/*********************************************************/
/*********************************************************/

//Store data
void PFiltering::_StoreData(_Processed_img img)
{
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
            }
        }
    }

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


//Process FFT filtering
void PFiltering::_Process_FFT_Filtering()
{
    qDebug()<<"PFiltering::_Process_FFT_Filtering()";

    //Do not process whole data filtering operation if all conditions are not required
    if((!_M_enable_data_correction && !_M_enable_low_pass_filtering) || !_M_enable_filtering)
    {
        _M_process_is_finished = true;
        _M_stop = true;
        return;
    }

    /*************************************************
     **********Initialization********* ***************
     *************************************************/

    //Axis
    QVector<float> x;
    x.clear();
    for(int i=0;i<_M_tot_frames;i++)
        x.push_back(i+1);

    /*************************************************
     ********** Define Low Pass Filter ***************
     *************************************************/
    QVector<float> filter_low_pass;
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
                #pragma omp critical
                {
                    emit newProgressStatut("Data correction",(int)((100*id*nb_thread)/_M_nb_Spatial_pixels));
                }
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
                #pragma omp critical
                {
                    emit newProgressStatut("Filtering",(int)((100*id*nb_thread)/_M_nb_Spatial_pixels));
                }

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
                #pragma omp critical
                {
                    emit newProgressStatut("Filtering",(int)((100*id*nb_thread)/_M_nb_Spatial_pixels));
                }
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



    _M_stop = true;
    _M_process_is_finished = true;

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




