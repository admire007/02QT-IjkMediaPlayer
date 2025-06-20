/********************************************************************************
** Form generated from reading UI file 'homewindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HOMEWINDOW_H
#define UI_HOMEWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include <customslider.h>
#include <displaywind.h>
#include <playlist.h>

QT_BEGIN_NAMESPACE

class Ui_HomeWindow
{
public:
    QAction *openFileAction;
    QAction *action_3;
    QAction *action_4;
    QAction *action_5;
    QAction *action_6;
    QAction *action_7;
    QAction *action_8;
    QAction *action_10;
    QAction *action_11;
    QAction *action_12;
    QAction *openUrlAction;
    QWidget *centralWidget;
    QWidget *ctrlBar;
    CustomSlider *playSlider;
    CustomSlider *volumeSlider;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *playOrPauseBtn;
    QPushButton *stopBtn;
    QPushButton *backFastBtn;
    QPushButton *forwardFastBtn;
    QPushButton *prevBtn;
    QPushButton *nextBtn;
    QLabel *curPosition;
    QLabel *label_7;
    QLabel *totalDuration;
    QPushButton *speedBtn;
    QPushButton *screenBtn;
    QPushButton *displayRatioBtn;
    QWidget *layoutWidget1;
    QGridLayout *gridLayout;
    QLabel *label_5;
    QLineEdit *audioBufEdit;
    QLabel *label_3;
    QComboBox *bufDurationBox;
    QLabel *label_4;
    QLineEdit *videoBufEdit;
    QLabel *label_2;
    QComboBox *jitterBufBox;
    QPushButton *listBtn;
    QPushButton *settingBtn;
    QLineEdit *lineEdit_3;
    QLabel *label;
    DisplayWind *display;
    Playlist *playList;
    QMenuBar *menuBar;
    QMenu *menu_2;
    QMenu *menu_3;
    QMenu *menu_4;
    QMenu *menu;

    void setupUi(QMainWindow *HomeWindow)
    {
        if (HomeWindow->objectName().isEmpty())
            HomeWindow->setObjectName(QString::fromUtf8("HomeWindow"));
        HomeWindow->resize(1040, 645);
        HomeWindow->setMinimumSize(QSize(400, 200));
        HomeWindow->setMaximumSize(QSize(16777215, 16777215));
        openFileAction = new QAction(HomeWindow);
        openFileAction->setObjectName(QString::fromUtf8("openFileAction"));
        action_3 = new QAction(HomeWindow);
        action_3->setObjectName(QString::fromUtf8("action_3"));
        action_4 = new QAction(HomeWindow);
        action_4->setObjectName(QString::fromUtf8("action_4"));
        action_5 = new QAction(HomeWindow);
        action_5->setObjectName(QString::fromUtf8("action_5"));
        action_6 = new QAction(HomeWindow);
        action_6->setObjectName(QString::fromUtf8("action_6"));
        action_7 = new QAction(HomeWindow);
        action_7->setObjectName(QString::fromUtf8("action_7"));
        action_8 = new QAction(HomeWindow);
        action_8->setObjectName(QString::fromUtf8("action_8"));
        action_10 = new QAction(HomeWindow);
        action_10->setObjectName(QString::fromUtf8("action_10"));
        action_11 = new QAction(HomeWindow);
        action_11->setObjectName(QString::fromUtf8("action_11"));
        action_12 = new QAction(HomeWindow);
        action_12->setObjectName(QString::fromUtf8("action_12"));
        openUrlAction = new QAction(HomeWindow);
        openUrlAction->setObjectName(QString::fromUtf8("openUrlAction"));
        centralWidget = new QWidget(HomeWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        ctrlBar = new QWidget(centralWidget);
        ctrlBar->setObjectName(QString::fromUtf8("ctrlBar"));
        ctrlBar->setGeometry(QRect(0, 520, 1040, 95));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ctrlBar->sizePolicy().hasHeightForWidth());
        ctrlBar->setSizePolicy(sizePolicy);
        ctrlBar->setMinimumSize(QSize(800, 95));
        ctrlBar->setMaximumSize(QSize(16777215, 95));
        playSlider = new CustomSlider(ctrlBar);
        playSlider->setObjectName(QString::fromUtf8("playSlider"));
        playSlider->setGeometry(QRect(10, 8, 781, 20));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(playSlider->sizePolicy().hasHeightForWidth());
        playSlider->setSizePolicy(sizePolicy1);
        playSlider->setMinimumSize(QSize(300, 20));
        playSlider->setMaximumSize(QSize(16777215, 20));
        playSlider->setMaximum(65536);
        playSlider->setOrientation(Qt::Horizontal);
        volumeSlider = new CustomSlider(ctrlBar);
        volumeSlider->setObjectName(QString::fromUtf8("volumeSlider"));
        volumeSlider->setGeometry(QRect(930, 30, 100, 20));
        volumeSlider->setMinimumSize(QSize(100, 20));
        volumeSlider->setMaximumSize(QSize(100, 20));
        volumeSlider->setOrientation(Qt::Horizontal);
        layoutWidget = new QWidget(ctrlBar);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 60, 571, 32));
        horizontalLayout_2 = new QHBoxLayout(layoutWidget);
        horizontalLayout_2->setSpacing(3);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        playOrPauseBtn = new QPushButton(layoutWidget);
        playOrPauseBtn->setObjectName(QString::fromUtf8("playOrPauseBtn"));
        playOrPauseBtn->setMinimumSize(QSize(40, 30));
        playOrPauseBtn->setMaximumSize(QSize(40, 30));

        horizontalLayout_2->addWidget(playOrPauseBtn);

        stopBtn = new QPushButton(layoutWidget);
        stopBtn->setObjectName(QString::fromUtf8("stopBtn"));
        stopBtn->setMinimumSize(QSize(40, 30));
        stopBtn->setMaximumSize(QSize(40, 30));

        horizontalLayout_2->addWidget(stopBtn);

        backFastBtn = new QPushButton(layoutWidget);
        backFastBtn->setObjectName(QString::fromUtf8("backFastBtn"));
        backFastBtn->setMinimumSize(QSize(40, 30));
        backFastBtn->setMaximumSize(QSize(40, 30));

        horizontalLayout_2->addWidget(backFastBtn);

        forwardFastBtn = new QPushButton(layoutWidget);
        forwardFastBtn->setObjectName(QString::fromUtf8("forwardFastBtn"));
        forwardFastBtn->setMinimumSize(QSize(40, 30));
        forwardFastBtn->setMaximumSize(QSize(40, 30));

        horizontalLayout_2->addWidget(forwardFastBtn);

        prevBtn = new QPushButton(layoutWidget);
        prevBtn->setObjectName(QString::fromUtf8("prevBtn"));
        prevBtn->setMinimumSize(QSize(40, 30));
        prevBtn->setMaximumSize(QSize(40, 30));

        horizontalLayout_2->addWidget(prevBtn);

        nextBtn = new QPushButton(layoutWidget);
        nextBtn->setObjectName(QString::fromUtf8("nextBtn"));
        nextBtn->setMinimumSize(QSize(40, 30));
        nextBtn->setMaximumSize(QSize(40, 30));

        horizontalLayout_2->addWidget(nextBtn);

        curPosition = new QLabel(layoutWidget);
        curPosition->setObjectName(QString::fromUtf8("curPosition"));
        curPosition->setMaximumSize(QSize(65, 16777215));

        horizontalLayout_2->addWidget(curPosition);

        label_7 = new QLabel(layoutWidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setMaximumSize(QSize(5, 16777215));

        horizontalLayout_2->addWidget(label_7);

        totalDuration = new QLabel(layoutWidget);
        totalDuration->setObjectName(QString::fromUtf8("totalDuration"));
        totalDuration->setMaximumSize(QSize(65, 16777215));

        horizontalLayout_2->addWidget(totalDuration);

        speedBtn = new QPushButton(layoutWidget);
        speedBtn->setObjectName(QString::fromUtf8("speedBtn"));
        speedBtn->setMaximumSize(QSize(70, 30));

        horizontalLayout_2->addWidget(speedBtn);

        screenBtn = new QPushButton(layoutWidget);
        screenBtn->setObjectName(QString::fromUtf8("screenBtn"));
        screenBtn->setMinimumSize(QSize(40, 30));
        screenBtn->setMaximumSize(QSize(40, 30));

        horizontalLayout_2->addWidget(screenBtn);

        displayRatioBtn = new QPushButton(layoutWidget);
        displayRatioBtn->setObjectName(QString::fromUtf8("displayRatioBtn"));
        displayRatioBtn->setMinimumSize(QSize(40, 30));
        displayRatioBtn->setMaximumSize(QSize(40, 30));

        horizontalLayout_2->addWidget(displayRatioBtn);

        layoutWidget1 = new QWidget(ctrlBar);
        layoutWidget1->setObjectName(QString::fromUtf8("layoutWidget1"));
        layoutWidget1->setGeometry(QRect(612, 37, 291, 51));
        gridLayout = new QGridLayout(layoutWidget1);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        label_5 = new QLabel(layoutWidget1);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setMinimumSize(QSize(50, 0));
        label_5->setMaximumSize(QSize(60, 16777215));

        gridLayout->addWidget(label_5, 0, 0, 1, 1);

        audioBufEdit = new QLineEdit(layoutWidget1);
        audioBufEdit->setObjectName(QString::fromUtf8("audioBufEdit"));
        audioBufEdit->setMinimumSize(QSize(60, 0));
        audioBufEdit->setMaximumSize(QSize(70, 30));

        gridLayout->addWidget(audioBufEdit, 0, 1, 1, 1);

        label_3 = new QLabel(layoutWidget1);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setMinimumSize(QSize(40, 0));
        label_3->setMaximumSize(QSize(60, 16777215));

        gridLayout->addWidget(label_3, 0, 2, 1, 1);

        bufDurationBox = new QComboBox(layoutWidget1);
        bufDurationBox->addItem(QString());
        bufDurationBox->addItem(QString());
        bufDurationBox->addItem(QString());
        bufDurationBox->addItem(QString());
        bufDurationBox->addItem(QString());
        bufDurationBox->addItem(QString());
        bufDurationBox->addItem(QString());
        bufDurationBox->addItem(QString());
        bufDurationBox->addItem(QString());
        bufDurationBox->setObjectName(QString::fromUtf8("bufDurationBox"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(bufDurationBox->sizePolicy().hasHeightForWidth());
        bufDurationBox->setSizePolicy(sizePolicy2);
        bufDurationBox->setMaximumSize(QSize(80, 30));

        gridLayout->addWidget(bufDurationBox, 0, 3, 1, 1);

        label_4 = new QLabel(layoutWidget1);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
        label_4->setSizePolicy(sizePolicy3);
        label_4->setMinimumSize(QSize(50, 0));
        label_4->setMaximumSize(QSize(60, 16777215));

        gridLayout->addWidget(label_4, 1, 0, 1, 1);

        videoBufEdit = new QLineEdit(layoutWidget1);
        videoBufEdit->setObjectName(QString::fromUtf8("videoBufEdit"));
        videoBufEdit->setMinimumSize(QSize(60, 0));
        videoBufEdit->setMaximumSize(QSize(70, 30));

        gridLayout->addWidget(videoBufEdit, 1, 1, 1, 1);

        label_2 = new QLabel(layoutWidget1);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        sizePolicy3.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy3);
        label_2->setMinimumSize(QSize(40, 0));
        label_2->setMaximumSize(QSize(60, 16777215));

        gridLayout->addWidget(label_2, 1, 2, 1, 1);

        jitterBufBox = new QComboBox(layoutWidget1);
        jitterBufBox->addItem(QString());
        jitterBufBox->addItem(QString());
        jitterBufBox->addItem(QString());
        jitterBufBox->addItem(QString());
        jitterBufBox->addItem(QString());
        jitterBufBox->addItem(QString());
        jitterBufBox->addItem(QString());
        jitterBufBox->setObjectName(QString::fromUtf8("jitterBufBox"));
        sizePolicy2.setHeightForWidth(jitterBufBox->sizePolicy().hasHeightForWidth());
        jitterBufBox->setSizePolicy(sizePolicy2);
        jitterBufBox->setMaximumSize(QSize(80, 30));

        gridLayout->addWidget(jitterBufBox, 1, 3, 1, 1);

        listBtn = new QPushButton(ctrlBar);
        listBtn->setObjectName(QString::fromUtf8("listBtn"));
        listBtn->setGeometry(QRect(987, 60, 40, 30));
        listBtn->setMinimumSize(QSize(40, 30));
        listBtn->setMaximumSize(QSize(40, 30));
        settingBtn = new QPushButton(ctrlBar);
        settingBtn->setObjectName(QString::fromUtf8("settingBtn"));
        settingBtn->setGeometry(QRect(940, 60, 40, 30));
        settingBtn->setMinimumSize(QSize(40, 30));
        settingBtn->setMaximumSize(QSize(40, 30));
        lineEdit_3 = new QLineEdit(ctrlBar);
        lineEdit_3->setObjectName(QString::fromUtf8("lineEdit_3"));
        lineEdit_3->setGeometry(QRect(50, 30, 491, 20));
        label = new QLabel(ctrlBar);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 30, 21, 16));
        QFont font;
        font.setPointSize(9);
        label->setFont(font);
        display = new DisplayWind(centralWidget);
        display->setObjectName(QString::fromUtf8("display"));
        display->setGeometry(QRect(0, 0, 800, 520));
        display->setMinimumSize(QSize(400, 240));
        display->setStyleSheet(QString::fromUtf8(""));
        playList = new Playlist(centralWidget);
        playList->setObjectName(QString::fromUtf8("playList"));
        playList->setGeometry(QRect(800, 0, 240, 520));
        playList->setMinimumSize(QSize(120, 0));
        playList->setMaximumSize(QSize(240, 16777215));
        playList->setStyleSheet(QString::fromUtf8(""));
        HomeWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(HomeWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1040, 18));
        menu_2 = new QMenu(menuBar);
        menu_2->setObjectName(QString::fromUtf8("menu_2"));
        menu_3 = new QMenu(menu_2);
        menu_3->setObjectName(QString::fromUtf8("menu_3"));
        menu_4 = new QMenu(menu_2);
        menu_4->setObjectName(QString::fromUtf8("menu_4"));
        menu = new QMenu(menuBar);
        menu->setObjectName(QString::fromUtf8("menu"));
        HomeWindow->setMenuBar(menuBar);

        menuBar->addAction(menu->menuAction());
        menuBar->addAction(menu_2->menuAction());
        menu_2->addAction(menu_3->menuAction());
        menu_2->addAction(menu_4->menuAction());
        menu_3->addAction(action_6);
        menu_3->addAction(action_7);
        menu_3->addAction(action_8);
        menu_4->addAction(action_10);
        menu_4->addAction(action_11);
        menu_4->addAction(action_12);
        menu->addAction(openFileAction);
        menu->addAction(openUrlAction);

        retranslateUi(HomeWindow);

        QMetaObject::connectSlotsByName(HomeWindow);
    } // setupUi

    void retranslateUi(QMainWindow *HomeWindow)
    {
        HomeWindow->setWindowTitle(QCoreApplication::translate("HomeWindow", "NovaPlay", nullptr));
        openFileAction->setText(QCoreApplication::translate("HomeWindow", "\346\211\223\345\274\200\346\226\207\344\273\266", nullptr));
        action_3->setText(QCoreApplication::translate("HomeWindow", "\345\217\215\350\275\254", nullptr));
        action_4->setText(QCoreApplication::translate("HomeWindow", "\346\232\226\350\211\262", nullptr));
        action_5->setText(QCoreApplication::translate("HomeWindow", "\345\206\267\350\211\262", nullptr));
        action_6->setText(QCoreApplication::translate("HomeWindow", "\346\227\213\350\275\254", nullptr));
        action_7->setText(QCoreApplication::translate("HomeWindow", "\347\277\273\350\275\254", nullptr));
        action_8->setText(QCoreApplication::translate("HomeWindow", "\346\255\243\345\270\270", nullptr));
        action_10->setText(QCoreApplication::translate("HomeWindow", "\346\232\226\350\211\262", nullptr));
        action_11->setText(QCoreApplication::translate("HomeWindow", "\345\206\267\350\211\262", nullptr));
        action_12->setText(QCoreApplication::translate("HomeWindow", "\346\255\243\345\270\270", nullptr));
        openUrlAction->setText(QCoreApplication::translate("HomeWindow", "\346\211\223\345\274\200\347\275\221\347\273\234\344\270\262\346\265\201", nullptr));
        playOrPauseBtn->setText(QCoreApplication::translate("HomeWindow", "\346\222\255\346\224\276", nullptr));
        stopBtn->setText(QCoreApplication::translate("HomeWindow", "\345\201\234\346\255\242", nullptr));
        backFastBtn->setText(QCoreApplication::translate("HomeWindow", "\345\277\253\351\200\200", nullptr));
        forwardFastBtn->setText(QCoreApplication::translate("HomeWindow", "\345\277\253\350\277\233", nullptr));
        prevBtn->setText(QCoreApplication::translate("HomeWindow", "\345\211\215\344\270\200", nullptr));
        nextBtn->setText(QCoreApplication::translate("HomeWindow", "\345\220\216\344\270\200", nullptr));
        curPosition->setText(QCoreApplication::translate("HomeWindow", "20:10:00", nullptr));
        label_7->setText(QCoreApplication::translate("HomeWindow", "/", nullptr));
        totalDuration->setText(QCoreApplication::translate("HomeWindow", "24:00:00", nullptr));
        speedBtn->setText(QCoreApplication::translate("HomeWindow", "1.0\345\200\215\351\200\237", nullptr));
        screenBtn->setText(QCoreApplication::translate("HomeWindow", "\346\210\252\345\261\217", nullptr));
        displayRatioBtn->setText(QCoreApplication::translate("HomeWindow", "16:9", nullptr));
        label_5->setText(QCoreApplication::translate("HomeWindow", "\351\237\263\351\242\221\347\274\223\345\255\230", nullptr));
        audioBufEdit->setText(QCoreApplication::translate("HomeWindow", "\351\237\263\351\242\221\347\274\223\345\255\230", nullptr));
        label_3->setText(QCoreApplication::translate("HomeWindow", "\347\274\223\345\255\230\351\230\210\345\200\274", nullptr));
        bufDurationBox->setItemText(0, QCoreApplication::translate("HomeWindow", "30ms", nullptr));
        bufDurationBox->setItemText(1, QCoreApplication::translate("HomeWindow", "100ms", nullptr));
        bufDurationBox->setItemText(2, QCoreApplication::translate("HomeWindow", "200ms", nullptr));
        bufDurationBox->setItemText(3, QCoreApplication::translate("HomeWindow", "400ms", nullptr));
        bufDurationBox->setItemText(4, QCoreApplication::translate("HomeWindow", "600ms", nullptr));
        bufDurationBox->setItemText(5, QCoreApplication::translate("HomeWindow", "800ms", nullptr));
        bufDurationBox->setItemText(6, QCoreApplication::translate("HomeWindow", "1000ms", nullptr));
        bufDurationBox->setItemText(7, QCoreApplication::translate("HomeWindow", "2000ms", nullptr));
        bufDurationBox->setItemText(8, QCoreApplication::translate("HomeWindow", "4000ms", nullptr));

        label_4->setText(QCoreApplication::translate("HomeWindow", "\350\247\206\351\242\221\347\274\223\345\255\230", nullptr));
        videoBufEdit->setText(QCoreApplication::translate("HomeWindow", "\350\247\206\351\242\221\347\274\223\345\255\230", nullptr));
        label_2->setText(QCoreApplication::translate("HomeWindow", "\346\212\226\345\212\250\345\200\274", nullptr));
        jitterBufBox->setItemText(0, QCoreApplication::translate("HomeWindow", "30ms", nullptr));
        jitterBufBox->setItemText(1, QCoreApplication::translate("HomeWindow", "100ms", nullptr));
        jitterBufBox->setItemText(2, QCoreApplication::translate("HomeWindow", "200ms", nullptr));
        jitterBufBox->setItemText(3, QCoreApplication::translate("HomeWindow", "400ms", nullptr));
        jitterBufBox->setItemText(4, QCoreApplication::translate("HomeWindow", "600ms", nullptr));
        jitterBufBox->setItemText(5, QCoreApplication::translate("HomeWindow", "800ms", nullptr));
        jitterBufBox->setItemText(6, QCoreApplication::translate("HomeWindow", "1000ms", nullptr));

        listBtn->setText(QCoreApplication::translate("HomeWindow", "\345\210\227\350\241\250", nullptr));
        settingBtn->setText(QCoreApplication::translate("HomeWindow", "\350\256\276\347\275\256", nullptr));
        label->setText(QCoreApplication::translate("HomeWindow", "URL", nullptr));
        menu_2->setTitle(QCoreApplication::translate("HomeWindow", "\347\224\273\351\235\242", nullptr));
        menu_3->setTitle(QCoreApplication::translate("HomeWindow", "\346\226\271\345\220\221", nullptr));
        menu_4->setTitle(QCoreApplication::translate("HomeWindow", "\350\211\262\350\260\203", nullptr));
        menu->setTitle(QCoreApplication::translate("HomeWindow", "\346\226\207\344\273\266", nullptr));
    } // retranslateUi

};

namespace Ui {
    class HomeWindow: public Ui_HomeWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HOMEWINDOW_H
