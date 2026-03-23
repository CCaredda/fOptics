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

    /** Set HMI mode (user, guru) */
    void onNew_Guru_Mode(bool);

private slots:

    /** on new stat type */
    void onnewStatType(int v);

signals:
//    /** Statistical level of significance */
//    void newStatisticalLevel(int);

    /** New SPM setting (Full half width of Gaussian kernel) */
    void newSettingStatisticZone(double);

    /** Request Filtered and non filtered signals */
    void RequireFilteredNonFilteredSignals(bool);
    /** Request measurement of distance between grey matter and the closest blood vessel */
    void RequireDistanceToBloodVessels(bool);
    /** New statitc type */
    void newStatType(int);

    /** New z threshold value (used to define activated cortical areas) */
    void newZThreshValue(double);

    /** Stat path */
    void newStatsPath(QString);


    /** Enable Resting state */
    void enableRestingState(bool);
    /** Request resting state seed init */
    void requestSeedInit();
    /** New resting state seed radius */
    void newSeedRadius(double);
    /** New resting state grid seed */
    void newGridSeeds(bool);
    /** Request seed connexion */
    void requestSeedConnexion(bool);
    /** Request seed extraction */
    void requestSeedsExtraction();
    /** New resing state seed method */
    void newRestingStateMethod(int);
    /** New number of independatn component */
    void newNbofIndependantSources_ICA(double);
    /** New ICA source */
    void newICASourceofInterest(int);
    /** New Undersampling for the computation of ICA maps */
    void NewRestingStateMapsSampling(double);
    /** New number of clusters for KMeans resting state */
    void newClustersRestingState(double);




    /** New contrast idx for phase analysis */
    void newcontrastIndexes_Phase_Analysis(int id1,int id2);


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
