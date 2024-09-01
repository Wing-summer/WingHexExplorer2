#ifndef GOTOWIDGET_H
#define GOTOWIDGET_H

#include <QWidget>

namespace Ui {
class GotoWidget;
}

class GotoWidget : public QWidget {
    Q_OBJECT

    enum SEEKPOS { Invaild, Start, End, Relative };

public:
    explicit GotoWidget(QWidget *parent = nullptr);
    ~GotoWidget();

public slots:
    void activeInput(qsizetype oldrow, qsizetype oldcolumn, qsizetype oldoffset,
                     qsizetype maxfilebytes, qsizetype maxfilelines);

    void handleLineChanged();

    void jumpCancel();

    void jumpConfirm();

signals:
    void jumpToLine(qsizetype pos, bool isline);
    void pressEsc();

private slots:
    void on_btnClose_clicked();

    void on_btnGoto_clicked();

private:
    qsizetype convert2Pos(QString value, SEEKPOS &ps, bool isline);

private:
    Ui::GotoWidget *ui;

    qsizetype m_rowBeforeJump = 0;
    qsizetype m_columnBeforeJump = 0;
    qsizetype m_oldFileOffsetBeforeJump = 0;
    qsizetype m_maxFileBytes = 0;
    qsizetype m_maxFilelines = 0;
};

#endif // GOTOWIDGET_H
