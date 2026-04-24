/**
 * @file HPlot.h
 *
 * @brief This class aims to plot time curves processed in the analysis classes. It could be concentration changes time curves measured
 * at a given point, intensity signals or FFT signals.
 *
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef HPLOT_H
#define HPLOT_H

#include <QWidget>

namespace Ui {
class HPlot;
}

class HPlot : public QWidget
{
    Q_OBJECT

public:
    explicit HPlot(QWidget *parent = 0);
    ~HPlot();

    /** Set plot names */
    void setPlotNames(QVector<QString> n);
    /** Set y axis name */
    void setYName(QString);
    /** Set x axis name */
    void setXName(QString n);

    /** plot HS registration results */
    void Plot_HS_registration_results(QVector<QVector<QVector<float> > >);

    /** Set Y interval */
    void setYInterval(double min, double max);
    /** Set X interval */
    void setXInterval(double x);

    /** Set X start */
    void setXStart(double v);

    /** Set Y values */
    void setYValue(QVector<QVector<float> > v);

    /** Set frame rate */
    void setFrameRate(double v)    {_M_FS=v;}


public slots:

    /** New process times */
    void onNewActivationTimes(QVector<double> v);

    /** Set Bold signal */
    void onnewBoldSignal(QVector<float> v);



private:
    Ui::HPlot *ui;

    QVector<QVector<float> > _M_plot1;
    double _M_FS;
};

#endif // HPLOT_H
