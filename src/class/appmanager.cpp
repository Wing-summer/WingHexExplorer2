#include "appmanager.h"

#include <QFont>

#include "languagemanager.h"
#include "settingmanager.h"
#include "skinmanager.h"

#include "dbghelper.h"
#include "dialog/mainwindow.h"
#include "utilities.h"

AppManager *AppManager::_instance = nullptr;

AppManager::AppManager(int &argc, char *argv[])
    : SingleApplication(argc, argv, true) {
    ASSERT_SINGLETON;

    setApplicationName(APP_NAME);
    setOrganizationName(APP_ORG);
    setApplicationVersion(WINGHEX_VERSION);

    auto args = arguments();
    if (isSecondary()) {
        QByteArray buffer;
        QDataStream out(&buffer, QIODevice::WriteOnly);
        if (args.size() > 1) {
            for (auto var = args.begin() + 1; var != args.end(); ++var) {
                out << *var;
            }
        }
        sendMessage(buffer);
    }

    auto &set = SettingManager::instance();
    QFont font(set.appFontFamily(), set.appfontSize());
    setFont(font);

    SkinManager::instance();
    LanguageManager::instance();

    _w = new MainWindow;

    connect(this, &SingleApplication::instanceStarted, this, [this] {
        Q_ASSERT(_w);
        if (!_w->isEnabled()) {
            return;
        }
        _w->show();
        _w->raise();
        _w->activateWindow();
    });

    connect(this, &SingleApplication::receivedMessage, this,
            [this](quint32 instanceId, QByteArray message) {
                Q_UNUSED(instanceId);
                QDataStream out(&message, QIODevice::WriteOnly);
                while (!out.atEnd()) {
                    QString param;
                    out >> param;
                    openFile(param);
                }
            });

    if (args.size() > 1) {
        for (auto var = args.begin() + 1; var != args.end(); ++var) {
            openFile(*var);
        }
    }

    _instance = this;
}

AppManager::~AppManager() {
    _w->deleteLater();
    _w = nullptr;
}

AppManager *AppManager::instance() { return _instance; }

MainWindow *AppManager::mainWindow() const { return _w; }

void AppManager::openFile(const QString &file, bool autoDetect, qsizetype start,
                          qsizetype stop) {
    EditorView *editor = nullptr;
    Q_ASSERT(_w);
    if (Utilities::isStorageDevice(file)) {
        openDriver(file);
    } else {
        ErrFile ret = ErrFile::Error;
        if (autoDetect) {
            ret = _w->openWorkSpace(file, &editor);
        }
        if (ret == ErrFile::Error) {
            if (start >= 0 && stop > 0) {
                ret = _w->openRegionFile(file, &editor, start, stop);
            } else {
                ret = _w->openFile(file, &editor);
            }
            if (ret == ErrFile::AlreadyOpened) {
                Q_ASSERT(editor);
                editor->raise();
                editor->setFocus();
            }
        } else if (ret == ErrFile::AlreadyOpened) {
            Q_ASSERT(editor);
            editor->raise();
            editor->setFocus();
        }
    }
}

void AppManager::openRawFile(const QString &file) {
    EditorView *editor = nullptr;
    Q_ASSERT(_w);
    auto ret = _w->openFile(file, &editor);
    if (ret == ErrFile::AlreadyOpened) {
        Q_ASSERT(editor);
        editor->raise();
        editor->setFocus();
    }
}

void AppManager::openDriver(const QString &driver) {
    EditorView *editor = nullptr;
    if (Utilities::isStorageDevice(driver)) {
        auto ret = _w->openDriver(driver, &editor);
        if (ret == ErrFile::AlreadyOpened) {
            Q_ASSERT(editor);
            editor->raise();
            editor->setFocus();
        }
    }
}

void AppManager::openRegionFile(const QString &region, qsizetype start,
                                qsizetype length) {
    EditorView *editor = nullptr;
    Q_ASSERT(_w);
    auto ret = _w->openRegionFile(region, &editor, start, length);
    if (ret == ErrFile::AlreadyOpened) {
        Q_ASSERT(editor);
        editor->raise();
        editor->setFocus();
    }
}

void AppManager::openWorkSpace(const QString &ws) {
    EditorView *editor = nullptr;
    Q_ASSERT(_w);
    auto ret = _w->openWorkSpace(ws, &editor);
    if (ret == ErrFile::AlreadyOpened) {
        Q_ASSERT(editor);
        editor->raise();
        editor->setFocus();
    }
}
