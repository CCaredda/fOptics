#include "HKeypad.h"
#include "ui_HKeypad.h"


#define HKeypad_KEY_H_SIZE 60
#define HKeypad_KEY_V_SIZE 60

HKeypad::HKeypad(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HKeypad)
{

    ui->setupUi(this);

    m_selected=false;
    m_decimalSeparator=QChar(',' );
    m_min=-9999999999;
    m_max=9999999999;
    m_isIntegerMode=false;

    disableRangeDisplay();
    QGridLayout* m_layout=dynamic_cast<QGridLayout*>(ui->_keys->layout());
    m_layout->setVerticalSpacing(0);
    m_layout->setHorizontalSpacing(0);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
    setModal(true);
    setMaximumSize(264,324);
    ui->_1->setMinimumSize(HKeypad_KEY_H_SIZE,HKeypad_KEY_V_SIZE);
    ui->_2->setMinimumSize(HKeypad_KEY_H_SIZE,HKeypad_KEY_V_SIZE);
    ui->_3->setMinimumSize(HKeypad_KEY_H_SIZE,HKeypad_KEY_V_SIZE);
    ui->_4->setMinimumSize(HKeypad_KEY_H_SIZE,HKeypad_KEY_V_SIZE);
    ui->_5->setMinimumSize(HKeypad_KEY_H_SIZE,HKeypad_KEY_V_SIZE);
    ui->_6->setMinimumSize(HKeypad_KEY_H_SIZE,HKeypad_KEY_V_SIZE);
    ui->_7->setMinimumSize(HKeypad_KEY_H_SIZE,HKeypad_KEY_V_SIZE);
    ui->_8->setMinimumSize(HKeypad_KEY_H_SIZE,HKeypad_KEY_V_SIZE);
    ui->_9->setMinimumSize(HKeypad_KEY_H_SIZE,HKeypad_KEY_V_SIZE);
    ui->_0->setMinimumHeight(HKeypad_KEY_V_SIZE);
    ui->_clear1->setMinimumSize(HKeypad_KEY_H_SIZE,HKeypad_KEY_V_SIZE);
    ui->_clear->setMinimumSize(HKeypad_KEY_H_SIZE,HKeypad_KEY_V_SIZE);
    ui->_Point->setMinimumSize(HKeypad_KEY_H_SIZE,HKeypad_KEY_V_SIZE);
    ui->_PlusOuMoins->setMinimumSize(HKeypad_KEY_H_SIZE,HKeypad_KEY_V_SIZE);
    ui->_Entrer->setMinimumSize(HKeypad_KEY_H_SIZE,HKeypad_KEY_V_SIZE);
    ui->_Entrer->setDefault(true);

    connect(ui->_0,SIGNAL(clicked()),this,SLOT(on0()));
    connect(ui->_1,SIGNAL(clicked()),this,SLOT(on1()));
    connect(ui->_2,SIGNAL(clicked()),this,SLOT(on2()));
    connect(ui->_3,SIGNAL(clicked()),this,SLOT(on3()));
    connect(ui->_4,SIGNAL(clicked()),this,SLOT(on4()));
    connect(ui->_5,SIGNAL(clicked()),this,SLOT(on5()));
    connect(ui->_6,SIGNAL(clicked()),this,SLOT(on6()));
    connect(ui->_7,SIGNAL(clicked()),this,SLOT(on7()));
    connect(ui->_8,SIGNAL(clicked()),this,SLOT(on8()));
    connect(ui->_9,SIGNAL(clicked()),this,SLOT(on9()));
    connect(ui->_Point,SIGNAL(clicked()),this,SLOT(onPoint()));
    connect(ui->_Entrer,SIGNAL(clicked()),this,SLOT(onEnter()));
    connect(ui->_clear,SIGNAL(clicked()),this,SLOT(onClear()));
    connect(ui->_PlusOuMoins,SIGNAL(clicked()),this,SLOT(onPlusOuMoins()));
    connect(ui->_clear1,SIGNAL(clicked()),this,SLOT(onClear1()));
}
HKeypad::~HKeypad()
{
    delete ui;
}

double HKeypad::value()
{
    return _valFromStr(ui->_afficheur->text());
}

double HKeypad::_valFromStr(QString str)
{
    if(str.isEmpty())
        return 0;

    bool isNegatif=false;
    str.replace(m_decimalSeparator, QLocale::c().decimalPoint());
    if(str.at(0)==QChar('-'))
    {
        isNegatif=true;
        str.remove(0,1);
    }
    double v=str.toDouble();
    if(isNegatif)
        return -v;
    else
        return v;
}

void HKeypad::addDigit(int digit)
{
    QString str;
    if(!m_selected)
        str=ui->_afficheur->text();

    if(str==QString("0"))
        str.clear();
    QString str2;
    str2.setNum(digit);
    str.append(str2);

    double newVal=_valFromStr(str);
    if(/*newVal>=m_min &&*/ newVal<=m_max)
    {
        if(m_selected)
        {
            ui->_afficheur->clear();
            m_selected=false;
        }

        ui->_afficheur->setText(str);

    }
}
void HKeypad::on0()
{
    addDigit(0);
}
void HKeypad::on1()
{
    addDigit(1);
}
void HKeypad::on2()
{
    addDigit(2);

}
void HKeypad::on3()
{
    addDigit(3);
}
void HKeypad::on4()
{
    addDigit(4);
}
void HKeypad::on5()
{
    addDigit(5);
}
void HKeypad::on6()
{
    addDigit(6);
}
void HKeypad::on7()
{
    addDigit(7);
}
void HKeypad::on8()
{
    addDigit(8);
}
void HKeypad::on9()
{
    addDigit(9);
}
void HKeypad::onPoint()
{
    if(m_selected)
    {
        ui->_afficheur->clear();
        m_selected=false;
    }
    QString str=ui->_afficheur->text();
    if(str.isEmpty())
        str.append(QChar('0'));

    if(str.contains(m_decimalSeparator) || m_isIntegerMode)
        return ;
    else
    {
        str.append(m_decimalSeparator);
        ui->_afficheur->setText(str);
    }
}
void HKeypad::onEnter()
{
    if(value()<=m_max && value()>=m_min)
        emit  valueEntred(value() );
    else
        emit valueEntred(m_old_value);
}
void HKeypad::onClear()
{
    ui->_afficheur->clear();
}
void HKeypad::onClear1()
{
    QString str=ui->_afficheur->text();
    str.remove(str.size()-1,1);
    ui->_afficheur->setText(str);
    m_selected=false;
}
void HKeypad::onPlusOuMoins()
{
    QString str=ui->_afficheur->text();
    if(!str.isEmpty())
    {
        if(str.at(0)==QChar('-'))
        {
            QString str2(str);
            str2.remove(0,1);
            if(_valFromStr(str2)<=m_max && _valFromStr(str2)>=m_min)
            {
                str.remove(0,1);
            }
        }
        else
        {
            if((-1*(_valFromStr(str)))<=m_max && (-1*(_valFromStr(str)))>=m_min)
            {
                str=QString(QChar('-')).append(str);
            }
        }
    }
    else
    {
        if(m_min<0)
            str=QString(QChar('-')).append(str);
    }
    ui->_afficheur->setText(str);
}
QChar  HKeypad::decimalSeparator()
{
    return m_decimalSeparator;
}
void HKeypad::setDecimalSeparator(QChar c)
{
    m_decimalSeparator=c;
}
void HKeypad::setValueRange(double min,double max , QString unite)
{
    m_min=min;
    m_max=max;
    m_unite=unite;
    QString str;
    str.setNum(min);
    str.append(QChar(' ')) ;
    str.append(m_unite);
    str=QString(tr("Min = ")).append(str);
    ui->_min->setText(str);
    str.clear();
    str.setNum(max);
    str.append(QChar(' ')) ;
    str.append(m_unite);
    str=QString(tr("Max = ")).append(str);
    ui->_max->setText(str);
}
void HKeypad::disableRangeDisplay()
{
    ui->_min->clear();
    ui->_max->clear();
}
void HKeypad::setValue(double v)
{
    m_old_value=v;
    QString str;
    str.setNum(v);
    str.replace(QChar('.'),m_decimalSeparator);
    ui->_afficheur->setText(str);
    ui->_afficheur->selectAll();
    m_selected=true;
}
void HKeypad::setValue(QString v)
{
    m_old_value=_valFromStr(v);
    setValue( _valFromStr(v));
}





void HKeypad::setIntegerMode(bool integerMode)
{
    m_isIntegerMode=integerMode;
}
