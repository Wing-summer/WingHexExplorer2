#include "languagemanager.h"
#include "localizedmetadata.h"

#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QResource>
#include <cstdio>

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    const auto resource = qEnvironmentVariable("WING_TEST_PLUGIN_RESOURCE");
    if (!resource.isEmpty() && !QResource::registerResource(resource)) {
        return 2;
    }
    auto &language = LanguageManager::instance();
    const auto plugin =
        language.try2LoadPluginLang(QStringLiteral("i18n_fixture"));
    const auto localized =
        QJsonObject{{QStringLiteral("zh_CN"), QStringLiteral("中文")},
                    {QStringLiteral("en_US"), QStringLiteral("English")}};
    const auto empty =
        QJsonObject{{QStringLiteral("zh_CN"), QStringLiteral(" ")},
                    {QStringLiteral("en_US"), QStringLiteral("English")}};
    if (LocalizedMetadata::string(localized, QStringLiteral("zh_CN")) !=
            QStringLiteral("中文") ||
        LocalizedMetadata::string(localized, QStringLiteral("zh_TW")) !=
            QStringLiteral("English") ||
        LocalizedMetadata::string(empty, QStringLiteral("zh_CN")) !=
            QStringLiteral("English") ||
        LocalizedMetadata::string(QJsonValue(QStringLiteral("Direct")),
                                  QStringLiteral("zh_CN")) !=
            QStringLiteral("Direct") ||
        !LocalizedMetadata::string(QJsonValue(12), QStringLiteral("en_US"))
             .isEmpty()) {
        return 3;
    }
    const auto data = language.data();
    const QJsonObject result{
        {QStringLiteral("pluginLoaded"), plugin != nullptr},
        {QStringLiteral("pluginText"),
         QCoreApplication::translate("Fixture", "Plugin action")},
        {QStringLiteral("locale"), language.defaultLocale().name()},
        {QStringLiteral("system"), QLocale::system().name()},
        {QStringLiteral("configured"),
         qEnvironmentVariable("WING_TEST_LANGUAGE")},
        {QStringLiteral("englishAvailable"),
         language.langs().contains(QStringLiteral("en_US"))},
        {QStringLiteral("translated"), !data.trFiles.isEmpty()},
        {QStringLiteral("about"), !data.about.isEmpty()},
        {QStringLiteral("developers"), !data.dev.isEmpty()},
        {QStringLiteral("open"),
         QCoreApplication::translate("MainWindow", "Open")},
        {QStringLiteral("missing"),
         QCoreApplication::translate("MainWindow", "A missing translation")}};
    const auto output = QJsonDocument(result).toJson(QJsonDocument::Compact);
    std::puts(output.constData());
}
