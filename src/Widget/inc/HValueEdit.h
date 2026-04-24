/**
 * @file HValueEdit.h
 *
 * @brief Class that aims to display a keypad when clicking on a numeric edit field.
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef HVALUEEDIT_H
#define HVALUEEDIT_H

#include <QLineEdit>

#include "HKeypad.h"
#include <QDebug>

class HValueEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit HValueEdit(QWidget *parent = 0);
    virtual ~HValueEdit();
    void setRange(double min,double max,QString unite=QString());
    void setValue(double );
    double value();
    void setIntegerMode(bool integerMode=true)  {m_integer_mode=integerMode;}


signals:
    void valueEdited(double);

private slots :
    void onEditingFinished();



public slots:



private :
    double m_current_value;
    bool m_integer_mode;
    double m_min;
    double m_max;
    QString m_unite;

};

#endif // HVALUEEDIT_H
