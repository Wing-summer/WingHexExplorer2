#ifndef CODEEDITCONTROLWIDGET_H
#define CODEEDITCONTROLWIDGET_H

#include <QAbstractButton>
#include <QWidget>

class CodeEditControlWidget : public QWidget {
    Q_OBJECT
public:
    explicit CodeEditControlWidget(QWidget *parent = nullptr);

public:
    virtual QSize sizeHint() const override;
    virtual void accept();
    virtual void cancel();

    QAbstractButton *defaultButton() const;
    void setDefaultButton(QAbstractButton *newDefaultButton);

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;

private:
    QAbstractButton *m_defaultButton = nullptr;
};

#endif // CODEEDITCONTROLWIDGET_H
