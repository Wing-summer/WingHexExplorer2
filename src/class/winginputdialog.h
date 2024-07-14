#ifndef WINGINPUTDIALOG_H
#define WINGINPUTDIALOG_H

#include <QInputDialog>

/**
 * @brief A hacker class for QInputDialog
 */
class WingInputDialog {
public:
    static QString getText(QWidget *parent, const QString &title,
                           const QString &label,
                           QLineEdit::EchoMode echo = QLineEdit::Normal,
                           const QString &text = QString(), bool *ok = nullptr,
                           Qt::InputMethodHints inputMethodHints = Qt::ImhNone);
    static QString
    getMultiLineText(QWidget *parent, const QString &title,
                     const QString &label, const QString &text = QString(),
                     bool *ok = nullptr,
                     Qt::InputMethodHints inputMethodHints = Qt::ImhNone);

    static QString getItem(QWidget *parent, const QString &title,
                           const QString &label, const QStringList &items,
                           int current = 0, bool editable = true,
                           bool *ok = nullptr,
                           Qt::InputMethodHints inputMethodHints = Qt::ImhNone);

    static int getInt(QWidget *parent, const QString &title,
                      const QString &label, int value = 0,
                      int minValue = -2147483647, int maxValue = 2147483647,
                      int step = 1, bool *ok = nullptr);

    static double getDouble(QWidget *parent, const QString &title,
                            const QString &label, double value = 0,
                            double minValue = -2147483647,
                            double maxValue = 2147483647, int decimals = 1,
                            bool *ok = nullptr, double step = 1);

private:
    WingInputDialog();

    Q_DISABLE_COPY_MOVE(WingInputDialog)
};

#endif // WINGINPUTDIALOG_H
