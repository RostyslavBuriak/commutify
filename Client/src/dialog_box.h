#pragma once

#ifndef DIALOGBOX_H
#define DIALOGBOX_H

#include <QWidget>
#include <QLabel>
#include <QTime>
#include <QGridLayout>

class DialogBox : public QWidget
{
    Q_OBJECT
public:
    explicit DialogBox(int chatId, QString chatName, QWidget *parent = nullptr);
    ~DialogBox();
    void updateMsgText(QString text, QString time);
    QLabel *chatTitle = new QLabel();
private:
    bool isSelected;
    int _id;
    QLabel *msgText = new QLabel();
    QLabel *msgTime = new QLabel();
    QLabel *unreadBadge = new QLabel();
    QTime *time = new QTime(QTime::currentTime());
    QGridLayout *boxLayout = new QGridLayout();

    void setSelection();
    void clearSelection();
    void cropText();
    void createDialog();
    void paintEvent(QPaintEvent *);
protected:
    void mousePressEvent(QMouseEvent *event);
};

#endif // DIALOGBOX_H
