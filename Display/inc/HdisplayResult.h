/**
 * @file HdisplayResult.h
 *
 * @brief This class is the graphical interface used to set up the display of the results.
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef HDISPLAYRESULT_H
#define HDISPLAYRESULT_H

#include <QWidget>
#include "conversion.h"

#define Max_Img_Definition 575000//598000
// Max height and width are defined with max img definition according width-height img ratio (w = 1.878453039 x h)
// Max_Img_Definition = 1.878453039 x h²
#define Max_Height 564
#define Max_Width 1059


namespace Ui {
class HdisplayResult;
}

class HdisplayResult : public QWidget
{
    Q_OBJECT

public:
    explicit HdisplayResult(QWidget *parent = 0);
    ~HdisplayResult();


    /** new stat infos at selected point */
    void setPlotStats(QVector<double> stat);

    /** set Analysis zone */
    void setAnalysisZone(QVector<QPoint> v,cv::Size s);



signals:
    /** Grey outside the contour is requested */
    void GreyOutsideContourIsRequested(bool);

    /** New display value */
    void newDisplayValue(double);

    /** New initial img */
    void newInitialImg(Mat);


//    /** Save img results */
//    void requestSaveResults(bool);

    /** New SPM setting (Full Half width of the Gaussian kernel) */
    void newSettingStatisticZone(double);

    /** New Activation map */
    void newActivationMap(Mat);
    /** New Activation map */
    void newActivationMap(QVector<bool>);
    /** New Activation map */
    void newActivationMap(QVector<bool>,int);


    //new Statistic mask
    /** new activated cortical area definition */
    void newCorticalAreaDefinition(Rect ref,QVector<Rect> non_func,QVector<Mat>);
    /** new activated cortical area definition */
    void newCorticalAreaDefinition(QVector<Rect>,QVector<Mat>);


    /** Request resting state seed init */
    void requestSeedInit();
    /** Request resting state grid seeds */
    void newGridSeeds(bool);
    /** New resting state seed radius */
    void newSeedRadius(double);
    /** Request seed drawing */
    void requestSeeds();
    /** New resting state seeds */
    void newRestingStateSeeds(QVector<Mat>);

    /** New Rect ROI */
    void newRectROI(QPoint,QPoint);
    /** Request Rect ROI drawing */
    void RectROIRequested();

    /** Request line drawing */
    void RequestLineDrawing();
    /** New line emission */
    void newLine(QPoint,QPoint);

    /** Analysis zone drawing */
    void onAnalysisZoneRequested(int);
    /** Send new analysis zone */
    void newAnalysisZone(QVector<QPoint>,cv::Size);

    /** Clear image requested */
    void ClearImg();

    /** request launch process */
    void LaunchProcess();

    /** New Point clicked */
    void PointSelected(Point);
    /** MEAN ROI radius*/
    void newMeanROIRadius(double);
    /** Request mean roi drawing */
    void requestMeanROI(bool);

    /** new Correlation threshold */
    void newCorrelationThreshold(double);

    /** new chromophore ID */
    void newChromophoreID(int);

    /** ROI extraction type */
    void newROI_Extraction_Type(int);

public slots:

    /** New stat type (SPM, T test, ...) */
    void onnewStatType(int);


    /** New contrast image */
    void onnewContrastImg(QVector<Mat>);
    /** Contrast image changed */
    void onContrastImgchanged();



    /** on New Img */
    void onNewImage(Mat);
    /** On new initial image */
    void onNewInitialImg(Mat);

    /** on new ROI Rect */
    void onNewRectROI(QPoint,QPoint);

    //resting states maps
    /** On new resting state maps */
    void onnewRestingStateMaps(QVector<QVector<Mat> >);
    /** On new resting state KMeans maps */
    void onnewKmeans_RestingStateMaps(QVector<Mat>);
    /** Enable resting state */
    void onenableRestingState(bool);
    /** Seed connexion */
    void onSeedConnexionRequested(bool);
    /** Seed extraction */
    void onrequestSeedsExtraction();
    /** New resting state method */
    void onnewRestingStateMethod(int);
    /** New ICA source of interest */
    void onnewICASourceofInterest(int v);

    /** mode RGB or Hyperspectral */
    void onnewRGBMode(bool);

    /**  New activation map */
    void onNewActivationMap(Mat map);

    /** Set HMI mode (user, guru) */
    void onNew_Guru_Mode(bool v);

    /** new result directory */
    void onNewResultDirectory(QString);

private slots:

    /** new analysis zone */
    void onnewAnalysisZone(QVector<QPoint>,cv::Size);

    /**  New cut off display value */
    void onnewCutoffValue();

    /** Update cartography mode */
    void updateCartographyMode();

    /** Display min max value changed */
    void onMinMaxDisplayValueChanged();

    /** Apply Median filter */
    void onMedianFilterON(bool v);
    /** Change window size of the median filter */
    void onWindowSizeChanged(double v);

    /** add or remove oxCCO quantification */
    void updateContrastList();

    /** Resting state new grid seed */
    void onnewGridSeeds(bool v);

    /** new Z thresh */
    void onNewZThresh(double v) {_M_z_thresh = v;}


private:
    Ui::HdisplayResult *ui;

    //Contrast img calculation functions
    void _ContrastImg_Mode_Correlation();
    void _ContrastImg_Mode_Mean(double, double, double cutoff);
    void _ContrastImg_Mode_SPM(double min,double max,double cutoff);

    void _ContrastImg_RestingState();
    void _Process_RestingStateMap_Seeds(Mat img_contrast, Mat &out, int seed_id);
    Mat _Process_Color_Maps(Mat img_contrast);
    Mat _Process_ActivationMap(Mat map);

    //Processing type
    int _M_processing_type;

    //Update color bar display
    void _UpdateColorBar();

    //Contrast image
    QVector<Mat> _M_contrast_img;
    Mat          _M_mask_contrast_img;

    //resting state maps
    QVector<QVector<Mat> > _M_resting_states_maps;
    bool _M_enable_resting_states;
    bool _M_resting_state_seed_connexion;
    int _M_resting_state_method;
    int _M_id_source_IC_resting_state;
    bool _M_grid_seed;
    void _Get_Seed_Connexion(Mat &mask, int seed_id, Mat &falseColormap);


    // Cartography type changed
    bool _M_carto_changed;

    //Display cutoff value
    double _M_cutoff;

    //Corr coeff
    double _M_corr_coef;

    //last processed id
    int     _M_last_processed_id;

    //mode RGB or Hyperspectral
    bool    _M_RGB_mode;

    //Z threshold (RFT threhsold)
    double _M_z_thresh;

};

#endif // HDISPLAYRESULT_H
