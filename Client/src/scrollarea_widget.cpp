#include "scrollarea_widget.h"

ScrollAreaWidget::ScrollAreaWidget(QWidget *parent) : QWidget(parent)
{
}

void ScrollAreaWidget::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);
    emit upd();
}
