/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *serverLayout;
    QLabel *serverLabel;
    QLineEdit *serverIpEdit;
    QPushButton *onlineButton;
    QLabel *statusLabel;
    QHBoxLayout *buttonLayout;
    QPushButton *recordButton;
    QPushButton *stopButton;
    QLabel *infoLabel;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(500, 300);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        serverLayout = new QHBoxLayout();
        serverLayout->setObjectName("serverLayout");
        serverLabel = new QLabel(centralwidget);
        serverLabel->setObjectName("serverLabel");

        serverLayout->addWidget(serverLabel);

        serverIpEdit = new QLineEdit(centralwidget);
        serverIpEdit->setObjectName("serverIpEdit");

        serverLayout->addWidget(serverIpEdit);

        onlineButton = new QPushButton(centralwidget);
        onlineButton->setObjectName("onlineButton");
        onlineButton->setMinimumSize(QSize(100, 30));
        QFont font;
        font.setPointSize(10);
        font.setBold(true);
        onlineButton->setFont(font);

        serverLayout->addWidget(onlineButton);


        verticalLayout->addLayout(serverLayout);

        statusLabel = new QLabel(centralwidget);
        statusLabel->setObjectName("statusLabel");
        QFont font1;
        font1.setPointSize(12);
        font1.setBold(true);
        statusLabel->setFont(font1);
        statusLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(statusLabel);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setObjectName("buttonLayout");
        recordButton = new QPushButton(centralwidget);
        recordButton->setObjectName("recordButton");
        recordButton->setMinimumSize(QSize(150, 80));
        QFont font2;
        font2.setPointSize(16);
        font2.setBold(true);
        recordButton->setFont(font2);

        buttonLayout->addWidget(recordButton);

        stopButton = new QPushButton(centralwidget);
        stopButton->setObjectName("stopButton");
        stopButton->setMinimumSize(QSize(150, 80));
        stopButton->setFont(font2);

        buttonLayout->addWidget(stopButton);


        verticalLayout->addLayout(buttonLayout);

        infoLabel = new QLabel(centralwidget);
        infoLabel->setObjectName("infoLabel");
        infoLabel->setAlignment(Qt::AlignCenter);
        infoLabel->setWordWrap(true);

        verticalLayout->addWidget(infoLabel);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 500, 22));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Voice Chat", nullptr));
        serverLabel->setText(QCoreApplication::translate("MainWindow", "Server IP:", nullptr));
        serverIpEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "\303\266rn: 192.168.1.100", nullptr));
        serverIpEdit->setText(QCoreApplication::translate("MainWindow", "127.0.0.1", nullptr));
        onlineButton->setText(QCoreApplication::translate("MainWindow", "Connect", nullptr));
        statusLabel->setText(QCoreApplication::translate("MainWindow", "\342\232\253 Disconnected", nullptr));
        recordButton->setText(QCoreApplication::translate("MainWindow", "\360\237\216\244 TALK", nullptr));
        stopButton->setText(QCoreApplication::translate("MainWindow", "\360\237\224\207 STOP", nullptr));
        infoLabel->setText(QCoreApplication::translate("MainWindow", "Server \303\247al\304\261\305\237t\304\261ran ki\305\237inin IP adresini girin ve Connect'e bas\304\261n.", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
