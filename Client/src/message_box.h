#pragma once

#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QWidget>
#include <QTime>

class MessageBox : public QWidget
{
    Q_OBJECT
public:
    explicit MessageBox(int msgId, QString sender, QString messageText, QTime time, QWidget *parent = nullptr);
    ~MessageBox();
    int msgId;
private:
    QString contentType;
    void paintEvent(QPaintEvent *) override;
signals:

};

#endif // MESSAGEBOX_H
