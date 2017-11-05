/********************************************************************************
** Form generated from reading UI file 'FTP_Client.ui'
**
** Created by: Qt User Interface Compiler version 5.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FTP_CLIENT_H
#define UI_FTP_CLIENT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FTP_Client
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QVBoxLayout *mainLayout;
    QHBoxLayout *inputLayout;
    QVBoxLayout *infoLayout;
    QHBoxLayout *addressLayout;
    QLabel *ipLabel;
    QLineEdit *ipLineEdit;
    QLabel *portLabel;
    QLineEdit *portLineEdit;
    QHBoxLayout *userLayout;
    QLabel *userLabel;
    QLineEdit *userLineEdit;
    QLabel *passwdLabel;
    QLineEdit *passwdLineEdit;
    QVBoxLayout *buttonLayout;
    QPushButton *connectButton;
    QPushButton *disconnectButton;
    QHBoxLayout *commandLayout;
    QLabel *commandLabel;
    QRadioButton *pasvRadioButton;
    QRadioButton *portRadioButton;
    QTextEdit *commandTextEdit;
    QLabel *label;
    QTableWidget *fileTableWidget;
    QLabel *label_2;
    QTableWidget *taskTableWidget;
    QLabel *label_3;

    void setupUi(QMainWindow *FTP_ClientClass)
    {
        if (FTP_ClientClass->objectName().isEmpty())
            FTP_ClientClass->setObjectName(QStringLiteral("FTP_ClientClass"));
        FTP_ClientClass->resize(600, 800);
        centralWidget = new QWidget(FTP_ClientClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        mainLayout = new QVBoxLayout();
        mainLayout->setSpacing(6);
        mainLayout->setObjectName(QStringLiteral("mainLayout"));
        mainLayout->setContentsMargins(0, -1, -1, -1);
        inputLayout = new QHBoxLayout();
        inputLayout->setSpacing(6);
        inputLayout->setObjectName(QStringLiteral("inputLayout"));
        infoLayout = new QVBoxLayout();
        infoLayout->setSpacing(6);
        infoLayout->setObjectName(QStringLiteral("infoLayout"));
        addressLayout = new QHBoxLayout();
        addressLayout->setSpacing(6);
        addressLayout->setObjectName(QStringLiteral("addressLayout"));
        ipLabel = new QLabel(centralWidget);
        ipLabel->setObjectName(QStringLiteral("ipLabel"));
        QFont font;
        font.setFamily(QStringLiteral("Microsoft YaHei UI Light"));
        font.setPointSize(12);
        ipLabel->setFont(font);

        addressLayout->addWidget(ipLabel);

        ipLineEdit = new QLineEdit(centralWidget);
        ipLineEdit->setObjectName(QStringLiteral("ipLineEdit"));
        QFont font1;
        font1.setFamily(QStringLiteral("Microsoft YaHei UI"));
        font1.setPointSize(10);
        ipLineEdit->setFont(font1);
        ipLineEdit->setEchoMode(QLineEdit::Normal);

        addressLayout->addWidget(ipLineEdit);

        portLabel = new QLabel(centralWidget);
        portLabel->setObjectName(QStringLiteral("portLabel"));
        portLabel->setFont(font);

        addressLayout->addWidget(portLabel);

        portLineEdit = new QLineEdit(centralWidget);
        portLineEdit->setObjectName(QStringLiteral("portLineEdit"));
        portLineEdit->setFont(font1);

        addressLayout->addWidget(portLineEdit);


        infoLayout->addLayout(addressLayout);

        userLayout = new QHBoxLayout();
        userLayout->setSpacing(6);
        userLayout->setObjectName(QStringLiteral("userLayout"));
        userLabel = new QLabel(centralWidget);
        userLabel->setObjectName(QStringLiteral("userLabel"));
        userLabel->setFont(font);

        userLayout->addWidget(userLabel);

        userLineEdit = new QLineEdit(centralWidget);
        userLineEdit->setObjectName(QStringLiteral("userLineEdit"));
        userLineEdit->setFont(font1);

        userLayout->addWidget(userLineEdit);

        passwdLabel = new QLabel(centralWidget);
        passwdLabel->setObjectName(QStringLiteral("passwdLabel"));
        passwdLabel->setFont(font);

        userLayout->addWidget(passwdLabel);

        passwdLineEdit = new QLineEdit(centralWidget);
        passwdLineEdit->setObjectName(QStringLiteral("passwdLineEdit"));
        passwdLineEdit->setFont(font1);
        passwdLineEdit->setEchoMode(QLineEdit::Password);

        userLayout->addWidget(passwdLineEdit);


        infoLayout->addLayout(userLayout);


        inputLayout->addLayout(infoLayout);

        buttonLayout = new QVBoxLayout();
        buttonLayout->setSpacing(6);
        buttonLayout->setObjectName(QStringLiteral("buttonLayout"));
        connectButton = new QPushButton(centralWidget);
        connectButton->setObjectName(QStringLiteral("connectButton"));
        connectButton->setFont(font);

        buttonLayout->addWidget(connectButton);

        disconnectButton = new QPushButton(centralWidget);
        disconnectButton->setObjectName(QStringLiteral("disconnectButton"));
        disconnectButton->setFont(font);

        buttonLayout->addWidget(disconnectButton);


        inputLayout->addLayout(buttonLayout);


        mainLayout->addLayout(inputLayout);

        commandLayout = new QHBoxLayout();
        commandLayout->setSpacing(6);
        commandLayout->setObjectName(QStringLiteral("commandLayout"));
        commandLabel = new QLabel(centralWidget);
        commandLabel->setObjectName(QStringLiteral("commandLabel"));
        commandLabel->setFont(font);

        commandLayout->addWidget(commandLabel);

        pasvRadioButton = new QRadioButton(centralWidget);
        pasvRadioButton->setObjectName(QStringLiteral("pasvRadioButton"));
        pasvRadioButton->setFont(font);

        commandLayout->addWidget(pasvRadioButton);

        portRadioButton = new QRadioButton(centralWidget);
        portRadioButton->setObjectName(QStringLiteral("portRadioButton"));
        portRadioButton->setFont(font);

        commandLayout->addWidget(portRadioButton);


        mainLayout->addLayout(commandLayout);

        commandTextEdit = new QTextEdit(centralWidget);
        commandTextEdit->setObjectName(QStringLiteral("commandTextEdit"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(commandTextEdit->sizePolicy().hasHeightForWidth());
        commandTextEdit->setSizePolicy(sizePolicy);
        QFont font2;
        font2.setFamily(QStringLiteral("Consolas"));
        font2.setPointSize(12);
        commandTextEdit->setFont(font2);
        commandTextEdit->setReadOnly(true);

        mainLayout->addWidget(commandTextEdit);

        label = new QLabel(centralWidget);
        label->setObjectName(QStringLiteral("label"));
        label->setFont(font);

        mainLayout->addWidget(label);

        fileTableWidget = new QTableWidget(centralWidget);
        fileTableWidget->setObjectName(QStringLiteral("fileTableWidget"));
        QFont font3;
        font3.setFamily(QStringLiteral("Microsoft YaHei UI"));
        fileTableWidget->setFont(font3);

        mainLayout->addWidget(fileTableWidget);

        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setFont(font);

        mainLayout->addWidget(label_2);

        taskTableWidget = new QTableWidget(centralWidget);
        taskTableWidget->setObjectName(QStringLiteral("taskTableWidget"));
        sizePolicy.setHeightForWidth(taskTableWidget->sizePolicy().hasHeightForWidth());
        taskTableWidget->setSizePolicy(sizePolicy);
        taskTableWidget->setFont(font3);

        mainLayout->addWidget(taskTableWidget);

        label_3 = new QLabel(centralWidget);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setFont(font1);
        label_3->setAlignment(Qt::AlignCenter);

        mainLayout->addWidget(label_3);


        verticalLayout->addLayout(mainLayout);

        FTP_ClientClass->setCentralWidget(centralWidget);

        retranslateUi(FTP_ClientClass);

        QMetaObject::connectSlotsByName(FTP_ClientClass);
    } // setupUi

    void retranslateUi(QMainWindow *FTP_ClientClass)
    {
        FTP_ClientClass->setWindowTitle(QApplication::translate("FTP_ClientClass", "FTP_Client", Q_NULLPTR));
        ipLabel->setText(QApplication::translate("FTP_ClientClass", "IP:", Q_NULLPTR));
        portLabel->setText(QApplication::translate("FTP_ClientClass", "Port:", Q_NULLPTR));
        portLineEdit->setPlaceholderText(QApplication::translate("FTP_ClientClass", "21", Q_NULLPTR));
        userLabel->setText(QApplication::translate("FTP_ClientClass", "User:", Q_NULLPTR));
        userLineEdit->setPlaceholderText(QApplication::translate("FTP_ClientClass", "anonymous", Q_NULLPTR));
        passwdLabel->setText(QApplication::translate("FTP_ClientClass", "Password:", Q_NULLPTR));
        connectButton->setText(QApplication::translate("FTP_ClientClass", "Connect", Q_NULLPTR));
        disconnectButton->setText(QApplication::translate("FTP_ClientClass", "Disconnect", Q_NULLPTR));
        commandLabel->setText(QApplication::translate("FTP_ClientClass", "Command Prompt:", Q_NULLPTR));
        pasvRadioButton->setText(QApplication::translate("FTP_ClientClass", "PASV mode", Q_NULLPTR));
        portRadioButton->setText(QApplication::translate("FTP_ClientClass", "PORT mode", Q_NULLPTR));
        label->setText(QApplication::translate("FTP_ClientClass", "File List:", Q_NULLPTR));
        label_2->setText(QApplication::translate("FTP_ClientClass", "Tasks:", Q_NULLPTR));
        label_3->setText(QApplication::translate("FTP_ClientClass", "\302\251Bill Jia 2017", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class FTP_Client: public Ui_FTP_Client {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FTP_CLIENT_H
