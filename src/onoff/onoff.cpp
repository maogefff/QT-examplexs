#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <QMessageBox>
#include <QTimer>
#include "onoff.h"
#include "ui_onoff.h"
#include <QDebug>

#define ONOFF_DATAFILE      "/etc/xdg/onoff"
#define ONOFF_STARTUP_FILE  "/etc/xdg/autostart/onoff.desktop"
#define ONOFF_STATEFILE      "/etc/xdg/onoffstate"

Onoff::Onoff(QWidget *parent) :
	QWidget(parent),
    ui(new Ui::Onoff)
{
    ui->setupUi(this);

    msTimer = NULL;
    initTimer(0, 0, 0);

    QFile qfile( ONOFF_DATAFILE );
    if(!qfile.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug()<<"Can't open the file!"<<endl;
        ui->pushButton->setText("START");
        return;
    }
    if(qfile.size()<=0)
    {
        qfile.close();
        system("sync");
        qDebug()<<"Can't open the file!"<<endl;
        ui->pushButton->setText("START");
        delete_onoff_startup();
        return;
    }

    QString pline = qfile.readLine();
    qfile.close();
    remove(ONOFF_DATAFILE);
    sync();
    if(!qfile.open(QIODevice::ReadWrite | QIODevice::Text)) {
        ui->pushButton->setText("START");
        return;
    }
    if(qfile.size()>0)
    {
        qDebug()<< "file creat error" << qfile.size();
    }

    ui->pushButton->setText("STOP");
    int pcount=pline.toInt()+1;
    pline =QString::number(pcount);

    ui->runtimescount->setText(pline);
    qDebug() << pline;

    qfile.reset();
    qfile.seek(0);
    qfile.write(pline.toLatin1());
    qfile.close();
    system("sync");

    QFile qfile2( ONOFF_STATEFILE );
    if(qfile2.exists()==false)
    {
        add_timer_file();
    }
    else
    {
        qfile2.open(QIODevice::ReadWrite | QIODevice::Text);
        qfile2.seek(0);
        QString line = qfile2.readLine();

        QStringList sl = line.split(" | ");
        if(sl.size()!=2)
        {
            add_timer_file();
        }

        bool ok;
        type = sl.at(0);

        if(type=="cold start")
        {
            ui->lineEdit_time->setEnabled(false);
        }
        else if(type=="hot start")
        {
            ui->lineEdit_time->setEnabled(true);
        }

        timer_s = sl.at(1).toInt(&ok,10);
        ui->comboBox_type->setCurrentText(type);
        qDebug()<<type;

        ui->lineEdit_time->setText(QString::number(timer_s,10));
        qDebug()<<timer_s;
    }
    qfile2.close();


    msTimer->start(200);
}

Onoff::~Onoff()
{
    delete msTimer;
	delete ui;
}

void Onoff::add_startup_program()
{
    QFile qfile( ONOFF_STARTUP_FILE );
    if(!qfile.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug()<<"Can't open the file!"<<endl;
        ui->pushButton->setText("START");
        return;
    }
    qfile.reset();
    qfile.seek(0);
    qfile.write("[Desktop Entry]\n");
    qfile.write("Exec=/usr/share/utils\n");
    qfile.close();
    system("sync");
}

void Onoff::add_datafile()
{
    QFile qfile( ONOFF_DATAFILE );
    if(!qfile.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug()<<"Can't open the file!"<<endl;
        ui->pushButton->setText("START");
        return;
    }
    qfile.reset();
    qfile.seek(0);
    qfile.write("0");

    qfile.close();
    system("sync");
}

void Onoff::delete_onoff_startup()
{
    ::remove(ONOFF_DATAFILE);
    ::remove(ONOFF_STARTUP_FILE);
    system("sync");
}

bool Onoff::add_timer_file()
{
    type = ui->comboBox_type->currentText();
    qDebug()<<type;

    bool ok;
    timer_s = ui->lineEdit_time->text().toInt(&ok, 10);
    qDebug()<<timer_s;

    if(timer_s==0)
    {
        QMessageBox::information(this, "Error", "input ture number.");
        return false;
    }

    remove(ONOFF_STATEFILE);

    QFile qfile( ONOFF_STATEFILE );

    qfile.open(QIODevice::ReadWrite | QIODevice::Text);
    qfile.seek(0);
    QString pline = type + " | " + QString::number(timer_s,10);
    qfile.write(pline.toLatin1());
    qfile.close();
    return true;
}

void Onoff::on_pushButton_clicked()
{

    QString state = ui->pushButton->text();
    if (state == "START") {
        ui->pushButton->setText("STOP");
        qDebug() << "start";

        if(add_timer_file()==false)
        {
            return;
        }

        add_startup_program();
        add_datafile();
        ui->runtimescount->setText("0");
        msTimer->start(200);
        system("sync");
    } else if (state == "STOP") {
        ui->pushButton->setText("START");
        qDebug() << "stop";
        delete_onoff_startup();
        ui->runtimescount->setText(" ");
        msTimer->stop();
        remove(ONOFF_STATEFILE);
        initTimer(0, 0, 0);
        system("sync");
    }
}


//timer
void Onoff::initTimer(int h, int m , int s)
{
    if(msTimer!=NULL)
    {
        delete msTimer;
    }
    msTimer= new QTimer(this);  //this说明是当前类对象的定时器
    connect(msTimer,SIGNAL(timeout()),this,SLOT(TimeSlot()));

    countTemp = 0;
    hourTemp = h;
    minuteTemp = m;
    secondTemp = s;
    QString hourstr = QString::number(hourTemp);
    QString minutestr = QString::number(minuteTemp);
    QString secondstr = QString::number(secondTemp);
    displayTimer(hourstr,minutestr,secondstr);
    msTimer->stop();
}

void Onoff::TimeSlot()
{
    countTemp+=1;
//    if(countTemp==100)
    if(countTemp==5)
    {
        countTemp=0;
        secondTemp+=1;
        if(secondTemp==60)
        {
            secondTemp=0;
            minuteTemp+=1;
            if(minuteTemp==60)
            {
                minuteTemp=0;
                hourTemp+=1;
                if(hourTemp==24)
                {
                    hourTemp=0;
                }
            }
        }
        qDebug()<<"secondTemp+minuteTemp*60+hourTemp*3600="<<secondTemp+minuteTemp*60+hourTemp*3600;
        qDebug()<<"timer_s="<<timer_s;
    }

    if((secondTemp+minuteTemp*60+hourTemp*3600)==timer_s && type=="hot start")
    {
        system("reboot");
    }
    //把整数转换成字符串
    QString hourstr = QString::number(hourTemp);
    QString minutestr = QString::number(minuteTemp);
    QString secondstr = QString::number(secondTemp);
    displayTimer(hourstr,minutestr,secondstr);
}

void Onoff::displayTimer(QString hour, QString minute, QString second)
{
    ui->lineEdit_timerH->setText(hour);
    ui->lineEdit_timerM->setText(minute);
    ui->lineEdit_timerS->setText(second);
}

void Onoff::on_comboBox_type_currentIndexChanged(const QString &arg1)
{
    if(arg1=="cold start")
    {
        ui->lineEdit_time->setEnabled(false);
    }
    else if(arg1=="hot start")
    {
        ui->lineEdit_time->setEnabled(true);
    }
}
