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

    //Draw title at first graph apparition
    _M_first_draw=true;

    //Registration results
    _M_reg_mean.clear();
    _M_reg_std.clear();
    _M_reg_slope.clear();

    //registration results
    ui->_result->hide();

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

    //Elapsed values
    _M_elapsed_values.clear();


    //Display plot mean values
    _M_display_mean_values=false;

    //Save plots
    connect(ui->_save,SIGNAL(pressed()),this,SLOT(onSavePlot()));



    _M_crosshairs = new Crosshairs();


//    designMe(this,"Plot/style/plot.css");
}

HPLotProfile::~HPLotProfile()
{
    delete ui;
}


void HPLotProfile::mouseMoveEvent(QMouseEvent* ev)
{
    //Check if a graph has been defined
    if(!_M_crosshairs->get_Flag())
        return;

    //Plot the crosshair position
    _M_crosshairs->updatePosition(ev->pos());
}

//Save Plots
void HPLotProfile::onSavePlot()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 "/home/caredda/Pictures/Experimentation_cochon",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    QString path = dir+"/Acquisition control_"+QTime::currentTime().toString("hh:mm:ss.zzz")+".txt";



    if(_M_y_values.empty())
        return;

    QFile info_file(path);

    if (info_file.open(QIODevice::ReadWrite))
    {
        QTextStream Qt( &info_file );

        if(_M_x_values.empty())
            return;

        for(int i=0;i<_M_x_values[0].size();i++)
        {
            Qt<<_M_x_values[0][i]<<" ";
        }
        Qt<<"\n";

        for(int i=0;i<_M_y_values.size();i++)
        {
            for(int j=0;j<_M_y_values[i].size();j++)
                Qt<<_M_y_values[i][j]<<" ";
            Qt<<"\n";
        }
    }
    info_file.close();
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
void HPLotProfile::setYValue(QVector<Vec3b> v)
{
    ui->_result->hide();
    _M_y_values.clear();
    _M_y_values.resize(3);
    _M_x_values.clear();
    _M_x_values.resize(3);


    for(int i=0;i<v.size();i++)
    {
        _M_y_values[0].push_back(v[i][2]);
        _M_y_values[1].push_back(v[i][1]);
        _M_y_values[2].push_back(v[i][0]);
        _M_x_values[0].push_back(i*_M_x_interval);
        _M_x_values[1].push_back(i*_M_x_interval);
        _M_x_values[2].push_back(i*_M_x_interval);
    }


    UpdateGraphs();
}

void HPLotProfile::setYValue(QVector<QVector<float> > v)
{
    ui->_result->hide();
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

void HPLotProfile::setYValue(QVector<float> v)
{
    //Img demoisaiquée
    ui->_result->hide();
    _M_y_values.clear();
    _M_x_values.clear();
    _M_y_values.resize(1);
    _M_x_values.resize(1);

    for(int i=0;i<_M_y_values.size();i++)
    {
        for(int j=0;j<v.size();j++)
        {
            _M_y_values[i].push_back(v[j]);
            _M_x_values[i].push_back(_M_x_start+j*_M_x_interval);
        }
    }

    UpdateGraphs();
}

void HPLotProfile::setYValue(QVector<QVector<double> > neg,QVector<QVector<double> > pos)
{
    //Img demoisaiquée
    ui->_result->hide();
    _M_y_values.clear();
    _M_x_values.clear();

    _M_y_values.resize(4);
    _M_x_values.resize(4);


    for(int j=0;j<neg[0].size();j++)
    {
        _M_y_values[0].push_back(neg[0][j]);
        _M_x_values[0].push_back(_M_x_start+j*_M_x_interval);

        _M_y_values[1].push_back(pos[0][j]);
        _M_x_values[1].push_back(_M_x_start+j*_M_x_interval);

        _M_y_values[2].push_back(neg[1][j]);
        _M_x_values[2].push_back(_M_x_start+j*_M_x_interval);

        _M_y_values[3].push_back(pos[1][j]);
        _M_x_values[3].push_back(_M_x_start+j*_M_x_interval);
    }

    Update_HS_Graphs();
}

//protected
void HPLotProfile::closeEvent (QCloseEvent *)
{
    _M_elapsed_values.clear();
    _M_x_values.clear();
    _M_y_values.clear();
    emit requestLineEmissionStop();
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
        _M_elapsed_values.clear();
        _M_elapsed_values.resize(_M_nb_Measure);
    }
    for(int i=0;i<v.size();i++)
    {
        _M_y_values[id].push_back(v[i]);
        _M_x_values[id].push_back(i*_M_x_interval);
    }
    _M_elapsed_values[id] = elapsed_time;
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
    if(_M_display_mean_values)
    {
        for(int i=0;i<_M_y_values.size();i++)
        {
            if(_M_y_values[i].empty())
                legend[i]="";
            else
            {
                double val_mean=0.0;
                for(int k=0;k<_M_y_values[i].size();k++)
                    val_mean+=_M_y_values[i][k];


                val_mean/=_M_y_values[i].size();
                double Fe = _M_elapsed_values[i]/_M_y_values[i].size();
                legend[i] = legend[i]+" ,mean Ts : "+QString::number(val_mean)+" ms, Therical Fs : "+QString::number(1000/Fe)+" images per second";

            }
        }
    }

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

    //init crosshairs
    _M_crosshairs = new Crosshairs(chart);
}




void HPLotProfile::Update_HS_Graphs()
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

    QValueAxis *axisY= new QValueAxis();
    axisY->setRange(_M_min_y, _M_max_y);
    axisY->setTitleText("Y Axis");

    // Attach the axes to the chart
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);


    //Fill series with values
    QVector<QLineSeries*> series;
    for(int i=0;i<_M_y_values.size();i++)
    {
        QLineSeries *_serie = new QLineSeries();

        for(int j=0;i<_M_y_values[i].size();j++)
            series[i]->append(_M_x_values[i][j], _M_y_values[i][j]);

        series.push_back(_serie);
    }

    // //Set name to series
    // series[0]->setName(_M_names[0]);
    // series[1]->setName(_M_names[0]);
    // series[2]->setName(_M_names[1]);
    // series[3]->setName(_M_names[1]);


    //attach the series to the specific axis.
    for(int i=0;i<series.size();i++)
    {
        chart->addSeries(series[i]);

        series[i]->attachAxis(axisX);
        series[i]->attachAxis(axisY);
    }


    // Set up the QChartView
    ui->_plot_contrast->setChart(chart);
    ui->_plot_contrast->setRenderHint(QPainter::Antialiasing);

    //init crosshairs
    _M_crosshairs = new Crosshairs(chart);
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


//Display registration result
void HPLotProfile::DisplayRegistrationResult()
{
    ui->_result->show();

    // Check
    if(_M_y_values.empty())
        return;
    if(_M_y_values[0].empty())
        return;

    //Registration results
    int tot=0;
    int vec_size = _M_y_values.size();
    _M_reg_mean.fill(0,vec_size);
    _M_reg_std.fill(0,vec_size);
    _M_reg_slope.fill(0,vec_size);

    /**************************************/
    /**************************************/
    /**********Measure NCC infos***********/
    /**************************************/
    /**************************************/


    // Std and mean calculation
    for(int s=0;s<vec_size;s++)
    {
        QVector<double> x;
        x.clear();
        tot=0;
        for(int i=0;i<_M_y_values[s].size();i++)
        {
            tot++;
            _M_reg_mean[s]+=_M_y_values[s][i];
            _M_reg_std[s]+=_M_y_values[s][i]*_M_y_values[s][i];
            x.push_back(i+1);
        }
        if(tot==0)
            return;

        _M_reg_std[s]/=tot;
        _M_reg_mean[s]/=tot;
        _M_reg_std[s] = sqrt(_M_reg_std[s] - (_M_reg_mean[s]*_M_reg_mean[s]));


        //Get slope coefficient
        vector<double> _p;
        if(polyfit(x,_M_y_values[s],1,_p)==0)
        {
            qDebug()<<"[Polyfit] error";
        }

        _M_reg_slope[s] = _p[1];
    }

    /**************************************/
    /**************************************/
    /**********Get ref NCC infos***********/
    /**************************************/
    /**************************************/
    double ref_mean_Mean    = 0.0;
    double ref_std_Mean     = 0.0;
    double ref_slope_Mean   = 0.0;
    double ref_mean_Std     = 0.0;
    double ref_std_Std      = 0.0;
    double ref_slope_Std    = 0.0;
    tot=0;


    QFile infos_mean(QString(QCoreApplication::applicationDirPath())+"/../share/files/infos_mean.txt");
    QFile infos_std(QString(QCoreApplication::applicationDirPath())+"/../share/files/infos_std.txt");
    QFile infos_slope(QString(QCoreApplication::applicationDirPath())+"/../share/files/infos_slope.txt");
    if(infos_mean.open(QIODevice::ReadOnly))
    {
        //Mean
        QTextStream in_mean(&infos_mean);
        tot=0;
        while(!in_mean.atEnd())
        {
            QString line    = in_mean.readLine();
            double val      = line.toDouble();
            ref_mean_Mean   += val;
            ref_mean_Std    += val*val;
            tot++;
        }
        ref_mean_Std/=tot;
        ref_mean_Mean/=tot;
        ref_mean_Std = sqrt(ref_mean_Std - (ref_mean_Mean*ref_mean_Mean));
        infos_mean.close();
    }
    else
        qDebug()<<"Pb in reading ref mean ";
    if(infos_std.open(QIODevice::ReadOnly))
    {
        //Std
        QTextStream in_std(&infos_std);
        tot=0;
        while(!in_std.atEnd())
        {
            QString line    = in_std.readLine();
            double val      = line.toDouble();
            ref_std_Mean   += val;
            ref_std_Std    += val*val;
            tot++;
        }
        ref_std_Std/=tot;
        ref_std_Mean/=tot;
        ref_std_Std = sqrt(ref_std_Std - (ref_std_Mean*ref_std_Mean));
        infos_std.close();
    }
    else
        qDebug()<<"Pb in reading ref std ";
    if(infos_slope.open(QIODevice::ReadOnly))
    {
        //Slope
        QTextStream in_slope(&infos_slope);

        tot=0;
        while(!in_slope.atEnd())
        {
            QString line    = in_slope.readLine();
            double val      = line.toDouble();
            ref_slope_Mean   += val;
            ref_slope_Std    += val*val;
            tot++;
        }
        ref_slope_Std/=tot;
        ref_slope_Mean/=tot;
        ref_slope_Std = sqrt(ref_slope_Std - (ref_slope_Mean*ref_slope_Mean));
        infos_slope.close();
    }
    else
        qDebug()<<"Pb in reading ref slope ";

    /**************************************/
    /**************************************/
    /****Process similarity calculation****/
    /**************************************/
    /**************************************/

    QVector<double> sim_mean;
    QVector<double> sim_std;
    QVector<double> sim_slope;
    sim_mean.fill(0,vec_size);
    sim_std.fill(0,vec_size);
    sim_slope.fill(0,vec_size);

    for(int s=0;s<vec_size;s++)
    {
        sim_mean[s]     = abs(_M_reg_mean[s] - ref_mean_Mean);
        sim_std[s]      = abs(_M_reg_std[s] - ref_std_Mean);
        sim_slope[s]    = abs(_M_reg_slope[s] - ref_slope_Mean);

//        sim_mean[s]     = (sim_mean[s]>=3*ref_mean_Std) ? 0.0 : 100.0 - (sim_mean[s]/(10*ref_mean_Std))*100;
//        sim_std[s]      = (sim_std[s]>=3*ref_std_Std) ? 0.0 : 100.0 - (sim_std[s]/(10*ref_std_Std))*100;
//        sim_slope[s]    = (sim_slope[s]>=3*ref_slope_Std) ? 0.0 : 100.0 - (sim_slope[s]/(10*ref_slope_Std))*100;

        sim_mean[s]     = (sim_mean[s]>=10*ref_mean_Std) ? 0.0 : 100.0 - (sim_mean[s]/(10*ref_mean_Std))*100;
        sim_std[s]      = (sim_std[s]>=10*ref_std_Std) ? 0.0 : 100.0 - (sim_std[s]/(10*ref_std_Std))*100;
        sim_slope[s]    = (sim_slope[s]>=10*ref_slope_Std) ? 0.0 : 100.0 - (sim_slope[s]/(10*ref_slope_Std))*100;
    }


    /**************************************/
    /**************************************/
    /**********Display results*************/
    /**************************************/
    /**************************************/
    int id = (vec_size==2)? 1 : 0;

    //Similarity
    ui->_mean_result->setText(QString::number((int)(sim_mean[id]))+"%");
    ui->_std_result->setText(QString::number((int)(sim_std[id]))+"%");
    ui->_slope_result->setText(QString::number((int)(sim_slope[id]))+"%");
    ui->_global_result->setText(QString::number((int)((sim_mean[id]+sim_std[id]+sim_slope[id])/3))+"%");

    if(id==1)
    {
        ui->_widget_no_reg->show();
        ui->_result_no_registration->setText(QString::number((int)((sim_mean[0]+sim_std[0]+sim_slope[0])/3))+"%");
    }

    //Ref
    ui->_mean_ref->setText(QString::number(ref_mean_Mean));
    ui->_std_ref->setText(QString::number(ref_std_Mean));
    ui->_slope_ref->setText(QString::number(ref_slope_Mean));

    //measure
    ui->_mean->setText(QString::number(_M_reg_mean[id]));
    ui->_std->setText(QString::number(_M_reg_std[id]));
    ui->_slope->setText(QString::number(_M_reg_slope[id]));
}

void HPLotProfile::DisplayRegistrationResult_HS()
{
    ui->_result->show();


    //Registration results
    int tot=0;
    int vec_size = _M_y_values.size();


    _M_reg_mean.fill(0,vec_size);
    _M_reg_std.fill(0,vec_size);
    _M_reg_slope.fill(0,vec_size);

    /**************************************/
    /**************************************/
    /**********Measure NCC infos***********/
    /**************************************/
    /**************************************/


    // Std and mean calculation
    for(int s=0;s<vec_size;s++)
    {
        QVector<double> x;
        x.clear();
        tot=0;

        for(int i=0;i<_M_y_values[s].size();i++)
        {
            _M_y_values[s][i] = std::isnan(_M_y_values[s][i]) ? 1 : _M_y_values[s][i];
            tot++;
            _M_reg_mean[s]+=_M_y_values[s][i];
            _M_reg_std[s]+=_M_y_values[s][i]*_M_y_values[s][i];
            x.push_back(i+1);
        }
        if(tot==0)
            return;

        _M_reg_std[s]/=tot;
        _M_reg_mean[s]/=tot;
        _M_reg_std[s] = sqrt(_M_reg_std[s] - (_M_reg_mean[s]*_M_reg_mean[s]));


        //Get slope coefficient
        vector<double> _p;
        if(polyfit(x,_M_y_values[s],1,_p)==0)
        {
            qDebug()<<"[Polyfit] error";
        }

        _M_reg_slope[s] = _p[1];
    }

    /**************************************/
    /**************************************/
    /**********Get ref NCC infos***********/
    /**************************************/
    /**************************************/
    double ref_mean_Mean    = 0.0;
    double ref_std_Mean     = 0.0;
    double ref_slope_Mean   = 0.0;
    double ref_mean_Std     = 0.0;
    double ref_std_Std      = 0.0;
    double ref_slope_Std    = 0.0;
    tot=0;

    QFile infos_mean(QString(QCoreApplication::applicationDirPath())+"/../share/files/infos_mean.txt");
    QFile infos_std(QString(QCoreApplication::applicationDirPath())+"/../share/files/infos_std.txt");
    QFile infos_slope(QString(QCoreApplication::applicationDirPath())+"/../share/files/infos_slope.txt");
    if(infos_mean.open(QIODevice::ReadOnly))
    {
        //Mean
        QTextStream in_mean(&infos_mean);
        tot=0;
        while(!in_mean.atEnd())
        {
            QString line    = in_mean.readLine();
            double val      = line.toDouble();
            ref_mean_Mean   += val;
            ref_mean_Std    += val*val;
            tot++;
        }
        ref_mean_Std/=tot;
        ref_mean_Mean/=tot;
        ref_mean_Std = sqrt(ref_mean_Std - (ref_mean_Mean*ref_mean_Mean));
        infos_mean.close();
    }
    else
        qDebug()<<"Pb in reading ref mean ";
    if(infos_std.open(QIODevice::ReadOnly))
    {
        //Std
        QTextStream in_std(&infos_std);
        tot=0;
        while(!in_std.atEnd())
        {
            QString line    = in_std.readLine();
            double val      = line.toDouble();
            ref_std_Mean   += val;
            ref_std_Std    += val*val;
            tot++;
        }
        ref_std_Std/=tot;
        ref_std_Mean/=tot;
        ref_std_Std = sqrt(ref_std_Std - (ref_std_Mean*ref_std_Mean));
        infos_std.close();
    }
    else
        qDebug()<<"Pb in reading ref std ";
    if(infos_slope.open(QIODevice::ReadOnly))
    {
        //Slope
        QTextStream in_slope(&infos_slope);

        tot=0;
        while(!in_slope.atEnd())
        {
            QString line    = in_slope.readLine();
            double val      = line.toDouble();
            ref_slope_Mean   += val;
            ref_slope_Std    += val*val;
            tot++;
        }
        ref_slope_Std/=tot;
        ref_slope_Mean/=tot;
        ref_slope_Std = sqrt(ref_slope_Std - (ref_slope_Mean*ref_slope_Mean));
        infos_slope.close();
    }
    else
        qDebug()<<"Pb in reading ref slope ";

    /**************************************/
    /**************************************/
    /****Process similarity calculation****/
    /**************************************/
    /**************************************/

    QVector<double> sim_mean;
    QVector<double> sim_std;
    QVector<double> sim_slope;
    sim_mean.fill(0,vec_size);
    sim_std.fill(0,vec_size);
    sim_slope.fill(0,vec_size);

    for(int s=0;s<vec_size;s++)
    {
        sim_mean[s]     = abs(_M_reg_mean[s] - ref_mean_Mean);
        sim_std[s]      = abs(_M_reg_std[s] - ref_std_Mean);
        sim_slope[s]    = abs(_M_reg_slope[s] - ref_slope_Mean);

//        sim_mean[s]     = (sim_mean[s]>=3*ref_mean_Std) ? 0.0 : 100.0 - (sim_mean[s]/(10*ref_mean_Std))*100;
//        sim_std[s]      = (sim_std[s]>=3*ref_std_Std) ? 0.0 : 100.0 - (sim_std[s]/(10*ref_std_Std))*100;
//        sim_slope[s]    = (sim_slope[s]>=3*ref_slope_Std) ? 0.0 : 100.0 - (sim_slope[s]/(10*ref_slope_Std))*100;

        sim_mean[s]     = (sim_mean[s]>=10*ref_mean_Std) ? 0.0 : 100.0 - (sim_mean[s]/(10*ref_mean_Std))*100;
        sim_std[s]      = (sim_std[s]>=10*ref_std_Std) ? 0.0 : 100.0 - (sim_std[s]/(10*ref_std_Std))*100;
        sim_slope[s]    = (sim_slope[s]>=10*ref_slope_Std) ? 0.0 : 100.0 - (sim_slope[s]/(10*ref_slope_Std))*100;
    }


    /**************************************/
    /**************************************/
    /**********Display results*************/
    /**************************************/
    /**************************************/

    //Similarity
    ui->_mean_result->setText(QString::number((int)(sim_mean[2]))+"%");
    ui->_std_result->setText(QString::number((int)(sim_std[2]))+"%");
    ui->_slope_result->setText(QString::number((int)(sim_slope[2]))+"%");

    int resg1 = (int)((sim_mean[2]+sim_std[2]+sim_slope[2])/3);
    int resg2 = (int)((sim_mean[3]+sim_std[3]+sim_slope[3])/3);

    QString res = (resg1<resg2) ? QString::number(resg1)+" - "+QString::number(resg2)+"%" : QString::number(resg2)+" - "+QString::number(resg1)+"%";
    ui->_global_result->setText(res);

    resg1   = (int)((sim_mean[0]+sim_std[0]+sim_slope[0])/3);
    resg2   = (int)((sim_mean[1]+sim_std[1]+sim_slope[1])/3);
    res     = (resg1<resg2) ? QString::number(resg1)+" - "+QString::number(resg2)+"%" : QString::number(resg2)+" - "+QString::number(resg1)+"%";
    ui->_result_no_registration->setText(res);


    //Ref
    ui->_mean_ref->setText(QString::number(ref_mean_Mean));
    ui->_std_ref->setText(QString::number(ref_std_Mean));
    ui->_slope_ref->setText(QString::number(ref_slope_Mean));

    //measure
    ui->_mean->setText(QString::number(_M_reg_mean[2]));
    ui->_std->setText(QString::number(_M_reg_std[2]));
    ui->_slope->setText(QString::number(_M_reg_slope[2]));
}

