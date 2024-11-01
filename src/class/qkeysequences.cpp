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

#include "qkeysequences.h"

QKeySequences::QKeySequences() {
    _kseqs = {
        {Key::OPEN_REGION_FILE,
         QKeySequence(Qt::KeyboardModifier::ControlModifier |
                      Qt::KeyboardModifier::ShiftModifier | Qt::Key_O)},
        {Key::OPEN_WORKSPACE,
         QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_W)},
        {Key::REDO,
         QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_Y)},
        {Key::SAVE_AS,
         QKeySequence(Qt::KeyboardModifier::ControlModifier |
                      Qt::KeyboardModifier::ShiftModifier | Qt::Key_S)},
        {Key::EXPORT,
         QKeySequence(Qt::KeyboardModifier::ControlModifier |
                      Qt::KeyboardModifier::ShiftModifier | Qt::Key_E)},
        {Key::GOTO,
         QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_G)},
        {Key::REPLACE,
         QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_R)},
        {Key::ENCODING,
         QKeySequence(Qt::KeyboardModifier::ControlModifier |
                      Qt::KeyboardModifier::AltModifier | Qt::Key_E)},
        {Key::COPY_HEX,
         QKeySequence(Qt::KeyboardModifier::ControlModifier |
                      Qt::KeyboardModifier::ShiftModifier | Qt::Key_C)},
        {Key::CUT_HEX,
         QKeySequence(Qt::KeyboardModifier::ControlModifier |
                      Qt::KeyboardModifier::ShiftModifier | Qt::Key_X)},
        {Key::PASTE_HEX,
         QKeySequence(Qt::KeyboardModifier::ControlModifier |
                      Qt::KeyboardModifier::ShiftModifier | Qt::Key_V)},
        {Key::FILE_INFO,
         QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_I)},
        {Key::HEX_FILL,
         QKeySequence(Qt::KeyboardModifier::ControlModifier |
                      Qt::KeyboardModifier::AltModifier | Qt::Key_F)},
        {Key::HEX_FILL0,
         QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_0)},
        {Key::BOOKMARK,
         QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_B)},
        {Key::BOOKMARK_DEL,
         QKeySequence(Qt::KeyboardModifier::ControlModifier |
                      Qt::KeyboardModifier::AltModifier | Qt::Key_B)},
        {Key::BOOKMARK_CLS,
         QKeySequence(Qt::KeyboardModifier::ControlModifier |
                      Qt::KeyboardModifier::ShiftModifier |
                      Qt::KeyboardModifier::AltModifier | Qt::Key_B)},
        {Key::METADATA,
         QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_M)},
        {Key::METADATA_EDIT,
         QKeySequence(Qt::KeyboardModifier::ControlModifier |
                      Qt::KeyboardModifier::ShiftModifier | Qt::Key_M)},
        {Key::METADATA_DEL,
         QKeySequence(Qt::KeyboardModifier::ControlModifier |
                      Qt::KeyboardModifier::AltModifier | Qt::Key_M)},
        {Key::METADATA_CLS,
         QKeySequence(Qt::KeyboardModifier::ControlModifier |
                      Qt::KeyboardModifier::ShiftModifier |
                      Qt::KeyboardModifier::AltModifier | Qt::Key_M)},
        {Key::METADATA_FG,
         QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_1)},
        {Key::METADATA_BG,
         QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_2)},
        {Key::METADATA_COMMENT,
         QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_3)},
        {Key::METADATA_SHOW,
         QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_Plus)},
        {Key::METADATA_HIDE,
         QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_Minus)},
        {Key::SETTING_GENERAL,
         QKeySequence(Qt::KeyboardModifier::ControlModifier |
                      Qt::KeyboardModifier::ShiftModifier | Qt::Key_G)},
        {Key::SETTING_PLUGIN,
         QKeySequence(Qt::KeyboardModifier::ControlModifier |
                      Qt::KeyboardModifier::ShiftModifier | Qt::Key_P)}};
}

QKeySequence QKeySequences::keySequence(Key key) const {
    return _kseqs.value(key, QKeySequence());
}

const QKeySequences &QKeySequences::instance() {
    static QKeySequences instance;
    return instance;
}
