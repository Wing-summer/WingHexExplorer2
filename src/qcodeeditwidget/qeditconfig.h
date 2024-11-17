#ifndef QEDITCONFIG_H
#define QEDITCONFIG_H

#include "plugin/settingpage.h"
#include "qeditor.h"
#include <QWidget>

namespace Ui {
class QEditConfig;
}

class QEditConfig : public WingHex::SettingPage {
    Q_OBJECT

public:
    explicit QEditConfig(QWidget *parent = nullptr);
    virtual ~QEditConfig();

    QMap<QString, QVariant> dumpKeys() const;

public:
    virtual QIcon categoryIcon() const override;
    virtual QString name() const override;
    virtual QString id() const override;
    virtual void apply() override;
    virtual void cancel() override;
    virtual void reset() override;

private:
    void reload();

public slots:
    void loadKeys(const QMap<QString, QVariant> &keys);

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

    QEditor *_edit;
};

#endif // QEDITCONFIG_H
