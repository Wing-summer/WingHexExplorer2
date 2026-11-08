/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#ifndef DIFFRENDERER_H
#define DIFFRENDERER_H

#include "QHexView/document/qhexrenderer.h"
#include "class/diff/binarydiff.h"

class DiffRenderer final : public QHexRenderer {
public:
    explicit DiffRenderer(QHexDocument *document, QHexCursor *cursor,
                          const QFontMetricsF &metrics, QObject *parent,
                          bool right);

public:
    void setResults(std::shared_ptr<const WingDiff::ResultStore> results);
    void setDarkTheme(bool dark);

protected:
    virtual void applyLineFormats(qsizetype line,
                                  QVector<CellFormat> &formats) const override;

private:
    std::shared_ptr<const WingDiff::ResultStore> m_results;
    bool m_right;
    bool m_dark = false;
};

#endif
