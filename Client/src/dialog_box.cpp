#include "dialog_box.h"

#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>

DialogBox::DialogBox(int chatId, QString chatName, QWidget *parent) : QWidget(parent)
{

    isSelected = false;
    _id = chatId;

    setStyleSheet("DialogBox {background-color: #FFFFFF; border-radius: 10px;};");
    setCursor(Qt::PointingHandCursor);

    boxLayout->addWidget(chatTitle, 0, 1);
    boxLayout->addWidget(msgText, 1, 1);
    boxLayout->addWidget(msgTime, 2, 1);

    chatTitle->setStyleSheet("font-weight: bold; font-size: 14px; padding-top: 2px;");
    chatTitle->setText(chatName);

    msgText->setStyleSheet("font-size: 12px; padding-left: 1px;");
    msgText->setText("No messages here yet...");

    msgTime->setStyleSheet("font-size: 10px;");

    boxLayout->setSpacing(0);
    boxLayout->setContentsMargins(10, 0, 0, 0);


    setLayout(boxLayout);

    cropText();
    setFixedSize(275, 60);
    setAutoFillBackground(true);

    setSelection();
}

void DialogBox::setSelection() {
    setStyleSheet("DialogBox {background-color: #6874E8; border-radius: 10px;};");
    chatTitle->setStyleSheet("font-weight: bold; font-size: 14px; color: #FFFFFF; padding-top: 2px;");
    msgText->setStyleSheet("font-size: 12px; color: #FFFFFF; padding-left: 1px;");
    msgTime->setStyleSheet("font-size: 10px; color: #FFFFFF; padding-left: 1px;");
}

void DialogBox::clearSelection() {
    setStyleSheet("DialogBox {background-color: #FFFFFF; border-radius: 10px;};");
    chatTitle->setStyleSheet("font-weight: bold; font-size: 14px; color: #000000; padding-top: 2px;");
    msgText->setStyleSheet("font-size: 12px; color: #000000; padding-left: 1px;");
    msgTime->setStyleSheet("font-size: 10px; color: #000000; padding-left: 1px ;");
}

void DialogBox::mousePressEvent(QMouseEvent *) {
    if (isSelected) {
        clearSelection();
        isSelected = false;
    } else {
        setSelection();
        isSelected = true;
    }
}

void DialogBox::cropText() {
    QFontMetrics fm(msgText->font());
    if (fm.horizontalAdvance(msgText->text()) > 245) {
        QString text = "";
        for (const auto& i: msgText->text()) {
            if (fm.horizontalAdvance(text + i) <= 235) {
                text += i;
            } else {
                break;
            }
        }
        text += "...";
        msgText->setText(text);
    }
}

void DialogBox::updateMsgText(QString text, QString time) {
    msgText->setText(text);
    msgTime->setText(time);
    cropText();
}

void DialogBox::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

DialogBox::~DialogBox()
{
}
