#include "login.h"
#include "connection.h"

#include <QFontDatabase>
#include <QStyle>
#include <QTextStream>


Login::Login(QWidget *parent) :
    QWidget(parent)
{
    QFontDatabase::addApplicationFont(":/font/resources/Nunito/Nunito-Regular.ttf");


    connect(loginField, &LoginField::ret, this, [=] {emit loginButton->clicked();});
    connect(passwordField, &LoginField::ret, this, [=] {emit loginButton->clicked();});

    loginForm->setContentsMargins(70, 85, 70, 85);
    loginForm->setMinimumSize(366, 399);
    loginForm->setMaximumSize(610, 664);
    loginForm->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    loginForm->setLayout(loginFormLayout);


//    loginField->setPlaceholderText("Identyfikator");
    loginField->setPlaceholderText("ID");
    loginField->setProperty("valid", true);
//    passwordField->setPlaceholderText("Hasło");
    passwordField->setPlaceholderText("Password");
    passwordField->setProperty("valid", true);
    passwordField->setEchoMode(QLineEdit::Password);

    loginFormLayout->addWidget(loginText1);
    loginFormLayout->addWidget(loginText2);
    loginFormLayout->addWidget(loginField);
    loginFormLayout->addWidget(passwordField);
    loginFormLayout->addWidget(loginButton);

//    loginButton->setText("ZALOGUJ");
    loginButton->setText("LOG IN");
    loginButton->setCursor(Qt::PointingHandCursor);

//    loginText1->setText("Cześć!");
    loginText1->setText("Hello!");
    loginText1->setObjectName("loginText1");
    loginText1->setFixedSize(140, 62);
//    loginText2->setText("Zaloguj się aby kontynuować");
    loginText2->setText("Log in to continue");
    loginText2->setObjectName("loginText2");
    loginText2->setFixedSize(407, 42);


    copyright->setText("© COMMUTIFY");
    copyright->setObjectName("copyright");
    copyright->setFixedSize(139, 26);

    version->setText("v 0.1");
    version->setObjectName("version");
    version->setFixedSize(40, 23);

    loginLayout->addSpacerItem(topSpacer);
    loginLayout->addWidget(loginForm, 0, Qt::AlignHCenter);
    loginLayout->addWidget(copyright, 0, Qt::AlignHCenter);
    loginLayout->addWidget(version, 0, Qt::AlignHCenter);
    loginLayout->addSpacerItem(bottomSpacer);

    setObjectName("loginWidget");

    setLayout(loginLayout);
    setFixedSize(1200, 600);

}

void Login::redrawLoginField(bool valid) {
    loginField->setProperty("valid", valid);
    loginField->style()->unpolish(loginField);
    loginField->style()->polish(loginField);
}

void Login::redrawPasswordField(bool valid) {
    passwordField->setProperty("valid", valid);
    passwordField->style()->unpolish(passwordField);
    passwordField->style()->polish(passwordField);
}

void Login::handle_login_failed() {
    QLabel *fail_warn = new QLabel();
//    fail_warn->setText("Nieprawidłowy login lub hasło");
    fail_warn->setText("Login credentials are incorrect");
    fail_warn->setObjectName("loginFail");
    loginFormLayout->insertWidget(4, fail_warn);
}


Login::~Login()
{
}
