/**
 * @file HPLotProfile.h
 *
 * @brief This class aims to plot the time curves used to control the efficiency of the motion compensation algorithm.
 *
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef HPLotProfile_H
#define HPLotProfile_H

#include <QMouseEvent>
#include <QWidget>
#include "conversion.h"
#include <QFileDialog>
#include <QDebug>

#include "polyfit.hpp"

#include "Crosshairs.h"

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>
#include <QtCharts/QLegend>

namespace Ui {
class HPLotProfile;
}

class HPLotProfile : public QWidget
{
    Q_OBJECT

public:
    explicit HPLotProfile(QWidget *parent = 0);
    ~HPLotProfile();
    /** Set X interval */
    void setXInterval(double x)                 {_M_x_interval=x;}
    /** Set Plot names */
    void setPlotNames(QVector<QString> n);
    /** Set plot colors */
    void setPlotColors(QVector<QColor> c);

    /** Set y axis name */
    void setYName(QString s)                    {_M_y_name=s;}
    /** Set x axis name */
    void setXName(QString s)                    {_M_x_name=s;}
    /** Init min max values of plots */
    void InitializeMinMaxValues();
    /** Init min max values of plots with values */
    void InitializeMinMaxValues(double min, double max);
    /** Set Y values */
    void setYValue(QVector<Vec3b> v);
    /** Set Y values */
    void setYValue(QVector<QVector<float> > v);
    /** Set Y values */
    void setYValue(QVector<float> v);
    /** Set Y values */
    void setYValue(QVector<QVector<double> > v1, QVector<QVector<double> > v2);

    /** Set Y interval */
    void setYInterval(double min,double max)    {_M_min_y=min;_M_max_y=max;}
    /** Set plot title */
    void setTitle(QString v)                    {_M_graph_title=v;}

    /** Set X start */
    void setXStart(double v)                    {_M_x_start=v;}

    /** plot graphs */
    void PlotValues(QVector<double> v,int id,double elapsed_time);

    /** Display mean values */
    void setDisplayMeanValues(bool v)           {_M_display_mean_values=v;}

    /** Set Nb of measure */
    void setNbMeasure(int n)                    {_M_nb_Measure=n;}

    /** Display Registration results (RGB) */
    void DisplayRegistrationResult();
    /** Display Registration results (Hyperspectral camera) */
    void DisplayRegistrationResult_HS();

    /** set vertical lines x pos */
    void setVerticalLinesYPosition(QVector<double> v);

    /** set Bold Profile */
    void setBoldSignal(QVector<float> v);

public slots:
    /** init graphs */
    void InitValues();

protected:
    /** event when graph is closed */
    virtual void closeEvent (QCloseEvent *);

    /** Event when mouse is moved */
    virtual void mouseMoveEvent(QMouseEvent*);

private slots:
    /** Save plots */
    void onSavePlot();

signals:
    /** no more data has to be sent to this class */
    void requestLineEmissionStop();

private:
    Ui::HPLotProfile *ui;

    //Functions
    void UpdateGraphs();
    void Update_HS_Graphs();

    //Parameters
    double                      _M_x_interval;
    double                      _M_x_start;
    int                         _M_nb_Measure;

    QVector<QVector<double> >   _M_y_values;
    QVector<QVector<double> >   _M_x_values;

    QString                     _M_graph_title;
    QVector<QString>            _M_names;
    QString                     _M_y_name;
    QString                     _M_x_name;
    double                      _M_min_y;
    double                      _M_max_y;
    QVector<QColor>             _M_color;

    bool                        _M_display_mean_values;
    QVector<double>             _M_elapsed_values;
    bool                        _M_first_draw;

    //Registration results
    QVector<double>             _M_reg_mean;
    QVector<double>             _M_reg_std;
    QVector<double>             _M_reg_slope;

    //Draw vertical lines
    QVector<double>             _M_vertical_lines_pos;

    //Bold signal
    QVector<float>              _M_Bold_signal;

    //cross cursors
    Crosshairs                  *_M_crosshairs;
};

#endif // HPLotProfile_H
