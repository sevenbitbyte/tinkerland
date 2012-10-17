/********************************************************************************
** Form generated from reading ui file 'joystick.ui'
**
** Created: Mon Mar 31 02:51:24 2008
**      by: Qt User Interface Compiler version 4.3.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_JOYSTICK_H
#define UI_JOYSTICK_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QTabWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

class Ui_JoystickView
{
public:
    QGridLayout *gridLayout;
    QTabWidget *tabWidget;
    QWidget *infoTab;
    QGridLayout *gridLayout1;
    QHBoxLayout *hboxLayout;
    QVBoxLayout *vboxLayout;
    QLabel *locationLabel;
    QLabel *nameLabel;
    QLabel *driverVersionLabel;
    QLabel *buttonCountLabel;
    QLabel *axisCountLabel;
    QVBoxLayout *vboxLayout1;
    QLineEdit *locationEdit;
    QLineEdit *nameEdit;
    QLineEdit *driverVersionEdit;
    QLineEdit *buttonCountEdit;
    QLineEdit *axisCountEdit;
    QWidget *axesTab;
    QGridLayout *gridLayout2;
    QHBoxLayout *hboxLayout1;
    QVBoxLayout *vboxLayout2;
    QLabel *xAxislabel;
    QLabel *yAxisLabel;
    QLabel *wheelAxisLabel;
    QVBoxLayout *vboxLayout3;
    QLineEdit *xAxisEdit;
    QLineEdit *yAxisEdit;
    QLineEdit *wheelAxisEdit;
    QWidget *buttonTab;
    QGridLayout *gridLayout3;
    QTableWidget *buttonTable;

    void setupUi(QWidget *JoystickView)
    {
    if (JoystickView->objectName().isEmpty())
        JoystickView->setObjectName(QString::fromUtf8("JoystickView"));
    JoystickView->resize(296, 218);
    JoystickView->setMouseTracking(true);
    JoystickView->setWindowIcon(QIcon(QString::fromUtf8("images/joystick.png")));
    gridLayout = new QGridLayout(JoystickView);
#ifndef Q_OS_MAC
    gridLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
    gridLayout->setMargin(9);
#endif
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    tabWidget = new QTabWidget(JoystickView);
    tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
    infoTab = new QWidget();
    infoTab->setObjectName(QString::fromUtf8("infoTab"));
    gridLayout1 = new QGridLayout(infoTab);
#ifndef Q_OS_MAC
    gridLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
    gridLayout1->setMargin(9);
#endif
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    hboxLayout = new QHBoxLayout();
#ifndef Q_OS_MAC
    hboxLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
    hboxLayout->setMargin(0);
#endif
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    vboxLayout = new QVBoxLayout();
#ifndef Q_OS_MAC
    vboxLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
    vboxLayout->setMargin(0);
#endif
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    locationLabel = new QLabel(infoTab);
    locationLabel->setObjectName(QString::fromUtf8("locationLabel"));

    vboxLayout->addWidget(locationLabel);

    nameLabel = new QLabel(infoTab);
    nameLabel->setObjectName(QString::fromUtf8("nameLabel"));

    vboxLayout->addWidget(nameLabel);

    driverVersionLabel = new QLabel(infoTab);
    driverVersionLabel->setObjectName(QString::fromUtf8("driverVersionLabel"));

    vboxLayout->addWidget(driverVersionLabel);

    buttonCountLabel = new QLabel(infoTab);
    buttonCountLabel->setObjectName(QString::fromUtf8("buttonCountLabel"));

    vboxLayout->addWidget(buttonCountLabel);

    axisCountLabel = new QLabel(infoTab);
    axisCountLabel->setObjectName(QString::fromUtf8("axisCountLabel"));

    vboxLayout->addWidget(axisCountLabel);


    hboxLayout->addLayout(vboxLayout);

    vboxLayout1 = new QVBoxLayout();
#ifndef Q_OS_MAC
    vboxLayout1->setSpacing(6);
#endif
    vboxLayout1->setMargin(0);
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    locationEdit = new QLineEdit(infoTab);
    locationEdit->setObjectName(QString::fromUtf8("locationEdit"));
    locationEdit->setReadOnly(true);

    vboxLayout1->addWidget(locationEdit);

    nameEdit = new QLineEdit(infoTab);
    nameEdit->setObjectName(QString::fromUtf8("nameEdit"));
    nameEdit->setReadOnly(true);

    vboxLayout1->addWidget(nameEdit);

    driverVersionEdit = new QLineEdit(infoTab);
    driverVersionEdit->setObjectName(QString::fromUtf8("driverVersionEdit"));
    driverVersionEdit->setReadOnly(true);

    vboxLayout1->addWidget(driverVersionEdit);

    buttonCountEdit = new QLineEdit(infoTab);
    buttonCountEdit->setObjectName(QString::fromUtf8("buttonCountEdit"));
    buttonCountEdit->setReadOnly(true);

    vboxLayout1->addWidget(buttonCountEdit);

    axisCountEdit = new QLineEdit(infoTab);
    axisCountEdit->setObjectName(QString::fromUtf8("axisCountEdit"));
    axisCountEdit->setReadOnly(true);

    vboxLayout1->addWidget(axisCountEdit);


    hboxLayout->addLayout(vboxLayout1);


    gridLayout1->addLayout(hboxLayout, 0, 0, 1, 1);

    tabWidget->addTab(infoTab, QString());
    axesTab = new QWidget();
    axesTab->setObjectName(QString::fromUtf8("axesTab"));
    gridLayout2 = new QGridLayout(axesTab);
#ifndef Q_OS_MAC
    gridLayout2->setSpacing(6);
#endif
#ifndef Q_OS_MAC
    gridLayout2->setMargin(9);
#endif
    gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
    hboxLayout1 = new QHBoxLayout();
#ifndef Q_OS_MAC
    hboxLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
    hboxLayout1->setMargin(0);
#endif
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    vboxLayout2 = new QVBoxLayout();
#ifndef Q_OS_MAC
    vboxLayout2->setSpacing(6);
#endif
#ifndef Q_OS_MAC
    vboxLayout2->setMargin(0);
#endif
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    xAxislabel = new QLabel(axesTab);
    xAxislabel->setObjectName(QString::fromUtf8("xAxislabel"));

    vboxLayout2->addWidget(xAxislabel);

    yAxisLabel = new QLabel(axesTab);
    yAxisLabel->setObjectName(QString::fromUtf8("yAxisLabel"));

    vboxLayout2->addWidget(yAxisLabel);

    wheelAxisLabel = new QLabel(axesTab);
    wheelAxisLabel->setObjectName(QString::fromUtf8("wheelAxisLabel"));

    vboxLayout2->addWidget(wheelAxisLabel);


    hboxLayout1->addLayout(vboxLayout2);

    vboxLayout3 = new QVBoxLayout();
#ifndef Q_OS_MAC
    vboxLayout3->setSpacing(6);
#endif
    vboxLayout3->setMargin(0);
    vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));
    xAxisEdit = new QLineEdit(axesTab);
    xAxisEdit->setObjectName(QString::fromUtf8("xAxisEdit"));
    xAxisEdit->setEnabled(true);
    xAxisEdit->setReadOnly(true);

    vboxLayout3->addWidget(xAxisEdit);

    yAxisEdit = new QLineEdit(axesTab);
    yAxisEdit->setObjectName(QString::fromUtf8("yAxisEdit"));
    yAxisEdit->setReadOnly(true);

    vboxLayout3->addWidget(yAxisEdit);

    wheelAxisEdit = new QLineEdit(axesTab);
    wheelAxisEdit->setObjectName(QString::fromUtf8("wheelAxisEdit"));
    wheelAxisEdit->setReadOnly(true);

    vboxLayout3->addWidget(wheelAxisEdit);


    hboxLayout1->addLayout(vboxLayout3);


    gridLayout2->addLayout(hboxLayout1, 0, 0, 1, 1);

    tabWidget->addTab(axesTab, QString());
    buttonTab = new QWidget();
    buttonTab->setObjectName(QString::fromUtf8("buttonTab"));
    gridLayout3 = new QGridLayout(buttonTab);
#ifndef Q_OS_MAC
    gridLayout3->setSpacing(6);
#endif
#ifndef Q_OS_MAC
    gridLayout3->setMargin(9);
#endif
    gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));
    buttonTable = new QTableWidget(buttonTab);
    buttonTable->setObjectName(QString::fromUtf8("buttonTable"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(7));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(buttonTable->sizePolicy().hasHeightForWidth());
    buttonTable->setSizePolicy(sizePolicy);
    buttonTable->setLineWidth(1);
    buttonTable->setDragDropOverwriteMode(false);
    buttonTable->setAlternatingRowColors(true);
    buttonTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    gridLayout3->addWidget(buttonTable, 0, 0, 1, 1);

    tabWidget->addTab(buttonTab, QString());

    gridLayout->addWidget(tabWidget, 0, 0, 1, 1);


    retranslateUi(JoystickView);

    tabWidget->setCurrentIndex(2);


    QMetaObject::connectSlotsByName(JoystickView);
    } // setupUi

    void retranslateUi(QWidget *JoystickView)
    {
    JoystickView->setWindowTitle(QApplication::translate("JoystickView", "Joystick Viewer", 0, QApplication::UnicodeUTF8));
    locationLabel->setText(QApplication::translate("JoystickView", "Location", 0, QApplication::UnicodeUTF8));
    nameLabel->setText(QApplication::translate("JoystickView", "Name", 0, QApplication::UnicodeUTF8));
    driverVersionLabel->setText(QApplication::translate("JoystickView", "Driver Version", 0, QApplication::UnicodeUTF8));
    buttonCountLabel->setText(QApplication::translate("JoystickView", "Buttons", 0, QApplication::UnicodeUTF8));
    axisCountLabel->setText(QApplication::translate("JoystickView", "Axes", 0, QApplication::UnicodeUTF8));
    tabWidget->setTabText(tabWidget->indexOf(infoTab), QApplication::translate("JoystickView", "General", 0, QApplication::UnicodeUTF8));
    xAxislabel->setText(QApplication::translate("JoystickView", "X-Axis", 0, QApplication::UnicodeUTF8));
    yAxisLabel->setText(QApplication::translate("JoystickView", "Y-Axis", 0, QApplication::UnicodeUTF8));
    wheelAxisLabel->setText(QApplication::translate("JoystickView", "Wheel", 0, QApplication::UnicodeUTF8));
    tabWidget->setTabText(tabWidget->indexOf(axesTab), QApplication::translate("JoystickView", "Axes", 0, QApplication::UnicodeUTF8));
    if (buttonTable->columnCount() < 1)
        buttonTable->setColumnCount(1);

    QTableWidgetItem *__colItem = new QTableWidgetItem();
    __colItem->setText(QApplication::translate("JoystickView", "State", 0, QApplication::UnicodeUTF8));
    buttonTable->setHorizontalHeaderItem(0, __colItem);
    tabWidget->setTabText(tabWidget->indexOf(buttonTab), QApplication::translate("JoystickView", "Buttons", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(JoystickView);
    } // retranslateUi

};

namespace Ui {
    class JoystickView: public Ui_JoystickView {};
} // namespace Ui

#endif // UI_JOYSTICK_H
