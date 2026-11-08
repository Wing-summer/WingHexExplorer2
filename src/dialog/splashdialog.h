/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#ifndef SPLASHDIALOG_H
#define SPLASHDIALOG_H

#include <QDialog>
#include <QElapsedTimer>
#include <QTimer>

namespace Ui {
class SplashDialog;
}

class SplashDialog : public QDialog {
    Q_OBJECT

public:
    enum class DisplayMode { Immediate, Automatic };
    explicit SplashDialog(QWidget *parent = nullptr);
    explicit SplashDialog(DisplayMode mode, qint64 expectedDurationMs = 0,
                          QWidget *parent = nullptr);
    virtual ~SplashDialog() override;

    qint64 elapsedTime() const;
    void prepareForBlockingPhase();

public slots:
    void setInfoText(const QString &text);
    void cancel();
    void finish();

protected:
    void closeEvent(QCloseEvent *event) override;
#ifdef WINGHEX_ENABLE_STARTUP_PROFILE
    void paintEvent(QPaintEvent *event) override;
#endif

private:
    Ui::SplashDialog *ui;
    QElapsedTimer m_elapsed;
    QTimer m_showTimer;
    qint64 m_expectedDurationMs;
    bool m_shown = false;
    bool m_finished = false;
#ifdef WINGHEX_ENABLE_STARTUP_PROFILE
    bool m_paintRecorded = false;
#endif

    void showSplash();
};

#endif // SPLASHDIALOG_H
