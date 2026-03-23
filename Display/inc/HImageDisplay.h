/**
 * @file HImageDisplay.h
 *
 * @brief This class aims to display images (input image or results merged with input images)
 * in the graphical interface and to interract with it.
 *
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef HIMAGEDISPLAY_H
#define HIMAGEDISPLAY_H

//#include <QtSvg/QSvgGenerator>
#include <QWidget>
#include <QImage>
#include <QPainter>
#include "conversion.h"
#include "pobject.h"
#include "acquisition.h"
#include <QPainterPath>


class HImageDisplay : public QWidget
{
    Q_OBJECT

public:
    HImageDisplay(QWidget *parent = 0);
    ~HImageDisplay();

    /** Set analysis zone */
    void setAnalysisZone(QVector<QPoint>,cv::Size);

    /** On new initial image */
    void setInialImage(Mat img);

    /** new info bulle */
    void onNewInfosBulle(QStringList list);

    /** is mode resting state */
    bool ismodeRestingState() {return _M_enable_resting_state;}


    /******************************/
    /*********Resting state********/
    /******************************/

    /** Get resting state seeds */
    QVector<Mat> getSeeds();
    /** Get seeds positions */
    QVector<Point> getSeeds_Point ();
    /** New resting state seed method */
    void setNewRestingStateMethod(int v);
    /** Get seed color */
    Scalar getSeedColor(int id);


protected :
    /** Paint event */
    virtual void paintEvent(QPaintEvent *);
    /** Event when mouse is pressed */
    virtual void mousePressEvent(QMouseEvent*);
    /** Event when mouse is moved */
    virtual void mouseMoveEvent(QMouseEvent*);
    /** Event when mouse is released */
    virtual void mouseReleaseEvent(QMouseEvent*);


private slots:
    /** Update img */
    void onupdateImage(const Mat& img);

    /** request seeds */
    void onSeedsrequested();

public slots:
    /** Draw analysis zone */
    void onDrawAnalysisZone(bool v)                 {_M_draw_analysis_zone = v; repaint();}


    /** Grey the outside of the contour */
    void onGreyOutsideContourIsRequested(bool v)    {_M_grey_outside = v;repaint();}

    /** set MEAN ROI radius */
    void onnewMeanROIRadius(double v)               {_M_MeanROI_radius = (int)v;repaint();}
    /** Request mean ROI drawing */
    void onrequestMeanROI(bool v)                   {_M_requestMeanROI = v;repaint();}

    /** Set SPM setting (FHWM of the gaussian kernel) */
    void onnewSettingStatisticZone(double v);
    /** New statistic zone is requested */
//    void onnewStatisticZoneRequested(bool v)        {_M_plot_stat_zone = v; _getStatisticZones();}//onrequestStatsZoneInit();}
    /** New stastical analysis */
    void onnewStatType(int v)                       {_M_stat_zone_type = v; _getStatisticZones();}

    /** New activation map */
    void onnewActivationMap(QVector<bool> activation_map);
    /** New activation map */
    void onnewActivationMap(QVector<bool> activation_map,int proc_id);

    /** Pixel wise activation map */
    void onnewActivationMap(Mat map);


    //resting state
    /** enable resting state */
    void onenableRestingState(bool v)               {_M_enable_resting_state = v;}
    /** init seeds */
    void onrequestSeedInit();
    /** New Seed radius */
    void onnewSeedRadius(double v)                  {_M_radius_seed = (int)v;}
    /** Display Grid seeds */
    void onnewGridSeeds(bool v);


    /** On new ROI extraction type */
    void onnewROI_Extraction_Type(int v);

    /** Receive new Image */
    void newImage(const Mat& img);

    /** RequestLine Drawing */
    void RequestLineDrawing();

    /** Rectangle ROI requested */
    void onRectROIRequested();
    /** New Auto rect ROI */
    void onNewAutoROI(Point,Point);

    /** Analysis zone requested (0: Drawing, 4: Rect) */
    void onAnalysisZoneRequested(int);

    /** save img results */
    void onrequestSaveResults(bool v)   {_M_save_img_results = v;_M_img_saved=0;}

    /** on new alpha value (for transparency when merging results with initial image) */
    void onNewAlphaValue(double v)      {_M_alpha = v; repaint();}

    /** Clear img */
    void ClearImg();

    /** New contrast image */
    void onNewContrastImage(Mat);
    /** New contrast image */
    void onNewContrastImage(Mat,int);

    /** Save img */
    void saveImg(Mat img,int proc_index,QString path);

    /** Set on/off the median filter */
    void onMedianFilterON(bool v)       {_M_median_Filter=v; repaint();}
    /** Size of window for median filter */
    void onWindowSizeChanged(double v)  {_M_window_Median_filter=(int)v;}

    /** New result directory */
    void onNewResultDirectory(QString v)    {_M_result_directory = v;}

signals:

    /** ROI is drawn */
    void analysisZoneisDrawn(bool);

    /** new Line emission */
    void newLine(QPoint,QPoint);

    /** new Rect ROI emission */
    void newRectROI(QPoint,QPoint);

    /** New analysis zone emission */
    void newAnalysisZone(QVector<QPoint>,cv::Size);

    /** new Point selected */
    void PointSelected(Point);



    /** new activated cortical area definition */
    void newCorticalAreaDefinition(Rect,QVector<Rect>,QVector<Mat>);
    /** new activated cortical area definition */
    void newCorticalAreaDefinition(QVector<Rect>,QVector<Mat>);

    /** resting state seeds */
    void newRestingStateSeeds(QVector<Mat>);

    /** Init initial image */
    void newInitialImg(Mat);


private :

    /** Correct ROI point */
    QPoint _correct_ROI_Point(QPoint);




    //Original image
    Mat     _M_initial_image;
    Mat     _M_Matimage;
    Mat     _M_display_img;

    // Image size
    cv::Size _M_img_size;
    cv::Size _M_undersampled_img_size;

    //resize coefficient
    double  _M_x_coeff;
    double  _M_y_coeff;
    double  _M_x_undersampling_coeff;
    double  _M_y_undersampling_coeff;


    //Draw Line
    QPoint  _M_Point1;
    QPoint  _M_Point2;
    QPoint  _M_Point1_displayed;
    QPoint  _M_Point2_displayed;
    bool    _M_draw_Line;
    bool    _M_mouse_click;
    bool    _M_mouse_move;

    //User action
    bool    _M_enable_user_action;

    //Draw rect Roi
    bool    _M_draw_Rect_ROI;
    QPoint  _M_Pre_ROI_firstPoint;
    QPoint  _M_Pre_ROI_LastPoint;
    QPoint  _M_Pre_ROI_firstPoint_display;
    QPoint  _M_Pre_ROI_LastPoint_display;

    //Draw Analysis zone
    bool            _ROI_closed;
    bool            _M_draw_analysis_zone;
    QVector<QPoint> _M_ROI;
    QVector<QPoint> _M_ROI_display_img;

    //Save img results
    bool            _M_save_img_results;
    int             _M_img_saved;
    double          _M_alpha;


    //Contrast image
    bool            _M_draw_contrast_img;
    Mat             _M_contrast_img;

    //Median filter
    bool            _M_median_Filter;
    int             _M_window_Median_filter;

    //Draw Info Bulle
    bool                        _M_draw_info_bulle;
    QStringList                 _M_list_info_bulle;
    void drawInfobulle(QPainter * m_painter ,const QStringList& names ,const QPoint& _pos);

    //Draw Point
    QPoint          _M_selected_Point;
    bool            _M_draw_Point;
    bool            _M_point_is_drawn;

    //ROI type
    int _M_ROI_type;



    //Statistic zone
    //Get stat zone
    void _getStatisticZones();
    void _findResidualContours(Mat dst, Rect roi_rect, QVector<Mat> &residual_cortical_areas);

    int                 _M_setting_stat_zone;
    int                 _M_stat_zone_type;
    bool                _M_request_reference_area;
    QPoint              _M_reference_area_Point;

    QVector<Rect>               _M_cortical_areas;
    QVector<QVector<QPoint> >   _M_residual_cortical_areas_display;


    vector<vector<Point> >      _M_residual_cortical_areas;

    Rect                _M_reference_cortical_area;
    QVector<bool>       _M_activated_functional_areas;


    //Resting state
    bool                _M_grid_seeds;
    bool                _M_enable_resting_state;
    int                 _M_radius_seed;
    QVector<QPoint>     _M_seeds_pos;
    QVector<Scalar>     _M_seeds_colors;
    int                 _M_resting_state_method;

    //MEAN ROI
    int _M_MeanROI_radius;
    bool _M_requestMeanROI;

    //grey outside of the contour
    bool                _M_grey_outside;

    //Result directory
    QString _M_result_directory;

};
#endif // HIMAGEDISPLAY_H
