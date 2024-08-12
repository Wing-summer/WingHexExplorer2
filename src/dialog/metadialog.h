#ifndef METADIALOG_H
#define METADIALOG_H

#include "framelessdialogbase.h"

#include <QCheckBox>
#include <QColor>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QPalette>

#include "control/huecolorpickerslider.h"

class MetaDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    explicit MetaDialog(QWidget *parent);
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
    QCheckBox *cforeground, *cbackground, *ccomment;
    QLineEdit *m_comment;
    HueColorPickerSlider *iforeground, *ibackground;

    QColor _foreground;
    QColor _background;
    QString _comment;
};

#endif // METADIALOG_H
