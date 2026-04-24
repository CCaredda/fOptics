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



private slots:
    /** Request results saving **/
    void _requestSaveResults();

    /** Enable mean roi box */
    void onEnableMeanROI(bool);


signals:
    /** Request results saving (send request to PVision class) **/
    void requestSaveResults(_data_saving_info);

    /** Mean ROI radius changed */
    void newMeanROIRadius(double);
    /** Request measurement averaged over the mean ROI */
    void requestMeanROIMeasure(bool);


private:
    Ui::HSaveData *ui;
};

#endif // HSAVEDATA_H
