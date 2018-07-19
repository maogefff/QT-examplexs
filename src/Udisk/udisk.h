#ifndef UDISK_H
#define UDISK_H

#include <QWidget>
#include <QProcess>
#include <QFileSystemModel>
#include <QThread>
#include <sys/time.h>
#include "../SD/mountthread.h"

namespace Ui {
class Udisk;
class MYThread;
}

class MYThread : public QThread
{
    Q_OBJECT
public:
    explicit MYThread(int type, QObject *parent = 0);

    static const int SIGNAL_RUNSTART        = 1;
    static const int SIGNAL_RUNEND          = 2;
    static const int SIGNAL_CLEANCACHESSTART= 3;
    static const int SIGNAL_CLEANCACHESEND  = 4;
    static const int SIGNAL_WRITEONE        = 5;
    static const int SIGNAL_READONE         = 6;
    static const int SIGNAL_WRITEMULTI      = 7;
    static const int SIGNAL_READMULTI       = 8;
    static const int SIGNAL_RMFILE          = 9;

//    static const int TYPE_CREATFILE   = 1;
    static const int TYPE_REMOVEFILE  = 2;
    static const int TYPE_WRITEONE    = 3;
    static const int TYPE_READONE     = 4;
    static const int TYPE_WRITEMULTI  = 5;
    static const int TYPE_READMULTI   = 6;

    static const int SLEEP_DELAY_MS = 500;

    void setFileName(QStringList name);
    QStringList fileName;
signals:
    void sendSingal(int mesg);
    void multiWriteFinished(int num, int size, QStringList fileName);
    void multiReadFinished();
private:
//    QProcess *myprocess;
    void run();
    int type;

};

class Udisk : public QWidget
{
    Q_OBJECT

public:
    explicit Udisk(QWidget *parent = 0);
    ~Udisk();

protected:
    void moveEvent(QMoveEvent *);
    void resizeEvent(QResizeEvent *);
    void closeEvent(QCloseEvent *);
    void refreshUi(bool mesg);
    int mountFlag;

private slots:
    void on_detectButton_clicked();
    void on_umountButton_clicked();
    void on_writeButton_clicked();    //
    void on_readButton_clicked();
    void on_cleanData_clicked();
    void on_comboBox_type_currentIndexChanged(const QString &arg1);
    void on_comboBox_count_currentIndexChanged(int index);
    void on_radioButton_onefiletest_clicked();
    void on_radioButton_multifiletest_clicked();
    void on_comboBox_currentIndexChanged(int index);
    void on_pushButton_writefile_clicked();
    void on_pushButton_removefile_clicked();
    void on_pushButton_resetTimer_clicked();
    void on_pushButton_readMulti_clicked();

    /**
     * 处理MountThread线程发送过来的preMount()和postMount()信号
     */
    void dealwithUi(int mesg);
    void myThreadWithUi(int mesg);
    void myThreadmultifinishedUi(int num, int size, QStringList list);   //
    void myThreadMultiReadFinishedUi();
    void TimeSlot();
    void result();
    void finishedProcess();

private:
    QFileSystemModel model;
    Ui::Udisk *ui;
    QString ExNode;
    int count;
//    int testTimes;
//    int testTimesTmp;
    int testMethod;  //0: one  1: multi

    //thread
    QProcess *myprocess;        //shell thread
    MountThread *mountThread;    // 挂载线程
    MYThread *myThread;

    //timer
    QTimer *msTimer;
    int hourTemp;           //Hour
    int minuteTemp;         //Minute
    int secondTemp;         //Second
    int countTemp;

    void initView();
    void initTimer();
    void resetTimer();
    void displayTimer(QString,QString,QString);

    //
    int totalNum;
    int totalSize;
    QStringList fileName;
};

#endif // UDISK_H
