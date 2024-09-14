#ifndef QEDITCONFIG_H
#define QEDITCONFIG_H

#include <QWidget>

namespace Ui {
class QEditConfig;
}

class QEditConfig : public QWidget {
    Q_OBJECT

public:
    explicit QEditConfig(QWidget *parent = nullptr);
    virtual ~QEditConfig();

    bool hasUnsavedChanges() const;

    bool applyImmediately() const;

    QMap<QString, QVariant> dumpKeys() const;

public slots:
    void retranslate();

    void apply();
    void cancel();
    void restore();

    void loadKeys(const QMap<QString, QVariant> &keys);

    void setApplyImmediately(bool y);

signals:
    void keyChanged(const QString &key, const QVariant &value);

private slots:
    void on_spnFontSize_valueChanged(int size);
    void on_cbFont_currentFontChanged(QFont font);

    void on_spnTabWidth_valueChanged(int n);

    void on_chkReplaceTabs_toggled(bool y);

    void on_chkShowTabsInText_toggled(bool y);
    void on_chkShowLeadingWhitespace_toggled(bool y);
    void on_chkShowTrailingWhitespace_toggled(bool y);

    void on_cbLineEndings_currentIndexChanged(int idx);
    void on_chkDetectLE_toggled(bool y);
    void on_chkAutoRemoveTrailingWhitespace_toggled(bool y);
    void on_chkPreserveTrailingIndent_toggled(bool y);

private:
    Ui::QEditConfig *ui;

    bool m_direct;
};

#endif // QEDITCONFIG_H
