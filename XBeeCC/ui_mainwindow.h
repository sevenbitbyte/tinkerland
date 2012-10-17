/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Tue May 8 01:25:42 2012
**      by: Qt User Interface Compiler version 4.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QStatusBar>
#include <QtGui/QTabWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionSave_Log;
    QAction *actionQuit;
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout;
    QTabWidget *tabWidget;
    QWidget *manageTab;
    QHBoxLayout *horizontalLayout_4;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QLabel *deviceLabel;
    QLineEdit *deviceEdit;
    QLabel *baudLabel;
    QComboBox *baudBox;
    QPushButton *addButton;
    QTableWidget *xbeeTable;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *applyButton;
    QSpacerItem *horizontalSpacer;
    QPushButton *removeButton;
    QWidget *scanTab;
    QWidget *snoopTab;
    QGridLayout *gridLayout;
    QTableWidget *tableWidget_2;
    QWidget *destroyTab;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(600, 403);
        actionSave_Log = new QAction(MainWindow);
        actionSave_Log->setObjectName(QString::fromUtf8("actionSave_Log"));
        actionQuit = new QAction(MainWindow);
        actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        horizontalLayout = new QHBoxLayout(centralWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setEnabled(true);
        tabWidget->setFocusPolicy(Qt::TabFocus);
        manageTab = new QWidget();
        manageTab->setObjectName(QString::fromUtf8("manageTab"));
        horizontalLayout_4 = new QHBoxLayout(manageTab);
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        deviceLabel = new QLabel(manageTab);
        deviceLabel->setObjectName(QString::fromUtf8("deviceLabel"));

        horizontalLayout_2->addWidget(deviceLabel);

        deviceEdit = new QLineEdit(manageTab);
        deviceEdit->setObjectName(QString::fromUtf8("deviceEdit"));
        deviceEdit->setFocusPolicy(Qt::StrongFocus);

        horizontalLayout_2->addWidget(deviceEdit);

        baudLabel = new QLabel(manageTab);
        baudLabel->setObjectName(QString::fromUtf8("baudLabel"));

        horizontalLayout_2->addWidget(baudLabel);

        baudBox = new QComboBox(manageTab);
        baudBox->setObjectName(QString::fromUtf8("baudBox"));

        horizontalLayout_2->addWidget(baudBox);

        addButton = new QPushButton(manageTab);
        addButton->setObjectName(QString::fromUtf8("addButton"));
        addButton->setEnabled(false);

        horizontalLayout_2->addWidget(addButton);


        verticalLayout->addLayout(horizontalLayout_2);

        xbeeTable = new QTableWidget(manageTab);
        if (xbeeTable->columnCount() < 7)
            xbeeTable->setColumnCount(7);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        xbeeTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        xbeeTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        xbeeTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        xbeeTable->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        xbeeTable->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        xbeeTable->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        xbeeTable->setHorizontalHeaderItem(6, __qtablewidgetitem6);
        xbeeTable->setObjectName(QString::fromUtf8("xbeeTable"));

        verticalLayout->addWidget(xbeeTable);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        applyButton = new QPushButton(manageTab);
        applyButton->setObjectName(QString::fromUtf8("applyButton"));

        horizontalLayout_3->addWidget(applyButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer);

        removeButton = new QPushButton(manageTab);
        removeButton->setObjectName(QString::fromUtf8("removeButton"));

        horizontalLayout_3->addWidget(removeButton);


        verticalLayout->addLayout(horizontalLayout_3);


        horizontalLayout_4->addLayout(verticalLayout);

        tabWidget->addTab(manageTab, QString());
        scanTab = new QWidget();
        scanTab->setObjectName(QString::fromUtf8("scanTab"));
        tabWidget->addTab(scanTab, QString());
        snoopTab = new QWidget();
        snoopTab->setObjectName(QString::fromUtf8("snoopTab"));
        gridLayout = new QGridLayout(snoopTab);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        tableWidget_2 = new QTableWidget(snoopTab);
        if (tableWidget_2->columnCount() < 4)
            tableWidget_2->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(0, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(1, __qtablewidgetitem8);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(2, __qtablewidgetitem9);
        QTableWidgetItem *__qtablewidgetitem10 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(3, __qtablewidgetitem10);
        tableWidget_2->setObjectName(QString::fromUtf8("tableWidget_2"));

        gridLayout->addWidget(tableWidget_2, 0, 0, 1, 1);

        tabWidget->addTab(snoopTab, QString());
        destroyTab = new QWidget();
        destroyTab->setObjectName(QString::fromUtf8("destroyTab"));
        tabWidget->addTab(destroyTab, QString());

        horizontalLayout->addWidget(tabWidget);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 600, 28));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        MainWindow->setMenuBar(menuBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);
#ifndef QT_NO_SHORTCUT
        deviceLabel->setBuddy(deviceEdit);
        baudLabel->setBuddy(baudBox);
#endif // QT_NO_SHORTCUT
        QWidget::setTabOrder(deviceEdit, baudBox);
        QWidget::setTabOrder(baudBox, addButton);
        QWidget::setTabOrder(addButton, xbeeTable);
        QWidget::setTabOrder(xbeeTable, applyButton);
        QWidget::setTabOrder(applyButton, removeButton);
        QWidget::setTabOrder(removeButton, tableWidget_2);

        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(actionSave_Log);
        menuFile->addAction(actionQuit);

        retranslateUi(MainWindow);
        QObject::connect(deviceEdit, SIGNAL(returnPressed()), addButton, SLOT(click()));

        tabWidget->setCurrentIndex(0);
        baudBox->setCurrentIndex(3);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "QXBee Command Center", 0, QApplication::UnicodeUTF8));
        actionSave_Log->setText(QApplication::translate("MainWindow", "Save Log", 0, QApplication::UnicodeUTF8));
        actionQuit->setText(QApplication::translate("MainWindow", "Quit", 0, QApplication::UnicodeUTF8));
        deviceLabel->setText(QApplication::translate("MainWindow", "Device:", 0, QApplication::UnicodeUTF8));
        baudLabel->setText(QApplication::translate("MainWindow", "Baud Rate:", 0, QApplication::UnicodeUTF8));
        baudBox->clear();
        baudBox->insertItems(0, QStringList()
         << QApplication::translate("MainWindow", "1200 bps", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "2400 bps", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "4800 bps", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "9600 bps", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "19200 bps", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "38400 bps", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "57600 bps", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "115200 bps", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "Auto Select", 0, QApplication::UnicodeUTF8)
        );
        addButton->setText(QApplication::translate("MainWindow", "Add", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = xbeeTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("MainWindow", "Serial Port", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = xbeeTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("MainWindow", "Status", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem2 = xbeeTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("MainWindow", "Channel", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem3 = xbeeTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("MainWindow", "Address64", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem4 = xbeeTable->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QApplication::translate("MainWindow", "Coordinator", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem5 = xbeeTable->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QApplication::translate("MainWindow", "PanID", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem6 = xbeeTable->horizontalHeaderItem(6);
        ___qtablewidgetitem6->setText(QApplication::translate("MainWindow", "Signal Strength", 0, QApplication::UnicodeUTF8));
        applyButton->setText(QApplication::translate("MainWindow", "Apply", 0, QApplication::UnicodeUTF8));
        removeButton->setText(QApplication::translate("MainWindow", "Remove", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(manageTab), QApplication::translate("MainWindow", "Manage", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(scanTab), QApplication::translate("MainWindow", "Scan", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem7 = tableWidget_2->horizontalHeaderItem(0);
        ___qtablewidgetitem7->setText(QApplication::translate("MainWindow", "Source", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem8 = tableWidget_2->horizontalHeaderItem(1);
        ___qtablewidgetitem8->setText(QApplication::translate("MainWindow", "Pan", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem9 = tableWidget_2->horizontalHeaderItem(2);
        ___qtablewidgetitem9->setText(QApplication::translate("MainWindow", "Channel", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem10 = tableWidget_2->horizontalHeaderItem(3);
        ___qtablewidgetitem10->setText(QApplication::translate("MainWindow", "Data", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(snoopTab), QApplication::translate("MainWindow", "Snoop", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(destroyTab), QApplication::translate("MainWindow", "Destroy", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        statusBar->setToolTip(QString());
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        statusBar->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_ACCESSIBILITY
        statusBar->setAccessibleDescription(QString());
#endif // QT_NO_ACCESSIBILITY
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
