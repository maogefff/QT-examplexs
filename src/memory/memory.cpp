#include "memory.h"
#include "ui_memory.h"
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <QFileInfo>
#include <QMessageBox>
#include <QTreeView>
#include <QDebug>
#include <QProcess>
#include <QTimer>

Memory::Memory(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Memory)
{

    ui->setupUi(this);

    initTimer();

    process_mem = new QProcess(this);
    connect(process_mem, SIGNAL(readyReadStandardOutput()),this, SLOT(freemem_result()));
    connect(process_mem, SIGNAL(readyReadStandardError()),this, SLOT(freemem_result()));
    process_mem->start(QString( "free -lh" ));
}

Memory::~Memory()
{
    delete ui;
}

void Memory::result()
{
    int i;
    QString abc = myprocess->readAllStandardOutput();

    if(displayList.size()==0)
    {
        displayList<<"";
    }

    QString lastline = displayList[displayList.size()-1];

    while(abc.indexOf("\b", 0)!=-1)
    {
        i = abc.indexOf("\b", 0);
        if(i == 0)
        {
            abc.remove(0, 1);
            lastline.remove(lastline.size()-1-i, 1);   //
            lastline.append(abc.mid(0,abc.indexOf("\b", 0)==-1?0:abc.indexOf("\b", 0)));
            abc.remove(0, abc.indexOf("\b", 0)==-1?0:abc.indexOf("\b", 0));
        }
        else
        {
            lastline.append(abc.mid(0,abc.indexOf("\b", 0)==-1?0:abc.indexOf("\b", 0)));
            abc.remove(0, abc.indexOf("\b", 0)==-1?0:abc.indexOf("\b", 0));
        }
    }
    lastline.append(abc);
    displayList[displayList.size()-1] = lastline;
}

void Memory::finished_result()
{
    msTimer->stop();
    ui->textBrowser->setText("");
    for(int i=0; i<displayList.size(); i++)
    {
        ui->textBrowser->append(displayList[i]);
        qDebug()<<displayList[i];
    }

    ui->cleanData->setEnabled(true);
    ui->start->setEnabled(true);
    ui->lineEdit_value->setEnabled(true);
}

void Memory::freemem_result()
{
    QString abc = process_mem->readAllStandardOutput();
    ui->textBrowser_memInfo->append(abc);

    QString efg = process_mem->readAllStandardError();
    if(efg.length()>1)
    {
        ui->textBrowser_memInfo->append(efg);
    }
}

int Memory::isDigitStr(QString src)
{
    QByteArray ba = src.toLatin1();
     const char *s = ba.data();

    while(*s && *s>='0' && *s<='9') s++;

    if (*s)
    {
        return -1;
    }
    else
    { //num
        return 0;
    }
}

void Memory::on_start_clicked()
{
    QString size = ui->lineEdit_value->text();
    qDebug()<<"size=" << size;

    if(isDigitStr(size))
    {
        QMessageBox::warning(this, "WARNING", "Please Enter a Number.");
        return;
    }

    ui->cleanData->setEnabled(false);
    ui->start->setEnabled(false);
    ui->lineEdit_value->setEnabled(false);

    msTimer->start(10);
    displayList.clear();

    if(myprocess!=NULL)
    {
        delete myprocess;
    }
    myprocess = new QProcess(this);
    connect(myprocess, SIGNAL(readyReadStandardOutput()),this, SLOT(result()));
    connect(myprocess, SIGNAL(readyReadStandardError()),this, SLOT(result()));
    connect(myprocess, SIGNAL(finished(int)),this, SLOT(finished_result()));
    //myprocess->start(QString( "/usr/share/memtester 1m 1" ));
    myprocess->start(QString( "/usr/share/memtester " ).append(size).append("m 1"));

}

void Memory::on_pushButton_clicked()
{
    resetTimer();
}

void Memory::on_cleanData_clicked()
{
    ui->textBrowser->setText("");
}

void Memory::on_pushButton_mem_clicked()
{
    if(process_mem!=NULL)
        delete process_mem;

    ui->textBrowser_memInfo->setText("");

    process_mem = new QProcess(this);
    connect(process_mem, SIGNAL(readyReadStandardOutput()),this, SLOT(freemem_result()));
    connect(process_mem, SIGNAL(readyReadStandardError()),this, SLOT(freemem_result()));
    process_mem->start(QString( "free -lh" ));
}

//timer
void Memory::initTimer()
{
    msTimer= new QTimer(this);  //this说明是当前类对象的定时器
    connect(msTimer,SIGNAL(timeout()),this,SLOT(TimeSlot()));
    resetTimer();
}

void Memory::resetTimer()
{
    countTemp = 0;
    hourTemp = 0;
    minuteTemp = 0;
    secondTemp = 0;
    QString hourstr = QString::number(hourTemp);
    QString minutestr = QString::number(minuteTemp);
    QString secondstr = QString::number(secondTemp);
    displayTimer(hourstr,minutestr,secondstr);
    msTimer->stop();
}

void Memory::TimeSlot()
{
    countTemp+=1;
    if(countTemp==100)
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
    }
    ui->textBrowser->setText("");
    for(int i=0; i<displayList.size(); i++)
    {
        ui->textBrowser->append(displayList[i]);
        qDebug()<<displayList[i];
    }

    //把整数转换成字符串
    QString hourstr = QString::number(hourTemp);
    QString minutestr = QString::number(minuteTemp);
    QString secondstr = QString::number(secondTemp);
    displayTimer(hourstr,minutestr,secondstr);
}

void Memory::displayTimer(QString hour, QString minute, QString second)
{
    ui->lineEdit_timerH->setText(hour);
    ui->lineEdit_timerM->setText(minute);
    ui->lineEdit_timerS->setText(second);
}

void Memory::moveEvent(QMoveEvent *)
{
    this->move( QPoint( 0, 0 ) );
}

void Memory::resizeEvent(QResizeEvent *)
{
    this->showMaximized();
}

void Memory::closeEvent(QCloseEvent *)
{
    exit(0);
}


