#ifndef BURNCPU_H
#define BURNCPU_H

#include <QWidget>
#include <QProcess>
#include <QDebug>
#include <QPushButton>
#include <QThread>
#include <QTime>
#include <QFileSystemWatcher>
#include <QDir>
#include "parser.h"
#include "runthread.h"

namespace Ui {
class BurnCpu;
}

class BurnCpu : public QWidget
{
    Q_OBJECT

public:
    explicit BurnCpu(QWidget *parent = 0);
    ~BurnCpu();

private:
    void add_startup_program();
    void add_datafile();
    void delete_onoff_startup();
    void delete_datafile();
signals:
    void controlState(bool);

private slots:
    void on_pushButton_clicked();
    void usageSlot(float iUsage);
    void temperatureSlot(float iTemperture);
    void on_pushButton_removefile_clicked();
    void TimeSlot();
    void on_pushButton_startTimer_clicked();
    void on_pushButton_stopTimer_clicked();

private:
    Ui::BurnCpu *ui;
    Parser * pParser;
    Runthread * pRunthread;
    bool status;
    //timer
    QTimer *msTimer;
    int hourTemp;           //Hour
    int minuteTemp;         //Minute
    int secondTemp;         //Second
    int countTemp;

    void initTimer(int h, int m , int s);
    void displayTimer(QString,QString,QString);
};


#endif // MAINWINDOW_H
