#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include "QHexView/qhexview.h"
#include "framelessdialog.h"

#include <QComboBox>
#include <QLineEdit>
#include <QObject>
#include <QRadioButton>
#include <QSpinBox>

enum class SearchDirection { None, Region, Foreword, Backword, Selection };

class FindDialog : public FramelessDialog {
    Q_OBJECT
public:
    struct Result {
        SearchDirection dir = SearchDirection::None;
        qsizetype start = 0;
        qsizetype stop = 0;
    };

public:
    FindDialog(bool isBigFile, int start, int stop, bool sel = true,
               QWidget *parent = nullptr);
    QByteArray getResult(Result &result);

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

    QSpinBox *m_regionStart;
    QSpinBox *m_regionStop;

    Result _result;
};

#endif // FINDDIALOG_H
