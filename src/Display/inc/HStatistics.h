/**
 * @file HStatistics.h
 *
 * @brief This class contains the graphical interface to set up the statical anaylises computed on the measured signals:
 * Statistical Parametric Mapping (identification of functional brain areas using the fMRI standard)
 * functional connectivity at rest
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef HSTATISTICS_H
#define HSTATISTICS_H

#include <QWidget>
#include <QFileDialog>
#include <QDebug>
#include "acquisition.h"

namespace Ui {
class HStatistics;
}

class HStatistics : public QWidget
{
    Q_OBJECT

public:
    explicit HStatistics(QWidget *parent = 0);
    ~HStatistics();

    /** Enable statistics */
    void enableActivatedAreasDefinition(bool v);


private slots:

    /** on new stat type */
    void onnewStatType(int v);

signals:

    /** New SPM setting (Full half width of Gaussian kernel) */
    void newSettingStatisticZone(double);

    /** New statitc type */
    void newStatType(int);

    /** New z threshold value (used to define activated cortical areas) */
    void newZThreshValue(double);


public slots:

    /** Set analysis choice (from acquisition) */
    void AnalysisChoice(int);


private:
    Ui::HStatistics *ui;

    //-1: no analysis
    //0 : task-based
    //1: resting state
    //2: Impulsion
    int _M_analysis_choice;
};

#endif // HSTATISTICS_H
