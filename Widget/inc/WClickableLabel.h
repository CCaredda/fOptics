/**
 * @file WClickableLabel.h
 *
 * @brief Class that aims to display a keypad when clicking on a label (text displayed on the graphic interface).
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef WCLICKABLELABEL_H
#define WCLICKABLELABEL_H

#include "HKeypad.h"
#include <QLabel>
#include <QMouseEvent>

class WClickableLabel : public QLabel
{
Q_OBJECT
public:
    explicit WClickableLabel(QWidget* parent=0 );
    ~WClickableLabel();
    void setRange(double min,double max,QString unite);
    void setValue(double);
    double getValue();
    void setIntegerMode(bool v=true);
    void setDisplayTxtUnit(bool v)  {m_display_unit=v;}
    void setLabel(QString v)        {m_label=v;}

private slots:
    void onValueChanged(double v);
signals:
    void newLabelValue(double);

protected:
    void mousePressEvent(QMouseEvent*);
    void closeEvent(QCloseEvent *);

private:
    HKeypad     m_clavier;
    double      m_current_value;
    QString     m_unit;
    double      m_min;
    double      m_max;
    bool        m_display_unit;
    QString     m_label;

};


#endif // WCLICKABLELABEL_H
