#ifndef LOCALIZEDMETADATA_H
#define LOCALIZEDMETADATA_H

#include <QJsonObject>
#include <QJsonValue>
#include <QString>

namespace LocalizedMetadata {
// Script metadata may contain a direct string or an object keyed by locale.
inline QString string(const QJsonValue &value, const QString &language) {
    if (value.isString()) {
        return value.toString();
    }
    if (value.isObject()) {
        const auto object = value.toObject();
        for (const auto &locale : {language, QStringLiteral("en_US")}) {
            const auto text = object.value(locale);
            if (text.isString() && !text.toString().trimmed().isEmpty()) {
                return text.toString();
            }
        }
    }
    return {};
}
} // namespace LocalizedMetadata

#endif // LOCALIZEDMETADATA_H
