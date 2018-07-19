#ifndef DOUBLE_LED_H
#define DOUBLE_LED_H

#include <QWidget>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <linux/input.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>

#include <QProcess>

namespace Ui {
class Double_led;
}

class Double_led : public QWidget
{
    Q_OBJECT
    
public:
    explicit Double_led(QWidget *parent = 0);
    ~Double_led();

private slots:
    void on_pushButton_clicked();

private:
    Ui::Double_led *ui;
    QProcess *myprocess;
};

#endif // DOUBLE_LED_H
