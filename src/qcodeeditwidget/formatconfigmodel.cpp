#include "formatconfigmodel.h"
#include "qformat.h"

FormatConfigModel::FormatConfigModel(QObject *parent)
    : QAbstractTableModel(parent) {}

void FormatConfigModel::setScheme(QFormatScheme *scheme) {
    if (_scheme != scheme) {
        _scheme = scheme;
        emit this->layoutChanged();
    }
}

int FormatConfigModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    if (_scheme) {
        return _scheme->formatCount();
    } else {
        return 0;
    }
}

int FormatConfigModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 10;
}

QVariant FormatConfigModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole: {
        auto r = index.row();
        auto fmt = _scheme->format(r);
        switch (index.column()) {
        case 0: // Identifier
            return _scheme->id(r);
        case 1: // Bold
            return fmt.weight == QFont::Bold;
        case 2: // Italic
            return fmt.italic;
        case 3: // Underline
            return fmt.underline;
        case 4: // Overline
            return fmt.overline;
        case 5: // Strikeout
            return fmt.strikeout;
        case 6: // WaveUnderline
            return fmt.waveUnderline;
        case 7: // Textcolor
            return fmt.foreground.name();
        case 8: // Fillcolor
            return fmt.background.name();
        case 9: // Strokecolor
            return fmt.linescolor.name();
        }
    } break;
    case Qt::DecorationRole: {
        auto r = index.row();
        auto fmt = _scheme->format(r);
        switch (index.column()) {
        case 7: // Textcolor
            return fmt.foreground;
        case 8: // Fillcolor
            return fmt.background;
        case 9: // Strokecolor
            return fmt.linescolor;
        default:
            break;
        }
    } break;
    case Qt::TextAlignmentRole:
        return int(Qt::AlignCenter);
    }
    return QVariant();
}

QVariant FormatConfigModel::headerData(int section, Qt::Orientation orientation,
                                       int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return tr("Identifier");
            case 1:
                return tr("Bold");
            case 2:
                return tr("Italic");
            case 3:
                return tr("Underline");
            case 4:
                return tr("Overline");
            case 5:
                return tr("Strikeout");
            case 6:
                return tr("WaveUnderline");
            case 7:
                return tr("Textcolor");
            case 8:
                return tr("Fillcolor");
            case 9:
                return tr("Strokecolor");
            }
        }
    }
    return QVariant();
}
