#ifndef MEMORY_H
#define MEMORY_H

#include <QWidget>
#include <QFileSystemModel>
#include <QThread>
#include <QProcess>

namespace Ui {
class Memory;
}

class Memory : public QWidget
{
    Q_OBJECT

public:
    explicit Memory(QWidget *parent = 0);
    ~Memory();

protected:
    void moveEvent(QMoveEvent *);
    void resizeEvent(QResizeEvent *);
    void closeEvent(QCloseEvent *);

private slots:
    void result();
    void finished_result();
    void freemem_result();
    void on_start_clicked();
    void on_cleanData_clicked();
    void TimeSlot();

    void on_pushButton_clicked();

    void on_pushButton_mem_clicked();

private:
    int isDigitStr(QString src);
    Ui::Memory *ui;
    QProcess *myprocess;
    QProcess *process_mem;
    QStringList displayList;

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

};

#endif // MAINWINDOW_H
