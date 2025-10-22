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
**
** The original License is MIT from Dax89/QHexView. I have modified a lot so I
** decide to change the Open Source License. You can use the original library
** under MIT. Thanks for Dax89's efforts.
** =============================================================================
*/

#ifndef UNDOCOMMANDBASE_H
#define UNDOCOMMANDBASE_H

#include <QUndoCommand>

/**
 * @brief The UndoCommandBase class
 * @note Text visual grammar:
 *       * For Bookmark and HexEdit:
 *          [<UndoCmdID>] {pos:<offset>} {len:<length>}
 *              Ex.: [B+] {pos: 16-0x10}
 *                   [H+] {pos: 16-0x10} {len: 15-0xF}
 *       * For Metadata Edit:
 *          [<UndoCmdID>] {pos:<offset>} F<+|-|*>|B<+|-|*>|C<+|-|*>
 *          // F: foreground color
 *          // B: background color
 *          // C: comment
 *              Ex: [M+] {pos: 16-0x10} {len: 15-0xF} FB
 *                  [M*] {pos: 16-0x10} F+B*
 *
 *      * For Group Edit for same type
 *          [<UndoCmdID>G] {cnt: <count>}
 */

class UndoCommandBase : public QUndoCommand {
public:
    enum UndoCommandID {                          // UndoCmdID
        UndoID_HexAppend,                         // H<
        UndoID_HexReplaceInsert,                  // H+
        UndoID_HexRemove,                         // H-
        UndoID_MetaCmdBegin,                      // Reserved
        UndoID_SetBaseAddr = UndoID_MetaCmdBegin, // A
        UndoID_BookMarkAdd,                       // B+
        UndoID_BookMarkClear,                     // B~
        UndoID_BookMarkRemove,                    // B-
        UndoID_BookMarkReplace,                   // B*
        UndoID_MetaAdd,                           // M+
        UndoID_MetaClear,                         // M~
        UndoID_MetaRemove,                        // M-
        UndoID_MetaRemovePos,                     // M-
        UndoID_MetaReplace,                       // M*
        UndoID_MetaCmdEnd                         // Reserved
    };

public:
    explicit UndoCommandBase(const QString &text,
                             QUndoCommand *parent = nullptr);
};

#endif // UNDOCOMMANDBASE_H
