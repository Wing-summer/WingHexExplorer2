#include "winginputdialog.h"
#include "../dialog/framelessdialogbase.h"

#include <QDialogButtonBox>
#include <QPointer>

QString WingInputDialog::getText(QWidget *parent, const QString &title,
                                 const QString &label, QLineEdit::EchoMode echo,
                                 const QString &text, bool *ok,
                                 Qt::InputMethodHints inputMethodHints) {
    auto dialog = new QInputDialog(parent);

    dialog->setLabelText(label);
    dialog->setTextValue(text);
    dialog->setTextEchoMode(echo);
    dialog->setInputMethodHints(inputMethodHints);
    dialog->setWindowFlag(Qt::Widget);

    FramelessDialogBase d(parent);
    d.buildUpContent(dialog);
    d.setWindowTitle(title);

    QObject::connect(dialog, &QInputDialog::finished, &d,
                     &FramelessDialogBase::done);

    const int ret = d.exec();

    if (ok)
        *ok = !!ret;
    if (ret) {
        return dialog->textValue();
    } else {
        return QString();
    }
}

QString WingInputDialog::getMultiLineText(
    QWidget *parent, const QString &title, const QString &label,
    const QString &text, bool *ok, Qt::InputMethodHints inputMethodHints) {
    auto dialog = new QInputDialog(parent);
    dialog->setOptions(QInputDialog::UsePlainTextEditForTextInput);
    dialog->setLabelText(label);
    dialog->setTextValue(text);
    dialog->setInputMethodHints(inputMethodHints);
    dialog->setWindowFlag(Qt::Widget);

    FramelessDialogBase d(parent);
    d.buildUpContent(dialog);
    d.setWindowTitle(title);

    QObject::connect(dialog, &QInputDialog::finished, &d,
                     &FramelessDialogBase::done);

    const int ret = d.exec();

    if (ok)
        *ok = !!ret;
    if (ret) {
        return dialog->textValue();
    } else {
        return QString();
    }
}

QString WingInputDialog::getItem(QWidget *parent, const QString &title,
                                 const QString &label, const QStringList &items,
                                 int current, bool editable, bool *ok,
                                 Qt::InputMethodHints inputMethodHints) {
    QString text(items.value(current));
    auto dialog = new QInputDialog(parent);
    dialog->setLabelText(label);
    dialog->setComboBoxItems(items);
    dialog->setTextValue(text);
    dialog->setComboBoxEditable(editable);
    dialog->setInputMethodHints(inputMethodHints);
    dialog->setWindowFlag(Qt::Widget);

    FramelessDialogBase d(parent);
    d.buildUpContent(dialog);
    d.setWindowTitle(title);

    QObject::connect(dialog, &QInputDialog::finished, &d,
                     &FramelessDialogBase::done);

    const int ret = d.exec();

    if (ok)
        *ok = !!ret;
    if (ret) {
        return dialog->textValue();
    } else {
        return text;
    }
}

int WingInputDialog::getInt(QWidget *parent, const QString &title,
                            const QString &label, int value, int minValue,
                            int maxValue, int step, bool *ok) {
    auto dialog = new QInputDialog(parent);
    dialog->setLabelText(label);
    dialog->setIntRange(minValue, maxValue);
    dialog->setIntValue(value);
    dialog->setIntStep(step);
    dialog->setWindowFlag(Qt::Widget);

    FramelessDialogBase d(parent);
    d.buildUpContent(dialog);
    d.setWindowTitle(title);

    QObject::connect(dialog, &QInputDialog::finished, &d,
                     &FramelessDialogBase::done);

    const int ret = d.exec();

    if (ok)
        *ok = !!ret;
    if (ret) {
        return dialog->intValue();
    } else {
        return value;
    }
}

double WingInputDialog::getDouble(QWidget *parent, const QString &title,
                                  const QString &label, double value,
                                  double minValue, double maxValue,
                                  int decimals, bool *ok, double step) {
    auto dialog = new QInputDialog(parent);
    dialog->setLabelText(label);
    dialog->setDoubleDecimals(decimals);
    dialog->setDoubleRange(minValue, maxValue);
    dialog->setDoubleValue(value);
    dialog->setDoubleStep(step);
    dialog->setWindowFlag(Qt::Widget);

    FramelessDialogBase d(parent);
    d.buildUpContent(dialog);
    d.setWindowTitle(title);

    QObject::connect(dialog, &QInputDialog::finished, &d,
                     &FramelessDialogBase::done);

    const int ret = d.exec();

    if (ok)
        *ok = !!ret;
    if (ret) {
        return dialog->doubleValue();
    } else {
        return value;
    }
}

WingInputDialog::WingInputDialog() {}
