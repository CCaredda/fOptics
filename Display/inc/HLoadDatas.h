/**
 * @file HLoadDatas.h
 *
 * @brief This class controls the graphical interface to send parameters to the instance of ALoaDatas class
 * for image loading.
 *
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef HLOADDATAS_H
#define HLOADDATAS_H

#include <QWidget>
#include <QFileDialog>

namespace Ui {
class HLoadDatas;
}

class HLoadDatas : public QWidget
{
    Q_OBJECT

public:
    explicit HLoadDatas(QWidget *parent = 0);
    ~HLoadDatas();

    /** Update img path */
    void updateImgPath(QString);

    /** is data loaded from hard drive ? */
    bool isDataLoadedFromHardDrive()    {return _M_data_is_loaded_from_HardDrive;}

signals:
    /** New video path */
    void newVideoPath(QString);

    /** Pre ROI requested */
    void PreROIRequested();
    /** Init Pre ROI */
    void InitROIRequested();
    /** New result directory */
    void newResultDirectory(QString);


public slots:
    /** Write indication message */
    void onNewMessage(QString);

private slots:
    /** On Directory choosed */
    void onDirclicked();


private:
    Ui::HLoadDatas *ui;

    bool _M_data_is_loaded_from_HardDrive;
};

#endif // HLOADDATAS_H
