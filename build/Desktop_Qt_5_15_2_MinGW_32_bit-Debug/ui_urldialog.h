/********************************************************************************
** Form generated from reading UI file 'urldialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_URLDIALOG_H
#define UI_URLDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>

QT_BEGIN_NAMESPACE

class Ui_UrlDialog
{
public:
    QDialogButtonBox *buttonBox;
    QLineEdit *urlLineEdit;
    QLabel *label;

    void setupUi(QDialog *UrlDialog)
    {
        if (UrlDialog->objectName().isEmpty())
            UrlDialog->setObjectName(QString::fromUtf8("UrlDialog"));
        UrlDialog->resize(653, 157);
        buttonBox = new QDialogButtonBox(UrlDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(220, 100, 301, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        urlLineEdit = new QLineEdit(UrlDialog);
        urlLineEdit->setObjectName(QString::fromUtf8("urlLineEdit"));
        urlLineEdit->setGeometry(QRect(30, 50, 591, 31));
        label = new QLabel(UrlDialog);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(40, 20, 231, 16));

        retranslateUi(UrlDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), UrlDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), UrlDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(UrlDialog);
    } // setupUi

    void retranslateUi(QDialog *UrlDialog)
    {
        UrlDialog->setWindowTitle(QCoreApplication::translate("UrlDialog", "Dialog", nullptr));
        label->setText(QCoreApplication::translate("UrlDialog", "\350\276\223\345\205\245\347\275\221\347\273\234\345\234\260\345\235\200,\346\257\224\345\246\202http/rtmp", nullptr));
    } // retranslateUi

};

namespace Ui {
    class UrlDialog: public Ui_UrlDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_URLDIALOG_H
