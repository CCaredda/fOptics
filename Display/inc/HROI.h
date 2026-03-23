/**
 * @file HROI.h
 *
 * @brief This class contains the graphical interface to set up the region of interest of the input image. This could be computed
 * automatically using a neural network or manually by drawing a contour on the input image.
 *
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef HROI_H
#define HROI_H

#include <QWidget>

namespace Ui {
class HROI;
}

class HROI : public QWidget
{
    Q_OBJECT

public:
    explicit HROI(QWidget *parent = 0);
    ~HROI();

    /** Enable or disable buttons */
    void enableButtons(bool v);
    /** HMI in mode Guro or standard */
    void onNew_Guru_Mode(bool v);


private slots:


    /** request ROI zone drawing (Manual drawing or rectangular zone) */
    void requestAnalysisZoneDrawing();

private:
    Ui::HROI *ui;

signals:


    //Pre ROI requested
    void PreROIRequested();
    void InitROIRequested();

    //Analysis zone (0: Drawing, 4: ROI)
    void requestAnalysisZoneDrawing(int);

    //New maximum number of analyzed data
    void maximumNumberDataChanged(double);

};

#endif // HROI_H
