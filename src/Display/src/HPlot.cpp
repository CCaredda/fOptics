#include "HPlot.h"
#include "ui_HPlot.h"

HPlot::HPlot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HPlot)
{
    ui->setupUi(this);

    //Plot
    _M_plot1.clear();

    //Framerate
    _M_FS=30;
}

HPlot::~HPlot()
{
    delete ui;
}





//plot names
void HPlot::setPlotNames(QVector<QString> n)
{

    ui->_plot0->setPlotNames(n);
}

void HPlot::setYName(QString n)
{
    ui->_plot0->setYName(n);
}

void HPlot::setXName(QString n)
{
    ui->_plot0->setXName(n);
}



//Set interval
void HPlot::setYInterval(double min,double max)
{
    ui->_plot0->setYInterval(min,max);
}

void HPlot::setXInterval(double v)
{
    ui->_plot0->setXInterval(v);
}

//Set X start
void HPlot::setXStart(double v)
{
    ui->_plot0->setXStart(v);
}

//Set Y values
void HPlot::setYValue(QVector<QVector<float> > v)
{
    _M_plot1.clear();
    _M_plot1=v;
    ui->_plot0->setYValue(_M_plot1);
}



//New process times
void HPlot::onNewActivationTimes(QVector<double> v)
{
    ui->_plot0->setVerticalLinesYPosition(v);
}

//Set Bold signal
void HPlot::onnewBoldSignal(QVector<float> v)
{
    ui->_plot0->setBoldSignal(v);
}


