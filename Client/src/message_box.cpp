#include "message_box.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QTime>
#include <QTextBrowser>
#include <QDebug>
#include <QStyleOption>
#include <QPainter>



MessageBox::MessageBox(int mId, QString sender, QString messageText, QTime time, QWidget *parent) : QWidget(parent)
{
    QLabel *senderLabel = new QLabel();
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QLabel *msgTime = new QLabel();

    setStyleSheet("MessageBox {background-color: #FFFFFF; border-radius: 10px;}");
    mainLayout->addWidget(senderLabel);

    contentType = "text";
    senderLabel->setText(sender);
    senderLabel->setStyleSheet("font-size: 12px; color: #6874E8; font-weight: 600;");

    msgId = mId;

    QLabel *msgText = new QLabel();
    msgText->setStyleSheet("font-size: 14px; color: #333333; font-weight: normal;");
    msgText->setText(messageText);
    msgText->setWordWrap(true);

    msgText->setTextInteractionFlags(Qt::TextSelectableByMouse);
    mainLayout->addWidget(msgText);

    if (contentType == "photo") {
        QLabel *photo = new QLabel();
        QPixmap p;
        photo->setPixmap(p);
        mainLayout->addWidget(photo);
    }

    msgTime->setText(time.toString("hh:mm"));
    msgTime->setStyleSheet("font-size: 11px; color: #828282; font-weight: 600;");

    mainLayout->addWidget(msgTime);


    QFontMetrics fm(senderLabel->font());
    setMinimumWidth(fm.horizontalAdvance(senderLabel->text()));
    setMaximumWidth(290);

    setContentsMargins(12, 5, 5, 12);
    setLayout(mainLayout);

}


MessageBox::~MessageBox()
{
}

void MessageBox::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}


