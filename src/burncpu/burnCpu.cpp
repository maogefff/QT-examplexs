#include "burnCpu.h"
#include "ui_burnCpu.h"
#include <QDebug>
#include <QFile>
#include <QTimer>
#include <QMessageBox>
#include <unistd.h>
#include <QFileInfo>

#define BURNCPU_DATAFILE      "/etc/xdg/burncpu"
#define BURNCPU_STARTUP_FILE  "/etc/xdg/autostart/burncpu.desktop"

BurnCpu::BurnCpu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BurnCpu)
{
    ui->setupUi(this);

    msTimer = NULL;
    status = false;
    pParser = new Parser();
    connect(pParser, SIGNAL(parsercpuusageSignal(float)), this, SLOT(usageSlot(float)));
    connect(pParser, SIGNAL(parsercputemperatureSignal(float)), this, SLOT(temperatureSlot(float)));

    pParser->changestatus(-1);
    pParser->start();

    pRunthread = new Runthread();
    pRunthread->changestatus(-1);
    pRunthread->start();

    initTimer(0, 0, 0);

    QFile qfile( BURNCPU_DATAFILE );
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
        delete_datafile();
        return;
    }
    QString pline = qfile.readLine();

    QStringList sl = pline.split(" | ");
    if(sl.size()!=2)
    {
        qfile.close();
        system("sync");
        qDebug()<<"Can't open the file!"<<endl;
        ui->pushButton->setText("START");
        delete_onoff_startup();
        delete_datafile();
        return;
    }

    QString count = sl.at(0);
    QString time = sl.at(1);

    qfile.close();
    remove(BURNCPU_DATAFILE);
    sync();
    if(!qfile.open(QIODevice::ReadWrite | QIODevice::Text)) {
        ui->pushButton->setText("START");
        return;
    }
    if(qfile.size()>0)
    {
        qDebug()<< "file creat error" << qfile.size();
    }

    status = true;

    ui->pushButton->setText("STOP");
    int pcount=count.toInt()+1;
    count =QString::number(pcount);

    QStringList timelist = time.split(":");

    QString h = timelist.at(0);
    QString m = timelist.at(1);
    QString s = timelist.at(2);
    bool ok;

    initTimer(h.toInt(&ok , 10), m.toInt(&ok , 10), s.toInt(&ok , 10));
//    msTimer->start(10);
    msTimer->start(200);
    qDebug() << "h="+timelist.at(0)+"m="+timelist.at(1)+"s="+timelist.at(2);
    qDebug() << count;

    qfile.reset();
    qfile.seek(0);

    pline = count+" | "+time;
    qfile.write(pline.toLatin1());

    qfile.close();
    system("sync");

    pParser->changestatus(1);
    pRunthread->changestatus(1);
}

BurnCpu::~BurnCpu()
{
    if(pParser)
    {
        pParser->stop();
        pParser->deleteLater();
    }
    if(pRunthread)
    {
        pRunthread->stop();
        pRunthread->deleteLater();
    }
    delete ui;
}

void BurnCpu::on_pushButton_clicked()
{

    QString state = ui->pushButton->text();
    if (state == "START") {
        qDebug() << "start";
        pParser->changestatus(1);
        pRunthread->changestatus(1);
        ui->pushButton->setText("STOP");
        status = true;
        add_startup_program();
        add_datafile();
//        msTimer->start(10);
        msTimer->start(200);
        system("sync");
    } else if (state == "STOP") {
        qDebug() << "stop";
        pParser->changestatus(-1);
        pRunthread->changestatus(-1);
        ui->pushButton->setText("START");
        status = false;
        delete_onoff_startup();
        msTimer->stop();
        system("sync");
    }
}

void BurnCpu::on_pushButton_removefile_clicked()
{
    QMessageBox::StandardButton button;
    button = QMessageBox::question(this, tr("delete data file"),
    QString(tr("confirm delete file?")),
    QMessageBox::Yes | QMessageBox::No);
    if (button == QMessageBox::No)
    {
        qDebug()<<"NO";
    }
    else if (button == QMessageBox::Yes)
    {
        qDebug()<<"YSE";
        ::remove(BURNCPU_DATAFILE);
        initTimer(0, 0, 0);
        system("sync");
    }
}

void BurnCpu::add_startup_program()
{
    QFile qfile( BURNCPU_STARTUP_FILE );
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

void BurnCpu::add_datafile()
{
    QFileInfo file(BURNCPU_DATAFILE);
    if(file.exists()==true)
    {
        return;
    }
    QFile qfile( BURNCPU_DATAFILE );
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


void BurnCpu::delete_onoff_startup()
{
    ::remove(BURNCPU_STARTUP_FILE);
    system("sync");
}

void BurnCpu::delete_datafile()
{
    ::remove(BURNCPU_DATAFILE);
    system("sync");
}

void BurnCpu::usageSlot(float iUsage)
{
    QString tempString = QString::number(iUsage,'g',4);
    tempString += "%";

    ui->cpuusagevalue->setText(tempString);
}

void BurnCpu::temperatureSlot(float iTemperature)
{
    QString tempString = QString::number(iTemperature,'g',4);
    tempString += "°C";

    ui->cputempvalue->setText(tempString);
}

//timer
void BurnCpu::initTimer(int h, int m , int s)
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

void BurnCpu::TimeSlot()
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

        if(status==true)
        {
            //把整数转换成字符串
            QString hourstr = QString::number(hourTemp);
            QString minutestr = QString::number(minuteTemp);
            QString secondstr = QString::number(secondTemp);

            QFile qfile( BURNCPU_DATAFILE );
            qfile.open(QIODevice::ReadWrite | QIODevice::Text);
            QString pline = qfile.readLine();
            QStringList sl = pline.split(" | ");

            QString count = sl.at(0);

            qfile.reset();
            qfile.seek(0);

            pline = count+" | "+hourstr+":"+minutestr+":"+secondstr;
            qfile.write(pline.toLatin1());

            qfile.close();
            system("sync");

            //QFile file( BURNCPU_DATAFILE );
            qfile.open(QIODevice::ReadOnly | QIODevice::Text);
            QString pall = qfile.readAll();
            qfile.close();
            ui->textBrowser->setText(pall);
        }

    }
    //把整数转换成字符串
    QString hourstr = QString::number(hourTemp);
    QString minutestr = QString::number(minuteTemp);
    QString secondstr = QString::number(secondTemp);
    displayTimer(hourstr,minutestr,secondstr);
}

void BurnCpu::displayTimer(QString hour, QString minute, QString second)
{
    ui->lineEdit_timerH->setText(hour);
    ui->lineEdit_timerM->setText(minute);
    ui->lineEdit_timerS->setText(second);
}



void BurnCpu::on_pushButton_startTimer_clicked()
{
    status = false;
    msTimer->start(200);
}

void BurnCpu::on_pushButton_stopTimer_clicked()
{
    status = false;
    msTimer->stop();
}
