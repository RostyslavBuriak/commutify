#pragma once

#ifndef SCROLLAREAWIDGET_H
#define SCROLLAREAWIDGET_H

#include <QWidget>

class ScrollAreaWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScrollAreaWidget(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *) override;
signals:
    void upd();
};

#endif // SCROLLAREAWIDGET_H
