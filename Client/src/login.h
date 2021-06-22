#pragma once

#ifndef LOGIN_H
#define LOGIN_H

#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QKeyEvent>

class LoginField : public QLineEdit
{
    Q_OBJECT
protected:
    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
            emit ret();
        }
        QLineEdit::keyPressEvent(event);
    };
signals:
    void ret();
};


class Login : public QWidget
{
    Q_OBJECT

public:
    QVBoxLayout *loginLayout = new QVBoxLayout(), *loginFormLayout = new QVBoxLayout();
    QSpacerItem *topSpacer = new QSpacerItem(20,40), *bottomSpacer = new QSpacerItem(20, 40);
    QFrame *loginForm = new QFrame();
    QLabel *loginText1 = new QLabel(), *loginText2 = new QLabel(), *copyright = new QLabel(), *version = new QLabel();
    LoginField *loginField = new LoginField(), *passwordField = new LoginField();
    QPushButton *loginButton = new QPushButton();
    QWidget *mainWidget = new QWidget();

    explicit Login(QWidget *parent = nullptr);
    ~Login();

    void redrawLoginField(bool);
    void redrawPasswordField(bool);
    void handle_login_failed();

signals:
    void login_field_correct(bool);
    void pass_field_correct(bool);
    void login_failed();

};



#endif // LOGIN_H
