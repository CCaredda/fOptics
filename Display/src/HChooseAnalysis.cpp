#include "HChooseAnalysis.h"
#include "ui_HChooseAnalysis.h"

HChooseAnalysis::HChooseAnalysis(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HChooseAnalysis)
{

    this->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    ui->setupUi(this);
    _M_analysis_choice = -1;
    _M_analyses.clear();

    connect(ui->_analysis,SIGNAL(currentIndexChanged(int)),this,SLOT(analysisChoiceChanged(int)));
    connect(ui->_ok,SIGNAL(pressed()),this,SLOT(onAnalyseChosen()));
}

HChooseAnalysis::~HChooseAnalysis()
{
    delete ui;
}

void HChooseAnalysis::analysisChoiceChanged(int v)
{
    if(_M_analyses.empty())
    {
        _M_analysis_choice = -1;
        return;
    }
    _M_analysis_choice = _M_analyses[v];

    if(_M_analysis_choice == 0)
        qDebug() <<"Task-based analysis";
    if(_M_analysis_choice == 1)
        qDebug() <<"Resting-state analysis";
    if(_M_analysis_choice == 2)
        qDebug() <<"Impulsion analysis";
}


void HChooseAnalysis::setAnalysis(QVector<int> v)
{
    qDebug()<<"[HChooseAnalysis::setAnalysis] "<< v;
    this->show();

    if(v.empty())
    {
        ui->_msg->setText("No analysis. Please update Video_acquisition.txt file");
        _M_analyses.clear();
        _M_analysis_choice = -1;
        return;
    }
    ui->_analysis->clear();
    _M_analyses = v;

    for(int i=0;i<v.size();i++)
    {
        if(v[i] == 0)
            ui->_analysis->addItem("Task-based analysis");
        if(v[i] == 1)
            ui->_analysis->addItem("Resting-state analysis");
        if(v[i] == 2)
            ui->_analysis->addItem("Impulsion analysis");
    }
    if(!_M_analyses.empty())
    {
        ui->_analysis->setCurrentIndex(0);
        _M_analysis_choice = v[0];
    }
}

void HChooseAnalysis::onAnalyseChosen()
{
    if(_M_analysis_choice == -1)
        qDebug()<<"No analysis. Please update Video_acquisition.txt file";
    if(_M_analysis_choice == 0)
        qDebug() <<"Task-based analysis";
    if(_M_analysis_choice == 1)
        qDebug() <<"Resting-state analysis";
    if(_M_analysis_choice == 2)
        qDebug() <<"Impulsion analysis";

    emit enableAnalysis(_M_analysis_choice);

    this->close();
}

void HChooseAnalysis::closeEvent (QCloseEvent *event)
{
    onAnalyseChosen();
}

