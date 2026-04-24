#include "HPLotProfile.h"
#include "ui_HPLotProfile.h"


HPLotProfile::HPLotProfile(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HPLotProfile)
{
    //Inits
    ui->setupUi(this);

    //Bold signal
     _M_Bold_signal.clear();

    //Draw vertical lines
    _M_vertical_lines_pos.clear();


    //nb of measure
    _M_nb_Measure=1;

    //interval for x axis
    _M_x_interval=1;
    _M_x_start=0;

    //X and Y plotting values
    _M_min_y=0;
    _M_max_y=0;

    _M_x_values.clear();
    _M_y_values.clear();

    //Graphs names
    _M_names.clear();
    _M_names.resize(3);

    _M_color.clear();
    _M_color.resize(3);

    _M_y_name.clear();
    _M_y_name="";

    _M_x_name.clear();
    _M_x_name="";



}

HPLotProfile::~HPLotProfile()
{
    delete ui;
}





//Initialize value for scale display
void HPLotProfile::InitializeMinMaxValues(double min, double max)
{
    _M_min_y=min;
    _M_max_y=max;
}

void HPLotProfile::setPlotNames(QVector<QString> n)
{
    _M_color.clear();
    _M_names.clear();

    float currentHue = 0.0;
    for (int i = 0; i < n.size(); i++)
    {
        _M_names.push_back(n[i]);
        _M_color.push_back( QColor::fromHslF(currentHue, 1.0, 0.5) );
        currentHue += 0.618033988749895f;
        currentHue = std::fmod(currentHue, 1.0f);
    }
}

void HPLotProfile::setPlotColors(QVector<QColor> c)
{
    _M_color.clear();
    _M_color = c;
}

void HPLotProfile::setYValue(QVector<QVector<float> > v)
{
    _M_y_values.clear();
    _M_x_values.clear();
    _M_y_values.resize(v.size());
    _M_x_values.resize(v.size());

    for(int i=0;i<v.size();i++)
    {
        for(int j=0;j<v[i].size();j++)
        {
            _M_y_values[i].push_back(v[i][j]);
            _M_x_values[i].push_back(_M_x_start+j*_M_x_interval);
        }
    }

    UpdateGraphs();
}






////PRIVATE SLOTS

//Initialize value for scale display
void HPLotProfile::InitializeMinMaxValues()
{
    _M_min_y=0;
    _M_max_y=0;
}


//Init values
void HPLotProfile::InitValues()
{
    _M_x_values.clear();
    _M_y_values.clear();
}

//Plot values
void HPLotProfile::PlotValues(QVector<double> v, int id, double elapsed_time)
{
    if(_M_y_values.empty())
    {
        _M_y_values.resize(_M_nb_Measure);
        _M_x_values.resize(_M_nb_Measure);

    }
    for(int i=0;i<v.size();i++)
    {
        _M_y_values[id].push_back(v[i]);
        _M_x_values[id].push_back(i*_M_x_interval);
    }

    UpdateGraphs();
}



//PUBLIC SLOTS

void HPLotProfile::UpdateGraphs()
{

    // //Font
    // QFont pfont("sans",24);
    // pfont.setStyleHint(QFont::SansSerif);
    // pfont.setPointSize(24);

    if(_M_y_values.empty())
    {
        qDebug()<<"[HPLotProfile] Y empty";
        return;
    }
    //Create a line series (x,y values)
    QVector<QLineSeries*> series;

    // Create a chart
    QChart *chart = new QChart();
    chart->setTitle(_M_graph_title);



    //look min max x
    int id=0;
    for(int i=0;i<_M_x_values.size();i++)
    {
        if(!_M_x_values[id].empty())
            id = (_M_x_values[id].size()<_M_x_values[i].size()) ? i : id;
    }
    double min_x =_M_x_values[id][0];
    double max_x =_M_x_values[id][_M_x_values[id].size()-1];


    // Create X and Y axes and set range
    QValueAxis *axisX = new QValueAxis();
    axisX->setRange(min_x, max_x);
    axisX->setTitleText(_M_x_name);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(_M_min_y, _M_max_y);
    axisY->setTitleText(_M_y_name);

    // Attach the axes to the chart
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);


    //Modify legend if it required and create QLineSeries
    QVector<QString> legend;
    legend = _M_names;

    //Set X, Y values
    for(int i=0;i<_M_y_values.size();i++)
    {
        QLineSeries * _serie = new QLineSeries();
        for(int k=0;k<_M_y_values[i].size();k++)
            _serie->append(_M_x_values[i][k], _M_y_values[i][k]);

        //Set name to series
        _serie->setName(legend[i]);
        //Set color to series
        _serie->setColor(_M_color[i]);

        series.push_back(_serie);
    }




    //Get min max y value
    double max_y_val = _M_y_values[0][0];
    for(int i=0;i<_M_y_values.size();i++)
    {
        for(int j=0;j<_M_y_values[i].size();j++)
            max_y_val = (_M_y_values[i][j]>max_y_val) ? _M_y_values[i][j] : max_y_val;
    }


    //Plot Bold signal
    if(_M_Bold_signal.size()>0)
    {
        QVector<float> Bold_signal = _M_Bold_signal;

        if(Bold_signal.size()>_M_x_values[0].size())
            Bold_signal.erase(Bold_signal.begin()+_M_x_values[0].size(),Bold_signal.end());
        if(Bold_signal.size()<_M_x_values[0].size())
        {
            for(int i=0;i<_M_x_values[0].size()-Bold_signal.size();i++)
                Bold_signal.push_back(0);
        }
        if(Bold_signal.size()!=_M_x_values[0].size())
            qDebug()<<"HPLotProfile::UpdateGraphs wrong bold vector";


        //Add bold to series
        QLineSeries * bold_serie = new QLineSeries();
        for(int i=0;i<Bold_signal.size();i++)
            bold_serie->append(_M_x_values[0][i],Bold_signal[i]*max_y_val);

        bold_serie->setName("Bold signal");
        bold_serie->setColor(QColor(0,0,0));

        series.push_back(bold_serie);
    }



    //Plot vertical lines
    if(!_M_vertical_lines_pos.empty())
    {
        //inifinite line
        for(int i=0;i<_M_vertical_lines_pos.size();i++)
        {
            QLineSeries *vertical_series = new QLineSeries();
            vertical_series->append(_M_vertical_lines_pos[i], _M_min_y);
            vertical_series->append(_M_vertical_lines_pos[i], _M_max_y);
            vertical_series->setColor(QColor(0,0,0));
            vertical_series->setName("");
            series.push_back(vertical_series);
        }
    }



    //attach the series to the specific axis.
    for(int i=0;i<series.size();i++)
    {
        chart->addSeries(series[i]);

        series[i]->attachAxis(axisX);
        series[i]->attachAxis(axisY);
    }

    // Enable and customize the legend
    chart->legend()->setVisible(true);   // Enable the legend
    chart->legend()->setAlignment(Qt::AlignBottom);  // Position the legend at the bottom

    // Set up the QChartView
    ui->_plot_contrast->setChart(chart);
    ui->_plot_contrast->setRenderHint(QPainter::Antialiasing);

}




//set vertical lines x pos
void HPLotProfile::setVerticalLinesYPosition(QVector<double> v)
{
    _M_vertical_lines_pos.clear();
    for(int i=0;i<v.size();i++)
        _M_vertical_lines_pos.push_back(v[i]);
}

//set Plot Profile
void HPLotProfile::setBoldSignal(QVector<float> v)
{
    _M_Bold_signal.clear();

    //Find max value
    float max = v[0];
    for(int i=0;i<v.size();i++)
        max = (v[i]>max) ? v[i] : max;

    //Normalize Bold signal
    for(int i=0;i<v.size();i++)
        _M_Bold_signal.push_back(v[i]/max);

}

