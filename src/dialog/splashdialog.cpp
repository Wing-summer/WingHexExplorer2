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

#include "splashdialog.h"
#include "ui_splashdialog.h"

#include <QCloseEvent>

#include "class/application/startupfeedback.h"
#include "class/application/startuptrace.h"
#include "class/settings/thememanager.h"

SplashDialog::SplashDialog(QWidget *parent)
    : SplashDialog(DisplayMode::Immediate, 0, parent) {}

SplashDialog::SplashDialog(DisplayMode mode, qint64 expectedDurationMs,
                           QWidget *parent)
    : QDialog(parent), ui(new Ui::SplashDialog),
      m_expectedDurationMs(expectedDurationMs > 0
                               ? expectedDurationMs
                               : StartupFeedback::UnknownDurationMs) {
    m_elapsed.start();
    ui->setupUi(this);
    // The application QSS is deferred while the main window is assembled.
    // Style only this subtree when feedback is actually needed.
    auto &theme = ThemeManager::instance();
    connect(&theme, &ThemeManager::sigOnThemeChanged, this, [this]() {
        if (m_shown && !m_finished) {
            setStyleSheet(ThemeManager::instance().currentStyleSheet());
        }
    });
    ui->label->setText(
        QStringLiteral("<html><head/><body><p><span style=\" font-size:16pt; "
                       "font-weight:600;\">%1</span></p></body></html>")
            .arg(qAppName()));
    setWindowFlags(Qt::SplashScreen | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(true);
    m_showTimer.setSingleShot(true);
    connect(&m_showTimer, &QTimer::timeout, this, &SplashDialog::showSplash);
    if (mode == DisplayMode::Immediate) {
        showSplash();
    } else {
        m_showTimer.start(int(qMax(qint64(0), StartupFeedback::ShowDelayMs -
                                                  m_elapsed.elapsed())));
    }
}

void SplashDialog::showSplash() {
    if (m_finished || m_shown) {
        return;
    }
    m_showTimer.stop();
    m_shown = true;
    setStyleSheet(ThemeManager::instance().currentStyleSheet());
    show();
}

SplashDialog::~SplashDialog() { delete ui; }

void SplashDialog::setInfoText(const QString &text) {
    if (m_finished) {
        return;
    }
    ui->lblinfo->setText(text + QStringLiteral("..."));
    if (m_elapsed.elapsed() >= StartupFeedback::ShowDelayMs) {
        showSplash();
    }
    qApp->processEvents();
}

qint64 SplashDialog::elapsedTime() const { return m_elapsed.elapsed(); }

void SplashDialog::prepareForBlockingPhase() {
    // A timer cannot fire while the GUI thread applies QSS or runs plugin init.
    // Paint beforehand if recent starts predict that we will miss the budget.
    if (m_elapsed.elapsed() >= StartupFeedback::ShowDelayMs ||
        m_expectedDurationMs >= StartupFeedback::ShowDelayMs) {
        showSplash();
        if (!m_finished) {
            qApp->processEvents();
        }
    }
}

void SplashDialog::finish() {
    if (m_finished) {
        return;
    }
    m_finished = true;
    m_showTimer.stop();
    close();
    // Also release an automatic splash that finished before ever being shown.
    deleteLater();
}

void SplashDialog::closeEvent(QCloseEvent *event) {
    m_finished = true;
    m_showTimer.stop();
    QDialog::closeEvent(event);
}

void SplashDialog::cancel() { finish(); }

#ifdef WINGHEX_ENABLE_STARTUP_PROFILE
void SplashDialog::paintEvent(QPaintEvent *event) {
    QDialog::paintEvent(event);
    if (!m_paintRecorded && !m_finished) {
        m_paintRecorded = true;
        StartupTrace::mark("process.splash_first_paint");
    }
}
#endif
