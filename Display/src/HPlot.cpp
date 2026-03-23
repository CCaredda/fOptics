#include "HPlot.h"
#include "ui_HPlot.h"

HPlot::HPlot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HPlot)
{
    ui->setupUi(this);

    //Plot selection
    onPlotSelectionChanged(0);

    //Plot
    _M_plot1.clear();
    _M_plot2.clear();

    //Framerate
    _M_FS=30;

    //registration resulsts
    ui->_plot1->setXName("Time (s)");
    ui->_plot1->setYName("Normalized cross Correlation");
    ui->_plot1->InitializeMinMaxValues(0,1);
    ui->_plot1->setTitle("Normalized cross correlation between I(0) and I(t)");
    QVector<QString> names;
    names.push_back("Initial images");
    names.push_back("Registered images");
    ui->_plot1->setPlotNames(names);

}

HPlot::~HPlot()
{
    delete ui;
}



//on plot selection changed
void HPlot::onPlotSelectionChanged(int v)
{
    switch (v)
    {
    case 0:
        //Draw filter
        ui->_plot1->hide();
        ui->_plot0->show();
        break;
    case 1:
        ui->_plot0->hide();
        ui->_plot1->show();
        break;
    default:
        break;
    }
}



//plot names
void HPlot::setPlotNames(QVector<QString> n,int plot_id)
{
    switch (plot_id)
    {
    case 0:
        ui->_plot0->setPlotNames(n);
        break;
    case 1:
        ui->_plot1->setPlotNames(n);
        break;
    default:
        break;
    }
}

void HPlot::setYName(QString n,int v)
{
    switch (v)
    {
    case 0:
        ui->_plot0->setYName(n);
        break;
    case 1:
        ui->_plot1->setYName(n);
        break;
    default:
        break;
    }
}

void HPlot::setXName(QString n,int v)
{
    switch (v)
    {
    case 0:
        ui->_plot0->setXName(n);
        break;
    case 1:
        ui->_plot1->setXName(n);
        break;
    default:
        break;
    }
}



//Set interval
void HPlot::setYInterval(double min,double max,int plot_id)
{
    switch (plot_id)
    {
    case 0:
        ui->_plot0->setYInterval(min,max);
        break;
    case 1:
        ui->_plot1->setYInterval(min,max);
        break;
    default:
        break;
    }
}

void HPlot::setXInterval(double v,int plot_id)
{
    switch (plot_id)
    {
    case 0:
        ui->_plot0->setXInterval(v);
        break;
    case 1:
        ui->_plot1->setXInterval(v);
        break;
    default:
        break;
    }
}

//Set X start
void HPlot::setXStart(double v,int id)
{
    switch (id)
    {
    case 0:
        ui->_plot0->setXStart(v);
        break;
    case 1:
        ui->_plot1->setXStart(v);
        break;
    default:
        break;
    }
}

//Set Y values
void HPlot::setYValue(QVector<QVector<float> > v,int plot_id)
{

    switch (plot_id)
    {
    case 0:
        _M_plot1.clear();
        _M_plot1=v;
        ui->_plot0->setYValue(_M_plot1);
        break;
    case 1:
        _M_plot2.clear();
        _M_plot2=v;
        ui->_plot1->setYValue(_M_plot2);
        ui->_plot1->DisplayRegistrationResult();
        break;
    default:
        break;
    }
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


