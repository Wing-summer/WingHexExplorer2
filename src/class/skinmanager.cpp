#include "skinmanager.h"

#include "dbghelper.h"
#include "settingmanager.h"

#include <QApplication>
#include <QFile>
#include <QFont>
#include <QScreen>
#include <QStyleFactory>

SkinManager::SkinManager(QObject *parent) : QObject(parent) {
    ASSERT_SINGLETON;

    int theme = SettingManager::instance().themeID();

    QFile qss;
    switch (Theme(theme)) {
    case Theme::Dark:
        setTheme(Theme::Dark);
        qss.setFileName("://dark/stylesheet.qss");
        break;
    case Theme::Light:
        setTheme(Theme::Light);
        qss.setFileName("://light/stylesheet.qss");
        break;
    }
    qss.open(QFile::ReadOnly | QFile::Text);
    qApp->setStyle(QStyleFactory::create("fusion"));
    qApp->setStyleSheet(qss.readAll());
    qss.close();
}

SkinManager::~SkinManager() {}

SkinManager &SkinManager::instance() {
    static SkinManager instance;
    return instance;
}

void SkinManager::setTheme(SkinManager::Theme theme) { m_theme = theme; }

SkinManager::Theme SkinManager::currentTheme() const { return m_theme; }

void SkinManager::translate() {
    tr("Dark");
    tr("Light");
}
