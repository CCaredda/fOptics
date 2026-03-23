#include "HLoadDatas.h"
#include "ui_HLoadDatas.h"
#include <QDebug>

HLoadDatas::HLoadDatas(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HLoadDatas)
{
    ui->setupUi(this);

    //Dir button
    connect(ui->_dir,SIGNAL(pressed()),this,SLOT(onDirclicked()));

    //Text
    ui->_path->setText("Select a data directory");

    _M_data_is_loaded_from_HardDrive = false;

}

HLoadDatas::~HLoadDatas()
{
    delete ui;
}

//Update img path
void HLoadDatas::updateImgPath(QString n)
{
    ui->_path->setText(n);
}

//Dir clicked
void HLoadDatas::onDirclicked()
{
    //Chose a directory for loading data
    QString dir = QFileDialog::getOpenFileName(NULL,"Open video","/home/caredda/Videos","*.avi *.mp4 *.mpg *.MOV *.tif *.png");

    if(dir.size()<4)
    {
        return;
    }
    QString file_type = dir.mid(dir.size()-4,4);
    if(file_type!=".tif" && file_type!=".png" && file_type!=".avi" && file_type!=".mpg" && file_type!=".mp4" && file_type!=".MOV")
    {
        return;
    }

    //Choose a directory for saving results
    QString dir_res = QFileDialog::getExistingDirectory(this, tr("Chose directory for saving results"),
                                                 "/home",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);

    if(dir_res == "")
        dir_res = "/home";

    dir_res += "/results/";
    QDir().mkdir(dir_res);



    //Update IHM
    QFileInfo fi(dir);
    updateImgPath(fi.fileName());


     _M_data_is_loaded_from_HardDrive = true;


    emit newVideoPath(dir);
    emit newResultDirectory(dir_res);

}

//Write indication message
void HLoadDatas::onNewMessage(QString v)
{
    ui->_message->setText(v);
}


