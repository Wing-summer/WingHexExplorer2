#ifndef CHANGEDSTRINGLIST_H
#define CHANGEDSTRINGLIST_H

#include <QHash>
#include <QString>
#include <QStringList>

class ChangedStringList {
public:
    bool containChanges() const;
    bool containChanges(const QString &item) const;
    void clear();

    void pushAddItem(const QString &item);
    void pushRemoveItem(const QString &item);

    QStringList getContents() const;
    void setContents(const QStringList &newContents);

private:
    QStringList contents;
    QHash<QString, bool> mods;
};

#endif // CHANGEDSTRINGLIST_H
