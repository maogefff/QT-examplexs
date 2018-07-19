#include "double_led.h"
#include "ui_double_led.h"
#include <QMouseEvent>
#include <QDebug>

Double_led::Double_led(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Double_led)
{
    ui->setupUi(this);
    myprocess = NULL;
}

Double_led::~Double_led()
{
    delete ui;
}

void Double_led::on_pushButton_clicked()
{
    system("/etc/init.d/panel_led stop");

    QString cmd = ui->comboBox->currentText();
    if(myprocess!=NULL)
    {
        delete myprocess;
    }
    myprocess = new QProcess(this);

    if(cmd=="system starting")
    {
        qDebug()<<"ok:"<<cmd;
        myprocess->start(QString( "./home/root/panel_led system_start" ));
    }
    else if(cmd=="System over temperature")
    {
        qDebug()<<"ok:"<<cmd;
        myprocess->start(QString( "./home/root/panel_led over_temprature" ));
    }
    else if(cmd=="Normal operation")
    {
        qDebug()<<"ok:"<<cmd;
        myprocess->start(QString( "./home/root/panel_led normal_operation" ));
    }
    else if(cmd=="Backlight Off")
    {
        qDebug()<<"ok:"<<cmd;
        myprocess->start(QString( "./home/root/panel_led backligt_off" ));
    }
}
