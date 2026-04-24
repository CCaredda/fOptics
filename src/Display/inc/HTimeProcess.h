/**
 * @file HTimeProcess.h
 *
 * @brief This class contains the graphical interface to control which activation period has to be used
 * in the computation of quantitative brain maps (maps of concentration changes measured for each pixel and averaged during the
 * selected activation period)
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef HTIMEPROCESS_H
#define HTIMEPROCESS_H

#include <QWidget>
#include <QDebug>


namespace Ui {
class HTimeProcess;
}

class HTimeProcess : public QWidget
{
    Q_OBJECT

public:
    explicit HTimeProcess(QWidget *parent = 0);
    ~HTimeProcess();

    /** Set number of activity periods */
    void setNbActivityPeriod(int);

signals:
    /** Send paradigm info (activation period selected) */
    void newActivationStepsToConsider(int);


private:
    Ui::HTimeProcess *ui;
};

#endif // HTIMEPROCESS_H
