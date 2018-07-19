#ifndef BACKLIGHT_H
#define BACKLIGHT_H

#include <QWidget>

namespace Ui {
class Backlight;
}

class Backlight : public QWidget
{
    Q_OBJECT
    
public:
    explicit Backlight(QWidget *parent = 0);
    ~Backlight();
    
private:
    Ui::Backlight *ui;
    int fd;
    int fd2;
private slots:
    void sliderchanged(int v);
    void on_pushButton_on_clicked();

    void on_pushButton_min_clicked();

    void on_pushButton_max_clicked();

protected:
    void moveEvent(QMoveEvent *);
    void resizeEvent(QResizeEvent *);
    void writeCommand(unsigned char reg, unsigned char val);

};

#endif // MAINWINDOW_H
