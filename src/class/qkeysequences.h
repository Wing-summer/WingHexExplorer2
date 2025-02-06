/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
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

#ifndef QKEYSEQUENCES_H
#define QKEYSEQUENCES_H

#include <QKeySequence>
#include <QMap>

class QKeySequences {
public:
    enum class Key {
        OPEN_WORKSPACE,
        REDO,
        SAVE_AS,
        EXPORT,
        GOTO,
        REPLACE,
        ENCODING,
        COPY_HEX,
        CUT_HEX,
        PASTE_HEX,
        FILE_INFO,
        HEX_FILL,
        HEX_FILL0,
        BOOKMARK,
        BOOKMARK_DEL,
        BOOKMARK_CLS,
        METADATA,
        METADATA_EDIT,
        METADATA_DEL,
        METADATA_CLS,
        METADATA_FG,
        METADATA_BG,
        METADATA_COMMENT,
        METADATA_SHOW,
        METADATA_HIDE,
        SETTING_GENERAL,
        SETTING_PLUGIN
    };

public:
    QKeySequence keySequence(Key key) const;

    static const QKeySequences &instance();

private:
    QKeySequences();

private:
    QMap<Key, QKeySequence> _kseqs;
};

#endif // QKEYSEQUENCES_H
