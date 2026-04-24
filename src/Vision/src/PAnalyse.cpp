#include "PAnalyse.h"

PAnalyse::PAnalyse(QObject *parent):
    QThread(parent)
{

    //Hyperspectral img dynamic
    _M_HS_Dyn        = 16;


    //moveToThread(this);

    //Init result directory
    _M_result_directory =  "/home/results/";


    //Send acquisition info
    connect(&_M_filtering,SIGNAL(newAcquisitionInfo(QString)),this,SIGNAL(newAcquisitionInfo(QString)));

    //Max display value
    _M_max_display_value = 1;

    //chromophore ID (HbO2, Hb, (oxCCO))
    _M_chromophore_id = 0;

    //MEan ROI
    _M_Mean_ROI_radius = 5;

    //Enable measurement
    _M_request_Mean_ROI_Measure = false;

    //Save resutls
    _M_save_results = false;
//    connect(this,SIGNAL(request_onGetAll_maps()),this,SLOT(onGetAll_maps()));


    _M_processing_type                      = Process_Mean_Delta_C;

    //Z threshold
    connect(&_M_SPM,SIGNAL(newZThresh(double)),this,SIGNAL(newZThresh(double)));

    //Data info
    _M_nb_channels=3;
    _M_nb_temporal_vectors=0;
    _M_tot_frames=0;

    //Fs
    _M_frame_rate=0;

    //process times
    connect(&_M_paradigm_times,SIGNAL(newReferenceTime(int,int)),&_M_MBLL,SLOT(onnewReferenceTime(int,int)));
    connect(&_M_paradigm_times,SIGNAL(newCorrelationTimes(int,int)),&_M_MBLL,SLOT(onnewCorrelationTimes(int,int)));


    connect(&_M_paradigm_times,SIGNAL(newActivationtimes(QVector<double>)),this,SIGNAL(newActivationtimes(QVector<double>)));

    //Bold signal
    connect(&_M_paradigm_times,SIGNAL(newBoldSignal(QVector<float>)),this,SIGNAL(newBoldSignal(QVector<float>)));

    //Indicators
    _M_learningDone         =false;

    //ROI and pixel pos
    _M_ROI.clear();
    _M_pixels_pos.clear();


    /*********************************************/
    /***************FILTERING*********************/
    /*********************************************/


    //process not real Time process
    connect(&_M_filtering,SIGNAL(readyForProcessing()),this,SLOT(DataFinishLoading()));

    //Filtering mesg
    connect(&_M_filtering,SIGNAL(newProgressStatut(QString,int)),this,SIGNAL(newProgressStatut(QString,int)));



    /*********************************************/
    /***************Molar coeff handling**********/
    /*********************************************/
    //Launch process each time a new segmentation occured or each time a parameter changed
    connect(&_M_PixelWise_Molar_Coeff,SIGNAL(processFinished()),this,SLOT(LaunchProcess()));

}

PAnalyse::~PAnalyse()
{
    if (isRunning())
    {
        requestInterruption();
        wait();
    }
}


/** New result directory */
void PAnalyse::onNewResultDirectory(QString v)
{
    _M_result_directory = v;
    _M_filtering.onNewResultDirectory(v);
    _M_PixelWise_Molar_Coeff.onNewResultDirectory(v);
    _M_paradigm_times.onNewResultDirectory(v);
}




//Save results
void PAnalyse::onrequestSaveResults(_data_saving_info info)
{
    _M_saving_info = info;
    _M_save_results = true;

    LaunchProcess();
}

//Set number of resels
void PAnalyse::onNewReselsNumber(int r)
{
    _M_SPM.setNbResels(r);

    if(_M_learningDone)
        this->start();
}



// Get Delta C maps for statistical tests
QVector<Mat> PAnalyse::_Get_Delta_C_Maps(int nb_chromopores)
{
    //Nb of temporal elements
//    int T = _M_paradigm_times.get_last_analyzed_frame_for_Correlation() - _M_paradigm_times.get_first_analyzed_frame_for_Correlation();

    //Analysis length
    int start_corr = _M_paradigm_times.get_first_analyzed_frame_for_Correlation();
    int end_corr = _M_paradigm_times.get_last_analyzed_frame_for_Correlation();

    //Nb of temporal elements
    // int T = _M_tot_frames;
    int T = end_corr - start_corr;

    //Reference period
    int start_rest = _M_paradigm_times.get_rest_start_id();
    int end_rest = _M_paradigm_times.get_rest_end_id();

    //Process frame idx for data correction
    // _M_paradigm_times.process_frame_indexes_for_data_correction(start_corr,end_corr);


    qDebug()<<"[PAnalyse::_Get_Delta_C_Maps] start id "<<start_corr;
    qDebug()<<"[PAnalyse::_Get_Delta_C_Maps] end id "<<end_corr;
    qDebug()<<"[PAnalyse::_Get_Delta_C_Maps] start rest "<<start_rest;
    qDebug()<<"[PAnalyse::_Get_Delta_C_Maps] end rest "<<end_rest;

    //Init Delta C maps (HbO2,Hb,HbT)
    QVector<Mat> Delta_C_maps;
    for(int c=0;c<nb_chromopores;c++)
        Delta_C_maps.push_back(Mat::zeros(T,_M_nb_temporal_vectors,CV_32F)); //Size (T,N)

    //Get concentration changes maps
    //Process Loop
    #pragma omp parallel
    {
        int nb_thread = omp_get_num_threads();
        #pragma omp for
        for(int id=0;id<_M_nb_temporal_vectors;id++)
        {
            QVector<Mat> Molar_coeff;
            if(!_M_PixelWise_Molar_Coeff.getMolarCoeff(id,Molar_coeff))
                continue;

            #pragma omp critical
            {
                emit newProgressStatut("Get Delta C maps",(int)((100*id*nb_thread)/_M_nb_temporal_vectors));
            }

            //get concentration changes time courses
            QVector<QVector<float> > Delta_C;

            QVector<QVector<float> > temporal_vector = _M_filtering.getTemporalVector(id);
            _M_MBLL.get_Chromophore_Time_courses(temporal_vector,Molar_coeff,Delta_C,start_rest,end_rest,start_corr,end_corr);

            //Loop over time
            for(int t=0;t<T;t++)
            {
                //Store the HbO2,Hb, (oxCCO) values
                for(int c=0;c<nb_chromopores;c++)
                    Delta_C_maps[c].at<float>(t,id) = Delta_C[c][t];
            }
        }
    }


    qDebug()<<"[PAnalyse::_Get_Delta_C_Maps] ok";
    return Delta_C_maps;
}


//Get Camera intensities
QVector<Mat> PAnalyse::_Get_Camera_Intensity()
{
    //Get temp vector (spectral x temporal dimensions)
    QVector<QVector<float> > temp =  _M_filtering.getTemporalVector(0);

    //Nb of spectral channels
    int nb_spectral_channels = temp.size();

    //Nb of temporal elements
    int T = temp[0].size();


    //Init Intensity maps (one map per channel)
    QVector<Mat> I_maps;
    for(int c=0;c<nb_spectral_channels;c++)
        I_maps.push_back(Mat::zeros(T,_M_nb_temporal_vectors,CV_32F)); //Size (T,N)

    //Process Loop
    #pragma omp parallel
    {
        int nb_thread = omp_get_num_threads();
        #pragma omp for
        for(int id=0;id<_M_nb_temporal_vectors;id++)
        {
            #pragma omp critical
            {
                emit newProgressStatut("Get Intensity maps",(int)((100*id*nb_thread)/_M_nb_temporal_vectors));
            }
            QVector<Mat> Molar_coeff;
            if(!_M_PixelWise_Molar_Coeff.getMolarCoeff(id,Molar_coeff))
                continue;

            //get concentration changes time courses
            QVector<QVector<float> > temporal_vector = _M_filtering.getTemporalVector(id);

            //Loop over time
            for(int t=0;t<T;t++)
            {
                //Store the HbO2,Hb, (oxCCO) values
                for(int c=0;c<nb_spectral_channels;c++)
                    I_maps[c].at<float>(t,id) = temporal_vector[c][t];
            }
        }
    }

    return I_maps;
}



void PAnalyse::_Save_Results()
{

    qDebug()<<"[PAnalyse::_Save_Results] start id "<<_M_paradigm_times.get_first_analyzed_frame_for_Correlation();
    qDebug()<<"[PAnalyse::_Save_Results] end id "<<_M_paradigm_times.get_last_analyzed_frame_for_Correlation();
    qDebug()<<"[PAnalyse::_Save_Results] start rest "<<_M_paradigm_times.get_rest_start_id();
    qDebug()<<"[PAnalyse::_Save_Results] end rest "<<_M_paradigm_times.get_rest_start_id();



    emit newProgressStatut("Save info",0);
    //Create a directory to store the results
//    QString saving_dir  =QString(_M_result_directory)+""+QTime::currentTime().toString("hh:mm:ss.zzz")+"/";
//    QDir().mkdir(saving_dir);

    QString saving_dir  =QString(_M_result_directory);


    //Save general info
    //Write pixel pos
    WritePointVector(saving_dir+"pixel_pos.txt",_M_pixels_pos);

    //Initial image
    if(type2str(_M_initial_img.type()) == "8UC3")
    {
        Mat in_BGR;
        cvtColor( _M_initial_img,in_BGR,CV_RGB2BGR);
        imwrite(QString(saving_dir+"initial_img.png").toStdString(),in_BGR);
    }
    else
        imwrite(QString(saving_dir+"initial_img.png").toStdString(),_M_initial_img);




    qDebug()<<"[PAnalyse::_Save_Results] mode tb";

    //Write Bold signal
    WriteTemporalVector(saving_dir+"Bold_signal.txt",_M_paradigm_times.get_Bold_Signal());
    WriteTemporalVector(saving_dir+"Neuronal_Activity.txt",_M_paradigm_times.get_Activation_Signal());


    //get id activity steps
    QVector<int> activity_start = _M_paradigm_times.get_Full_activity_start();
    QVector<int> activity_end   = _M_paradigm_times.get_Full_activity_end();

    //Write activity idx (correlation times)
    QFile file(saving_dir+"info_activity_idx.txt");
    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );
        for(int i=0;i<activity_start.size();i++)
            stream <<activity_start[i]-_M_paradigm_times.get_first_analyzed_frame_for_Correlation()<<" "<<activity_end[i]-_M_paradigm_times.get_first_analyzed_frame_for_Correlation()<<"\n";

        file.close();
    }

    //Write full activity idx
    file.setFileName(saving_dir+"info_activity_full_idx.txt");
    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );

        for(int i=0;i<activity_start.size();i++)
            stream <<activity_start[i]<<" "<<activity_end[i]<<"\n";

        file.close();
    }


    //Save mask for ROI
    Mat mask = Mat::zeros(_M_img_size,CV_8UC1);
    vector<vector<Point> > c;
    c.push_back(_M_ROI);
    drawContours(mask,c,0,Scalar(255),CV_FILLED);
    resize(mask,mask,_M_initial_img.size());
    threshold(mask,mask,0,255,CV_THRESH_BINARY);
    imwrite(QString(saving_dir+"mask.png").toStdString(),mask);





    //Write Acquisition info
    file.setFileName(saving_dir+"img_size.txt");
    // Write infos.txt containing times of the paradigm and Fs
    if (file.open(QIODevice::ReadWrite))
    {
        QTextStream stream( &file );
        stream<<"Img size: "<<_M_img_size.height<<" "<<_M_img_size.width<<"\n";

    }
    file.close();


    //Write rest (full idx)
    file.setFileName(saving_dir+"info_rest_full_idx.txt");
    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );

        stream <<_M_paradigm_times.get_rest_start_id()<<" "<<_M_paradigm_times.get_rest_end_id()<<"\n";

        file.close();
    }



    //Write start and end idx
    file.setFileName(saving_dir+"info_start_end_idx.txt");
    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );
        stream <<_M_paradigm_times.get_first_analyzed_frame_for_Correlation()<<" "<<_M_paradigm_times.get_last_analyzed_frame_for_Correlation()<<"\n";
        file.close();
    }

    //Nb of chromophore
    int nb_chromopores = _M_PixelWise_Molar_Coeff.getChromophoreNumber()+1;


    //Model for correlation
    QVector<QVector<float> > model_for_correlation;
    //HbO2
    model_for_correlation.push_back(_M_paradigm_times.get_Bold_Signal());
    //Hb
    QVector<float> Hb_Bold = _M_paradigm_times.get_Bold_Signal();
    for(int i=0;i<Hb_Bold.size();i++)
        Hb_Bold[i]*=-1;
    model_for_correlation.push_back(Hb_Bold);
    //HbT
    model_for_correlation.push_back(_M_paradigm_times.get_Bold_Signal());



    emit newProgressStatut("Save info",100);




    //Check if at least one option is checked
    if(!_M_saving_info.save_camera_intensity &&  !_M_saving_info.save_Delta_C)
        return;



    //Save Delta C
    qDebug()<<"[PAnalyse::_Save_Results] save_Delta_C";
    if(_M_saving_info.save_Delta_C)
    {
        //Get Delta C maps (HbO2,Hb,HbT) Get full signal
        QVector<Mat> Delta_C_maps = _Get_Delta_C_Maps(nb_chromopores);

        //Create directory
        QString contrast_dir  = saving_dir+"Filtered_Delta_C/";
        QDir().mkdir(contrast_dir);
        emit newProgressStatut("Save Delta C",0);

        //Save contrasts
        for(int c=0;c<Delta_C_maps.size();c++)
        {
            QString file_path = (contrast_dir+ "MBLL_"+QString::number(c)+".dat");
            // WriteFloatImg(file_path,Delta_C_maps[c]);
            WriteFloatImg_bin(file_path,Delta_C_maps[c]);

            emit newProgressStatut("Save Delta C",(int)(100*(float)(c/(nb_chromopores-1))));
        }
    }


    //Save Camera intensity
    qDebug()<<"[PAnalyse::_Save_Results] save_camera_intensity";
    if(_M_saving_info.save_camera_intensity)
    {
        //Get camera intensities
        QVector<Mat> I_maps = _Get_Camera_Intensity();

        //Create directory
        QString I_dir  = saving_dir+"Filtered_Camera_intensity/";
        QDir().mkdir(I_dir);
        emit newProgressStatut("Save camera intensity",0);

        //Save intensity maps
        for(int c=0;c<I_maps.size();c++)
        {

            QString file_path = I_dir+ "I_"+QString::number(c)+".dat";
            //WriteFloatImg(file_path,I_maps[c]);
            WriteFloatImg_bin(file_path,I_maps[c]);
            emit newProgressStatut("Save camera intensity",(int)(100*(float)(c/(I_maps.size()-1))));
        }

    }

    emit newProgressStatut("Save data",100);
}




// Slot activated when process button is clicked
// Start Qthread run function
void PAnalyse::LaunchProcess()
{

    if(_M_ROI.empty())
    {
        qDebug()<<"Bad learning";
        return;
    }

    if(!_M_learningDone)
        return;

    qDebug()<<"PAnalyse::LaunchProcess start analysis";
    this->start();
}

//Run function herited from QThread
void PAnalyse::run()
{
    qDebug()<<"[PAnalyse::run]";

    QElapsedTimer elapsed_timer;
    elapsed_timer.start();


    //save results
    if(_M_save_results)
    {
        _Save_Results();
        _M_save_results = false;
        return;
    }


    switch (_M_processing_type)
    {

    case Activation_GLM_Pixel_wise:
        _General_Linear_Model_Random_Field_Theory();
        break;
    case Activation_GLM_auto_thesh:
        _General_Linear_Model_Auto_thresh();
        break;
    case Process_Mean_Delta_C:
        _ProcessMeanConcentrationMeasure();
        break;
    case Process_Correlation:
        _ProcessCorrelationMeasure();
        break;
    default:
        _General_Linear_Model_Random_Field_Theory();
        break;
    }

    emit Elapsed_ProcessingTime(elapsed_timer.elapsed());
}


/*************************************************
 ********** Process Correlation measure **********
 *************************************************/

void PAnalyse::_ProcessCorrelationMeasure()
{

    QVector<QVector<float> > Correlation_maps;
    _ProcessCorrelation(Correlation_maps);

    //Send Contrast Cartography to IHM
    QVector<Mat> contrast_img;
    for(int i =0 ;i<Correlation_maps.size();i++)
        contrast_img.push_back(_Display(Correlation_maps[i]));


    //Write results
    QString temp_name = "Corr_MBLL";

//    QString saving_dir  =QString(_M_result_directory)+temp_name+QTime::currentTime().toString("hh:mm:ss.zzz")+"/";
    QString saving_dir  =QString(_M_result_directory)+temp_name+"/";
    QDir dir(saving_dir);

    //Save results only if saving is not created
    if(!dir.exists())
        dir.mkdir(saving_dir);


    //Write correlation cartographies
    for(int i =0 ;i<contrast_img.size();i++)
        WriteFloatImg(saving_dir+"Corr_map_"+QString::number(i)+".txt",contrast_img[i]);

    //Save Initial image
    if(type2str(_M_initial_img.type()) == "8UC3")
    {
        Mat in_BGR;
        cvtColor( _M_initial_img,in_BGR,CV_RGB2BGR);
        imwrite(QString(saving_dir+"initial_img.png").toStdString(),in_BGR);
    }
    else
    {
        imwrite(QString(saving_dir+"initial_img.png").toStdString(),_M_initial_img);
    }

    //Save mask for ROI
    Mat mask = Mat::zeros(_M_img_size,CV_8UC1);
    vector<vector<Point> > c;
    c.push_back(_M_ROI);
    drawContours(mask,c,0,Scalar(255),CV_FILLED);
    resize(mask,mask,_M_initial_img.size());
    threshold(mask,mask,0,255,CV_THRESH_BINARY);
    imwrite(QString(saving_dir+"mask.png").toStdString(),mask);


    emit newContrastImage(contrast_img);
}



//Process correlation
void PAnalyse::_ProcessCorrelation(QVector<QVector<float> > &Correlation_map)
{

    //Init
    emit newProgressStatut("Processing",0);


    //Control on row data vector
    if(!_M_filtering.isRowDataValid())
    {
        qDebug()<<"[PAnalyse] row data is not valid";
        return;
    }

    //results vectors
    int nb_Measure  = _M_PixelWise_Molar_Coeff.getChromophoreNumber()+1 ; //Hb, HbO2, (oxCCO), HbT


    //Analysis length
    int correlation_start = _M_paradigm_times.get_first_analyzed_frame_for_Correlation();
    int correlation_end = _M_paradigm_times.get_last_analyzed_frame_for_Correlation();

    //reference
    int rest_start = _M_paradigm_times.get_rest_start_id();
    int rest_end = _M_paradigm_times.get_rest_end_id();

    //Model for correlation
    QVector<QVector<float> > model_for_correlation;
    //HbO2
    model_for_correlation.push_back(_M_paradigm_times.get_Bold_Signal());
    //Hb
    QVector<float> Hb_Bold = _M_paradigm_times.get_Bold_Signal();
    for(int i=0;i<Hb_Bold.size();i++)
        Hb_Bold[i]*=-1;
    model_for_correlation.push_back(Hb_Bold);
    //HbT
    model_for_correlation.push_back(_M_paradigm_times.get_Bold_Signal());


    WriteTemporalVector(_M_result_directory + "model.txt",model_for_correlation);

    Correlation_map.clear();
    Correlation_map.resize(nb_Measure);

    for(int i=0;i<nb_Measure;i++)
        Correlation_map[i].fill(0,_M_nb_temporal_vectors);



    //Process Loop ([Hb], [HbO2] calculation)
    #pragma omp parallel
    {
        int nb_thread = omp_get_num_threads();
        #pragma omp for
        for(int id=0;id<_M_nb_temporal_vectors;id++)
        {
            QVector<Mat> Molar_coeff;
            if(!_M_PixelWise_Molar_Coeff.getMolarCoeff(id,Molar_coeff))
                continue;

            #pragma omp critical
            {
               emit newProgressStatut("Measure",(int)((100*id*nb_thread)/_M_nb_temporal_vectors));
            }


            QVector<double> temp_Corr;
            QVector<QVector<float> > contrast;

            //get concentration changes time courses
            _M_MBLL.get_Chromophore_Time_courses(_M_filtering.getTemporalVector(id),Molar_coeff,contrast,rest_start,rest_end,correlation_start,correlation_end);



            // Compare bold vs contrast measures
            Compare_Bold_Vs_Oxy(contrast,model_for_correlation,temp_Corr);


            //Store results in array
            for(int i=0;i<nb_Measure;i++)
            {
                Correlation_map[i][id]  = temp_Corr[i];
            }
        }
    }


//    //TEMP
//    WriteTemporalVector(QString(_M_result_directory)+"Corr.txt",Concentration_map);
//    //END TEMP

    if(Correlation_map[0].empty() || _M_pixels_pos.size()!=Correlation_map[0].size())
    {
        qDebug()<<"[NotRealTimeProcess] Vector has not the same dimension";
        return;
    }

    emit newProgressStatut("Measure",100);
}


/*************************************************
 ****** Process mean concentration measure *******
 *************************************************/

void  PAnalyse::_ProcessMeanConcentrationMeasure()
{
    QVector<Mat> contrast = _Process_Mean_Delta_C();

    //Write results
    QString temp_name = "Mean_C_MBLL";

//    QString saving_dir  =QString(_M_result_directory)+temp_name+QTime::currentTime().toString("hh:mm:ss.zzz")+"/";
    QString saving_dir  =QString(_M_result_directory)+temp_name+"/";
    QDir dir(saving_dir);

    //Save results only if saving is not created
    if(!dir.exists())
        dir.mkdir(saving_dir);

    //Write correlation cartographies
    for(int i =0 ;i<contrast.size();i++)
        WriteFloatImg(saving_dir+"Delta_C_map_"+QString::number(i)+".txt",contrast[i]);

    //Save Initial image
    if(type2str(_M_initial_img.type()) == "8UC3")
    {
        Mat in_BGR;
        cvtColor( _M_initial_img,in_BGR,CV_RGB2BGR);
        imwrite(QString(saving_dir+"initial_img.png").toStdString(),in_BGR);
    }
    else
    {
        imwrite(QString(saving_dir+"initial_img.png").toStdString(),_M_initial_img);
    }

    //Save mask for ROI
    Mat mask = Mat::zeros(_M_img_size,CV_8UC1);
    vector<vector<Point> > c;
    c.push_back(_M_ROI);
    drawContours(mask,c,0,Scalar(255),CV_FILLED);
    resize(mask,mask,_M_initial_img.size());
    threshold(mask,mask,0,255,CV_THRESH_BINARY);
    imwrite(QString(saving_dir+"mask.png").toStdString(),mask);

}

// Compute Mean Delta C
QVector<Mat> PAnalyse::_Process_Mean_Delta_C()
{
    qDebug()<<"PAnalyse::_Process_Mean_Beer_Lambert_Method";

    //Get activity times
    QVector<int> id_start   = _M_paradigm_times.get_Full_activity_start();
    QVector<int> id_end     = _M_paradigm_times.get_Full_activity_end();


    //Analysis length
    int start_corr = _M_paradigm_times.get_first_analyzed_frame_for_Correlation();
    int end_corr = _M_paradigm_times.get_last_analyzed_frame_for_Correlation();

    //reference
    int start_ref = _M_paradigm_times.get_rest_start_id();
    int end_ref = _M_paradigm_times.get_rest_end_id();



    qDebug()<<"[PAnalyse::_Process_Mean_Delta_C] Start corr: "<<start_corr;
    qDebug()<<"[PAnalyse::_Process_Mean_Delta_C] End corr: "<<end_corr;

    qDebug()<<"[PAnalyse::_Process_Mean_Delta_C] Start activity: "<<id_start;
    qDebug()<<"[PAnalyse::_Process_Mean_Delta_C] End activity: "<<id_end;

    qDebug()<<"[PAnalyse::_Process_Mean_Delta_C] Start ref: "<<start_ref;
    qDebug()<<"[PAnalyse::_Process_Mean_Delta_C] End ref: "<<end_ref;


    // _M_paradigm_times.process_frame_indexes_for_data_correction(start_corr,end_corr);



    emit newProgressStatut("Processing",0);

    int nb_Measure  =  _M_PixelWise_Molar_Coeff.getChromophoreNumber()+1 ; //HbO2, Hb (oxCCO), HbT


    //results vectors
    QVector<Mat> contrast;
    contrast.resize(nb_Measure);


    //Control on row data vector
    if(!_M_filtering.isRowDataValid())
    {
        qDebug()<<"[PAnalyse] row data is not valid";
        return contrast;
    }


    QVector<QVector<float> > proc_mod;
    proc_mod.resize(nb_Measure);


    for(int i=0;i<nb_Measure;i++)
    {
        proc_mod[i].fill(0,_M_nb_temporal_vectors);
    }

    qDebug()<<"PAnalyse::_Process_Mean_Beer_Lambert_Method nb measure: "<<nb_Measure;


    //Process Loop (Chromophore quantification)
    #pragma omp parallel
    {
        #pragma omp for
        for(int id=0;id<_M_nb_temporal_vectors;id++)
        {
            QVector<Mat> Molar_coeff;
            if(!_M_PixelWise_Molar_Coeff.getMolarCoeff(id,Molar_coeff))
                continue;

            // #pragma omp critical
            // {
            //     emit newProgressStatut("Measure",(int)((100*id*nb_thread)/_M_nb_temporal_vectors));
            // }

            QVector<float> temp_proc_mod;
            temp_proc_mod.fill(0,nb_Measure);

            for(int i=0;i<id_start.size();i++)
            {
                QVector<float> temp;
                _M_MBLL.getAveragedConcentrationChanges(_M_filtering.getTemporalVector(id),Molar_coeff,temp,id_start[i],id_end[i],start_ref,end_ref,start_corr,end_corr);

                for(int y=0;y<nb_Measure;y++)
                    temp_proc_mod[y] += temp[y];
            }

            //Store results in array
            for(int y=0;y<nb_Measure;y++)
                proc_mod[y][id] = temp_proc_mod[y]/id_start.size();
        }
    }

    qDebug()<<"[QVector<Mat> PAnalyse::_Process_Mean_Delta_C] Processing finished";


    if(proc_mod.empty())
    {
        qDebug()<<"[NotRealTimeProcess] Empty vectors";
        return contrast;
    }
    if(_M_pixels_pos.size()!=proc_mod[0].size())
    {
        qDebug()<<"[NotRealTimeProcess] Wrong vectors dimension";
        return contrast;
    }

    for(int nb=0;nb<nb_Measure;nb++)
        contrast[nb]=_Display(proc_mod[nb]);

    //Send Contrast Cartography to IHM
    emit newContrastImage(contrast);
    emit newProgressStatut("Measure",100);

    return contrast;
}





//Inform Data finish loading (for not Real time Process)
void PAnalyse::DataFinishLoading()
{
    qDebug()<<"[PAnalyse] filtering is finished, process data in not real time";
    if(_M_ROI.empty())
    {
        qDebug()<<"Bad learning";
        return;
    }
    _M_learningDone = true;

    //Require auto coeff computation
    // _Compute_Auto_Coeff();

    this->start();
}


/*************************************************
 ************ Compute result display**************
 ************************************************/

//Display contrast oxygnation for each pixel contained in the selected ROI
Mat PAnalyse::_Display(QVector<float> &contrast_proc)
{
    qDebug()<<"in PAnalyse::_Display";
    //Creating file for display
    Mat img_contrast=Mat::zeros(_M_img_size,CV_32F);

    for(int i=0;i<_M_pixels_pos.size();i++)
        img_contrast.at<float>(_M_pixels_pos[i].y,_M_pixels_pos[i].x)        = (float)(contrast_proc[i]);

    //Apply Median blur
//    medianBlur(img_contrast,img_contrast,5);

    return img_contrast;
}

void PAnalyse::_writeFloatCartography(Mat &contrast_proc,QString filename)
{
    //Creating file for display
    Mat img_contrast=Mat::zeros(_M_img_size,CV_32F);

    for(int i=0;i<_M_pixels_pos.size();i++)
        img_contrast.at<float>(_M_pixels_pos[i].y,_M_pixels_pos[i].x)    = contrast_proc.at<float>(0,i);


    // Apply the specified morphology operation

    WriteFloatImg(filename,img_contrast);
}


Mat PAnalyse::_Convert_to_FalseColor(Mat &in,Mat &mask_img,double &min,double &max)
{
    if (max == -1)
        max = _M_max_display_value/1000000;
    if(min == -1)
        min = -max;

    //Convert into false colormap
    Mat img_contrast    = Mat::zeros(in.size(),CV_8UC1);
    Mat mask            = Mat::zeros(img_contrast.size(),CV_8UC1);

    for(int row=0;row<img_contrast.rows;row++)
    {
        uchar *ptr_u        = img_contrast.ptr<uchar>(row);
        uchar * ptr_mask    = mask.ptr<uchar>(row);
        float *ptr_f        = in.ptr<float>(row);

        for(int col=0;col<img_contrast.cols;col++)
        {
            if(ptr_f[col]!=0)
            {
                ptr_mask[col]=255;
                if(ptr_f[col]>0)
                {
                    //Ajout de 127 (255/2) qui correspond au 0
                    //On normalise les valeurs entre 0->maxval à 127->255
                    int v       = (ptr_f[col] * ((255-127)/(max))) + 127;
                    v           = (v>255) ? 255 : v;
                    ptr_u[col]  = uchar(v);

//                    if(ptr_u[col]>=127)
//                        ptr_mask[col]=255;
                }
                else
                {
                    //On ramène les valeurs négatives supérieure à 0 et inférieure à 127
                    //On normalise les valeurs entre -maxval->0 à 0->127
                    int v       = (ptr_f[col] -min)* (127/(-min));
                    v           = (v<0)? 0 : v;
                    ptr_u[col]  = uchar(v);

//                    if(ptr_u[col]<=127 )
//                        ptr_mask[col]=255;
                }
            }
        }
    }

    //bitwise and between maskf and the img mask
    bitwise_and(mask,mask_img,mask);

    Mat falseColorsMap;
    applyColorMap(img_contrast, falseColorsMap, COLORMAP_JET);
    for(int row=0;row<falseColorsMap.rows;row++)
    {
        _Mycolor *ptr   = falseColorsMap.ptr<_Mycolor>(row);
        uchar *ptr_mask = mask.ptr<uchar>(row);

        for(int col=0;col<falseColorsMap.cols;col++)
        {
            if(ptr_mask[col]==0)
            {
                ptr[col].b=0;
                ptr[col].g=0;
                ptr[col].r=0;
            }
        }
    }
    return falseColorsMap;

}



Mat PAnalyse::_drawTextOnImg(Mat img,String text,int pos,double font_size,int thick)
{
    Scalar font_color(255,255,255);

    int font = FONT_HERSHEY_COMPLEX_SMALL;


    Size size_txt = getTextSize(text,font,font_size,thick,0);
    int text_height = size_txt.height;
    int text_width = size_txt.width;

    text_height += 5;

    if (text_height%2==1)
        text_height = text_height +1;

    if (pos == 0) //bottom right corner
    {
        putText(img,text,Point(img.cols-text_width,img.rows-3),font,font_size,font_color,thick,LINE_AA);
        return img;
    }

    if (pos == 1) //top right corner
    {
        putText(img,text,Point(img.cols-text_width,text_height),font,font_size,font_color,thick,LINE_AA);
        return img;
    }

    if (pos == 2) //top left corner
    {
        putText(img,text,Point(text_width,text_height),font,font_size,font_color,thick,LINE_AA);
        return img;
    }

    putText(img,text,Point(img.cols-text_width,int(pos-text_height/4)),font,font_size,font_color,thick,LINE_AA);
    return img;
}

Mat PAnalyse::_create_colorbar(Mat in_img)
{
    //Colorbar
    Mat colorbar    = Mat::zeros(256,5,CV_8UC1);
    Mat falsecolor;
    for(int row=0;row<colorbar.rows;row++)
    {
        uchar *ptr=colorbar.ptr<uchar>(row);
        for(int col=0;col<colorbar.cols;col++)
            ptr[col] = 255-row;
    }
    applyColorMap(colorbar,falsecolor,COLORMAP_JET);

    cv::Size _size(int(in_img.cols/32),in_img.rows);
    resize(falsecolor,falsecolor,_size);
    return falsecolor;
}


Mat PAnalyse::_reconstruct_Float_Carography(Mat &contrast_proc)
{
    //Creating file for display
    Mat img_contrast=Mat::zeros(_M_img_size,CV_32F);

    for(int i=0;i<_M_pixels_pos.size();i++)
        img_contrast.at<float>(_M_pixels_pos[i].y,_M_pixels_pos[i].x)    = contrast_proc.at<float>(0,i);

    return img_contrast;
}

Mat PAnalyse::_reconstruct_uchar_Carography(Mat &contrast_proc)
{
    //Creating file for display
    Mat img_contrast=Mat::zeros(_M_img_size,CV_8UC1);

    for(int i=0;i<_M_pixels_pos.size();i++)
        img_contrast.at<uchar>(_M_pixels_pos[i].y,_M_pixels_pos[i].x)    = contrast_proc.at<uchar>(0,i);

    return img_contrast;
}



Mat PAnalyse::_writeCartography(Mat &contrast_proc,QString filename)
{
    //Creating file for display
    Mat img_contrast=Mat::zeros(_M_img_size,CV_8UC1);

    for(int i=0;i<_M_pixels_pos.size();i++)
        img_contrast.at<uchar>(_M_pixels_pos[i].y,_M_pixels_pos[i].x)    = contrast_proc.at<uchar>(0,i);


    // Apply the specified morphology operation
    Mat write_img;
    Mat element = getStructuringElement(MORPH_ELLIPSE,Size(3,3));
    morphologyEx(img_contrast, write_img, MORPH_CLOSE, element,Point(-1,-1),2);

    //Apply Median blur
//    medianBlur(write_img,write_img,7);

    imwrite(filename.toStdString(),write_img);
    return img_contrast;
}


Mat PAnalyse::_Display(QVector<bool> &contrast_proc)
{
    //Creating file for display
    Mat img_contrast=Mat::zeros(_M_img_size,CV_8UC1);

    for(int i=0;i<_M_pixels_pos.size();i++)
        img_contrast.at<uchar>(_M_pixels_pos[i].y,_M_pixels_pos[i].x)        = (contrast_proc[i]);

    //Apply Median blur
//    medianBlur(img_contrast,img_contrast,5);

    return img_contrast;
}

Mat PAnalyse::_Display(Mat &contrast_proc)
{
    //Creating file for display
    Mat img_contrast=Mat::zeros(_M_img_size,CV_8UC1);

    for(int i=0;i<_M_pixels_pos.size();i++)
        img_contrast.at<uchar>(_M_pixels_pos[i].y,_M_pixels_pos[i].x)        = contrast_proc.at<uchar>(0,i);

//    // Apply the specified morphology operation
//    Mat element = getStructuringElement(MORPH_ELLIPSE,Size(3,3));
//    morphologyEx(img_contrast, img_contrast, MORPH_CLOSE, element,Point(-1,-1),2);

    return img_contrast;
}


Mat PAnalyse::_Display(const Mat &contrast_proc,const QVector<int> &id)
{
    //Creating file for display
    Mat img_contrast=Mat::zeros(_M_img_size,CV_32F);

    for(int i=0;i<id.size();i++)
        img_contrast.at<float>(_M_pixels_pos[id[i]].y,_M_pixels_pos[id[i]].x)        = contrast_proc.at<float>(0,i);

    //Apply Median blur
//    medianBlur(img_contrast,img_contrast,5);

    return img_contrast;
}

/********************************************************
 ********* Set ROI AND RESIZE DATA VECTOR****************
 ********* To fit NbFrames x nbChannels x NbPixels*******
 ********************************************************/


// Set ROI choosen by the user and get the pixel positions inside this ROI
void PAnalyse::setAnalysisZone(QVector<QPoint> c,Size img_size,int nbChannels,int NbFrames)
{
    qDebug()<<"PAnalyse::setAnalysisZone";

    //Write Analysis zone

    //Create output directory
    QString saving_dir  =QString(_M_result_directory)+"contours";
    if(!QDir().exists(saving_dir))
        QDir().mkdir(saving_dir);

    QFile file(saving_dir+"/ROI_contour.txt");
    if(!file.exists())
        WritePointVector(saving_dir+"/ROI_contour.txt",c);

    //Set size
    _M_img_size=img_size;

    //Initialize indicators
    _M_learningDone     =false;

    //ROI
    _M_ROI.clear();
    for(int i=0;i<c.size();i++)
        _M_ROI.push_back(Point(c[i].x(),c[i].y()));

    //Store pixels positions in vector _M_pixels_pos
    getPixelPos(_M_pixels_pos,_M_ROI,img_size,1);
    _M_PixelWise_Molar_Coeff.setPixelsPos(_M_pixels_pos);

    _M_PixelWise_Molar_Coeff.initMolarCoeff();


    //Data info
    _M_nb_channels          = nbChannels;
    _M_nb_temporal_vectors  = _M_pixels_pos.size();
    _M_tot_frames           = NbFrames;

//    //Control data weight
//    double data_weight = ((double)_M_pixels_pos.size()*(double)NbFrames*(double)nbChannels*sizeof(float));
//    qDebug()<<"total weight : "<<data_weight+_M_ROI.size()*sizeof(QPoint)+_M_pixels_pos.size()*sizeof(QPoint);


//    if(data_weight > MAX_data_WEIGHT_GB) // Si la prévision est supérieure
//    {
//        qDebug()<<"Too many datas > "<<MAX_data_WEIGHT_GB<<" GB ";
//        emit Error("Too many datas"+QString::number(data_weight)+" Go !! Change ROI or increase spatial sampling.");
//        return;
//    }

    //Init filtering class
    _M_filtering.setPixelPos(_M_pixels_pos);
    _M_filtering.init(_M_pixels_pos.size(),nbChannels,NbFrames);
//    if(!_M_filtering.init(_M_pixels_pos.size(),nbChannels,NbFrames))
//    {
//        emit Error("Too many datas"+QString::number(data_weight)+" Go !! Change ROI or increase spatial sampling.");
//        return;
//    }



    //init process times
    _M_paradigm_times.setNbTotFrames(NbFrames);




    qDebug()<<"PAnalyse::Data acquisition is ready";
    emit newPixelPos(_M_pixels_pos);
    emit DataAcquisitionIsReady();
}




/*************************************************
 **************** AddD atas **********************
 *************************************************/

void PAnalyse::addDatas(_Processed_img &img)
{
    //Send img to filtering class
    _M_filtering.new_Filtering(img);
}


//Initialization
void PAnalyse::Initialize()
{
    _M_ROI.clear();
    _M_pixels_pos.clear();
    _M_learningDone         = false;
}

/********************************************/
/********************************************/
/***************Statistics*******************/
/********************************************/
/********************************************/

//on new stats type
void PAnalyse::onnewStatType(int v)
{
    _M_processing_type=v;

    if(!_M_learningDone)
        return;

    if(!this->isRunning())
        this->start();
}


void PAnalyse::_General_Linear_Model_Auto_thresh()
{
    //Create a directory to store the results
    QString saving_dir  =QString(_M_result_directory)+"SPM_auto_thresh";
    saving_dir = saving_dir+"_MBLL/";

    if(!QDir().exists(saving_dir))
        QDir().mkdir(saving_dir);


    //Nb of chromophore
    int nb_chromopores = _M_PixelWise_Molar_Coeff.getChromophoreNumber()+1;

    //Init Delta C maps (HbO2,Hb,HbT)
    QVector<Mat> Delta_C_maps = _Get_Delta_C_Maps(nb_chromopores);

    //Model for correlation
    QVector<QVector<float> > model_for_correlation;
    //HbO2
    model_for_correlation.push_back(_M_paradigm_times.get_Bold_Signal());
    //Hb
    QVector<float> Hb_Bold = _M_paradigm_times.get_Bold_Signal();
    for(int i=0;i<Hb_Bold.size();i++)
        Hb_Bold[i]*=-1;
    model_for_correlation.push_back(Hb_Bold);
    //HbT
    model_for_correlation.push_back(_M_paradigm_times.get_Bold_Signal());

    //Calculate image mask
    Mat mask = Mat::zeros(_M_img_size,CV_8UC1);
    vector<vector<Point> > c;
    c.push_back(_M_ROI);
    drawContours(mask,c,0,Scalar(255),CV_FILLED);



    //Compute SPM
    Mat activation_map=Mat::zeros(_M_img_size,CV_8UC1);
    QVector<Mat> Z_maps;



    emit newProgressStatut("SPM",0);

    for(int c=0;c<nb_chromopores;c++)
    {
        qDebug()<<"[PAnalyse::_General_Linear_Model_Auto_thresh] Delta_C_maps[c] : "<<Delta_C_maps[c].rows<<";"<<Delta_C_maps[c].cols<<" model: "<<model_for_correlation[c].size();


        //Create a directory to store the results
        QString path =saving_dir+QString::number(c)+"/";
        QDir().mkdir(path);

        emit newProgressStatut("Save SPM",(int)(100*(float)(c/(nb_chromopores))));
        //Compute The Statistical Parametric Mapping
        Z_maps.push_back(_M_SPM.process_SPM_auto_thresh(Delta_C_maps[c],_M_pixels_pos,_M_img_size,path, model_for_correlation[c], mask, 1.15f));

    }

    //Save Initial image
    if(type2str(_M_initial_img.type()) == "8UC3")
    {
        Mat in_BGR;
        cvtColor( _M_initial_img,in_BGR,CV_RGB2BGR);
        imwrite(QString(saving_dir+"initial_img.png").toStdString(),in_BGR);
    }
    else
    {
        imwrite(QString(saving_dir+"initial_img.png").toStdString(),_M_initial_img);
    }

    //Save mask for ROI
    resize(mask,mask,_M_initial_img.size());
    threshold(mask,mask,0,255,CV_THRESH_BINARY);
    imwrite(QString(saving_dir+"mask.png").toStdString(),mask);


    //Write SPM info
    _M_SPM.Write_SPM_info(saving_dir);

    //Save model
    WriteTemporalVector(saving_dir+"model.txt",model_for_correlation);

    emit newContrastImage(Z_maps);
    emit newProgressStatut("SPM",100);

}





//General linear model
void PAnalyse::_General_Linear_Model_Random_Field_Theory()
{
    //Create a directory to store the results
    QString saving_dir  =QString(_M_result_directory)+"SPM";
    saving_dir = saving_dir+"_MBLL/";

    if(!QDir().exists(saving_dir))
        QDir().mkdir(saving_dir);


    //Nb of chromophore
    int nb_chromopores = _M_PixelWise_Molar_Coeff.getChromophoreNumber()+1;

    //Init Delta C maps (HbO2,Hb,HbT)
    QVector<Mat> Delta_C_maps = _Get_Delta_C_Maps(nb_chromopores);

    //Model for correlation
    QVector<QVector<float> > model_for_correlation;
    //HbO2
    model_for_correlation.push_back(_M_paradigm_times.get_Bold_Signal());
    //Hb
    QVector<float> Hb_Bold = _M_paradigm_times.get_Bold_Signal();
    for(int i=0;i<Hb_Bold.size();i++)
        Hb_Bold[i]*=-1;
    model_for_correlation.push_back(Hb_Bold);
    //HbT
    model_for_correlation.push_back(_M_paradigm_times.get_Bold_Signal());


    //Compute SPM
    Mat activation_map=Mat::zeros(_M_img_size,CV_8UC1);
    QVector<Mat> Z_maps;

    emit newProgressStatut("SPM",0);

    for(int c=0;c<nb_chromopores;c++)
    {
        qDebug()<<"[PAnalyse::_General_Linear_Model_pixel_wise] Delta_C_maps[c] : "<<Delta_C_maps[c].rows<<";"<<Delta_C_maps[c].cols<<" model: "<<model_for_correlation[c].size();


        //Create a directory to store the results
        QString path =saving_dir+QString::number(c)+"/";
        QDir().mkdir(path);

        emit newProgressStatut("Save SPM",(int)(100*(float)(c/(nb_chromopores))));
        //Compute The Statistical Parametric Mapping

        Mat mask_SPM;
        Z_maps.push_back(_M_SPM.process_SPM(Delta_C_maps[c],_M_pixels_pos,_M_img_size,path, model_for_correlation[c],mask_SPM));

        //Get threshold map
        bitwise_or(activation_map,mask_SPM,activation_map);
    }

    //Save Initial image
    if(type2str(_M_initial_img.type()) == "8UC3")
    {
        Mat in_BGR;
        cvtColor( _M_initial_img,in_BGR,CV_RGB2BGR);
        imwrite(QString(saving_dir+"initial_img.png").toStdString(),in_BGR);
    }
    else
    {
        imwrite(QString(saving_dir+"initial_img.png").toStdString(),_M_initial_img);
    }

    //Save mask for ROI
    Mat mask = Mat::zeros(_M_img_size,CV_8UC1);
    vector<vector<Point> > c;
    c.push_back(_M_ROI);
    drawContours(mask,c,0,Scalar(255),CV_FILLED);
    resize(mask,mask,_M_initial_img.size());
    threshold(mask,mask,0,255,CV_THRESH_BINARY);
    imwrite(QString(saving_dir+"mask.png").toStdString(),mask);

    //Write SPM info
    _M_SPM.Write_SPM_info(saving_dir);

    //Save model
    WriteTemporalVector(saving_dir+"model.txt",model_for_correlation);

    emit newContrastImage(Z_maps);
    emit newProgressStatut("SPM",100);

}









//Get contrast oxynation value for a temporal mixel (Contrast vs time)
void PAnalyse::setStudiedPoint(Point P)
{   
    //Find Point
    bool found =false;
    int Studied_Point=-1;

    for(int i=0;_M_pixels_pos.size();i++)
    {
        if(_M_pixels_pos[i]==P)
        {
            found=true;
            Studied_Point=i;
            _M_point_of_interest = i;

            break;
        }
    }

    if(!_M_learningDone)
        return;

    QString saving_dir  =QString(_M_result_directory)+"Signals/";
    QDir dir(saving_dir);

    //Create saving dir
    if(!dir.exists())
        dir.mkdir(saving_dir);



    int rest_start,rest_end,correlation_start,correlation_end;
    rest_start          = _M_paradigm_times.get_rest_start_id();
    rest_end            = _M_paradigm_times.get_rest_end_id();
    correlation_start   = _M_paradigm_times.get_first_analyzed_frame_for_Correlation();
    correlation_end     = _M_paradigm_times.get_last_analyzed_frame_for_Correlation();


    qDebug()<<"[PAnalyse::setStudiedPoint] rest_start"<<rest_start;
    qDebug()<<"[PAnalyse::setStudiedPoint] rest_end"<<rest_end;
    qDebug()<<"[PAnalyse::setStudiedPoint] start id"<<correlation_start;
    qDebug()<<"[PAnalyse::setStudiedPoint] end id"<<correlation_end;




    //Process frame idx for data correction
    // _M_paradigm_times.process_frame_indexes_for_data_correction(correlation_start,correlation_end);

    if(!found)
        return;

    QVector<Mat> Molar_coeff;
    if(!_M_PixelWise_Molar_Coeff.getMolarCoeff(Studied_Point,Molar_coeff))
        return;

    //Single Point Measurement
    if(Studied_Point>0 && Studied_Point<_M_pixels_pos.size())
    {

        QVector<QVector<float> > _mixel = _M_filtering.getTemporalVector(Studied_Point);
        QVector<QVector<float> > contrast;
        _M_MBLL.get_Chromophore_Time_courses(_mixel,Molar_coeff,contrast,rest_start,rest_end,correlation_start,correlation_end);


        qDebug()<<"[PAnalyse::_Save_Results] mode tb";

        //Write Bold signal
        WriteTemporalVector(saving_dir+"Bold_signal.txt",_M_paradigm_times.get_Bold_Signal());
        WriteTemporalVector(saving_dir+"Neuronal_Activity.txt",_M_paradigm_times.get_Activation_Signal());




        //if mean ROI is requested process
        if(_M_request_Mean_ROI_Measure)
            _getMeanROIMeasurement(P,rest_start,rest_end,correlation_start,correlation_end,contrast,saving_dir);
        else
        {
            //Write vectors
            WriteTemporalVector(saving_dir+"Contrast.txt",contrast);
            WriteTemporalVector(saving_dir+"Point_Reflectance.txt",_mixel);
        }


        emit newContrastplot(contrast);
    }
}



//Write Img frame (for img clustering)
void  PAnalyse::_WriteImg(Mat &img)
{
    if(_M_ROI.empty())
        return;

    vector<vector<Point> > c;
    c.push_back(_M_ROI);

    Mat mask = Mat::zeros(img.size(),CV_8UC1);
    drawContours(mask,c,0,Scalar(255),CV_FILLED);

    Mat out = Mat::zeros(img.size(),CV_8UC3);
    _Mycolor zero;
    zero.b=0;
    zero.g=0;
    zero.r=0;

    for(int row=0;row<img.rows;row++)
    {
        uchar *ptr_mask     =mask.ptr<uchar>(row);
        _Mycolor * ptr_img  =img.ptr<_Mycolor>(row);
        _Mycolor * ptr_out  =out.ptr<_Mycolor>(row);
        for(int col=0;col<img.cols;col++)
        {
            ptr_out[col] = (ptr_mask[col]==255)? ptr_img[col] : zero;
        }
    }

    imwrite("/home/caredda/DVP/files/img.png",out);
}


// Get stats mask position
void PAnalyse::_Get_Stats_Mask_pos(Mat mask,QVector<int> &pos)
{
    if(mask.empty())
        return;

    pos.clear();
    for(int i=0;i<_M_pixels_pos.size();i++)
    {

        if(mask.at<uchar>(_M_pixels_pos[i].y,_M_pixels_pos[i].x)==255 && !_M_PixelWise_Molar_Coeff.isEmptyMatrix(i))
            pos.push_back(i);
    }
}



void PAnalyse::_getMeanROIMeasurement(Point P,int rest_start,int rest_end,int correlation_start,int correlation_end,QVector<QVector<float> > &mean_contrast,QString saving_dir)
{
    //init point of interest
    QVector<int> id;
    id.clear();

    //Create Mean area of interest
    Mat mask = Mat::zeros(_M_img_size,CV_8UC1);
    circle(mask,P,_M_Mean_ROI_radius,Scalar(255),CV_FILLED);

    //Get img id
    _Get_Stats_Mask_pos(mask,id);


    if(id.empty())
        return;

    _getMeanContrast(mean_contrast,id,saving_dir,rest_start,rest_end,correlation_start,correlation_end);

}

void PAnalyse::_getMeanContrast(QVector<QVector<float> > &mean_contrast,QVector<int> id,QString write_file,int rest_start,int rest_end,int correlation_start,int correlation_end)
{
    //Get contrast
    mean_contrast.clear();

    QVector<QVector<float> > Mean_Reflectance;

    mean_contrast.clear();
    Mean_Reflectance.clear();

    for(int i=0;i<id.size();i++)
    {
        QVector<Mat> Molar_coeff;
        if(!_M_PixelWise_Molar_Coeff.getMolarCoeff(id[i],Molar_coeff))
            continue;


        QVector<QVector<float> > _mixel = _M_filtering.getTemporalVector(id[i]);
        QVector<QVector<float> > contrast;
        _M_MBLL.get_Chromophore_Time_courses(_mixel,Molar_coeff,contrast,rest_start,rest_end,correlation_start,correlation_end);


        //Some contrast time courses
        if(i==0)
        {
            mean_contrast       = contrast;
            Mean_Reflectance    = _mixel;
        }
        else
        {
            //Concentration changes
            for(int c=0;c<contrast.size();c++)
            {
                for(int t=0;t<contrast[c].size();t++)
                    mean_contrast[c][t]+= contrast[c][t];
            }

            //Reflectance
            for(int c=0;c<_mixel.size();c++)
            {
                for(int t=0;t<_mixel[c].size();t++)
                    Mean_Reflectance[c][t]+= _mixel[c][t];
            }
        }

    }

    //Get mean time courses
    for(int c=0;c<mean_contrast.size();c++)
    {
        for(int t=0;t<mean_contrast[c].size();t++)
            mean_contrast[c][t]/=id.size();
    }


    //Reflectance
    for(int c=0;c<Mean_Reflectance.size();c++)
    {
        for(int t=0;t<Mean_Reflectance[c].size();t++)
            Mean_Reflectance[c][t]/= id.size();
    }


    //Write mean time courses
    WriteTemporalVector(write_file +"Mean_contrast.txt",mean_contrast);
    WriteTemporalVector(write_file +"Mean_Reflectance.txt",Mean_Reflectance);
}



