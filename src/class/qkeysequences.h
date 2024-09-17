#ifndef QKEYSEQUENCES_H
#define QKEYSEQUENCES_H

#include <QKeySequence>
#include <QMap>

class QKeySequences {
public:
    enum class Key {
        OPEN_REGION_FILE,
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
