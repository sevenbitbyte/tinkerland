#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QList>
#include "QXBee/QXBee.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    bool portUsed(QString port);

protected slots:
    void addXBee();
    void testSerialPort(QString port);
    void updateManageTab();
    void reportError(QString str);

private:
    Ui::MainWindow *ui;
    QList<QXBee*> xbeeList;
};

#endif // MAINWINDOW_H
