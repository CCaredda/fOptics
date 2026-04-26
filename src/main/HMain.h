/**
 * @file HMain.h
 *
 * @brief Main class of the software. The ui only contains the HMainDisplay class.
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef HMAIN_H
#define HMAIN_H

#include <QMainWindow>
#include <QFile>
#include <QCloseEvent>

namespace Ui {
class HMain;
}

class HMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit HMain(QWidget *parent = 0);
    ~HMain();

protected:
    virtual void closeEvent(QCloseEvent *);

private:
    Ui::HMain *ui;
};

#endif // HMAIN_H
