#include "PAnalyse.h"

PAnalyse::PAnalyse(QObject *parent):
    QThread(parent)
{

    //moveToThread(this);

    //Init result directory
    _M_result_directory =  "/home/results/";

    //request acquisition info
    connect(this,SIGNAL(requestAcquisitionInfo(bool)),&_M_filtering,SLOT(onrequestAcquisitionInfo(bool)));

    //Send acquisition info
    connect(&_M_filtering,SIGNAL(newAcquisitionInfo(QString)),this,SIGNAL(newAcquisitionInfo(QString)));

    //grey outside contour
    _M_grey_outside_contour = false;

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

    //resting state
    _M_resting_state_method = 0; //seed based method
    _M_resting_states_independant_sources = 5;
    _M_enable_resting_state = false;
    _M_resting_state_seeds.clear();

    _M_resting_state_map_sampling = 5;
    _M_nb_clusters_resting_state = 5;

    //Export data
    _M_exported_data_path = "";
    _M_enable_data_export = false;


    //Statistics
    _M_reference_area_pos.clear();
    _M_cortical_areas_pos.clear();

    _M_get_filtered_non_filtered_signals    = false;
    _M_get_distance_to_Blood_Vessels        = false;
    _M_processing_type                      = Process_Mean_Delta_C;
    //Statistical significance
    _M_statistical_significance             = 0.05;
    //Physiological apriori
    _M_HbO2_apriori                         = 5*1e-6;
    _M_Hb_apriori                           = -3.75*1e-6;
    _M_oxCCO_apriori                        = 0.5*1e-6;

    //Z threshold
    connect(&_M_SPM,SIGNAL(newZThresh(double)),this,SIGNAL(newZThresh(double)));

    //Data info
    _M_nb_channels=Spectral_bands;
    _M_nb_temporal_vectors=0;
    _M_tot_frames=0;

    //Correlation threshold
    _M_correlation_threshold = 0.2;


    //Fs
    _M_frame_rate=0;



    //process times
    connect(&_M_paradigm_times,SIGNAL(newRealTimeStarId(int)),&_M_filtering,SLOT(onNewRT_StartID(int)));
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

    //Create video
    _M_create_video = false;
    _M_video_nb_frames = 100;
    _M_video_framerate = 5;

    /*********************************************/
    /***************SEGMENTATION******************/
    /*********************************************/
    connect(&_M_PixelWise_Molar_Coeff,SIGNAL(newHyperspectralSegmentation(QVector<int>)),this,SIGNAL(newHyperspectralSegmentation(QVector<int>)));

    /*********************************************/
    /***************FILTERING*********************/
    /*********************************************/
    onFilteringTypeChanged(FFT_FILTERING);

    //process not real Time process
    connect(&_M_filtering,SIGNAL(readyForNotRealTimeProcess()),this,SLOT(DataFinishLoading()));

    //Filtering mesg
    connect(&_M_filtering,SIGNAL(newProgressStatut(QString,int)),this,SIGNAL(newProgressStatut(QString,int)));


    _M_point_of_interest            = -1;


    /*********************************************/
    /***************Molar coeff handling**********/
    /*********************************************/
    //Launch process each time a new segmentation occured or each time a parameter changed
    connect(&_M_PixelWise_Molar_Coeff,SIGNAL(processFinished()),this,SLOT(LaunchProcess()));
    //new segmentation
    connect(this,SIGNAL(ConsiderBloodVesselDistance(bool)),&_M_PixelWise_Molar_Coeff,SLOT(ConsiderBloodVesselDistance(bool)));

    //Send Hyperspectral cam idx
//    connect(&_M_PixelWise_Molar_Coeff,SIGNAL(newHyperspectralWavelengthConfig(QVector<QVector<int> >)),&_M_MBLL,SLOT(onnewWavelengthIDX(QVector<QVector<int> >)));
    connect(&_M_PixelWise_Molar_Coeff,SIGNAL(newWavelengthIDX(QVector<int>)),&_M_filtering,SLOT(onnewWavelengthIDX(QVector<int>)));

//    //TEMP
//    _function_temporaire();
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

// Option for saving non filtered data
void PAnalyse::onrequestNonFilteredDataSaving(bool v)
{
    _M_filtering.save_NonFilteredSignals(v);
}


// Get Delta C maps for statistical tests
QVector<Mat> PAnalyse::_Get_Delta_C_Maps(int nb_chromopores, bool use_filtered_data)
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

            emit newProgressStatut("Get Delta C maps",(int)((100*id*nb_thread)/_M_nb_temporal_vectors));

            //get concentration changes time courses
            QVector<QVector<float> > Delta_C;

            QVector<QVector<float> > temporal_vector = (use_filtered_data) ? _M_filtering.getTemporalVector(id) : _M_filtering.getNonFilteredTemporalVector(id);
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
QVector<Mat> PAnalyse::_Get_Camera_Intensity(bool use_filtered_data)
{
    //Get temp vector (spectral x temporal dimensions)
    QVector<QVector<float> > temp = (use_filtered_data) ? _M_filtering.getTemporalVector(0) : _M_filtering.getNonFilteredTemporalVector(0);

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
            emit newProgressStatut("Get Intensity maps",(int)((100*id*nb_thread)/_M_nb_temporal_vectors));

            QVector<Mat> Molar_coeff;
            if(!_M_PixelWise_Molar_Coeff.getMolarCoeff(id,Molar_coeff))
                continue;

            //get concentration changes time courses
            QVector<QVector<float> > temporal_vector = (use_filtered_data) ? _M_filtering.getTemporalVector(id) : _M_filtering.getNonFilteredTemporalVector(id);

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

//Get Absorbance changes
QVector<Mat> PAnalyse::_Get_Absorbance_Changes(bool use_filtered_data)
{


    //Get temp vector (spectral x temporal dimensions)
    QVector<QVector<float> > temp = (use_filtered_data) ? _M_filtering.getTemporalVector(0) : _M_filtering.getNonFilteredTemporalVector(0);

    //Nb of spectral channels
    int nb_spectral_channels = temp.size();

    //Nb of temporal elements
    int T = temp[0].size();


    //init process times
    int rest_start,rest_end,correlation_start,correlation_end;
    rest_start          = _M_paradigm_times.get_rest_start_id();
    rest_end            = _M_paradigm_times.get_rest_end_id();
    correlation_start   = _M_paradigm_times.get_first_analyzed_frame_for_Correlation();
    correlation_end     = _M_paradigm_times.get_last_analyzed_frame_for_Correlation();


    //Init Intensity maps (one map per channel)
    QVector<Mat> Delta_A_maps;
    for(int c=0;c<nb_spectral_channels;c++)
        Delta_A_maps.push_back(Mat::zeros(T,_M_nb_temporal_vectors,CV_32F)); //Size (T,N)

    //Process Loop
    #pragma omp parallel
    {
        int nb_thread = omp_get_num_threads();
        #pragma omp for
        for(int id=0;id<_M_nb_temporal_vectors;id++)
        {
            emit newProgressStatut("Get Absorbance changes",(int)((100*id*nb_thread)/_M_nb_temporal_vectors));

            QVector<Mat> Molar_coeff;
            if(!_M_PixelWise_Molar_Coeff.getMolarCoeff(id,Molar_coeff))
                continue;

            //get Camera intensities
            QVector<QVector<float> > temporal_vector = (use_filtered_data) ? _M_filtering.getTemporalVector(id) : _M_filtering.getNonFilteredTemporalVector(id);

            //Get Delta A
            QVector<QVector<float> > Delta_A;
             _M_MBLL.getAbsorbancechanges(temporal_vector,rest_start,rest_end,correlation_start,correlation_end,Delta_A);

            //Loop over time
            for(int t=0;t<T;t++)
            {
                //Store the HbO2,Hb, (oxCCO) values
                for(int c=0;c<nb_spectral_channels;c++)
                    Delta_A_maps[c].at<float>(t,id) = Delta_A[c][t];
            }
        }
    }

    return Delta_A_maps;
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



    if (!_M_enable_resting_state)
    {
        qDebug()<<"[PAnalyse::_Save_Results] mode tb";

        //Write Bold signal
        WriteTemporalVector(saving_dir+"Bold_signal.txt",_M_paradigm_times.get_Bold_Signal());
        WriteTemporalVector(saving_dir+"Neuronal_Activity.txt",_M_paradigm_times.get_Activation_Signal());


    }
    else
    {
        qDebug()<<"[PAnalyse::_Save_Results] mode resting_state";
    }

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
    //oxCCO
    if(nb_chromopores == 4) //if oxCCO quantification
        model_for_correlation.push_back(_M_paradigm_times.get_Metabolic_response());
    //HbT
    model_for_correlation.push_back(_M_paradigm_times.get_Bold_Signal());



    emit newProgressStatut("Save info",100);




    //Check if at least one option is checked
    if(!_M_saving_info.save_camera_intensity && !_M_saving_info.save_Delta_A && !_M_saving_info.save_Delta_C && !_M_saving_info.save_SPM && ! _M_saving_info.save_Delta_C_rest_activity)
        return;



    //Save Delta C
    if(_M_saving_info.save_Delta_C)
    {
        //Get Delta C maps (HbO2,Hb,HbT) Get full signal
        QVector<Mat> Delta_C_maps = _Get_Delta_C_Maps(nb_chromopores,!_M_saving_info.save_non_filtered_data);

        //Create directory
        QString contrast_dir  = (_M_saving_info.save_non_filtered_data) ? saving_dir+"Non_filtered_Contrast/" : saving_dir+"Filtered_Contrast/";
        QDir().mkdir(contrast_dir);
        emit newProgressStatut("Save Delta C",0);

        //Save contrasts
        for(int c=0;c<Delta_C_maps.size();c++)
        {
            QString file_path = (contrast_dir+ "MBLL_"+QString::number(c)+".txt");
            WriteFloatImg(file_path,Delta_C_maps[c]);
            emit newProgressStatut("Save Delta C",(int)(100*(float)(c/(nb_chromopores-1))));
        }
    }


    //Save Delta C during rest and activity periods (for statistical test)
    if(_M_saving_info.save_Delta_C_rest_activity)
    {
        //Get Delta C maps (HbO2,Hb,HbT) Get signal during correlation time
        QVector<Mat> Delta_C_maps = _Get_Delta_C_Maps(nb_chromopores); //Size (T,N)

        //Save Delta C during rest period
        QString contrast_dir  =  saving_dir+"Rest/";
        QDir().mkdir(contrast_dir);
        emit newProgressStatut("Save Delta C rest",0);

        //Save contrasts
        for(int c=0;c<Delta_C_maps.size();c++)
        {
            QString file_path = (contrast_dir+ "MBLL_"+QString::number(c)+".txt");
            WriteFloatImg(file_path,Delta_C_maps[c].rowRange(_M_paradigm_times.get_rest_start_id()-_M_paradigm_times.get_first_analyzed_frame_for_Correlation(),_M_paradigm_times.get_rest_end_id()-_M_paradigm_times.get_first_analyzed_frame_for_Correlation()));
            emit newProgressStatut("Save Delta C rest",(int)(100*(float)(c/(nb_chromopores-1))));
        }

        //Save Delta C during activity periods
        for(int i=0;i<activity_start.size();i++)
        {
            //Create saving directory

            contrast_dir  =  saving_dir+"Activity_"+QString::number(i)+"/";
            QDir().mkdir(contrast_dir);
            emit newProgressStatut("Save Delta C activity "+QString::number(i),0);

            //Save contrasts
            for(int c=0;c<Delta_C_maps.size();c++)
            {
                QString file_path = (contrast_dir+ "MBLL_"+QString::number(c)+".txt");
                WriteFloatImg(file_path,Delta_C_maps[c].rowRange(activity_start[i]-_M_paradigm_times.get_first_analyzed_frame_for_Correlation(),activity_end[i]-_M_paradigm_times.get_first_analyzed_frame_for_Correlation()));
                emit newProgressStatut("Save Delta C activity "+QString::number(i),(int)(100*(float)(c/(nb_chromopores-1))));
            }
        }
    }



    //Save Delta A
    if(_M_saving_info.save_Delta_A)
    {
//        QVector<Mat> Delta_A_maps = _Get_Absorbance_Changes(!_M_saving_info.save_non_filtered_data);

//        QString Delta_A_dir  = (_M_saving_info.save_non_filtered_data) ? saving_dir+"Non_filtered_Delta_A/" : saving_dir+"Filtered_Delta_A/";
//        QDir().mkdir(Delta_A_dir);
//        emit newProgressStatut("Save Delta A",0);
//        for(int c=0;c<Delta_A_maps.size();c++)
//        {
//            QString file_path = Delta_A_dir+ "Delta_A_"+QString::number(c)+".txt";
//            WriteFloatImg(file_path,Delta_A_maps[c]);
//            emit newProgressStatut("Save Delta A",(int)(100*(float)(c/(Delta_A_maps.size()-1))));
//        }
    }

    //Save Camera intensity
    if(_M_saving_info.save_camera_intensity)
    {
        //Get camera intensities
        QVector<Mat> I_maps = _Get_Camera_Intensity(!_M_saving_info.save_non_filtered_data);

        //Create directory
        QString I_dir  = (_M_saving_info.save_non_filtered_data) ? saving_dir+"Non_filtered_Camera_intensity/" : saving_dir+"Filtered_Camera_intensity/";
        QDir().mkdir(I_dir);
        emit newProgressStatut("Save camera intensity",0);

        //Save intensity maps
        for(int c=0;c<I_maps.size();c++)
        {

            QString file_path = I_dir+ "I_"+QString::number(c)+".txt";
            WriteFloatImg(file_path,I_maps[c]);
            emit newProgressStatut("Save camera intensity",(int)(100*(float)(c/(I_maps.size()-1))));
        }

    }

    //Save SPM results
    if(_M_saving_info.save_SPM)
    {

        //Get Delta C maps (HbO2,Hb,HbT)
        QVector<Mat> Delta_C_maps = _Get_Delta_C_Maps(nb_chromopores);

        //Create SPM directory
        QString path_SPM =saving_dir+"SPM/";
        QDir().mkdir(path_SPM);


        //Save Initial image
        if(type2str(_M_initial_img.type()) == "8UC3")
        {
            Mat in_BGR;
            cvtColor( _M_initial_img,in_BGR,CV_RGB2BGR);
            imwrite(QString(path_SPM+"initial_img.png").toStdString(),in_BGR);
        }
        else
        {
            imwrite(QString(path_SPM+"initial_img.png").toStdString(),_M_initial_img);
        }

        //Save mask for ROI
        Mat mask = Mat::zeros(_M_img_size,CV_8UC1);
        vector<vector<Point> > c;
        c.push_back(_M_ROI);
        drawContours(mask,c,0,Scalar(255),CV_FILLED);
        resize(mask,mask,_M_initial_img.size());
        threshold(mask,mask,0,255,CV_THRESH_BINARY);
        imwrite(QString(path_SPM+"mask.png").toStdString(),mask);

        //Write SPM info
        _M_SPM.Write_SPM_info(path_SPM);

        emit newProgressStatut("Save SPM",0);

        //Save SPM
        for(int c=0;c<nb_chromopores;c++)
        {
            //Create a directory to store the results
            QString path =path_SPM+QString::number(c)+"/";
            QDir().mkdir(path);


            //Compute The Statistical Parametric Mapping

            Mat mask;
            Mat z_map = _M_SPM.process_SPM(Delta_C_maps[c],_M_pixels_pos,_M_img_size,path, model_for_correlation[c],mask);
            emit newProgressStatut("Save SPM",(int)(100*(float)(c/(nb_chromopores-1))));
        }

        //Write model
        WriteTemporalVector(path_SPM+"model.txt",model_for_correlation);
    }

    emit newProgressStatut("Save data",100);
}


//create video mask
void PAnalyse::onCreateNewVideo()
{
    _M_create_video = true;
    LaunchProcess();
}

void PAnalyse::_Create_Video()
{
    //Nb of temporal elements
    int T = _M_paradigm_times.get_last_analyzed_frame_for_Correlation() - _M_paradigm_times.get_first_analyzed_frame_for_Correlation();
    //Nb of chromophore
    int nb_chromopores = _M_PixelWise_Molar_Coeff.getChromophoreNumber()+1;

    //Init Delta C maps (HbO2,Hb,HbT)
    QVector<Mat> Delta_C_maps = _Get_Delta_C_Maps(nb_chromopores);

    //Img mask
    Mat mask = Mat::zeros(_M_img_size,CV_8UC1);
    vector<vector<Point> > c;
    c.push_back(_M_ROI);
    drawContours(mask,c,0,Scalar(255),CV_FILLED);

    int itteration = (_M_video_nb_frames>=T)? 1 : int(T/_M_video_nb_frames);

    QVector<int> temporal_idx;

    Mat in_img;


    //Save Initial image
    if(type2str(_M_initial_img.type()) == "8UC3")
        cvtColor( _M_initial_img,in_img,CV_RGB2BGR);
    else
        return;



    qDebug()<<"Initial img channels "<<in_img.channels()<<" img type "<<type2str(in_img.type());



    //Save results
    QString saving_dir  =QString(_M_result_directory)+"Videos/";
    QDir dir(saving_dir);

    //Create saving dir
    if(!dir.exists())
        dir.mkdir(saving_dir);

    //video titles
    //Init txt variables
    double font_size = 0.5;
    int thick = 1;

    //Create colorbar
    Mat colorbar = _create_colorbar(in_img);
    Mat out_c = Mat::zeros(in_img.rows,in_img.cols+colorbar.cols,in_img.type());

    //Create frame size depending on the nb of chromophores
    Mat out_frame;
    if(nb_chromopores==3)
        out_frame = Mat::zeros(out_c.rows,out_c.cols*3,in_img.type());
    if(nb_chromopores==4)
        out_frame = Mat::zeros(out_c.rows*2,out_c.cols*2,in_img.type());



    //Create output video
    QString video = saving_dir+"MBLL.avi";

    #if (CV_VERSION_MAJOR >= 4)
    VideoWriter outputVideo(video.toStdString(), cv::VideoWriter::fourcc('M','J','P','G'), 30, out_frame.size());
    #else
        VideoWriter outputVideo(video.toStdString(), CV_FOURCC('M','J','P','G'), 30, out_frame.size());
    #endif

    if (!outputVideo.isOpened())
    {
        qDebug()  << "Could not open the output video for write";
        return;
    }

    //Create dir for img
    QString saving_dir2  = saving_dir+"MBLL/";
    QDir dir2(saving_dir2);

    //Create saving dir
    if(!dir2.exists())
        dir2.mkdir(saving_dir2);




    int id = 0;
    for(int t=0;t<T;t+=itteration)
    {
        out_frame = Mat::zeros(out_frame.size(),in_img.type());
        emit newProgressStatut("emit video",(int)(t/T));
        for(int c=0;c<nb_chromopores;c++)
        {
            out_c = Mat::zeros(out_c.size(),in_img.type());

            if(c==0)
                temporal_idx.push_back(t);


            //Get subMatrix
            Mat subMat  = Delta_C_maps[c].row(t);

            //reconstruct img (float img)
            Mat overlay = _reconstruct_Float_Carography(subMat);

            //Convert float img into falscolor
            double max = _M_max_display_value/1000000;
            //Divide by ten for oxCCO
            if(nb_chromopores==4 && c == 2)
                max = 3*1e-6;
            double min = -max;

            Mat out1 = _Convert_to_FalseColor(overlay,mask,min,max);


            //Apply Median blur
            medianBlur(out1,out1,5);
            //Merge to input img
            Mat out     = mergeImg(in_img,out1,0.4);
            //Grey outside contour
            if(_M_grey_outside_contour)
                _GreyOutsideContour(mask,out);


            //Add colorbar
            out_c = Mat::zeros(out.rows,out.cols+colorbar.cols,out.type());
            Mat insetImage(out_c, cv::Rect(0, 0, out.cols, out.rows));
            out.copyTo(insetImage);
            Mat insetImage2(out_c, cv::Rect(out.cols, 0, colorbar.cols, colorbar.rows));
            colorbar.copyTo(insetImage2);

            //Draw text on image
            float coeff = 1e6;
            QString txt = QString::number(min*coeff,'g', 2);
            out_c = _drawTextOnImg(out_c,txt.toStdString(),0,font_size,thick);
            txt = QString::number(max*coeff,'g', 2);
            out_c = _drawTextOnImg(out_c,txt.toStdString(),1,font_size,thick);

            // txt = video_title[c];
            // out_c = _drawTextOnImg(out_c,txt.toStdString(),2,font_size,thick);

            //Put out_c into out_frame
            if(nb_chromopores == 3)
                out_c.copyTo(out_frame(cv::Rect(out_c.cols*c, 0, out_c.cols, out_c.rows)));


            if(nb_chromopores == 4)
            {
                int row = c / 2;  // 0 or 1
                int col = c % 2;  // 0 or 1
                out_c.copyTo(out_frame(cv::Rect(col*out_c.cols,
                                                row * out_c.rows,
                                                out_c.cols,
                                                out_c.rows)));
            }


        }


        //Write img
        outputVideo<<out_frame;
        QString path = saving_dir2+QString::number(id)+".png";
        imwrite(path.toStdString(),out_frame);
        id++;

    }
    emit newProgressStatut("emit video",100);

    //Write Temporal idx
    WriteTemporalVector(saving_dir+"temporal_idx_video.txt",temporal_idx);


//     for(int c=0;c<nb_chromopores;c++)
//     {

//         Mat out_2 = Mat::zeros(in_img.rows,in_img.cols+colorbar.cols,in_img.type());

//         QString video = _M_methode_auto ? saving_dir+"Auto_"+QString::number(c)+".avi" : saving_dir+"MBLL_"+QString::number(c)+".avi";

//         #if (CV_VERSION_MAJOR >= 4)
//         VideoWriter outputVideo(video.toStdString(), cv::VideoWriter::fourcc('M','J','P','G'), 30, out2.size());
//         #else
//             VideoWriter outputVideo(video.toStdString(), CV_FOURCC('M','J','P','G'), 30, out2.size());
//         #endif

//         if (!outputVideo.isOpened())
//         {
//             qDebug()  << "Could not open the output video for write";
//             break;
//         }

//         //Create dir for img
//         QString saving_dir2  = _M_methode_auto ? saving_dir+"Auto_"+QString::number(c)+"/" : saving_dir+"MBLL_"+QString::number(c)+"/";
//         QDir dir2(saving_dir2);

//         //Create saving dir
//         if(!dir2.exists())
//             dir2.mkdir(saving_dir2);

//         int id = 0;
//         for(int t=0;t<T;t+=itteration)
//         {
//             if(c==0)
//                 temporal_idx.push_back(t);

//             emit newProgressStatut("emit video",(int)(t/T));
//             //Get subMatrix
//             Mat subMat  = Delta_C_maps[c].row(t);

//             //reconstruct img (float img)
//             Mat overlay = _reconstruct_Float_Carography(subMat);
// //            WriteFloatImg(saving_dir2+QString::number(id)+".txt",overlay);

//             //Convert float img into falscolor
//             double min = -1;
//             double max = -1;
//             Mat out1 = _Convert_to_FalseColor(overlay,mask,min,max);
//             //save min max values
//             QFile f(saving_dir2+"min_max.txt");
//             if (f.open(QIODevice::ReadWrite))
//             {
//                 QTextStream Qt( &f );
//                 Qt<<min<<"\n";
//                 Qt<<max<<"\n";
//             }
//             f.close();

//             //Apply Median blur
//             medianBlur(out1,out1,5);
//             //Merge to input img
//             Mat out     = mergeImg(in_img,out1,0.4);
//             //Grey outside contour
//             if(_M_grey_outside_contour)
//                 _GreyOutsideContour(mask,out);


//             //Add colorbar
//             out2 = Mat::zeros(out.rows,out.cols+colorbar.cols,out.type());
//             Mat insetImage(out2, cv::Rect(0, 0, out.cols, out.rows));
//             out.copyTo(insetImage);
//             Mat insetImage2(out2, cv::Rect(out.cols, 0, colorbar.cols, colorbar.rows));
//             colorbar.copyTo(insetImage2);

//             // //Draw text on image
//             // float coeff = _M_methode_auto ? 1 : 1e6;
//             // QString txt = QString::number(min*coeff,'g', 2);
//             // out2 = _drawTextOnImg(out2,txt.toStdString(),0,font_size,thick);
//             // txt = QString::number(max*coeff,'g', 2);
//             // out2 = _drawTextOnImg(out2,txt.toStdString(),1,font_size,thick);

//             // txt = video_title[c];
//             // out2 = _drawTextOnImg(out2,txt.toStdString(),2,font_size,thick);



//             //Write img
//             outputVideo<<out2;
//             QString path = saving_dir2+QString::number(id)+".png";
//             imwrite(path.toStdString(),out2);

//             id++;

//         }

//     }

//     emit newProgressStatut("emit video",100);

//     //Write Temporal idx
//     WriteTemporalVector(saving_dir+"temporal_idx_video.txt",temporal_idx);
}

//New correlation threshold
void PAnalyse::onNewCorrelationThreshold(double v)
{
    _M_correlation_threshold=v;

//    if(_M_display_mode==Display_Variation_Correlated)
//        LaunchProcess();

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

    if(_M_processing_type==No_Analysis)
        return;

    qDebug()<<"[PAnalyse::run]";

    QElapsedTimer elapsed_timer;
    elapsed_timer.start();

    //Create video
    if(_M_create_video)
    {
        _Create_Video();
        _M_create_video = false;
        return;
    }


    //save results
    if(_M_save_results)
    {
        _Save_Results();
        _M_save_results = false;
        return;
    }

    //resting state
    if(_M_enable_resting_state)
    {
        _ProcessRestingState();
        return;
    }


    switch (_M_processing_type)
    {
    case Activation_GLM_Pixel_wise:
        _General_Linear_Model_pixel_wise();
        break;

    case Process_Mean_Delta_C:
        _ProcessMeanConcentrationMeasure();
        break;
    case Process_Correlation:
        _ProcessCorrelationMeasure();
        break;
    default:
        _General_Linear_Model_pixel_wise();
        break;
    }

    emit Elapsed_ProcessingTime(elapsed_timer.elapsed());
}

//On new segmentation layers
void PAnalyse::onnewSegmentationLayers(Mat SurfaceBlood,Mat BuriedBlood,Mat nonused)
{
    _M_PixelWise_Molar_Coeff.setImgBuriedBlood(BuriedBlood);
    _M_PixelWise_Molar_Coeff.setImgNonUsedPixels(nonused);
    _M_PixelWise_Molar_Coeff.setImgSurfaceBlood(SurfaceBlood);
    _M_PixelWise_Molar_Coeff.processMolarCoeff();
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
    //oxCCO
    if(nb_Measure == 4) //if oxCCO quantification
        model_for_correlation.push_back(_M_paradigm_times.get_Metabolic_response());
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

            emit newProgressStatut("Measure",(int)((100*id*nb_thread)/_M_nb_temporal_vectors));


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


    if (_M_enable_resting_state)
    {
        qDebug()<<"[PAnalyse::_Process_Mean_Delta_C] mode resting state";
        id_start.clear();
        id_start.push_back(start_corr);

        id_end.clear();
        id_end.push_back(end_corr);
    }
    else
        qDebug()<<"[PAnalyse::_Process_Mean_Delta_C] mode tb";



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
        int nb_thread = omp_get_num_threads();
        #pragma omp for
        for(int id=0;id<_M_nb_temporal_vectors;id++)
        {
            QVector<Mat> Molar_coeff;
            if(!_M_PixelWise_Molar_Coeff.getMolarCoeff(id,Molar_coeff))
                continue;


            emit newProgressStatut("Measure",(int)((100*id*nb_thread)/_M_nb_temporal_vectors));

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

void PAnalyse::_GreyOutsideContour(Mat mask,Mat &inout)
{
    resize(mask,mask,inout.size());

    for(int row=0;row<mask.rows;row++)
    {
        uchar *ptr_mask     =mask.ptr<uchar>(row);
        _Mycolor *ptr_dst   =inout.ptr<_Mycolor>(row);
        for(int col=0;col<mask.cols;col++)
        {
            //dst = src1*alpha + src2*beta + gamma;
            ptr_dst[col].r = (ptr_mask[col]==255)? ptr_dst[col].r : 127*0.5 + ptr_dst[col].r*0.5;
            ptr_dst[col].g = (ptr_mask[col]==255)? ptr_dst[col].g : 127*0.5 + ptr_dst[col].g*0.5;
            ptr_dst[col].b = (ptr_mask[col]==255)? ptr_dst[col].b : 127*0.5 + ptr_dst[col].b*0.5;

        }
    }
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


Mat PAnalyse::_Display_ROI_activation(QVector<bool> &contrast_proc)
{
    //Creating file for display
    Mat img_contrast=Mat::zeros(_M_img_size,CV_8UC3);

    for(int id_mask=0;id_mask<_M_cortical_areas_pos.size();id_mask++)
    {
        for(int i=0;i<_M_cortical_areas_pos[id_mask].size();i++)
        {
            int id_pixel = _M_cortical_areas_pos[id_mask][i];
            if(contrast_proc[id_mask])
                img_contrast.at<Vec3b>(_M_pixels_pos[id_pixel].y,_M_pixels_pos[id_pixel].x)[2] = 255;
            else
                img_contrast.at<Vec3b>(_M_pixels_pos[id_pixel].y,_M_pixels_pos[id_pixel].x)[1] = 255;

        }
    }

    return img_contrast;
}

Mat PAnalyse::_Display_ROI_activation(QVector<float> &contrast_proc)
{
    //Creating file for display
    Mat img_contrast=Mat::zeros(_M_img_size,CV_32F);

    for(int id_mask=0;id_mask<_M_cortical_areas_pos.size();id_mask++)
    {
        for(int i=0;i<_M_cortical_areas_pos[id_mask].size();i++)
        {
            int id_pixel = _M_cortical_areas_pos[id_mask][i];
            img_contrast.at<float>(_M_pixels_pos[id_pixel].y,_M_pixels_pos[id_pixel].x) = contrast_proc[id_mask];
        }
    }

    return img_contrast;
}

Mat PAnalyse::_Display_ROI_activation(Mat &contrast_proc)
{
    //Creating file for display
    Mat img_contrast=Mat::zeros(_M_img_size,CV_32F);

    for(int id_mask=0;id_mask<_M_cortical_areas_pos.size();id_mask++)
    {
        for(int i=0;i<_M_cortical_areas_pos[id_mask].size();i++)
        {
            int id_pixel = _M_cortical_areas_pos[id_mask][i];
            img_contrast.at<float>(_M_pixels_pos[id_pixel].y,_M_pixels_pos[id_pixel].x) = contrast_proc.at<float>(0,id_mask);
        }
    }

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

    //Init filtering class
    _M_filtering.setPixelPos(_M_pixels_pos);
    _M_filtering.init(_M_pixels_pos.size(),nbChannels,NbFrames);

    //init process times
    _M_paradigm_times.setNbTotFrames(NbFrames);


    qDebug()<<"PAnalyse::Data acquisition is ready";
    emit newPixelPos(_M_pixels_pos);
    emit DataAcquisitionIsReady();
}


/*************************************************
 **************** Hyperspectral cam config********
 *************************************************/
void PAnalyse::onnew_HS_config(int v)
{
    _M_filtering.setNew_HS_config(v);
}

/*************************************************
 **************** AddD atas **********************
 *************************************************/

void PAnalyse::addDatas(_Processed_img &img)
{
    //Send img to filtering class
    _M_filtering.new_Filtering(img);
}

void PAnalyse::addDatas(_Processed_img_HS &img)
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


void PAnalyse::onnewCorticalAreaDefinition(Rect ref, QVector<Rect> cort, QVector<Mat> cort_residu)
{
    _M_reference_area_pos.clear();
    _M_cortical_areas_pos.clear();

    //reference area
    _Get_Stats_Mask_pos(ref,_M_reference_area_pos);

    //Cortical area
    for(int i=0;i<cort.size();i++)
    {
        QVector<int> temp;
        _Get_Stats_Mask_pos(cort[i],temp);
        _M_cortical_areas_pos.push_back(temp);
    }

    //non functional residu
    for(int i=0;i<cort_residu.size();i++)
    {
        QVector<int> temp;
        _Get_Stats_Mask_pos(cort_residu[i],temp);
        _M_cortical_areas_pos.push_back(temp);
    }

    if(_M_learningDone)
        this->start();
}

void PAnalyse::onnewCorticalAreaDefinition(QVector<Rect> cort, QVector<Mat> cort_residu)
{
    //set the number of resels
    _M_SPM.setNbResels(cort.size());//+cort_residu.size());


//    //temp
//    Mat test = Mat::zeros(_M_img_size,CV_8UC3);
//    for(int i=0;i<cort.size();i++)
//        rectangle(test,cort[i],Scalar(0,255,0),2);
//    imwrite(QString(_M_result_directory).toStdString()+"test.png",test);


    _M_reference_area_pos.clear();
    _M_cortical_areas_pos.clear();

    //Cortical area
    for(int i=0;i<cort.size();i++)
    {
        QVector<int> temp;
        _Get_Stats_Mask_pos(cort[i],temp);
        _M_cortical_areas_pos.push_back(temp);
    }

    //non functional residu
    for(int i=0;i<cort_residu.size();i++)
    {
        QVector<int> temp;
        _Get_Stats_Mask_pos(cort_residu[i],temp);
        _M_cortical_areas_pos.push_back(temp);
    }

    if(_M_learningDone)
        this->start();
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

void PAnalyse::_Get_Stats_Mask_pos(Rect mask,QVector<int> &pos)
{
    pos.clear();
    for(int i=0;i<_M_pixels_pos.size();i++)
    {
        if(mask.contains(_M_pixels_pos[i]))
            pos.push_back(i);
    }
}

//Get QMAP and corr
void PAnalyse::_Get_QMAP_AND_CORR(int i, QVector<double> &Corr_C, QVector<QVector<double> > &Mean_C, QVector<int> &activity_id_start, QVector<int> &activity_id_end)
{

    //Analysis length
    int correlation_start = _M_paradigm_times.get_first_analyzed_frame_for_Correlation();
    int correlation_end = _M_paradigm_times.get_last_analyzed_frame_for_Correlation();

    //reference
    int rest_start = _M_paradigm_times.get_rest_start_id();
    int rest_end = _M_paradigm_times.get_rest_end_id();


    //Get filtered temporal vector
    QVector<QVector<float> > _mixel = _M_filtering.getTemporalVector(i);
    QVector<QVector<float> > contrast;

    //Molar coeff
    QVector<Mat> Molar_coeff;
    if(!_M_PixelWise_Molar_Coeff.getMolarCoeff(i,Molar_coeff))
        return;

    int nb_Measure  = _M_PixelWise_Molar_Coeff.getChromophoreNumber()+1 ; //Hb, HbO2, (oxCCO), HbT

    //Model for correlation
    QVector<QVector<float> > model_for_correlation;
    //HbO2
    model_for_correlation.push_back(_M_paradigm_times.get_Bold_Signal());
    //Hb
    QVector<float> Hb_Bold = _M_paradigm_times.get_Bold_Signal();
    for(int i=0;i<Hb_Bold.size();i++)
        Hb_Bold[i]*=-1;
    model_for_correlation.push_back(Hb_Bold);
    //oxCCO
    if(nb_Measure == 4) //if oxCCO quantification
        model_for_correlation.push_back(_M_paradigm_times.get_Metabolic_response());
    //HbT
    model_for_correlation.push_back(_M_paradigm_times.get_Bold_Signal());


    _M_MBLL.get_Chromophore_Time_courses(_mixel,Molar_coeff,contrast,rest_start,rest_end,correlation_start,correlation_end);



    //Get QMAP values
    Mean_C.resize(activity_id_start.size());
    for(int i=0;i<Mean_C.size();i++)
        Mean_C[i].fill(0,contrast.size());


    for(int k=0;k<contrast.size();k++)
    {
        //loop over the number of considered activity step
        for(int step_activity=0;step_activity<activity_id_start.size();step_activity++)
        {
            int tot=0;
            //Loop over the activity step temporal idx
            for(int j=activity_id_start[step_activity];j<=activity_id_end[step_activity];j++)
            {
                Mean_C[step_activity][k]    += (double)(contrast[k][j]);
                tot++;
            }
            Mean_C[step_activity][k]    = (tot==0) ? 0 : Mean_C[step_activity][k]/tot;
        }
    }

    // Compare bold vs contrast measures
    Corr_C.fill(-1,contrast.size());

    Compare_Bold_Vs_Oxy(contrast,model_for_correlation,Corr_C);
}



void PAnalyse::_Get_QMAP_AND_CORR(QVector<double> &stat_vec,QVector<QVector<double> > &QMap_temp,QVector<QVector<float> > contrast,QVector<int> coeff_multi,QVector<int> &activity_id_start,QVector<int> &activity_id_end)
{
    //Get QMAP values
    QMap_temp.clear();
    QMap_temp.resize(activity_id_start.size());
    for(int i=0;i<QMap_temp.size();i++)
        QMap_temp[i].fill(0,contrast.size());

    int nb_Measure  = _M_PixelWise_Molar_Coeff.getChromophoreNumber()+1 ; //Hb, HbO2, (oxCCO), HbT

    //Model for correlation
    QVector<QVector<float> > model_for_correlation;
    //HbO2
    model_for_correlation.push_back(_M_paradigm_times.get_Bold_Signal());
    //Hb
    QVector<float> Hb_Bold = _M_paradigm_times.get_Bold_Signal();
    for(int i=0;i<Hb_Bold.size();i++)
        Hb_Bold[i]*=-1;
    model_for_correlation.push_back(Hb_Bold);
    //oxCCO
    if(nb_Measure == 4) //if oxCCO quantification
        model_for_correlation.push_back(_M_paradigm_times.get_Metabolic_response());
    //HbT
    model_for_correlation.push_back(_M_paradigm_times.get_Bold_Signal());



    //Loop over the nb of chromophore
    for(int k=0;k<contrast.size();k++)
    {

        //loop over the number of considered activity step
        for(int step_activity=0;step_activity<activity_id_start.size();step_activity++)
        {
            int tot=0;
            //Loop over the activity step temporal idx
            for(int j=activity_id_start[step_activity];j<=activity_id_end[step_activity];j++)
            {
                QMap_temp[step_activity][k] += (double)(contrast[k][j]);
                tot++;
            }
            QMap_temp[step_activity][k]    = (tot==0) ? 0 : QMap_temp[step_activity][k] /tot;
        }
    }


    // Compare bold vs contrast measures
    stat_vec.clear();
    stat_vec.fill(-1,contrast.size());

    if(_M_paradigm_times.get_Bold_Signal().empty())
        return;

    Compare_Bold_Vs_Oxy(contrast,model_for_correlation,stat_vec);
//    if(_M_methode_auto)
//    {
//        Compare_Bold_Vs_studied_Point(contrast,model_for_correlation,stat_vec/*,_M_proba_thresh*/);
//    }
//    else
//    {
//        Compare_Bold_Vs_Oxy(contrast,model_for_correlation,stat_vec);
//    }

}

//on new stats type
void PAnalyse::onnewStatType(int v)
{
    _M_processing_type=v;

    if(!_M_learningDone)
        return;

    if(!this->isRunning())
        this->start();
}


//General linear model
void PAnalyse::_General_Linear_Model_pixel_wise()
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
    //oxCCO
    if(nb_chromopores == 4) //if oxCCO quantification
        model_for_correlation.push_back(_M_paradigm_times.get_Metabolic_response());
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

//    emit newActivationMap(activation_map);
    emit newContrastImage(Z_maps);
    emit newProgressStatut("SPM",100);

}












void PAnalyse::_getMeanContrast(QVector<QVector<float> > &mean_contrast,QVector<int> id,QString write_file,int rest_start,int rest_end,int correlation_start,int correlation_end)
{
    //Get contrast
    mean_contrast.clear();

    QVector<QVector<QVector<float> > > all_Absorbance;
    QVector<QVector<float> > Mean_Absorbance;
    QVector<QVector<float> > Mean_Reflectance;

    all_Absorbance.clear();
    mean_contrast.clear();
    Mean_Absorbance.clear();
    Mean_Reflectance.clear();

    for(int i=0;i<id.size();i++)
    {
        QVector<Mat> Molar_coeff;
        if(!_M_PixelWise_Molar_Coeff.getMolarCoeff(id[i],Molar_coeff))
            continue;


        QVector<QVector<float> > _mixel = _M_filtering.getTemporalVector(id[i]);
        QVector<QVector<float> > contrast;
        _M_MBLL.get_Chromophore_Time_courses(_mixel,Molar_coeff,contrast,rest_start,rest_end,correlation_start,correlation_end);

        QVector<QVector<float> > Absorbance;
        _M_MBLL.getAbsorbancechanges(_mixel,rest_start,rest_end,correlation_start,correlation_end,Absorbance);


        //Store all absorbance values
        all_Absorbance.push_back(Absorbance);


        //Some contrast time courses
        if(i==0)
        {
            mean_contrast       = contrast;
            Mean_Absorbance     = Absorbance;
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
            //absorbance
            for(int c=0;c<Absorbance.size();c++)
            {
                for(int t=0;t<Absorbance[c].size();t++)
                    Mean_Absorbance[c][t]+= Absorbance[c][t];
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

    //absorbance
    for(int c=0;c<Mean_Absorbance.size();c++)
    {
        for(int t=0;t<Mean_Absorbance[c].size();t++)
            Mean_Absorbance[c][t]/= id.size();
    }

    //Reflectance
    for(int c=0;c<Mean_Reflectance.size();c++)
    {
        for(int t=0;t<Mean_Reflectance[c].size();t++)
            Mean_Reflectance[c][t]/= id.size();
    }


    //Write mean time courses
    WriteTemporalVector(write_file +"Mean_Absorbance.txt",Mean_Absorbance);
    WriteTemporalVector(write_file +"Mean_contrast.txt",mean_contrast);
    WriteTemporalVector(write_file +"Mean_Reflectance.txt",Mean_Reflectance);



    //Store all absorbance vector
    QString saving_dir  =QString(write_file)+"absorbance_data/";
    QDir dir(saving_dir);
    dir.mkdir(saving_dir);

    for (int i = 0;i<all_Absorbance.size();i++)
        WriteTemporalVector(saving_dir+QString::number(i)+".txt",all_Absorbance[i]);

}


void PAnalyse::_getMeanReflectance(QVector<int> id,QString write_file)
{
    //Get contrast
    QVector<QVector<float> > Mean_Reflectance;

    Mean_Reflectance.clear();

    for(int i=0;i<id.size();i++)
    {
        QVector<QVector<float> > _mixel = _M_filtering.getTemporalVector(id[i]);

        //Some contrast time courses
        if(i==0)
        {
            Mean_Reflectance    = _mixel;
        }
        else
        {
            //Reflectance
            for(int c=0;c<_mixel.size();c++)
            {
                for(int t=0;t<_mixel[c].size();t++)
                    Mean_Reflectance[c][t]+= _mixel[c][t];
            }
        }

    }

    //Reflectance
    for(int c=0;c<Mean_Reflectance.size();c++)
    {
        for(int t=0;t<Mean_Reflectance[c].size();t++)
            Mean_Reflectance[c][t]/= id.size();
    }


    //Write mean time courses
    WriteTemporalVector(write_file +"Mean_Reflectance.txt",Mean_Reflectance);

}


void PAnalyse::_getMeanContrast_AND_Stats(QVector<int> id,QString write_file)
{
    //  size: nb paradigm ; nb spatial id ; nb chromophore
    QVector<QVector<QVector<double> > > QMAP;
    QVector<QVector<double> >  Corr;

    //get id activity steps
    QVector<int> activity_start = _M_paradigm_times.get_Full_activity_start();
    QVector<int> activity_end   = _M_paradigm_times.get_Full_activity_end();
    // int nb_activity_steps = activity_start.size();
    // for(int i=0;i<nb_activity_steps;i++)
    // {
    //     activity_start[i]-=_M_paradigm_times.get_first_analyzed_frame_for_Correlation();
    //     activity_end[i]-=_M_paradigm_times.get_first_analyzed_frame_for_Correlation();
    // }

    //Analysis length
    int correlation_start = _M_paradigm_times.get_first_analyzed_frame_for_Correlation();
    int correlation_end = _M_paradigm_times.get_last_analyzed_frame_for_Correlation();

    //reference
    int rest_start = _M_paradigm_times.get_rest_start_id();
    int rest_end = _M_paradigm_times.get_rest_end_id();








    //init size
    int nb_chrom = _M_PixelWise_Molar_Coeff.getChromophoreNumber()+1;
    QMAP.resize(activity_start.size());

    for(int i=0;i<QMAP.size();i++)
    {
        QMAP[i].resize(id.size());
        for(int j=0;j<id.size();j++)
            QMAP[i][j].resize(nb_chrom);
    }

    QVector<int> coeff_multi;

    //Get contrast
    QVector<QVector<float> > mean_contrast;
    QVector<QVector<float> > Mean_Absorbance;
    QVector<QVector<float> > Mean_Reflectance;

    //(deconvolution of Hb, HbO2 4 lambda
    mean_contrast.clear();
    Mean_Absorbance.clear();
    Mean_Reflectance.clear();


    coeff_multi.push_back(1);
    coeff_multi.push_back(-1);
    coeff_multi.push_back(1);


    for(int i=0;i<id.size();i++)
    {
        QVector<Mat> Molar_coeff;
        if(!_M_PixelWise_Molar_Coeff.getMolarCoeff(id[i],Molar_coeff))
            continue;

        QVector<QVector<float> > _mixel = _M_filtering.getTemporalVector(id[i]);
        QVector<QVector<float> > contrast;
        _M_MBLL.get_Chromophore_Time_courses(_mixel,Molar_coeff,contrast,rest_start,rest_end,correlation_start,correlation_end);


        QVector<QVector<float> > Absorbance;
        _M_MBLL.getAbsorbancechanges(_mixel,rest_start,rest_end,correlation_start,correlation_end,Absorbance);

        //Get QMAP and CORR
        //  size: nb paradigm ; nb chromophore
        QVector<QVector<double> > QMAP_temp;
        QVector<double> Corr_temp;
        _Get_QMAP_AND_CORR(Corr_temp,QMAP_temp,contrast,coeff_multi,activity_start,activity_end);

        //Some contrast time courses
        if(i==0)
        {
            mean_contrast   = contrast;
            Mean_Absorbance = Absorbance;
            Mean_Reflectance = _mixel;
        }
        else
        {
            //Concentration changes
            for(int c=0;c<contrast.size();c++)
            {
                for(int t=0;t<contrast[c].size();t++)
                    mean_contrast[c][t]+= contrast[c][t];
            }
            //absorbance
            for(int c=0;c<Absorbance.size();c++)
            {
                for(int t=0;t<Absorbance[c].size();t++)
                    Mean_Absorbance[c][t]+= Absorbance[c][t];
            }
            //Reflectance
            for(int c=0;c<_mixel.size();c++)
            {
                for(int t=0;t<_mixel[c].size();t++)
                    Mean_Reflectance[c][t]+= _mixel[c][t];
            }


        }
        //loop over the nb of considered activity step
        for(int np=0;np<QMAP_temp.size();np++)
        {
            //Loop over the nub of chromophore
            for(int nc=0;nc<nb_chrom;nc++)
               QMAP[np][i][nc] = QMAP_temp[np][nc];
        }

        Corr.push_back(Corr_temp);

    }

    //Get mean time courses
    for(int c=0;c<mean_contrast.size();c++)
    {
        for(int t=0;t<mean_contrast[c].size();t++)
            mean_contrast[c][t]/=id.size();
    }

    //absorbance
    for(int c=0;c<Mean_Absorbance.size();c++)
    {
        for(int t=0;t<Mean_Absorbance[c].size();t++)
            Mean_Absorbance[c][t]/= id.size();
    }

    //reflectance
    for(int c=0;c<Mean_Reflectance.size();c++)
    {
        for(int t=0;t<Mean_Reflectance[c].size();t++)
            Mean_Reflectance[c][t]/= id.size();
    }


    //Write mean time courses
    WriteTemporalVector(write_file +"Mean_Absorbance.txt",Mean_Absorbance);
    WriteTemporalVector(write_file +"Mean_contrast.txt",mean_contrast);
    WriteTemporalVector(write_file +"Mean_Reflectance.txt",Mean_Reflectance);

    WriteTemporalVector(write_file +"Corr.txt",Corr);

    for(int np=0;np<QMAP.size();np++)
        WriteTemporalVector(write_file +"QMAP_Paradigm_"+QString::number(np)+".txt",QMAP[np]);

}

void PAnalyse::_getMeanReflectance_ROI_MEasurement(Point P,QString saving_dir)
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

    _getMeanReflectance(id,saving_dir);
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


//    if(!_M_enable_resting_state)
//    {
//        _getMeanContrast_AND_Stats(id,QString(_M_result_directory)+"");
//    }
//    else
//    {
//        _getMeanContrast(id,QString(_M_result_directory)+"",rest_start,rest_end,correlation_start,correlation_end);
//    }



//    ////////////////////////////TEMP

//    _getMeanContrast_AND_Stats(4,true,id,QString(_M_result_directory)+"Hb_HbO2_4_lambda_");
//    _getMeanContrast_AND_Stats(25,true,id,QString(_M_result_directory)+"Hb_HbO2_25_lambda_");
//    _getMeanContrast_AND_Stats(2,false,id,QString(_M_result_directory)+"Hb_HbO2_oxCCO_2_lambda_");
//    _getMeanContrast_AND_Stats(24,false,id,QString(_M_result_directory)+"Hb_HbO2_oxCCO_24_lambda_");
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
            _M_filtering.setPointOfInterest(i);
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

//    //If no analysis is selected
//    if(_M_processing_type==No_Analysis)
//    {
//        if(_M_request_Mean_ROI_Measure)
//            _getMeanReflectance_ROI_MEasurement(P,saving_dir);
//        else
//        {
//            QVector<QVector<float> > _mixel = _M_filtering.getTemporalVector(Studied_Point);
//            WriteTemporalVector(saving_dir+"Point_Reflectance.txt",_mixel);
//        }

//        return;
//    }


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


        QVector<QVector<float> > Absorbance;
        _M_MBLL.getAbsorbancechanges(_mixel,rest_start,rest_end,correlation_start,correlation_end,Absorbance);


        if (!_M_enable_resting_state)
        {
            qDebug()<<"[PAnalyse::_Save_Results] mode tb";

            //Write Bold signal
            WriteTemporalVector(saving_dir+"Bold_signal.txt",_M_paradigm_times.get_Bold_Signal());
            WriteTemporalVector(saving_dir+"Neuronal_Activity.txt",_M_paradigm_times.get_Activation_Signal());
        }




        //Write distance to blood vessel
        qDebug()<<"[PAnalyse::setStudiedPoint]  Get distance Grey matter to blood vessel"<<_M_PixelWise_Molar_Coeff.getDistanceMap_GreyMatter_BloodVessel(Studied_Point);


        // //TEMP compute phase diff between HbO2 and Hb
        // fftwf_complex  *in   = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*contrast[0].size());
        // fftwf_complex  *out  = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*contrast[0].size());

        // fftwf_plan plan_forward = fftwf_plan_dft_1d(contrast[0].size(),in,out,FFTW_FORWARD,FFTW_ESTIMATE);
        // fftwf_plan plan_backward = fftwf_plan_dft_1d(contrast[0].size(),out,in,FFTW_BACKWARD,FFTW_ESTIMATE);

        // QVector<float> Phase_HbO2_Hb = calculatePhaseShift_Hilbert(contrast[0], contrast[1],plan_forward,plan_backward);

        // //Temp
        // WriteTemporalVector(_M_result_directory + "R.txt",_mixel[0]);
        // WriteTemporalVector(_M_result_directory + "G.txt",_mixel[1]);
        // WriteTemporalVector(_M_result_directory + "B.txt",_mixel[2]);

        // WriteTemporalVector(_M_result_directory + "in1.txt",contrast[0]);
        // WriteTemporalVector(_M_result_directory + "in2.txt",contrast[1]);
        // WriteTemporalVector(_M_result_directory + "phase_diff.txt",Phase_HbO2_Hb);

        // //Phase diff using cross-correlation
        // calculatePhaseShift_xCorr(contrast[0], contrast[1]);

        // //Destroy FFT plans and datas
        // fftwf_destroy_plan(plan_forward);
        // fftwf_destroy_plan(plan_backward);
        // fftwf_free(in);
        // fftwf_free(out);




        //if mean ROI is requested process
        if(_M_request_Mean_ROI_Measure)
            _getMeanROIMeasurement(P,rest_start,rest_end,correlation_start,correlation_end,contrast,saving_dir);
        else
        {
            //Write vectors
            WriteTemporalVector(saving_dir+"Absorbance_changes.txt",Absorbance);
            WriteTemporalVector(saving_dir+"Contrast.txt",contrast);
            WriteTemporalVector(saving_dir+"Point_Reflectance.txt",_mixel);
        }


        emit newContrastplot(contrast);
    }
}


//void PAnalyse::onGetAll_maps()
//{
//    if(_M_id_img_saved>1 && _M_id_img_saved<=25)
//        _M_PixelWise_Molar_Coeff.setWavelengthGr(_M_id_img_saved);
//}


void PAnalyse::_function_temporaire()
{
    QString path_ref = QString(_M_result_directory)+"video_25_01_19/Ref/";
    QString path_S2 = QString(_M_result_directory)+"video_25_01_19/S2/";

    //Alternative hypothesis
    QVector<int> stats_type_Corr,stats_type_QMAP;
    stats_type_QMAP.fill(0,2);
    stats_type_Corr.push_back(2);
    stats_type_Corr.push_back(1);

    unsigned N = 16;
    double alpha = 0.01/606;
    double p_value;
    double t_stat;

    qDebug()<<"alpha: "<<alpha;

    //Load data of interest
    for(int i=1;i<7;i++)
    {
        QVector<double> C_mean_ref,C_mean_S2,C_std_ref,C_std_S2;
        QVector<double> Corr_mean_ref,Corr_mean_S2,Corr_std_ref,Corr_std_S2;

        //Corr
        ReadVector(path_ref+"Corr_ref_mean_"+QString::number(i)+".txt",Corr_mean_ref);
        ReadVector(path_S2+"Corr_ref_mean_"+QString::number(i)+".txt",Corr_mean_S2);
        ReadVector(path_ref+"Corr_ref_std_"+QString::number(i)+".txt",Corr_std_ref);
        ReadVector(path_S2+"Corr_ref_std_"+QString::number(i)+".txt",Corr_std_S2);

        //C
        ReadVector(path_ref+"QMAP_ref_mean_"+QString::number(i)+".txt",C_mean_ref);
        ReadVector(path_S2+"QMAP_ref_mean_"+QString::number(i)+".txt",C_mean_S2);
        ReadVector(path_ref+"QMAP_ref_std_"+QString::number(i)+".txt",C_std_ref);
        ReadVector(path_S2+"QMAP_ref_std_"+QString::number(i)+".txt",C_std_S2);

        qDebug()<<"/**************************/";
        qDebug()<<"Processing "<<i;

        for(int c=0;c<C_mean_ref.size();c++)
        {

            QString conf            = (c==0) ? "HbO2" : "Hb";
            QString results_c       = "Mean C "+conf+": ";
            QString results_Corr    = "Corr "+conf+": ";

            bool res_C = two_samples_t_test_different_sd(C_mean_S2[c],C_std_S2[c],N,C_mean_ref[c],C_std_ref[c],N,alpha,p_value,t_stat,stats_type_QMAP[c]) ;
            bool res_Corr = two_samples_t_test_different_sd(Corr_mean_S2[c],Corr_std_S2[c],N,Corr_mean_ref[c],Corr_std_ref[c],N,alpha,p_value,t_stat,stats_type_Corr[c]) ;

            results_c       = (res_C) ? results_c+"1" : results_c+"0";
            results_Corr    = (res_Corr) ? results_Corr+"1" : results_Corr+"0";

            qDebug()<<results_c;
            qDebug()<<results_Corr;
        }
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




/*********************************************/
/********Resting state************************/
/*********************************************/

//New resting state method (0: seed based method, 1: ICA based method)
void PAnalyse::onnewRestingStateMethod(int v)
{
    _M_resting_state_method = v;

    if(!_M_enable_resting_state)
        return;

    if(_M_ROI.empty())
        return;

    LaunchProcess();
}

//On new independent nb of sources
void PAnalyse::onNewIndependantNbofSources(int v)
{
    _M_resting_states_independant_sources=v;

    if(!_M_enable_resting_state)
        return;

    if(_M_ROI.empty())
        return;

    LaunchProcess();
}

//new seeds
void PAnalyse::onnewRestingStateSeeds(QVector<Mat> mask)
{
    if(mask.empty())
        return;

    _M_resting_state_seeds.clear();
    for(int i=0;i<mask.size();i++)
        _M_resting_state_seeds.push_back(mask[i]);


    if(!_M_enable_resting_state)
        return;

    if(_M_ROI.empty())
        return;

    LaunchProcess();
}

//init seeds
void PAnalyse::InitRestingStateSeeds()
{
    _M_resting_state_seeds.clear();
}





//ICA based method
void PAnalyse::_Process_Resting_state_ICA_based_method()
{

    //Resting state times (whole paradigm, first rest, last rest)
    int resting_state_time_start = _M_paradigm_times.get_rest_start_id();
    int resting_state_time_end = _M_paradigm_times.get_rest_end_id();
    int T = resting_state_time_end - resting_state_time_start;

    //Process frame idx for data correction
    // _M_paradigm_times.process_frame_indexes_for_data_correction(resting_state_time_start,resting_state_time_end);

    //Init Delta C maps (HbO2,Hb,(oxCCO),HbT)
    QVector<Mat> Delta_C_maps;
    for(int c=0;c<_M_PixelWise_Molar_Coeff.getChromophoreNumber()+1;c++)
        Delta_C_maps.push_back(Mat::zeros(T,_M_nb_temporal_vectors,CV_32FC1)); //Size (T,N)


    //Get concentration changes maps
    //Process Loop
    #pragma omp parallel
    {
        int nb_thread = omp_get_num_threads();
        #pragma omp for
        for(int id=0;id<_M_pixels_pos.size();id++)
        {
            QVector<Mat> Molar_coeff;
            if(!_M_PixelWise_Molar_Coeff.getMolarCoeff(id,Molar_coeff))
                continue;

            emit newProgressStatut("Get Delta C maps",(int)((100*id*nb_thread)/_M_nb_temporal_vectors));

            //get concentration changes time courses
            QVector<QVector<float> > Delta_C;
            _M_MBLL.get_Chromophore_Time_courses(_M_filtering.getTemporalVector(id),Molar_coeff,Delta_C,resting_state_time_start,resting_state_time_end,resting_state_time_start,resting_state_time_end);

            for(int c=0;c<Delta_C.size();c++)
            {
                for(int t=0;t<T;t++)
                    Delta_C_maps[c].at<float>(t,id) = Delta_C[c][t];
            }
        }
    }

    //Write Delta C values
    for(int i=0;i<Delta_C_maps.size();i++)
        WriteFloatImg(QString(_M_result_directory)+"Carto_chrom_"+QString::number(i)+".txt",Delta_C_maps[i]);

    //Write Pixel position
    WritePointVector(QString(_M_result_directory)+"pixel_pos.txt",_M_pixels_pos);

    //Write initial image
    imwrite(QString(_M_result_directory).toStdString()+"in.png",_M_initial_img);

    //Write Acquisition info
    QFile infos(QString(_M_result_directory)+"infos.txt");
    if(infos.exists())
        infos.remove();

    // Write infos.txt containing times of the paradigm and Fs
    if (infos.open(QIODevice::ReadWrite))
    {
        QTextStream Qt( &infos );

        // Frequency sampling (add) zero to produce an uniform txt file
        Qt<<"Img size: "<<_M_img_size.height<<" "<<_M_img_size.width<<"\n";

    }
    infos.close();

    /*
    //init ICA output
    QVector<QVector<Mat> > maps;
    maps.resize(Delta_C_maps.size()); //size: nb of chromophores +1


    //Nb of repeation of the total process
    int nb_process_iteration = 1;//20;


    #pragma omp parallel
    {
        #pragma omp for
        for(int id_process=0;id_process<nb_process_iteration;id_process++)
        {

            for(int i=0;i<Delta_C_maps.size();i++)
            {
                //Fast ICA
                FastICA fastICA;
                connect(&fastICA,SIGNAL(newProgressStatut(QString,int)),this,SIGNAL(newProgressStatut(QString,int)));

                qDebug()<<"fast ica";
                Mat out = fastICA.processFastICA(Delta_C_maps[i],_M_resting_states_independant_sources);

                qDebug()<<"Fast ICA out ("<<out.rows<<";"<<out.cols<<")";

                for(int row=0;row<out.rows;row++)
                {
                    Mat temp_res_ICA = out.row(row);
                    maps[i].push_back(_reconstruct_Float_Carography(temp_res_ICA));
                }

                emit newProgressStatut("Resting state ICA",i*100/Delta_C_maps.size());
            }
        }
    }


    emit newRestingStateMaps(maps);
    emit newProgressStatut("Resting state ICA",100);

    */
}


//Seed based method
void PAnalyse::_Process_Resting_state_seed_based_method()
{
    if(_M_resting_state_seeds.empty())
        return;

    bool exit=false;
    for(int i=0;i<_M_resting_state_seeds.size();i++)
    {
        if(_M_resting_state_seeds[i].empty())
        {
            exit = true;
            break;
        }
    }
    if(exit)
        return;

    qDebug()<<"[PAnalyse::_Process_Resting_state_seed_based_method] start analysis "<<_M_paradigm_times.get_first_analyzed_frame_for_Correlation();
    qDebug()<<"[PAnalyse::_Process_Resting_state_seed_based_method] end analysis "<<_M_paradigm_times.get_last_analyzed_frame_for_Correlation();
    qDebug()<<"[PAnalyse::_Process_Resting_state_seed_based_method] start ref "<<_M_paradigm_times.get_rest_start_id();
    qDebug()<<"[PAnalyse::_Process_Resting_state_seed_based_method] end ref "<<_M_paradigm_times.get_rest_end_id();



    //Init
    int seed_nb = _M_resting_state_seeds.size();
    QVector<QVector<QVector<float> > > Mean_seed_contrast;
    Mean_seed_contrast.clear();
    Mean_seed_contrast.resize(seed_nb);


    //Resting state times (whole paradigm, first rest, last rest)
    int resting_state_time_start = _M_paradigm_times.get_rest_start_id();
    int resting_state_time_end = _M_paradigm_times.get_rest_end_id();


    // _M_paradigm_times.process_frame_indexes_for_data_correction(resting_state_time_start,resting_state_time_end);


    qDebug()<<"[PAnalyse::_Process_Resting_state_seed_based_method] resting_state start "<<resting_state_time_start;
    qDebug()<<"[PAnalyse::_Process_Resting_state_seed_based_method] resting_state end "<<resting_state_time_end;



    /*****************************/
    /*********GET SEED************/
    /*****************************/
    /*****************************/

    qDebug()<<"[PAnalyse::_Process_Resting_state_seed_based_method] step 1";

    //Get Delta Hb and Mean Delta HbO2 time courses averaged over the seeds size
    for(int id_seed=0;id_seed<seed_nb;id_seed++)
    {
        //Get seed positions
        QVector<int> seeds_pos;
        _Get_Stats_Mask_pos(_M_resting_state_seeds[id_seed],seeds_pos);

        //Nb of element in seed
        int tot = 0;

        //Get Delta Hb and HbO2 and sum vectors
        for(int id=0;id<seeds_pos.size();id++)
        {
            QVector<Mat> Molar_coeff;
            if(!_M_PixelWise_Molar_Coeff.getMolarCoeff(seeds_pos[id],Molar_coeff))
                continue;

            //increment nb of element in seed
            tot++;

            //Get temporal vector
            QVector<QVector<float> > _mixel = _M_filtering.getTemporalVector(seeds_pos[id]);

            //Get Delta Hb and Delta HbO2 contrast
            QVector<QVector<float> > contrast;
            _M_MBLL.get_Chromophore_Time_courses(_mixel,Molar_coeff,contrast,resting_state_time_start,resting_state_time_end,resting_state_time_start,resting_state_time_end);


            //Add contrast during resting state period to the mean contrast
            if(id==0)
            {
                Mean_seed_contrast[id_seed] = contrast;
            }
            else
            {
                for(int id_chrom=0;id_chrom<contrast.size();id_chrom++)
                {
                    for(int t=0;t<Mean_seed_contrast[id_seed][id_chrom].size();t++)
                    {
                        Mean_seed_contrast[id_seed][id_chrom][t] += contrast[id_chrom][t];
                    }

                }
            }
        }

        //If the seed is located on specular points (Molar coeff is empty)
        if(Mean_seed_contrast[id_seed].empty() || tot == 0)
            continue;

        //Divide if the number of pixels which compose the seed is superiror to 1
        if(!seeds_pos.empty())
        {
            for(int id_chrom = 0;id_chrom<Mean_seed_contrast[id_seed].size();id_chrom++)
            {
                for(int t=0;t<Mean_seed_contrast[id_seed][id_chrom].size();t++)
                    Mean_seed_contrast[id_seed][id_chrom][t] /= tot;
            }
        }
    }



    /************************************/
    /*********Process resting state******/
    /************************************/
    /************************************/

    //Init concentration maps
    QVector<QVector<QVector<float> > > correlation_maps;
    correlation_maps.resize(Mean_seed_contrast.size());

    //resize (nb of chromophores)
    for(int i=0;i<Mean_seed_contrast.size();i++)
    {
        correlation_maps[i].resize(Mean_seed_contrast[i].size());
        //resize (nb of pixels)
        for(int j=0;j<Mean_seed_contrast[i].size();j++)
        {
            correlation_maps[i][j].fill(0,_M_pixels_pos.size());
        }
    }

    //Process Pearson correlation coefficient between
    //get Delta Hb/HbO2 time courses
    #pragma omp parallel
    {
        int nb_thread = omp_get_num_threads();
        #pragma omp for
        for(int id=0;id<_M_pixels_pos.size();id++)
        {
            QVector<Mat> Molar_coeff;
            if(!_M_PixelWise_Molar_Coeff.getMolarCoeff(id,Molar_coeff))
                continue;


            //Get temporal vector
            QVector<QVector<float> > _mixel = _M_filtering.getTemporalVector(id);

            //Get Delta Hb and Delta HbO2 contrast
            QVector<QVector<float> > contrast;
            _M_MBLL.get_Chromophore_Time_courses(_mixel,Molar_coeff,contrast,resting_state_time_start,resting_state_time_end,resting_state_time_start,resting_state_time_end);

    //        //TEMP
    //        if(id == (int)(_M_pixels_pos.size()/2))
    //        {
    //            qDebug()<<"resting state corr: "<<pearsoncoeff(Mean_seed_contrast[0][0],contrast[0],_M_process_times[1])<<" "<<pearsoncoeff(Mean_seed_contrast[0][1],contrast[1],_M_process_times[1]);
    //            qDebug()<<"length "<<_M_process_times[1];
    //            WriteTemporalVector(QString(_M_result_directory)+"contrast.txt",contrast);
    //        }
    //        //END TEMP

            //Seed id
            for(int id_seed=0;id_seed<correlation_maps.size();id_seed++)
            {
                //chromophore id
                for(int id_chrom=0;id_chrom<contrast.size();id_chrom++)
                {
                    double corrCoeff = pearsoncoeff(Mean_seed_contrast[id_seed][id_chrom],contrast[id_chrom]);
                    correlation_maps[id_seed][id_chrom][id] = (corrCoeff);
                }
            }
//            emit newProgressStatut("resting state",(int)((100*id)/_M_nb_temporal_vectors));

            emit newProgressStatut("Resting state",(int)((100*id*nb_thread)/_M_nb_temporal_vectors));

        }
    }


    QString saving_dir  =QString(_M_result_directory)+"Resting_state_seed_analysis/";
    QDir dir(saving_dir);

    //Save results only if saving is not created
    if(!dir.exists())
        dir.mkdir(saving_dir);


    //TEMP WRITE SEED CONTRAST
    for(int id_seed;id_seed<seed_nb;id_seed++)
        WriteTemporalVector(saving_dir+"seed_"+QString::number(id_seed)+".txt",Mean_seed_contrast[id_seed]);
    // END TEMP




    //Send Contrast Cartography to IHM
    QVector<QVector<Mat> > maps;
    for(int i =0 ;i<correlation_maps.size();i++)
    {
        QVector<Mat> temp;
        for(int j=0;j<correlation_maps[i].size();j++)
        {
            Mat temp2 = _Display(correlation_maps[i][j]);

//            //opening
//            Mat kernel = getStructuringElement(MORPH_ELLIPSE,Size(5,5));
//            morphologyEx(temp2,temp2,MORPH_OPEN,kernel);

            WriteFloatImg(saving_dir+"Seed_"+QString::number(i)+"_Chrom_"+QString::number(j)+".txt",temp2);
            temp.push_back(temp2);
        }
        maps.push_back(temp);
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


    emit newRestingStateMaps(maps);
    emit newProgressStatut("resting state",100);
}


//Seed Method with SPM analysis (only consider one seed)
void PAnalyse::_Process_Resting_state_seed_SPM()
{
    if(_M_resting_state_seeds.empty())
        return;

    if(_M_resting_state_seeds[0].empty())
        return;

    //Init

    QVector<QVector<float> > Mean_seed_contrast;
    Mean_seed_contrast.clear();

    //Resting state times (whole paradigm, first rest, last rest)
    int resting_state_time_start = _M_paradigm_times.get_rest_start_id();
    int resting_state_time_end = _M_paradigm_times.get_rest_end_id();


    // _M_paradigm_times.process_frame_indexes_for_data_correction(resting_state_time_start,resting_state_time_end);

    /*****************************/
    /*********GET SEED************/
    /*****************************/
    /*****************************/

    qDebug()<<"[PAnalyse::_Process_Resting_state_seed_SPM] step 1";

    //Get Delta Hb and Mean Delta HbO2 time courses averaged over the seed size

    //Get seed positions
    QVector<int> seeds_pos;
    _Get_Stats_Mask_pos(_M_resting_state_seeds[0],seeds_pos);

    //Nb of element in seed
    int tot = 0;

    //Get Delta Hb and HbO2 and sum vectors
    for(int id=0;id<seeds_pos.size();id++)
    {
        QVector<Mat> Molar_coeff;
        if(!_M_PixelWise_Molar_Coeff.getMolarCoeff(seeds_pos[id],Molar_coeff))
            continue;

        //increment nb of element in seed
        tot++;

        //Get temporal vector
        QVector<QVector<float> > _mixel = _M_filtering.getTemporalVector(seeds_pos[id]);

        //Get Delta Hb and Delta HbO2 contrast
        QVector<QVector<float> > contrast;
        _M_MBLL.get_Chromophore_Time_courses(_mixel,Molar_coeff,contrast,resting_state_time_start,resting_state_time_end,resting_state_time_start,resting_state_time_end);


        //Add contrast during resting state period to the mean contrast
        if(id==0)
        {
            Mean_seed_contrast = contrast;
        }
        else
        {
            for(int id_chrom=0;id_chrom<contrast.size();id_chrom++)
            {
                for(int t=0;t<Mean_seed_contrast[id_chrom].size();t++)
                {
                    Mean_seed_contrast[id_chrom][t] += contrast[id_chrom][t];
                }

            }
        }
    }

    //If the seed is located on specular points (Molar coeff is empty)
    if(Mean_seed_contrast.empty() || tot == 0)
        return;

    //Divide if the number of pixels which compose the seed is superiror to 1
    if(!seeds_pos.empty())
    {
        for(int id_chrom = 0;id_chrom<Mean_seed_contrast.size();id_chrom++)
        {
            for(int t=0;t<Mean_seed_contrast[id_chrom].size();t++)
                Mean_seed_contrast[id_chrom][t] /= tot;
        }
    }

    qDebug()<<"[PAnalyse::_Process_Resting_state_seed_SPM] step 2";

    //Init Delta C maps (HbO2,Hb,(oxCCO))
    QVector<Mat> Delta_C_maps;
    for(int c=0;c<Mean_seed_contrast.size();c++)
        Delta_C_maps.push_back(Mat::zeros(Mean_seed_contrast[c].size(),_M_nb_temporal_vectors,CV_32F)); //Size (T,N)

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

            emit newProgressStatut("Get Delta C maps",(int)((100*id*nb_thread)/_M_nb_temporal_vectors));

            //get concentration changes time courses
            QVector<QVector<float> > Delta_C;
            _M_MBLL.get_Chromophore_Time_courses(_M_filtering.getTemporalVector(id),Molar_coeff,Delta_C,resting_state_time_start,resting_state_time_end,resting_state_time_start,resting_state_time_end);



            //Store the HbO2,Hb, (oxCCO) values
            for(int c=0;c<Delta_C.size();c++)
            {
                //Loop over time
                for(int t=0;t<Delta_C[c].size();t++)
                    Delta_C_maps[c].at<float>(t,id) = Delta_C[c][t];
            }
        }
    }

    qDebug()<<"[PAnalyse::_Process_Resting_state_seed_SPM] step 3";

    emit newProgressStatut("SPM resting state",0);

    //Init resting state SPM maps
    QVector<QVector<Mat> > SPM_maps;
    SPM_maps.resize(1);

    //Compute The Statistical Parametric Mapping
    for(int c=0;c<Mean_seed_contrast.size();c++)
    {
        Mat Z_maps_reconstructed;
        Mat Z_map;
        Mat Design_Matrix = _M_SPM.Build_Design_Matrix(Mean_seed_contrast[c], Mean_seed_contrast[c].size());
        _M_SPM.GeneralLinearModel(Delta_C_maps[c],Design_Matrix);
        Z_map = _M_SPM.get_Z_map();
        Z_maps_reconstructed = _reconstruct_Float_Carography(Z_map);
        Z_maps_reconstructed = _M_SPM.gaussianBlur(Z_maps_reconstructed);
        Mat res_GLM = _M_SPM.getThresholdedZMap(Z_maps_reconstructed);

        //Write GLM results
        Mat T_map = _M_SPM.get_T_map();
        T_map = _reconstruct_Float_Carography(T_map);
        WriteFloatImg(QString(_M_result_directory)+"Resting_state_seed_T_map_"+QString::number(c)+".txt",T_map);
        QString path = QString(_M_result_directory)+"Resting_state_seed_SPM"+QString::number(c)+".png";
        imwrite(path.toStdString(),res_GLM);

        //Get threshold map
        SPM_maps[0].push_back(res_GLM);
    }

    qDebug()<<"[PAnalyse::_Process_Resting_state_seed_SPM] step 4";

    emit newProgressStatut("SPM resting state",100);
    emit newRestingStateMaps(SPM_maps);
}


//low frequency power analysis
void PAnalyse::_Process_Resting_state_Low_Frequency_Power_Method()
{

    //Resting state times (whole paradigm, first rest, last rest)
    int resting_state_time_start = _M_paradigm_times.get_rest_start_id();
    int resting_state_time_end = _M_paradigm_times.get_rest_end_id();


    // _M_paradigm_times.process_frame_indexes_for_data_correction(resting_state_time_start,resting_state_time_end);

    //Init concentration maps
    QVector<QVector<float> > std_maps;

    //resize (nb of chromophores)
    std_maps.resize(_M_PixelWise_Molar_Coeff.getChromophoreNumber()+1);
    //resize (nb of pixels)
    for(int j=0;j<std_maps.size();j++)
        std_maps[j].resize(_M_pixels_pos.size());

    #pragma omp parallel
    {
        int nb_thread = omp_get_num_threads();
        #pragma omp for
        //Get concentration changes time courses
        for(int id=0;id<_M_pixels_pos.size();id++)
        {
            QVector<Mat> Molar_coeff;
            if(!_M_PixelWise_Molar_Coeff.getMolarCoeff(id,Molar_coeff))
                continue;

            //Get temporal vector
            QVector<QVector<float> > _mixel = _M_filtering.getTemporalVector(id);

            //Get Delta Hb and Delta HbO2 contrast
            QVector<QVector<float> > contrast;
            _M_MBLL.get_Chromophore_Time_courses(_mixel,Molar_coeff,contrast,resting_state_time_start,resting_state_time_end,resting_state_time_start,resting_state_time_end);


            //Init mean
            QVector<float> mean_val;
            mean_val.resize(contrast.size());
            //Get concentration changes time courses during the resting state times
            for(int t=0;t<contrast[0].size();t++)
            {
                for(int chrom_id=0;chrom_id<contrast.size();chrom_id++)
                {
                    mean_val[chrom_id]      += contrast[chrom_id][t];
                    std_maps[chrom_id][id]  += contrast[chrom_id][t]*contrast[chrom_id][t];
                }
            }

            for(int chrom_id=0;chrom_id<contrast.size();chrom_id++)
            {
                mean_val[chrom_id]/=(resting_state_time_end-resting_state_time_start);
                std_maps[chrom_id][id] = (std_maps[chrom_id][id]/(resting_state_time_end-resting_state_time_start) - (mean_val[chrom_id]*mean_val[chrom_id]));
            }

            contrast.clear();

            emit newProgressStatut("Resting state",(int)((100*id*nb_thread)/_M_pixels_pos.size()));
        }
    }

    //Send Contrast Cartography to IHM
    QVector<QVector<Mat> > maps;
    maps.resize(1);

    for(int j=0;j<std_maps.size();j++)
        maps[0].push_back(_Display(std_maps[j]));


    emit newRestingStateMaps(maps);
    emit newProgressStatut("resting state",100);

}

void PAnalyse::_Process_Resting_state_K_means()
{
    //Resting state times (whole paradigm, first rest, last rest)
    int resting_state_time_start = _M_paradigm_times.get_rest_start_id();
    int resting_state_time_end = _M_paradigm_times.get_rest_end_id();

    // _M_paradigm_times.process_frame_indexes_for_data_correction(resting_state_time_start,resting_state_time_end);

    //Init maps
    QVector<Mat> KMeans_maps;
    for(int i=0;i<_M_PixelWise_Molar_Coeff.getChromophoreNumber()+1;i++)
        KMeans_maps.push_back(Mat::zeros(_M_pixels_pos.size(),(resting_state_time_end-resting_state_time_start),CV_32F));

    #pragma omp parallel
    {
        int nb_thread = omp_get_num_threads();
        #pragma omp for
        //Get concentration changes time courses
        for(int id=0;id<_M_pixels_pos.size();id++)
        {
            QVector<Mat> Molar_coeff;
            if(!_M_PixelWise_Molar_Coeff.getMolarCoeff(id,Molar_coeff))
                continue;

            //Get temporal vector
            QVector<QVector<float> > _mixel = _M_filtering.getTemporalVector(id);

            //Get Delta Hb and Delta HbO2 contrast
            QVector<QVector<float> > contrast;
            _M_MBLL.get_Chromophore_Time_courses(_mixel,Molar_coeff,contrast,resting_state_time_start,resting_state_time_end,resting_state_time_start,resting_state_time_end);



            //Create Maps
            for(int c=0;c<contrast.size();c++)
            {
                for(int t=0;t<contrast[c].size();t++)
                {
                    KMeans_maps[c].at<float>(id,t) = contrast[c][t];
                }
            }

            emit newProgressStatut("K-Means maps ",(int)((100*id*nb_thread)/_M_pixels_pos.size()));
        }
    }

    //Color for each cluster
    QVector<Scalar> color_cluster = getRandomColor(_M_nb_clusters_resting_state);

    //Kmeans processing
    QVector<Mat> out;
    for(int i=0;i<KMeans_maps.size();i++)
    {
        Mat centers;
        Mat temp;
        kmeans(KMeans_maps[i],_M_nb_clusters_resting_state, temp, TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 10000, 0.0001), 5, KMEANS_RANDOM_CENTERS, centers );

        out.push_back(_reconstruct_Kmeans_img(temp,color_cluster));
    }

    emit newProgressStatut("K-Means maps",100);
    emit newKmeans_RestingStateMaps(out);
}

Mat PAnalyse::_reconstruct_Kmeans_img(Mat &label,QVector<Scalar> &color_cluster)
{
    //Creating file for display
    Mat out=Mat::zeros(_M_img_size,CV_8UC3);

    for(int i=0;i<_M_pixels_pos.size();i++)
    {
        _Mycolor color;
        color.b = color_cluster[label.at<int>(0,i)][0];
        color.g = color_cluster[label.at<int>(0,i)][1];
        color.r = color_cluster[label.at<int>(0,i)][2];

        out.at<_Mycolor>(_M_pixels_pos[i].y,_M_pixels_pos[i].x) = color;
    }

    return out;
}

void PAnalyse::_ProcessRestingState()
{
    if(!_M_enable_resting_state)
        return;

    if(_M_ROI.empty())
        return;

    switch (_M_resting_state_method)
    {
    case 0: //Seed based method
        _Process_Resting_state_seed_based_method();
        break;
//    case 1: //ICA based method
//        _Process_Resting_state_ICA_based_method();
//        break;
//    case 2: //low frequency power analysis
//        _Process_Resting_state_Low_Frequency_Power_Method();
//        break;
//    case 3: //K-means clustering
//        _Process_Resting_state_K_means();
//        break;
    case 1: //Seed based method
        _Process_Resting_state_seed_SPM();
        break;

    default:
        break;
    }
}



