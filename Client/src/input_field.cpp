#include "input_field.h"
#include "main_window.h"

#include <QKeyEvent>
#include <QShortcut>

InputField::InputField(QWidget *parent) : super(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
//    setLineWrapMode();
}

QSize InputField::sizeHint() const {
    QSize s(document()->size().toSize());

    s.rwidth() = std::max(100, s.width());
    s.rheight() = std::max(100, s.height());
    return(s);
}

void InputField::resizeEvent(QResizeEvent *event) {
    updateGeometry();
    super::resizeEvent(event);
}

void InputField::keyPressEvent(QKeyEvent *event) {
    super::keyPressEvent(event);
    if (event->modifiers() & Qt::Key_Shift && event->key() == Qt::Key_Enter) {
        super::keyPressEvent(event);
    } else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        emit ret();
    }

}
