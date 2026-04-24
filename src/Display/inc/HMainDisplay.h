/**
 * @file HMainDisplay.h
 *
 * @brief This class is the main graphical interface and contains all the instance of the graphical interface classes.
 *
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */


#ifndef HMAINDISPLAY_H
#define HMAINDISPLAY_H

#include <QWidget>
#include <QDesktopServices>
#include <QUrl>
#include "conversion.h"
//#include "PAnalyse.h"
#include "pobject.h"

namespace Ui {
class HMainDisplay;
}

class HMainDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit HMainDisplay(QWidget *parent = 0);
    ~HMainDisplay();

    /** Get result directory */
    QString getResultDirectory();


private slots:

    /** on onNewResultDirectory(QString) */
    void onNewResultDirectory(QString v);

    /** receive a new analysis zone (manually drawn) */
    void onNewAnalysisZone(QVector<QPoint> roi,cv::Size s);


    /** ANALYSIS progress value */
    void onNewAnalysisProgress(QString msg,int v);

    /** Acquisition process progress value */
    void onNewAcquisitionProcessProgress(QString msg,int v);

    /** New frame rate */
     void onNewFrameRate(double);



    /** Display contrast at clicked point */
    void onNewPlot(QVector<QVector<float> > v);




    /** New HMI mode (user, guru) */
    void onNew_Guru_Mode(bool);

    /** Display help */
    void onhelp_required();

private:
    Ui::HMainDisplay *ui;
};

#endif // HMAINDISPLAY_H
