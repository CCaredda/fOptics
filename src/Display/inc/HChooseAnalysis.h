#ifndef HCHOOSEANALYSIS_H
#define HCHOOSEANALYSIS_H

#include <QWidget>
#include <QDebug>

namespace Ui {
class HChooseAnalysis;
}

class HChooseAnalysis : public QWidget
{
    Q_OBJECT

public:
    explicit HChooseAnalysis(QWidget *parent = nullptr);
    ~HChooseAnalysis();

    void setAnalysis(QVector<int>);

signals:
    //-1: no analysis
    //0 : task-based
    //1: resting state
    //2: Impulsion
    void enableAnalysis(int);

protected:
    void closeEvent (QCloseEvent *event);

private slots:
    void analysisChoiceChanged(int);
    void onAnalyseChosen();


private:
    Ui::HChooseAnalysis *ui;
    bool _M_exit_application;

    QVector<int> _M_analyses;
    int _M_analysis_choice;

};

#endif // HCHOOSEANALYSIS_H
