/**
 * @file PProcessTimes.h
 *
 * @brief Class that aims to extract rest and activation time temporal indexes stored in the acquisition info files generated
 * during data acquisition. This class also aims to compute the expected heodynamic response function.
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef PPROCESSTIMES_H
#define PPROCESSTIMES_H

#include <QObject>
#include "acquisition.h"
#include <QDir>
#include <QCoreApplication>


class PProcessTimes : public QObject
{
    Q_OBJECT
public:
    explicit PProcessTimes(QObject *parent = nullptr);

    /** Set resting state mode */
    void setModeRestingState(bool);

    /** Set first and last frames for analysis */
    void setFirstLastFrameForAnalysis(int start, int);

    /** Set the process times (rest and acticity periods) */
    void setProcessTimes(QVector<int>);
    /** Set the number of frames acquired */
    void setNbTotFrames(int v);
    /** Set the camera frame rate */
    void setFrameRate(double Fs);
    /** Set the activation steps to consider */
    void setActivationStepsToConsider(int v);





    /** Get the start index of the rest period */
    int get_rest_start_id()         {return _M_rest_time_start;}
    /** Get the end index of the rest period */
    int get_rest_end_id()           {return _M_rest_time_end;}

    /** get the idx of the last analyzed frame (for the correlation process) */
    int get_last_analyzed_frame_for_Correlation();
    /** get the idx of the first analyzed frame (for the correlation process) */
    int get_first_analyzed_frame_for_Correlation();

    /** Get the idx of frames for the start of activity periods */
    QVector<int> get_Full_activity_start();
    /** Get the idx of frames for the end of activity periods */
    QVector<int> get_Full_activity_end();



    /** Get the idx of the frame for the start of the current activity period */
    int get_activity_start()                {return _M_activity_time_start[0];}
    /** Get the idx of the frame for the end of the current activity period */
    int get_activity_end()                  {return _M_activity_time_end[0];}


    /** get the BOLD signal */
    QVector<float> get_Bold_Signal()                {return _M_BOLD_signal;}

    /** get metabolic response signal */
    QVector<float> get_Metabolic_response()         {return _M_Metabolic_response_signal;}

    /** get activation signal (neuronal activity)*/
    QVector<float> get_Activation_Signal();

    /** Process frame idx for for data correction */
    // void process_frame_indexes_for_data_correction(int start,int end);


signals:
    /** Send the indexes of the rest periods used as reference */
    void newReferenceTime(int,int);
    /** Send indexes of frames used for correlation analysis */
    void newCorrelationTimes(int,int);

    /** Send the activation period indexes */
    void newActivationtimes(QVector<double>);

    /** Send Bold signal */
    void newBoldSignal(QVector<float>);

    /** Send Metabolic response */
    void newMetabolicResponse(QVector<float>);

    /** Send data correction index */
    void newDataCorrectionIdx(int start_first_rest, int end_first_rest,int start_last_rest,int end_last_rest);


public slots:
    /** New result directory */
    void onNewResultDirectory(QString v)    {_M_result_directory = v;}

private:
    void _Compute_Bold_Signal();


    // Number of activity step to consider in the calculation of correlation and SPM
    int             _M_nb_activity_step_to_consider;

    //Global aquisiton info
    int             _M_nb_frames;
    double          _M_FS;

    //Paradigm infos
    int             _M_average_activity_period;
    int             _M_rest_time_start;
    int             _M_rest_time_end;
    QVector<int>    _M_last_analyzed_frame_for_correlation;
    QVector<int>    _M_first_analyzed_frame_for_correlation;

    // // idx for first and last rest of the task-based procedure
    // int             _M_rest_id_start_1st_tb;
    // int             _M_rest_id_end_1st_tb;
    // int             _M_rest_id_start_last_tb;
    // int             _M_rest_id_end_last_tb;


    // //real time calibration
    // int             _M_start_calib;
    // int             _M_end_calib;


    // Activity steps contained in the .txt file
    QVector<int>    _M_activity_time_start;
    QVector<int>    _M_activity_time_end;



    //BOLD Signal
    QVector<float>  _M_BOLD_signal;

    //Metabolic response
    QVector<float> _M_Metabolic_response_signal;

    //Result directory
    QString _M_result_directory;

    //mode resting state
    bool    _M_mode_resting_state;

    int _M_nb_of_frame_to_substract;
};

#endif // PPROCESSTIMES_H
