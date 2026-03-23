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

//    //Statistical significance level
//    ui->_statistical_level->setMaximum(50);
//    ui->_statistical_level->setMinimum(5);
//    ui->_statistical_level->setSingleStep(5);
//    ui->_statistical_level->setValue(5);
//    connect(ui->_statistical_level,SIGNAL(valueChanged(int)),this,SIGNAL(newStatisticalLevel(int)));



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
     ui->_label_setting->setText("Area width (px)");

     ui->_stats_type->addItem("SPM");
     ui->_stats_type->addItem("Mean concentration changes");
     ui->_stats_type->addItem("Hemodynamic response correlation");
     ui->_stats_type->addItem("No analysis");
     ui->_stats_type->setCurrentIndex(Process_Mean_Delta_C);
     connect(ui->_stats_type,SIGNAL(currentIndexChanged(int)),this,SLOT(onnewStatType(int)));



     /************************************************************************
      ************************************************************************
      ********************Resting state analysis******************************
      ************************************************************************/
     ui->_Resting_state->hide();


     //Resting state method
    ui->_resting_state_method->addItem("Seed based method");
//    ui->_resting_state_method->addItem("ICA based method");
//    ui->_resting_state_method->addItem("Low frequency power analysis");
//    ui->_resting_state_method->addItem("K-means based method");
    ui->_resting_state_method->addItem("Seed based SPM");

    ui->_resting_state_method->setCurrentIndex(0);
    connect(ui->_resting_state_method,SIGNAL(currentIndexChanged(int)),this,SIGNAL(newRestingStateMethod(int)));



////     //process analysis
////     connect(ui->_process_resting_state,SIGNAL(pressed()),this,SIGNAL(requestRestingStateProcessing()));

    //See init
     connect(ui->_init_seed,SIGNAL(pressed()),this,SIGNAL(requestSeedInit()));

     //Seeds radius
     ui->_seed_radius->setIntegerMode();
     ui->_seed_radius->setRange(0,10,"px");
     ui->_seed_radius->setValue(2);
     connect(ui->_seed_radius,SIGNAL(valueEdited(double)),this,SIGNAL(newSeedRadius(double)));
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

//Set HMI mode (user, guru)
void HStatistics::onNew_Guru_Mode(bool v)
{

    if(v)
    {
        ui->_widget_stat->show();
//        ui->_statistical_level->show();
    }
    else
    {
        //Hide Statistical apriori
        ui->_widget_stat->hide();
//        ui->_statistical_level->hide();
    }
}



void HStatistics::enableActivatedAreasDefinition(bool v)
{
    //Task based analysis
    if(v && _M_analysis_choice == 0)
        ui->_stats->setEnabled(v);

    //Resting state
    if(v && _M_analysis_choice == 1)
    {
        ui->_stats->hide();
        ui->_Resting_state->show();
    }
}


////Resting state method
//void HStatistics::onRestingStateMethodChanged(int v)
//{
//    switch (v)
//    {
//    case 0:
//        ui->_seed_based_method->show();
//        ui->_ICA_based_method->hide();
//        ui->_resting_state_frequency_power->hide();
//        ui->_resting_state_clustering->hide();
//        //Enable seed option
//        ui->_grid_seeds->setEnabled(true);
//        ui->_seed_option->setEnabled(true);
//        break;
//    case 1:
//        ui->_seed_based_method->hide();
//        ui->_ICA_based_method->show();
//        ui->_resting_state_frequency_power->hide();
//        ui->_resting_state_clustering->hide();
//        break;
//    case 2:
//        ui->_seed_based_method->hide();
//        ui->_ICA_based_method->hide();
//        ui->_resting_state_frequency_power->show();
//        ui->_resting_state_clustering->hide();
//        break;
//    case 3:
//        ui->_seed_based_method->hide();
//        ui->_ICA_based_method->hide();
//        ui->_resting_state_frequency_power->hide();
//        ui->_resting_state_clustering->show();
//        break;
//    case 4:
//        ui->_seed_based_method->show();
//        ui->_ICA_based_method->hide();
//        ui->_resting_state_frequency_power->hide();
//        ui->_resting_state_clustering->hide();
//        //Disable seed option
//        ui->_grid_seeds->setEnabled(false);
//        ui->_seed_option->setEnabled(false);
//        break;
//    default:
//        break;
//    }

//    emit newRestingStateMethod(v);
//}



//void HStatistics::onnewNbofIndependantSources_ICA(double v)
//{
//    emit newNbofIndependantSources_ICA(v);

//    ui->_ICA_source_of_interest->setRange(0,v-1);
//    if(ui->_ICA_source_of_interest->value()>=v)
//    {
//        ui->_ICA_source_of_interest->setValue(v-1);
//        emit newICASourceofInterest(v-1);
//    }
//}




////seed conexion
//void HStatistics::onrequestSeedConnexion(bool v)
//{
//    emit requestSeedConnexion(v);

//    //seed extraction
//    if(v)
//        ui->_extract_seeds->show();
//    else
//        ui->_extract_seeds->hide();
//}



/** Set analysis choice (from acquisition) */
void HStatistics::AnalysisChoice(int v)
{
    _M_analysis_choice = v;
    qDebug()<<"HStatistics::AnalysisChoice "<<v;
    //0: task based
    if(v==0)
    {
        ui->_Resting_state->hide();
        emit enableRestingState(false);
    }

    //1: resting state
    if(v == 1)
    {
        ui->_Resting_state->show();
        ui->_stats_type->setItemData(0, 0, Qt::UserRole - 1); //SPM
        ui->_stats_type->setItemData(1, 0, Qt::UserRole - 1); //Pixel-wise T-test on Delta C
        ui->_stats_type->setItemData(2, 0, Qt::UserRole - 1); //Area T-tests
        ui->_stats_type->setItemData(4, 0, Qt::UserRole - 1); //Hemodynamic response correlation
        ui->_stats_type->setItemData(6, 0, Qt::UserRole - 1); //Phase difference analysis
        ui->_stats_type->setItemData(7, 0, Qt::UserRole - 1); //Phase correlation analysis

        emit enableRestingState(true);
    }

    //2 impulsion
    if(v == 2)
    {
        ui->_Resting_state->hide();
        emit enableRestingState(false);
        //TODO
    }
}
