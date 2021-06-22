#pragma once

#include <QObject>
#include <QTextEdit>

class InputField : public QTextEdit
{
    Q_OBJECT
    using super = QTextEdit;
public:
    explicit InputField(QWidget *parent = nullptr);
    QSize sizeHint() const override;
protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
signals:
    void ret();
};
