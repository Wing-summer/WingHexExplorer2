#ifndef CLANGFORMATSETDIALOG_H
#define CLANGFORMATSETDIALOG_H

#include "WingPlugin/settingpage.h"

namespace Ui {
class ClangFormatSetDialog;
}

class ClangFormatSetDialog : public WingHex::SettingPage {
    Q_OBJECT

public:
    explicit ClangFormatSetDialog(QWidget *parent = nullptr);
    ~ClangFormatSetDialog();

private:
    Ui::ClangFormatSetDialog *ui;

    QStringList getEditableStyleTextLines();
    QString getEditableStyleTexts();

    void reload();

    // PageBase interface
public:
    virtual QIcon categoryIcon() const override;
    virtual QString name() const override;
    virtual QString id() const override;

    // SettingPage interface
public:
    virtual void apply() override;
    virtual void reset() override;
    virtual void cancel() override;

private slots:
    void on_cbStyle_currentTextChanged(const QString &arg1);
    void on_btnStyleCustom_clicked();
};

#endif // CLANGFORMATSETDIALOG_H
