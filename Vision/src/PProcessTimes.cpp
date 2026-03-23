#include "PProcessTimes.h"


/***********************************/
/***********************************/
/*********README********************/
/***********************************/
/***********************************/
//_M_rest_time_start : start id of the first rest (we consider the start N s before the start of the first activity. N is the average period of the activity steps)
//_M_rest_time_end: end id of the first rest (3s before the first activity step)


//_M_last_analyzed_frame_for_correlation: id of the last analyzed frame (N frame after the last activity step. N is the average period of the activity steps)
//_M_first_analyzed_frame_for_correlation

PProcessTimes::PProcessTimes(QObject *parent) : QObject(parent)
{
    //Init result directory
    _M_result_directory =  "/home/results/";



    _M_nb_frames            = 0;
    _M_FS                   = 0.0;
    _M_nb_activity_step_to_consider = -1;

    _M_average_activity_period = 0;
    _M_last_analyzed_frame_for_correlation.clear();
    _M_first_analyzed_frame_for_correlation.clear();
    _M_rest_time_start      = 0;
    _M_rest_time_end        = 0;
    _M_activity_time_start.clear();
    _M_activity_time_end.clear();
    _M_BOLD_signal.clear();
    _M_Metabolic_response_signal.clear();


    // _M_rest_id_start_1st_tb = 0;
    // _M_rest_id_end_1st_tb = 0;
    // _M_rest_id_start_last_tb = 0;
    // _M_rest_id_end_last_tb = 0;

    _M_mode_resting_state = false;


    _M_nb_of_frame_to_substract = 0;

}


/** get the idx of the last analyzed frame (for the correlation process) */
int PProcessTimes::get_last_analyzed_frame_for_Correlation()
{
    qDebug()<<"PProcessTimes::get_last_analyzed_frame_for_Correlation _M_nb_activity_step_to_consider "<<_M_nb_activity_step_to_consider;
    return _M_last_analyzed_frame_for_correlation[_M_nb_activity_step_to_consider];
}
/** get the idx of the first analyzed frame (for the correlation process) */
int PProcessTimes::get_first_analyzed_frame_for_Correlation()
{
    qDebug()<<"PProcessTimes::get_first_analyzed_frame_for_Correlation _M_nb_activity_step_to_consider "<<_M_nb_activity_step_to_consider;
    return _M_first_analyzed_frame_for_correlation[_M_nb_activity_step_to_consider];
}

/** Get the idx of frames for the start of activity periods */
QVector<int> PProcessTimes::get_Full_activity_start()
{
    qDebug()<<"PProcessTimes::get_Full_activity_start _M_nb_activity_step_to_consider "<<_M_nb_activity_step_to_consider;
    return _M_activity_time_start;
}

/** Get the idx of frames for the end of activity periods */
QVector<int> PProcessTimes::get_Full_activity_end()
{
    qDebug()<<"PProcessTimes::get_Full_activity_end _M_nb_activity_step_to_consider "<<_M_nb_activity_step_to_consider;
    return _M_activity_time_end;
}


/** Set resting state mode */
void PProcessTimes::setModeRestingState(bool v)
{
    qDebug()<<"[PProcessTimes::setModeRestingState] mode resting state "<<v;
    _M_mode_resting_state =v;
}


// Get activation signal (neuronal activity)
QVector<float> PProcessTimes::get_Activation_Signal()
{
    QVector<float> neuronal_activity;

    int length_BOLD = _M_nb_frames;

    if(length_BOLD<=0)
        return neuronal_activity;


    //Fill vector with zeros
    neuronal_activity.fill(0.0f,length_BOLD);

    //Set activity periods
    for(int i=0;i<_M_activity_time_start.size();i++)
    {
        // for(int t=_M_activity_time_start[i]-_M_first_analyzed_frame_for_correlation;t<_M_activity_time_end[i]-_M_first_analyzed_frame_for_correlation;t++)
        for(int t=_M_activity_time_start[i];t<_M_activity_time_end[i];t++)
            neuronal_activity[t] = 1.0f;
    }

    return neuronal_activity;

}

// Set activation step to consider
void PProcessTimes::setActivationStepsToConsider(int v)
{
    _M_nb_activity_step_to_consider = v;
    qDebug()<<"PProcessTimes::setActivationStepsToConsider "<<v;

    // if(_M_nb_frames!=0)
    //     setNbTotFrames(_M_nb_frames);
}

//Set nb of frame to substract
void PProcessTimes::setFirstLastFrameForAnalysis(int start, int)
{
    _M_nb_of_frame_to_substract = start;

    qDebug()<<"[PProcessTimes::setFirstLastFrameForAnalysis] nb of frame to substract: "<<_M_nb_of_frame_to_substract;
}


//set process times
void PProcessTimes::setProcessTimes(QVector<int> v)
{
    int nb_activity_period = floor(v.size()/2);

//    for(int i=0;i<v.size();i++)
//        qDebug()<<"[PProcesTimes] Activation time "<<i<<" "<<v[i];

    if(v.size()==1)
    {
        //Two ids for the activity period
        _M_activity_time_start.push_back(v[0]);
        _M_activity_time_end.push_back(2*v[0]);
        return;
    }

    for(int i=0;i<nb_activity_period;i++)
    {
        //Two ids for the activity period
        _M_activity_time_start.push_back(v[i*2]);
        _M_activity_time_end.push_back(v[i*2+1]);
    }
}




// Set the camera frame rate
void PProcessTimes::setFrameRate(double Fs)
{
    qDebug()<<"[PProcessTimes] Framerate: "<<Fs;
    _M_FS = Fs;
}


// // Process frame idx for for data correction
// void PProcessTimes::process_frame_indexes_for_data_correction(int start,int end)
// {
//     //Compute times for data correction (5 seconds)
//     _M_rest_id_start_1st_tb = start;
//     _M_rest_id_end_1st_tb = start + int(5*_M_FS);
//     _M_rest_id_start_last_tb = end - int(5*_M_FS);
//     _M_rest_id_end_last_tb = end;

//     emit newDataCorrectionIdx(_M_rest_id_start_1st_tb, _M_rest_id_end_1st_tb,_M_rest_id_start_last_tb,_M_rest_id_end_last_tb);
// }

//set tot nb of frames and process the processing times
void PProcessTimes::setNbTotFrames(int v)
{
    qDebug()<<"[PProcessTimes::setNbTotFrames]";

    _M_nb_frames=v;

    _M_first_analyzed_frame_for_correlation.clear();
    _M_last_analyzed_frame_for_correlation.clear();

    //If activity step is empty, no activity steps have been indicated during the data acquisition
    //Consider all frames
    if(_M_activity_time_start.empty() && !_M_mode_resting_state)
    {
        _M_nb_activity_step_to_consider=0;
        _M_rest_time_end = v-1;
        _M_rest_time_start = 0;
        _M_first_analyzed_frame_for_correlation.push_back(_M_rest_time_start);
        _M_last_analyzed_frame_for_correlation.push_back(_M_rest_time_end);

        _M_activity_time_start.push_back(_M_rest_time_start);
        _M_activity_time_end.push_back(_M_rest_time_end);

        emit newReferenceTime(_M_rest_time_start,_M_rest_time_end);
        emit newCorrelationTimes(_M_first_analyzed_frame_for_correlation[_M_nb_activity_step_to_consider],_M_last_analyzed_frame_for_correlation[_M_nb_activity_step_to_consider]);

        return;
    }



    if(_M_mode_resting_state)
    {
        qDebug()<<"[PProcessTimes::setNbTotFrames] mode resting state";
        _M_nb_activity_step_to_consider=0;
        _M_rest_time_end = v-1;
        _M_rest_time_start = 0;
        _M_first_analyzed_frame_for_correlation.push_back(_M_rest_time_start);
        _M_last_analyzed_frame_for_correlation.push_back(_M_rest_time_end);

        _M_BOLD_signal.clear();
        _M_BOLD_signal.resize(_M_nb_frames);
        _M_Metabolic_response_signal.clear();
        _M_Metabolic_response_signal.resize(_M_nb_frames);

        emit newReferenceTime(_M_rest_time_start,_M_rest_time_end);
        emit newCorrelationTimes(_M_first_analyzed_frame_for_correlation[_M_nb_activity_step_to_consider],_M_last_analyzed_frame_for_correlation[_M_nb_activity_step_to_consider]);

        return;
    }




    qDebug()<<"[PProcessTimes::setNbTotFrames] mode tb";

    //Set rest id
    _M_rest_time_start = 0;


    //Substract first frame to be considered to activity idx
    for(int i=0;i<_M_activity_time_start.size();i++)
    {
        //Start activity
        _M_activity_time_start[i] = _M_activity_time_start[i] - _M_nb_of_frame_to_substract;

        //Get the first analyzed frame
        _M_first_analyzed_frame_for_correlation.push_back(_M_rest_time_start);
    }


    for (int i=0;i<_M_activity_time_end.size();i++)
    {
        //end activity
        _M_activity_time_end[i] = _M_activity_time_end[i] - _M_nb_of_frame_to_substract;

        //Get the last analyzed frame

        //Set last frames if it is the last activation step
        if((i+1) == _M_activity_time_end.size())
            _M_last_analyzed_frame_for_correlation.push_back(_M_nb_frames-1);
        else
            _M_last_analyzed_frame_for_correlation.push_back(_M_activity_time_start[i+1] - int(3*_M_FS));

    }

    //Set the ending rest id (3s before the first activation)
    _M_rest_time_end    = _M_activity_time_start[0] - int(3*_M_FS);



    qDebug()<<"[PProcessTimes] first frame rest: "<<_M_rest_time_start;
    qDebug()<<"[PProcessTimes] last frame rest: "<<_M_rest_time_end;
    qDebug()<<"[PProcessTimes] _M_activity_time_start: "<<_M_activity_time_start;
    qDebug()<<"[PProcessTimes] _M_activity_time_end: "<<_M_activity_time_end;

    /*************************************************/
    /******First and Last analyzed frames*************/
    /*************************************************/

    // int nb_activity_step_to_consider = (_M_nb_activity_step_to_consider==-1) ? _M_activity_time_end.size()-1 : _M_nb_activity_step_to_consider;




    qDebug()<<"[PProcessTimes] first frame analyzed for correlation: "<<_M_first_analyzed_frame_for_correlation;
    qDebug()<<"[PProcessTimes] last frame analyzed for correlation: "<<_M_last_analyzed_frame_for_correlation;


    //Compute BOLD
    _Compute_Bold_Signal();

    //Computation task-based index
    QVector<double> activation_steps;
    for(int i =0;i<_M_activity_time_start.size();i++)
    {
        // activation_steps.push_back((_M_activity_time_start[i] - _M_first_analyzed_frame_for_correlation)/_M_FS);
        // activation_steps.push_back((_M_activity_time_end[i] - _M_first_analyzed_frame_for_correlation)/_M_FS);
        activation_steps.push_back(_M_activity_time_start[i]);
        activation_steps.push_back(_M_activity_time_end[i]);
    }


    // // Compute times for data correction (5 seconds)
    // _M_rest_id_start_1st_tb = _M_first_analyzed_frame_for_correlation;
    // _M_rest_id_end_1st_tb = _M_first_analyzed_frame_for_correlation + int(5*_M_FS);
    // _M_rest_id_start_last_tb = _M_last_analyzed_frame_for_correlation - int(5*_M_FS);
    // _M_rest_id_end_last_tb = _M_last_analyzed_frame_for_correlation;

    // emit newDataCorrectionIdx(_M_rest_id_start_1st_tb, _M_rest_id_end_1st_tb,_M_rest_id_start_last_tb,_M_rest_id_end_last_tb);




    //Send signal
    emit newActivationtimes(activation_steps);
    emit newReferenceTime(_M_rest_time_start,_M_rest_time_end);
    emit newCorrelationTimes(_M_first_analyzed_frame_for_correlation[_M_nb_activity_step_to_consider],_M_last_analyzed_frame_for_correlation[_M_nb_activity_step_to_consider]);



    QString saving_dir  =QString(_M_result_directory)+"Temporal_idx";
    if(!QDir().exists(saving_dir))
        QDir().mkdir(saving_dir);

    // Start end correlation
    QFile infos(saving_dir+"/correlation_idx.txt");
    if(infos.exists())
        infos.remove();

    if (infos.open(QIODevice::ReadWrite))
    {
        QTextStream Qt( &infos );
        Qt<<get_first_analyzed_frame_for_Correlation()<<Qt::endl;
        Qt<<get_last_analyzed_frame_for_Correlation()<<Qt::endl;
    }
    infos.close();

    //Rest period idx
    infos.setFileName(saving_dir+"/rest_idx.txt");
    if(infos.exists())
        infos.remove();
    if (infos.open(QIODevice::ReadWrite))
    {
        QTextStream Qt( &infos );
        Qt<<get_rest_start_id()<<Qt::endl;
        Qt<<get_rest_end_id()<<Qt::endl;
    }
    infos.close();

}


// Get Bold Signal with process excitation Datas process the whole BOLD signal in the range (_M_first_analyzed_frame_for_correlation;_M_last_analyzed_frame_for_correlation)
void PProcessTimes::_Compute_Bold_Signal()
{
    qDebug()<<"PProcessTimes::_Compute_Bold_Signal";

    _M_BOLD_signal.clear();
    _M_Metabolic_response_signal.clear();

    int length_BOLD = _M_nb_frames;

    if(length_BOLD<=0)
    {
        qDebug()<<"PProcessTimes::_Compute_Bold_Signal() negative length "<<length_BOLD;
        return;
    }

    // Prepare infos et output .txt files
    // QFile infos(QString(PROPATH)+"/../Python/src/infos.txt");
    QFile infos(_M_result_directory+"/infos.txt");
    if(infos.exists())
        infos.remove();

    // QFile Bold_file(QString(PROPATH)+"/../Python/src/Bold_convolved.txt");
    QFile Bold_file(_M_result_directory+"/Bold_convolved.txt");
    if(Bold_file.exists())
        Bold_file.remove();

    // QFile oxCCO_file(QString(PROPATH)+"/../Python/src/Metabolic_response_convolved.txt");
    QFile oxCCO_file(_M_result_directory+"/Metabolic_response_convolved.txt");
    if(oxCCO_file.exists())
        oxCCO_file.remove();



    // Write infos.txt containing times of the paradigm and Fs
    if (infos.open(QIODevice::ReadWrite))
    {
        QTextStream Qt( &infos );

        //activity periods
        for(int i=0;i<_M_activity_time_start.size();i++)
            // Qt<<_M_activity_time_start[i]-_M_first_analyzed_frame_for_correlation<<" "<<_M_activity_time_end[i]-_M_first_analyzed_frame_for_correlation<<"\n";
            Qt<<_M_activity_time_start[i]<<" "<<_M_activity_time_end[i]<<"\n";

        // Frequency sampling (add) zero to produce an uniform txt file
        Qt<<_M_FS<<" "<<0<<"\n";

        // Vector size
        Qt<<length_BOLD<<" "<<0;
    }
    infos.close();













    // Launch Python script for Convolve Bold signal calculation
    QString path_models = QString(QCoreApplication::applicationDirPath())+"/../share/python";
    bool sucess = launchPythonScript("get_Bold_Signal.py", _M_result_directory, path_models);


    //Read convolved Bold signal
    if(Bold_file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&Bold_file);
        while(!in.atEnd())
        {
            QString line = in.readLine();
            _M_BOLD_signal.push_back( line.toFloat());
        }
        Bold_file.close();
    }
    else
    {
        qDebug()<<"Pb ouverture Bold convolved file!!";
    }
    Bold_file.close();

    //Read convolved Metabolic response signal
    if(oxCCO_file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&oxCCO_file);
        while(!in.atEnd())
        {
            QString line = in.readLine();
            _M_Metabolic_response_signal.push_back( line.toFloat());
        }
        oxCCO_file.close();
    }
    else
    {
        qDebug()<<"Pb ouverture Metabolic response convolved file!!";
    }
    oxCCO_file.close();


    // Check Bold size
    if(_M_BOLD_signal.size()!=length_BOLD)
    {
        if(_M_BOLD_signal.size()> length_BOLD)
        {
            while (_M_BOLD_signal.size()!=length_BOLD)
            {
                _M_BOLD_signal.removeLast();
            }
        }
        else
        {
            while (_M_BOLD_signal.size()!=length_BOLD)
            {
                _M_BOLD_signal.push_back(0.0f);
            }
        }
    }


    // Check Bold size
    if(_M_Metabolic_response_signal.size()!=length_BOLD)
    {
        if(_M_Metabolic_response_signal.size()> length_BOLD)
        {
            while (_M_Metabolic_response_signal.size()!=length_BOLD)
            {
                _M_Metabolic_response_signal.removeLast();
            }
        }
        else
        {
            while (_M_Metabolic_response_signal.size()!=length_BOLD)
            {
                _M_Metabolic_response_signal.push_back(0.0f);
            }
        }
    }

    qDebug()<<"[Compute Bold] length BOLD: "<<_M_BOLD_signal.size();
    qDebug()<<"[Compute Bold] length meta: "<<_M_Metabolic_response_signal.size();




    //Remove files from harddrive
    if(Bold_file.exists())
        Bold_file.remove();
    if(infos.exists())
        infos.remove();
    if(oxCCO_file.exists())
        oxCCO_file.remove();

    emit newBoldSignal(_M_BOLD_signal);
    emit newMetabolicResponse(_M_Metabolic_response_signal);

//    //Test
//    WriteTemporalVector(QString(_M_result_directory)+"test_BOLD.txt",_M_BOLD_signal);
}




