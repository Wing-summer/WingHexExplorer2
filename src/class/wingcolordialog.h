#ifndef WINGCOLORDIALOG_H
#define WINGCOLORDIALOG_H

#include <QColor>
#include <QWidget>

class WingColorDialog {
public:
    static QColor getColor(const QString &caption, QWidget *parent = nullptr);

private:
    WingColorDialog();

    Q_DISABLE_COPY_MOVE(WingColorDialog)
};

#endif // WINGCOLORDIALOG_H
