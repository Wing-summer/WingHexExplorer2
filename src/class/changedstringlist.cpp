#include "changedstringlist.h"

bool ChangedStringList::containChanges() const { return !mods.isEmpty(); }

bool ChangedStringList::containChanges(const QString &item) const {
    return contents.contains(item);
}

void ChangedStringList::clear() {
    contents.clear();
    mods.clear();
}

void ChangedStringList::pushAddItem(const QString &item) {
    if (mods.contains(item)) {
        auto v = mods.value(item);
        if (v) {
            return;
        } else {
            mods.remove(item);
        }
    } else {
        mods.insert(item, true);
    }
    contents.append(item);
}

void ChangedStringList::pushRemoveItem(const QString &item) {
    if (mods.contains(item)) {
        auto v = mods.value(item);
        if (v) {
            mods.remove(item);
        } else {
            return;
        }
    } else {
        mods.insert(item, false);
    }
    contents.removeOne(item);
}

QStringList ChangedStringList::getContents() const { return contents; }

void ChangedStringList::setContents(const QStringList &newContents) {
    contents = newContents;
    mods.clear();
}
