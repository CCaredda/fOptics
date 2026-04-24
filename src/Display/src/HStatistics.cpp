#include "HStatistics.h"
#include "ui_HStatistics.h"

HStatistics::HStatistics(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HStatistics)
{
    ui->setupUi(this);

    //-1: no analysis
    //0 : task-based
    //1: resting state
    //2: Impulsion
    _M_analysis_choice = -1;




    /************************************************************************
     ************************************************************************
     ********************Cortical area definition*****************************
     ************************************************************************/

     //Stat zone setting
     ui->_setting->setRange(3,100,"px");
     ui->_setting->setValue(7);
     ui->_setting->setIntegerMode();
     connect(ui->_setting,SIGNAL(valueEdited(double)),this,SIGNAL(newSettingStatisticZone(double)));


     //statistic type
     ui->_widget_stat->hide();
     ui->_label_setting->setText("Area width (px)");

     ui->_stats_type->addItem("SPM (Random Field theory)");
     ui->_stats_type->addItem("SPM (auto. threshold)");
     ui->_stats_type->addItem("Mean concentration changes");
     ui->_stats_type->addItem("Hemodynamic response correlation");

     ui->_stats_type->setCurrentIndex(Process_Mean_Delta_C);
     connect(ui->_stats_type,SIGNAL(currentIndexChanged(int)),this,SLOT(onnewStatType(int)));
}

HStatistics::~HStatistics()
{
    delete ui;
}




//on new stat type
void HStatistics::onnewStatType(int v)
{
    emit newStatType(v);

}




void HStatistics::enableActivatedAreasDefinition(bool v)
{
    //Task based analysis
    if(v && _M_analysis_choice == 0)
        ui->_stats->setEnabled(v);
}


/** Set analysis choice (from acquisition) */
void HStatistics::AnalysisChoice(int v)
{
    _M_analysis_choice = v;
    qDebug()<<"HStatistics::AnalysisChoice "<<v;
}
