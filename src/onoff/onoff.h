#ifndef ONOFF_H
#define ONOFF_H

#include <QWidget>
#include <QFile>

namespace Ui {
class Onoff;
}

class Onoff : public QWidget
{
    Q_OBJECT

public:
    explicit Onoff(QWidget *parent = 0);
    ~Onoff();

private slots:
    void on_pushButton_clicked();
    void TimeSlot();

    void on_comboBox_type_currentIndexChanged(const QString &arg1);

private:
    void add_startup_program();
    void add_datafile();
    void delete_onoff_startup();
    bool add_timer_file();

    Ui::Onoff *ui;
//    int state;  //0:clod start  1 :hot start
    QString type;
    int timer_s;

    //timer
    QTimer *msTimer;
    int hourTemp;           //Hour
    int minuteTemp;         //Minute
    int secondTemp;         //Second
    int countTemp;

    void initTimer(int h, int m , int s);
    void displayTimer(QString,QString,QString);
};

#endif // ONOFF_H
