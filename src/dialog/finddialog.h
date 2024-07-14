#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include "../QHexView/qhexview.h"
#include "framelessdialog.h"

#include <QComboBox>
#include <QLineEdit>
#include <QObject>
#include <QRadioButton>

enum class SearchDirection { None, Foreword, Backword, Selection };

class FindDialog : public FramelessDialog {
    Q_OBJECT
public:
    FindDialog(bool sel = true, QWidget *parent = nullptr);
    QByteArray getResult(SearchDirection &dir);

private:
    void on_accept();
    void on_reject();

private:
    QHexView *m_hexeditor;
    QLineEdit *m_lineeditor;
    QRadioButton *m_string;
    QRadioButton *m_hex;
    QComboBox *m_encodings;
    QByteArray _findarr;

    SearchDirection _dir = SearchDirection::None;
};

#endif // FINDDIALOG_H
