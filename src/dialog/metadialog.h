#ifndef METADIALOG_H
#define METADIALOG_H

#include "framelessdialog.h"

#include <QCheckBox>
#include <QColor>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QPalette>
#include <QPushButton>

class MetaDialog : public FramelessDialog {
    Q_OBJECT
public:
    explicit MetaDialog(QWidget *parent = nullptr);
    QColor foreGroundColor();
    QColor backGroundColor();
    QString comment();

    void setForeGroundColor(QColor color);
    void setBackGroundColor(QColor color);
    void setComment(QString comment);

private:
    void on_accept();
    void on_reject();

private:
    void setPaintedColor(QPushButton *btn, const QColor &color);

private:
    QCheckBox *cforeground, *cbackground, *ccomment;
    QLineEdit *m_comment;
    QPushButton *iforeground, *ibackground;

    QColor _foreground;
    QColor _background;
    QString _comment;
};

#endif // METADIALOG_H
