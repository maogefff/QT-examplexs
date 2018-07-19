#include "backlight.h"
#include "ui_backlight.h"
#include <fcntl.h>
#include <stdio.h>
//#include <qpushbutton.h>
#include <sys/types.h>
#include <unistd.h>
#include <QDebug>
#include <QMessageBox>
#include "../eeprom/i2c.h"


#define MAX_BRIGHTNESS    "/sys/class/backlight/pwm-backlight/max_brightness"
#define CUR_BRIGHTNESS    "/sys/class/backlight/pwm-backlight/brightness"

Backlight::Backlight(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Backlight)
{
    ui->setupUi(this);
    connect(ui->slider,SIGNAL(valueChanged(int)),this,SLOT(sliderchanged(int)));

    fd = ::open(MAX_BRIGHTNESS,O_RDONLY|O_NONBLOCK);
    if(fd>=0)
    {
        char buf[16];
        memset(buf, 0, sizeof(buf));
        ::read(fd, buf, 6);

        int value=atoi(buf);
        ui->slider->setRange(1,value);
        ::close(fd);
        qDebug() << MAX_BRIGHTNESS << " : " << value;
        ui->label_2->setText(buf);
    }
    else
    {
        qDebug() << MAX_BRIGHTNESS << " Open failed !!!";
    }

    fd = ::open(CUR_BRIGHTNESS,O_RDWR|O_NONBLOCK);
    if(fd>=0)
    {
        char buf[16];
        memset(buf, 0, sizeof(buf));
        ::read(fd, buf, 6);

        int value=atoi(buf);
        ui->slider->setValue(value);
        qDebug() << CUR_BRIGHTNESS << " : " << value;
        ui->label_3->setText(buf);
    }
    else
    {
        qDebug() << CUR_BRIGHTNESS << " Open failed !!!";
    }
    ui->label->setText("1");
}

Backlight::~Backlight( )
{
    if(fd>=0)
    {
        ::close(fd);
        fd=-1;
    }
    delete ui;
}

void Backlight::sliderchanged(int v)
{
    char buf[20];
    memset(buf, 0, sizeof(buf));
    sprintf(buf,"%d",(int) v);
    ::write(fd,buf,strlen(buf));
    ui->label_3->setText(buf);
    system("cat /sys/class/backlight/pwm-backlight/brightness > /etc/brightness");
    system("sync");
//    qDebug() << (int)(v);
}

void Backlight::moveEvent(QMoveEvent *)
{   
    this->move(QPoint(0,0));
}

void Backlight::resizeEvent(QResizeEvent *)
{
    this->showMaximized();
}

void Backlight::writeCommand(unsigned char reg, unsigned char val)
{
    fd2 = open("/dev/i2c-2", O_RDWR);
    if (fd2 < 0) {
        perror("open i2c device fail");
        exit(1);
    }

    i2c_set(fd2, 3, 4);

    int ret;

    ret = i2c_byte_write(fd2, 0x66, reg, val);
    usleep(5000);

    if (ret==-1) {
        QMessageBox::information(this, tr("warning"), "write error!");
    }
    else
    {
        QMessageBox::information(this, tr("info"),"write success!");
    }
    ::close(fd2);
    fd2 = -1;
}


void Backlight::on_pushButton_on_clicked()
{
    unsigned char value;

    QString nextState = ui->pushButton_on->text();
    if(nextState=="ON")
    {
        value = 0x01;
        ui->pushButton_on->setText("OFF");
    }
    else if(nextState=="OFF")
    {
        value = 0x00;
        ui->pushButton_on->setText("ON");
    }

    writeCommand(1, value);
}

void Backlight::on_pushButton_min_clicked()
{
    QString value_s = ui->lineEdit_min->text();

    unsigned char value;
    bool ok;

    if (value_s.startsWith("0x") || value_s.startsWith("0X"))
    {
        QString value_t = value_s.mid(2);
        value = (unsigned char)value_t.toInt(&ok, 16);
    }
    else
    {
        value = value_s.toInt();
    }

    if(value>20)
    {
        QMessageBox::information(this, tr("info"),"greater than 20");
        return;
    }

    writeCommand(2, value);
}

void Backlight::on_pushButton_max_clicked()
{
    QString value_s = ui->lineEdit_max->text();

    unsigned char value;
    bool ok;

    if (value_s.startsWith("0x") || value_s.startsWith("0X"))
    {
        QString value_t = value_s.mid(2);
        value = (unsigned char)value_t.toInt(&ok, 16);
    }
    else
    {
        value = value_s.toInt();
    }

    if(value>100)
    {
        QMessageBox::information(this, tr("info"),"greater than 100");
        return;
    }else if(value<21)
    {
        QMessageBox::information(this, tr("info"),"less than 21");
        return;
    }
    writeCommand(3, value);
}
