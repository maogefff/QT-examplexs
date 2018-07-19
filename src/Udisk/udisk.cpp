#include "udisk.h"
#include "ui_udisk.h"
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <QFileInfo>
#include <QMessageBox>
#include <QTreeView>
#include <QDebug>
#include <QTimer>

/* sd卡生成的设备节点 */
#define EXUSB_NODE            "/dev/sda1"
#define EXSD_NODE             "/dev/mmcblk2p1"
#define EXEMMC_NODE           "/dev/mmcblk3p2"
/* sd卡挂载的文件系统节点 */
#define MOUNT_NODE            "/mnt"
#define TEST_FILE            "/udisk_test"

#define TEST_DIR            "/blockTest"

MYThread::MYThread(int type, QObject *parent) :
    QThread(parent)
{
    this->type = type;
}

void MYThread::run()
{
    emit sendSingal( SIGNAL_RUNSTART );

    if(this->type == TYPE_WRITEMULTI)
    {
        int totalSize=0, totalNum=0;
        int tmp1, tmp2, tmp3, tmp4;

        QFileInfo fileInfo( QString( MOUNT_NODE ).append( TEST_DIR ) );

        if(!fileInfo.exists()){
            system(QString( "mkdir " ).append( MOUNT_NODE ).append( TEST_DIR ).toLocal8Bit());
        }

        emit sendSingal( SIGNAL_WRITEMULTI );

        for(int i = 1; i <= 200; i++)
        {
            totalNum += 4;
            tmp1 = i;
            tmp2 = tmp1*10;
            tmp3 = tmp1*20;
            tmp4 = tmp1*40;
            totalSize += tmp1;
            totalSize += tmp2;
            totalSize += tmp3;
            totalSize += tmp4;
            fileName<<QString("/tmp1_").append( QString::number(tmp1, 10) ).append( "k" );
            fileName<<QString("/tmp2_").append( QString::number(tmp2, 10) ).append( "k" );
            fileName<<QString("/tmp3_").append( QString::number(tmp3, 10) ).append( "k" );
            fileName<<QString("/tmp4_").append( QString::number(tmp4, 10) ).append( "k" );
            system(QString( "dd if=" ).append("/dev/zero of=").append( MOUNT_NODE ).append( TEST_DIR ).append( "/tmp1_" ).append( QString::number(tmp1, 10) ).append( "k" ).append( " bs=1k count=" ).append( QString::number(tmp1, 10) ).append(" conv=fdatasync").toLocal8Bit());
            system(QString( "dd if=" ).append("/dev/zero of=").append( MOUNT_NODE ).append( TEST_DIR ).append( "/tmp2_" ).append( QString::number(tmp2, 10) ).append( "k" ).append( " bs=1k count=" ).append( QString::number(tmp2, 10) ).append(" conv=fdatasync").toLocal8Bit());
            system(QString( "dd if=" ).append("/dev/zero of=").append( MOUNT_NODE ).append( TEST_DIR ).append( "/tmp3_" ).append( QString::number(tmp3, 10) ).append( "k" ).append( " bs=1k count=" ).append( QString::number(tmp3, 10) ).append(" conv=fdatasync").toLocal8Bit());
            system(QString( "dd if=" ).append("/dev/zero of=").append( MOUNT_NODE ).append( TEST_DIR ).append( "/tmp4_" ).append( QString::number(tmp4, 10) ).append( "k" ).append( " bs=1k count=" ).append( QString::number(tmp4, 10) ).append(" conv=fdatasync").toLocal8Bit());
        }
        for(int i=0; i<fileName.size(); i++)
        {
               qDebug()<<"name"<<i<<"="<<fileName[i];
        }
        qDebug()<<"totalNum="<<totalNum;
        qDebug()<<"totalSize="<<totalSize;
        emit multiWriteFinished(totalNum, totalSize, fileName);

        emit sendSingal( SIGNAL_RUNEND );
    }
    else if(this->type == TYPE_READMULTI)
    {
        emit sendSingal( SIGNAL_CLEANCACHESSTART );
        system("sync && echo 3 > /proc/sys/vm/drop_caches");
        emit sendSingal( SIGNAL_CLEANCACHESEND );
        msleep( SLEEP_DELAY_MS );
        emit sendSingal( SIGNAL_READMULTI );
        qDebug()<<"enter read multi size="<<fileName.size();
        for(int i=0; i<fileName.size(); i++)
        {
           qDebug()<<"read name"<<i<<"="<<fileName[i];
           system(QString( "cp " ).append( MOUNT_NODE ).append( TEST_DIR ).append( fileName[i] ).append(" /dev/null").toLocal8Bit());
        }
        emit multiReadFinished();
        emit sendSingal( SIGNAL_RUNEND );
    }
    else if(this->type == TYPE_WRITEONE)
    {
        QFileInfo fileInfo( QString( MOUNT_NODE ).append( TEST_FILE ) );

        if(fileInfo.exists()){
            emit sendSingal( SIGNAL_RMFILE );
            system( QString( "rm " ).append( MOUNT_NODE ).append( TEST_FILE ).toLocal8Bit() );
        }
        emit sendSingal( SIGNAL_CLEANCACHESSTART );
        system("sync && echo 3 > /proc/sys/vm/drop_caches");
        emit sendSingal( SIGNAL_CLEANCACHESEND );
        msleep( SLEEP_DELAY_MS );
        emit sendSingal( SIGNAL_WRITEONE );
    }
    else if(this->type == TYPE_READONE)
    {
        emit sendSingal( SIGNAL_CLEANCACHESSTART );
        system("sync && echo 3 > /proc/sys/vm/drop_caches");
        emit sendSingal( SIGNAL_CLEANCACHESEND );
        msleep( SLEEP_DELAY_MS );
        emit sendSingal( SIGNAL_READONE );

    }
    else if(this->type == TYPE_REMOVEFILE)
    {
        system(QString( "rm " ).append( MOUNT_NODE ).append( TEST_DIR ).append( " -fr" ).toLocal8Bit());
        system(QString( "rm " ).append( MOUNT_NODE ).append( TEST_FILE ).append( " -fr" ).toLocal8Bit());
        emit sendSingal( SIGNAL_READONE );
    }
}

void MYThread::setFileName(QStringList name)
{
    this->fileName = name;
}


Udisk::Udisk(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Udisk)
{
    QString node;
    ui->setupUi(this);

    initView();
    initTimer();

    myprocess = NULL;
    myThread = NULL;
    node = ui->comboBox_type->currentText();

    if(node=="USB")
    {
        qDebug()<<"select USB";
        ExNode = EXUSB_NODE;
    }
    else if(node=="SD-card")
    {
        qDebug()<<"select SD-card";
        ExNode = EXSD_NODE;
    }
    else if(node=="eMMC")
    {
        qDebug()<<"select eMMC";
        ExNode = EXEMMC_NODE;
    }

    mountThread = new MountThread( ExNode, MOUNT_NODE );

    connect( mountThread, SIGNAL(preMount(int)), this, SLOT(dealwithUi(int)) );
    connect( mountThread, SIGNAL(postMount(int)), this, SLOT(dealwithUi(int)) );
    connect( mountThread, SIGNAL(deviceUnExist(int)), this, SLOT(dealwithUi(int)) );
}

void Udisk::initView()
{
    ui->showDir->setModel( &model );
    model.setRootPath( MOUNT_NODE );
    ui->showDir->setRootIndex( model.index( MOUNT_NODE ) );

    // Demonstrating look and feel features
    ui->showDir->setAnimated( false );
    ui->showDir->setIndentation( 20 );
    ui->showDir->setSortingEnabled( true );
    ui->showDir->setColumnWidth( 0, 250 );

    ExNode = ui->comboBox_type->currentText();
    ui->label_totalsize->setText("128M");
    ui->label_process->setText("");
    ui->label_fileExplan->setText("");

    count = 2;
    //testTimes = 1;

    if(ui->radioButton_onefiletest->isChecked())
    {
        testMethod = 0;
        ui->groupBox_onefileSet->setEnabled(true);
        ui->groupBox_multifileSet->setEnabled(false);
    }
    else if(ui->radioButton_onefiletest->isChecked())
    {
        testMethod = 1;
        ui->groupBox_onefileSet->setEnabled(false);
        ui->groupBox_multifileSet->setEnabled(true);
    }
}

void Udisk::myThreadWithUi(int mesg)
{
    if(mesg == MYThread::SIGNAL_RUNSTART)
    {
        qDebug()<<"CLEANSTART";
        refreshUi(false);
    }
    else if(mesg == MYThread::SIGNAL_RUNEND)
    {
        qDebug()<<"CLEANEND";
        refreshUi(true);
        msTimer->stop();
    }
    else if(mesg == MYThread::SIGNAL_WRITEONE)
    {
        ui->label_process->setText("write test...");
        msTimer->start(10);

        if(myprocess)
            delete myprocess;
        myprocess = new QProcess(this);
        //testTimesTmp = testTimes;
        connect(myprocess, SIGNAL(readyReadStandardOutput()),this, SLOT(result()));
        connect(myprocess, SIGNAL(readyReadStandardError()),this, SLOT(result()));
        connect(myprocess, SIGNAL(finished(int)),this, SLOT(finishedProcess()));

        //dd if=/dev/zero of=/mnt/udisk_test bs=64k  count=1k  conv=fdatasync
        myprocess->start(QString( "dd if=" ).append("/dev/zero of=").append( MOUNT_NODE ).append( TEST_FILE ).append( " bs=64k count=").append(QString::number(count*1024, 10)).append(" conv=fdatasync"));
    }
    else if(mesg == MYThread::SIGNAL_READONE)
    {
        ui->label_process->setText("read test...");
        msTimer->start(10);
        if(myprocess)
            delete myprocess;

        myprocess = new QProcess(this);
        //testTimesTmp = testTimes;
        connect(myprocess, SIGNAL(readyReadStandardOutput()),this, SLOT(result()));
        connect(myprocess, SIGNAL(readyReadStandardError()),this, SLOT(result()));
        connect(myprocess, SIGNAL(finished(int)),this, SLOT(finishedProcess()));
        myprocess->start(QString( "dd if=" ).append("/mnt").append( TEST_FILE ).append(" of=/dev/null").append( " bs=64k  count=").append(QString::number(count*1024, 10)));
        qDebug("test read=%s",qPrintable(QString( "dd if=" ).append("/mnt").append( TEST_FILE ).append(" of=/dev/null").append( " bs=64k  count=").append(QString::number(count*1024, 10)).toLocal8Bit()));
    }
    else if(mesg == MYThread::SIGNAL_CLEANCACHESSTART)
    {
        ui->label_process->setText("clean caches");
    }
    else if(mesg == MYThread::SIGNAL_CLEANCACHESEND)
    {
        ui->label_process->setText("clean caches finish...");
    }
    else if(mesg == MYThread::SIGNAL_WRITEMULTI)
    {
        msTimer->start(10);
        ui->label_process->setText("write test multi...");
    }
    else if(mesg == MYThread::SIGNAL_READMULTI)
    {
        msTimer->start(10);
        ui->label_process->setText("read test multi...");
    }

}

void Udisk::myThreadmultifinishedUi(int num, int size, QStringList list)
{
    fileName = list;
    QMessageBox::information(this, "Information", "test finish.");
    ui->label_fileExplan->setText(QString("NUM:").append(QString::number(num, 10)).append(" SIZE:").append(QString::number(size, 10)).append("K"));
}

void Udisk::myThreadMultiReadFinishedUi()
{
    QMessageBox::information(this, "Information", "test finish.");
}


void Udisk::dealwithUi(int mesg)
{
    mountFlag = mesg;

    if( MountThread::PRE_MOUNT == mesg ) {

        /* 将button设置为无效效果 */
        refreshUi(false);
        qDebug() << "premount." << endl;

    } else if ( MountThread::POST_MOUNT == mesg ) {
        refreshUi(true);
        qDebug() << "postmount." << endl;

    } else if (MountThread::DEVICE_UNEXIST == mesg ) {
        qDebug() << "device unexist." << endl;
        QMessageBox::warning(this, "WARNING", "Please check your SD card has plugin slot.");
    }
}


void Udisk::on_detectButton_clicked()
{
    mountThread->start();
}

void Udisk::on_cleanData_clicked()
{
    ui->result->setText("");
}

void Udisk::on_umountButton_clicked()
{
    delete myprocess;
    myprocess = NULL;
    mountFlag = 0;
    refreshUi(false);
    /* 恢复按钮有效效果 */
    /* 卸载sd卡 */
    system( QString( "umount " ).append( ExNode ).toLocal8Bit() );
    refreshUi(true);
}

void Udisk::on_writeButton_clicked()
{
    if( mountFlag != MountThread::POST_MOUNT) {
        QMessageBox::warning(this, "WARNING", "Please Mount your SD card.");
        return;
    }

    if(myThread)
        delete myThread;

    myThread = new MYThread(MYThread::TYPE_WRITEONE);
    connect( myThread, SIGNAL(sendSingal(int)), this, SLOT(myThreadWithUi(int)) );
    myThread->start();
}

void Udisk::on_readButton_clicked()
{
    QFileInfo fileInfo( QString( MOUNT_NODE ).append( TEST_FILE ) );

    if( mountFlag != MountThread::POST_MOUNT) {
        QMessageBox::warning(this, "WARNING", "Please Mount your SD card.");
        return;
    }

    if(!fileInfo.exists()){
        QMessageBox::warning(this, "WARNING", "No file, Please write first.");
        return;
    }

    if(myThread)
        delete myThread;

    myThread = new MYThread(MYThread::TYPE_READONE);
    connect( myThread, SIGNAL(sendSingal(int)), this, SLOT(myThreadWithUi(int)) );

    myThread->start();
}

void Udisk::refreshUi(bool mesg)
{
    if(mesg == true)
    {
        ui->detectButton->setEnabled( true );
        ui->umountButton->setEnabled( true );
        ui->groupBox_testMethod->setEnabled(true);

        ui->comboBox_type->setEnabled(true);
        if(testMethod == 0)
        {
            ui->groupBox_onefileSet->setEnabled(true);
            ui->groupBox_multifileSet->setEnabled(false);
        }
        else if(testMethod == 1)
        {
            ui->groupBox_onefileSet->setEnabled(false);
            ui->groupBox_multifileSet->setEnabled(true);
        }
    }
    else if(mesg == false)
    {
        ui->detectButton->setEnabled( false );
        ui->umountButton->setEnabled( false );
        ui->groupBox_testMethod->setEnabled(false);
        ui->groupBox_onefileSet->setEnabled(false);
        ui->groupBox_multifileSet->setEnabled(false);
        ui->comboBox_type->setEnabled(false);
    }

    model.setRootPath( "/home" );
    model.setRootPath( MOUNT_NODE );
    ui->showDir->setRootIndex( model.index( MOUNT_NODE ) );
}

void Udisk::result()
{
    QString abc = myprocess->readAllStandardOutput();
    ui->result->append(abc.trimmed());
    QString efg = myprocess->readAllStandardError();
    if(efg.length()>1)ui->result->append(efg.trimmed());
}

void Udisk::finishedProcess()
{
    msTimer->stop();
    refreshUi(true);
    ui->label_process->setText("test finish");
    QMessageBox::information(this, "Information", "test finish.");
}

void Udisk::on_comboBox_type_currentIndexChanged(const QString &test)
{
    if(test=="USB")
    {
        qDebug()<<"select USB";
        ExNode = EXUSB_NODE;
    }
    else if(test=="SD-card")
    {
        qDebug()<<"select SD-card";
        ExNode = EXSD_NODE;
    }
    else if(test=="eMMC")
    {
        qDebug()<<"select eMMC";
        ExNode = EXEMMC_NODE;
    }

    if(mountThread!=NULL)
    {
        delete mountThread;
    }
    mountThread = new MountThread( ExNode, MOUNT_NODE );

    connect( mountThread, SIGNAL(preMount(int)), this, SLOT(dealwithUi(int)) );
    connect( mountThread, SIGNAL(postMount(int)), this, SLOT(dealwithUi(int)) );
    connect( mountThread, SIGNAL(deviceUnExist(int)), this, SLOT(dealwithUi(int)) );

}

void Udisk::on_comboBox_count_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        count =  2;
        ui->label_totalsize->setText("128M");
        break;
    case 1:
        count = 8;
        ui->label_totalsize->setText("512M");
        break;
    case 2:
        count = 16;
        ui->label_totalsize->setText("1024M");
        break;
    case 3:
        count = 32;
        ui->label_totalsize->setText("2048M");
        break;
    case 4:
        count = 64;
        ui->label_totalsize->setText("4096M");
        break;
    default:
        break;
    }
}

//timer
void Udisk::initTimer()
{
    msTimer= new QTimer(this);  //this说明是当前类对象的定时器
    connect(msTimer,SIGNAL(timeout()),this,SLOT(TimeSlot()));
    resetTimer();
}

void Udisk::resetTimer()
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
    //msTimer->start(10);
}

void Udisk::TimeSlot()
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
    //把整数转换成字符串
    QString hourstr = QString::number(hourTemp);
    QString minutestr = QString::number(minuteTemp);
    QString secondstr = QString::number(secondTemp);
    displayTimer(hourstr,minutestr,secondstr);
}

void Udisk::displayTimer(QString hour, QString minute, QString second)
{
    ui->lineEdit_timerH->setText(hour);
    ui->lineEdit_timerM->setText(minute);
    ui->lineEdit_timerS->setText(second);
}

void Udisk::on_pushButton_resetTimer_clicked()
{
    resetTimer();
}

void Udisk::on_radioButton_onefiletest_clicked()
{
    ui->groupBox_multifileSet->setEnabled(false);
    ui->groupBox_onefileSet->setEnabled(true);
    testMethod = 0;
}

void Udisk::on_radioButton_multifiletest_clicked()
{
    ui->groupBox_multifileSet->setEnabled(true);
    ui->groupBox_onefileSet->setEnabled(false);
    testMethod = 1;
}

void Udisk::on_comboBox_currentIndexChanged(int index)
{
//    switch(index)
//    {
//        case 0:
//            testTimes = 1;
//            break;
//        case 1:
//            testTimes = 10;
//            break;
//        case 2:
//            testTimes = 50;
//            break;
//        case 3:
//            testTimes = 100;
//            break;
//        case 4:
//            testTimes = 500;
//            break;
//        case 5:
//            testTimes = 1000;
//            break;
//        default:
//            break;
//    }
}

void Udisk::on_pushButton_writefile_clicked()
{
    if( mountFlag != MountThread::POST_MOUNT) {
        QMessageBox::warning(this, "WARNING", "Please Mount your SD card.");
        return;
    }
    if(myThread!=NULL)
    {
        delete myThread;
    }
    myThread = new MYThread(MYThread::TYPE_WRITEMULTI);
    connect( myThread, SIGNAL(sendSingal(int)), this, SLOT(myThreadWithUi(int)) );
    connect( myThread, SIGNAL(multiWriteFinished(int, int, QStringList)), this, SLOT(myThreadmultifinishedUi(int, int, QStringList)) );

    myThread->start();
}

void Udisk::on_pushButton_readMulti_clicked()
{
    if( mountFlag != MountThread::POST_MOUNT) {
        QMessageBox::warning(this, "WARNING", "Please Mount your SD card.");
        return;
    }

    if(myThread!=NULL)
    {
        delete myThread;
    }

    myThread = new MYThread(MYThread::TYPE_READMULTI);
    connect( myThread, SIGNAL(sendSingal(int)), this, SLOT(myThreadWithUi(int)) );
    connect( myThread, SIGNAL(multiReadFinished()), this, SLOT(myThreadMultiReadFinishedUi()) );

    myThread->setFileName(fileName);
    myThread->start();

}

void Udisk::on_pushButton_removefile_clicked()
{
    if( mountFlag != MountThread::POST_MOUNT) {
        QMessageBox::warning(this, "WARNING", "Please Mount your SD card.");
        return;
    }
    if(myThread!=NULL)
    {
        delete myThread;
    }

    myThread = new MYThread(MYThread::TYPE_REMOVEFILE);
    connect( myThread, SIGNAL(sendSingal(int)), this, SLOT(myThreadWithUi(int)) );

    myThread->start();
}

Udisk::~Udisk()
{
    delete ui;
}
void Udisk::moveEvent(QMoveEvent *)
{
    this->move( QPoint( 0, 0 ) );
}

void Udisk::resizeEvent(QResizeEvent *)
{
    this->showMaximized();
}

void Udisk::closeEvent(QCloseEvent *)
{
    exit(0);
}


