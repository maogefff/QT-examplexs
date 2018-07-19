#ifndef EEPROM_H 
#define EEPROM_H

#include <QWidget>
#include <QtCore>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QTextStream>
#include <QCloseEvent>

namespace Ui {
    class Eeprom;
}

class Eeprom : public QWidget {
    Q_OBJECT
public:
    explicit Eeprom(QWidget *parent = 0);
    ~Eeprom();

protected:
    void moveEvent(QMoveEvent *);
    void resizeEvent(QResizeEvent *);
    void closeEvent(QCloseEvent *);
    void StrToHex(char *pbDest, char *pbSrc, int nLen);
    bool isDigit(char* str, int len);

private:
    Ui::Eeprom *ui;
	int fd;
	int len;
private slots:
    void on_pushButton_open_clicked();

    void on_pushButton_read_clicked();

    void on_pushButton_write_clicked();    
    void on_comboBox_activated(const QString &arg1);

protected:
};

#endif
