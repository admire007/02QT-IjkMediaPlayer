/********************************************************************************
** Form generated from reading UI file 'displaywind.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DISPLAYWIND_H
#define UI_DISPLAYWIND_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DisplayWind
{
public:
    QGridLayout *gridLayout;

    void setupUi(QWidget *DisplayWind)
    {
        if (DisplayWind->objectName().isEmpty())
            DisplayWind->setObjectName(QString::fromUtf8("DisplayWind"));
        DisplayWind->resize(771, 471);
        gridLayout = new QGridLayout(DisplayWind);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));

        retranslateUi(DisplayWind);

        QMetaObject::connectSlotsByName(DisplayWind);
    } // setupUi

    void retranslateUi(QWidget *DisplayWind)
    {
        DisplayWind->setWindowTitle(QCoreApplication::translate("DisplayWind", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DisplayWind: public Ui_DisplayWind {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DISPLAYWIND_H
