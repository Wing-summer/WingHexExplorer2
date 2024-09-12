#ifndef QFORMATCONFIG_H
#define QFORMATCONFIG_H

#include <QWidget>

namespace Ui {
class QFormatConfig;
}

class QFormatScheme;

class QFormatConfig : public QWidget {
    Q_OBJECT

public:
    explicit QFormatConfig(QWidget *parent = nullptr);
    virtual ~QFormatConfig();

    bool isAutonomous() const;

    bool hasUnsavedChanges() const;

    QList<QFormatScheme *> schemes() const;

public slots:
    void retranslate();

    void apply();
    void cancel();
    void restore();

    void setAutonomous(bool y);

    void addScheme(const QString &name, QFormatScheme *scheme);
    void removeScheme(QFormatScheme *scheme);

    void setCurrentScheme(QFormatScheme *scheme);

protected:
    virtual void hideEvent(QHideEvent *e);

private slots:
    void on_m_selector_currentIndexChanged(int idx);

private:
    QList<int> modifiedFormats() const;

private:
    Ui::QFormatConfig *ui;
    bool m_autonomous;
    QFormatScheme *m_currentScheme;
    QList<QFormatScheme *> m_schemes;
};

#endif // QFORMATCONFIG_H
