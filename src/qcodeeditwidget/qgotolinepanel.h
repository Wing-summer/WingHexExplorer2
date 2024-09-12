#ifndef QGOTOLINEPANEL_H
#define QGOTOLINEPANEL_H

#include "qpanel.h"

#include <QWidget>

namespace Ui {
class QGotoLinePanel;
}

class QGotoLinePanel : public QPanel {
    Q_OBJECT

public:
    Q_PANEL(QGotoLinePanel, "Goto Line Panel")

    explicit QGotoLinePanel(QWidget *parent = nullptr);
    virtual ~QGotoLinePanel();

    virtual QString type() const;

protected:
    virtual bool forward(QMouseEvent *e);
    virtual void editorChange(QEditor *e);
    virtual void showEvent(QShowEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);

private slots:
    void on_bClose_clicked();

    void on_bGo_clicked();

    void on_spLine_valueChanged(int v);
    void on_slLine_valueChanged(int v);

    void lineCountChanged(int n);
    void cursorPositionChanged();

private:
    Ui::QGotoLinePanel *ui;
};

#endif // QGOTOLINEPANEL_H
