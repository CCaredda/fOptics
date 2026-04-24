/**
 * @file PModifiedBeerLambertLaw.h
 *
 * @brief Class that aimsused to compute matrices of coefficients used in the modified Beer Lambert law.
 * These matrices and then used to get chromophore's concentration changes.
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */


#ifndef PMODIFIEDBEERLAMBERTLAW_H
#define PMODIFIEDBEERLAMBERTLAW_H

#include <QObject>
#include "oxygenation.h"
#include "molarcoefffunctions.h"
#include "acquisition.h"

class PModifiedBeerLambertLaw : public QObject
{
    Q_OBJECT
public:
    explicit PModifiedBeerLambertLaw(QObject *parent = nullptr);
    ~PModifiedBeerLambertLaw();

    /** Set reference idx (start and end indexes of the first rest period */
    void setReferenceIDX(int start,int end) {_M_start_ref=start; _M_end_ref=end;}
    /** Set activity idx */
    void setActivityIDX(int start,int end)  {_M_start_activity = start; _M_end_activity=end;}
    /** Set correlation time */
    void setCorrelationIDX(int start,int end){_M_start_correlation=start;_M_end_correlation=end;}


    /** Get the concentration changes averaged during the activity periode (using the modified Beer-Lambert law) */
//    void getAveragedConcentrationChanges(const QVector<QVector<float> > &temporal_vec, const QVector<Mat> &coeff, const QVector<int> &mask, QVector<float> &results);
    /** Get the concentration changes averaged during the activity periode (using the modified Beer-Lambert law) */
    void getAveragedConcentrationChanges(const QVector<QVector<float> > &temporal_vec, const QVector<Mat> &coeff, QVector<float> &results, int start_act=-1, int end_act=-1,int start_ref=-1, int end_ref=-1, int start_id=-1, int end_id=-1);

    /** Get the concentration changes time courses for a temporal pixel using the modified Beer-Lambert law */
    void get_Chromophore_Time_courses(const QVector<QVector<float> > &temporal_vec, const QVector<Mat> &coeff, QVector<QVector<float> > &contrast,int start_ref=-1, int end_ref=-1, int start_id=-1, int end_id=-1);



signals:

public slots:
    /** Set New reference times (start and end indexes of the first rest period) */
    void onnewReferenceTime(int start,int end)      {_M_start_ref=start;_M_end_ref=end;}

    /** Set the correlation times */
    void onnewCorrelationTimes(int start,int end)   {_M_start_correlation=start;_M_end_correlation=end;}



private:
    //Concentration changes averaged
    //temporal_vec size(Nb Channels,time)
    void _Single_Deconvolution(const QVector<QVector<float> > &temporal_vec, const Mat &coeff, QVector<float> &results, int start_act_id=-1, int end_act_id=-1, int start_ref=-1, int end_ref=-1);


    //Concentration changes time courses
    void _Single_Deconvolution_Time_courses(const QVector<QVector<float> > &temporal_vec, const Mat &coeff, QVector<QVector<float> > &contrast, int start_ref, int end_ref, int start_id, int end_id);



    //Reference time
    int             _M_start_ref;
    int             _M_end_ref;
    //Correlation time
    int             _M_start_correlation;
    int             _M_end_correlation;
    //Activity time
    int             _M_start_activity;
    int             _M_end_activity;

    //Task-based frame indexes
    QVector<float> _M_frames_idx;
    QVector<float> _M_first_last_rest_frames_idx;

    //Indexes last and first rest steps (for data correction)
    int _M_start_first_rest;
    int _M_end_first_rest;
    int _M_start_last_rest;
    int _M_end_last_rest;


};

#endif // PMODIFIEDBEERLAMBERTLAW_H
