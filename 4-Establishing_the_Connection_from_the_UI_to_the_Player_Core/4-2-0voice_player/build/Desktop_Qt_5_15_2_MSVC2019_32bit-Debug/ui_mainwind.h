/********************************************************************************
** Form generated from reading UI file 'mainwind.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWIND_H
#define UI_MAINWIND_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include <ctrlbar.h>
#include <displaywind.h>
#include <playlistwind.h>
#include <titlebar.h>

QT_BEGIN_NAMESPACE

class Ui_MainWind
{
public:
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QWidget *showCtrlBarBgWidget;
    QGridLayout *gridLayout_2;
    DisplayWind *showWind;
    CtrlBar *ctrlBarWind;
    QMenuBar *menuBar;
    QStatusBar *statusBar;
    QDockWidget *playListDockWidget;
    PlayListWind *playListContents;
    QDockWidget *titleDockWidget;
    TitleBar *titleContents;

    void setupUi(QMainWindow *MainWind)
    {
        if (MainWind->objectName().isEmpty())
            MainWind->setObjectName(QString::fromUtf8("MainWind"));
        MainWind->resize(1280, 800);
        centralWidget = new QWidget(MainWind);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(0);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        showCtrlBarBgWidget = new QWidget(centralWidget);
        showCtrlBarBgWidget->setObjectName(QString::fromUtf8("showCtrlBarBgWidget"));
        gridLayout_2 = new QGridLayout(showCtrlBarBgWidget);
        gridLayout_2->setSpacing(0);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        showWind = new DisplayWind(showCtrlBarBgWidget);
        showWind->setObjectName(QString::fromUtf8("showWind"));

        gridLayout_2->addWidget(showWind, 0, 0, 1, 1);

        ctrlBarWind = new CtrlBar(showCtrlBarBgWidget);
        ctrlBarWind->setObjectName(QString::fromUtf8("ctrlBarWind"));
        ctrlBarWind->setMinimumSize(QSize(300, 60));

        gridLayout_2->addWidget(ctrlBarWind, 1, 0, 1, 1);


        gridLayout->addWidget(showCtrlBarBgWidget, 0, 0, 1, 1);

        MainWind->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWind);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1280, 26));
        MainWind->setMenuBar(menuBar);
        statusBar = new QStatusBar(MainWind);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWind->setStatusBar(statusBar);
        playListDockWidget = new QDockWidget(MainWind);
        playListDockWidget->setObjectName(QString::fromUtf8("playListDockWidget"));
        playListDockWidget->setMinimumSize(QSize(100, 125));
        playListContents = new PlayListWind();
        playListContents->setObjectName(QString::fromUtf8("playListContents"));
        playListContents->setMinimumSize(QSize(100, 100));
        playListDockWidget->setWidget(playListContents);
        MainWind->addDockWidget(Qt::RightDockWidgetArea, playListDockWidget);
        titleDockWidget = new QDockWidget(MainWind);
        titleDockWidget->setObjectName(QString::fromUtf8("titleDockWidget"));
        titleDockWidget->setMinimumSize(QSize(200, 41));
        titleContents = new TitleBar();
        titleContents->setObjectName(QString::fromUtf8("titleContents"));
        titleDockWidget->setWidget(titleContents);
        MainWind->addDockWidget(Qt::TopDockWidgetArea, titleDockWidget);

        retranslateUi(MainWind);

        QMetaObject::connectSlotsByName(MainWind);
    } // setupUi

    void retranslateUi(QMainWindow *MainWind)
    {
        MainWind->setWindowTitle(QCoreApplication::translate("MainWind", "MainWind", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWind: public Ui_MainWind {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWIND_H
