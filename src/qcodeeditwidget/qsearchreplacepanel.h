#ifndef QSEARCHREPLACEPANEL_H
#define QSEARCHREPLACEPANEL_H

#include "qpanel.h"
#include <QWidget>

class QDocumentLine;
class QDocumentSearch;

namespace Ui {
class QSearchReplacePanel;
}

class QSearchReplacePanel : public QPanel {
    Q_OBJECT

public:
    Q_PANEL(QSearchReplacePanel, "Search Replace Panel")

    explicit QSearchReplacePanel(QWidget *parent = nullptr);
    virtual ~QSearchReplacePanel();

    virtual QString type() const;

public slots:
    void display(int mode, bool replace);

    void find(int backward = -1);

protected:
    virtual bool forward(QMouseEvent *e);
    virtual void editorChange(QEditor *e);

    virtual bool eventFilter(QObject *o, QEvent *e);

    virtual void hideEvent(QHideEvent *e);

private slots:
    void on_leFind_textEdited(const QString &text);
    void on_leReplace_textEdited(const QString &text);

    void on_cbReplace_toggled(bool on);

    void on_cbCase_toggled(bool on);
    void on_cbWords_toggled(bool on);
    void on_cbRegExp_toggled(bool on);
    void on_cbCursor_toggled(bool on);
    void on_cbHighlight_toggled(bool on);
    void on_cbSelection_toggled(bool on);
    void on_cbPrompt_toggled(bool on);
    void on_cbEscapeSeq_toggled(bool on);

    void on_bRefresh_clicked();

    void on_bNext_clicked();
    void on_bPrevious_clicked();

    void cursorPositionChanged();

    void on_bClose_clicked();

private:
    void init();
    void on_leFind_returnPressed(bool backward);

private:
    Ui::QSearchReplacePanel *ui;

    int lastDirection;
    QDocumentSearch *m_search;
};

#endif // QSEARCHREPLACEPANEL_H
