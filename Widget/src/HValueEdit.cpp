#include "HValueEdit.h"

HValueEdit::HValueEdit(QWidget *parent) :
    QLineEdit(parent)
{
    m_integer_mode=false;
    m_min=-9999999999;
    m_max=9999999999;
    m_unite = "";
    setAlignment(Qt::AlignRight);

    connect(this,SIGNAL(editingFinished()),this,SLOT(onEditingFinished()));
}

HValueEdit::~HValueEdit()
{

}


void HValueEdit::onEditingFinished()
{
    QString v = this->text();
    QString old_val = QString::number(m_current_value);




    //Check if the line contains a letter
    bool contain_letter = false;

    for(int i=0;i<v.size();i++)
    {
        QChar c = v.at(i);
        if(c.isLetter())
        {
            contain_letter = true;
            break;
        }
    }

    if(contain_letter)
    {
        this->setText(old_val);
        return;
    }


    //Integer mode
    if(m_integer_mode)
    {
        m_current_value = int(v.toDouble());
        this->setText(QString::number(m_current_value));
    }
    else
        m_current_value = v.toDouble();

    //Control ranges
    if(m_current_value>m_max)
    {
        m_current_value = m_max;
        this->setText(QString::number(m_current_value));
    }
    if(m_current_value<m_min)
    {
        m_current_value = m_min;
        this->setText(QString::number(m_current_value));
    }

    emit valueEdited(m_current_value);

}



void HValueEdit::setRange(double min,double max,QString unite)
{
    m_min=min;
    m_max=max;
    m_unite=unite;
}


void HValueEdit::setValue(double v )
{
    m_current_value=v;
    QString str;
    str.setNum(v);
    setText(str);
}


double HValueEdit::value()
{
    return m_current_value;
}

