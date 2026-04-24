#include "WClickableLabel.h"

WClickableLabel::WClickableLabel(QWidget* parent)
    : QLabel(parent)
{
    m_label         ="";
    m_unit          ="";
    m_current_value =0;
    m_min           =0;
    m_max           =0;
    m_display_unit  =false;
    m_clavier.close();
    connect(&m_clavier,SIGNAL(valueEntred(double)),this,SLOT(onValueChanged(double)));
}

WClickableLabel::~WClickableLabel()
{
}

void WClickableLabel::setIntegerMode(bool v)
{
    m_clavier.setIntegerMode(v);
}

void WClickableLabel::mousePressEvent(QMouseEvent*)
{
    m_clavier.setValue(m_current_value);
    m_clavier.show();
}

void WClickableLabel::closeEvent(QCloseEvent *)
{
    m_clavier.close();
}

void WClickableLabel::onValueChanged(double v)
{
    m_current_value=v;
    setValue(m_current_value);
    emit newLabelValue(v);
    m_clavier.close();
}

void WClickableLabel::setRange(double min,double max,QString unite)
{
    m_unit=unite;
    m_min=min;
    m_max=max;
    m_clavier.setValueRange(min,max,unite);
    setValue(m_current_value);

}

void WClickableLabel::setValue(double v)
{

    m_current_value=(v>m_max) ? m_max : v;
    m_current_value=(v<m_min) ? m_min : v;

    QString label;
    if(m_display_unit)
        label=QString::number(v)+" "+m_unit;
    else
        label=QString::number(v);

    if(m_label!="")
        label=m_label+label;

    this->setText(label);
}

double WClickableLabel::getValue()
{
    return m_current_value;
}
