/**
 * @file HKeypad.h
 *
 * @brief Class that aims to display a keypad to enter numbers in a calculator widget
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */


#ifndef HKeypad_H
#define HKeypad_H

#include <QWidget>
#include <QDialog>
namespace Ui {
class HKeypad;
}



class HKeypad : public QDialog
{
    Q_OBJECT

public:
    explicit HKeypad(QWidget *parent = 0);
    ~HKeypad();

    QChar decimalSeparator();
    void setDecimalSeparator(QChar c);

    void setIntegerMode(bool integerMode=true);
    void setValueRange(double min,double max , QString unite=QString());
    void disableRangeDisplay();

    double value();


private slots :
    void on0();
    void on1();
    void on2();
    void on3();
    void on4();
    void on5();
    void on6();
    void on7();
    void on8();
    void on9();
    void onPoint();
    void onEnter();
    void onClear();
    void onClear1();
    void onPlusOuMoins();

public slots :
    void setValue(double v);
    void setValue(QString v);
signals:
    void valueEntred(double );
private:

    double      _valFromStr(QString);
    void        addDigit(int digit);
    Ui::HKeypad *ui;
    QChar       m_decimalSeparator;
    double      m_min;
    double      m_max;
    QString     m_unite;
    double      m_old_value;
    bool        m_selected;
    bool        m_isIntegerMode;
};

#endif // HKeypad_H
