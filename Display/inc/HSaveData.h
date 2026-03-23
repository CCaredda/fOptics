/**
 * @file HSaveData.h
 *
 * @brief This class contains the graphical interface to select the data to save
 *
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */


#ifndef HSAVEDATA_H
#define HSAVEDATA_H

#include <QWidget>
#include "pobject.h"

namespace Ui {
class HSaveData;
}

class HSaveData : public QWidget
{
    Q_OBJECT

public:
    explicit HSaveData(QWidget *parent = 0);
    ~HSaveData();

public slots:
    /** Acquisition launch: disable option for saving non filtered signal.
     *  This has to be done before acquiring data.
     *  If option has been checked before acquiring, it is possible to check and uncheck the box since data have been stored in RAM*/
    void onDataAcquisitionIsReady();

    /** on Analysis choice (1: task-based) (2: resting-state), (3: impulsion)*/
    void AnalysisChoice(int);

private slots:
    /** Request results saving **/
    void _requestSaveResults();

    /** Enable mean roi box */
    void onEnableMeanROI(bool);


signals:
    /** Request results saving (send request to PVision class) **/
    void requestSaveResults(_data_saving_info);

    /** Enable or disable option for saving non filtered signals */
    void requestNonFilteredDataSaving(bool);

    /** Mean ROI radius changed */
    void newMeanROIRadius(double);
    /** Request measurement averaged over the mean ROI */
    void requestMeanROIMeasure(bool);

    /** Request video creation */
    void VideoCreationRequired();

    /** Number of frames in the video */
    void newVideo_nb_Frames(double);
    /** Video framerate */
    void newVideoFramerate(double);

private:
    Ui::HSaveData *ui;
};

#endif // HSAVEDATA_H
