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

#ifndef DIFFCONSTANTS_H
#define DIFFCONSTANTS_H

#include <QtGlobal>

#include <array>

namespace WingDiff {
inline constexpr quint64 KiB = 1024;
inline constexpr quint64 MiB = 1024 * KiB;
inline constexpr quint64 GiB = 1024 * MiB;

namespace Limits {
inline constexpr quint64 MinimumFileSize = 64 * MiB;
inline constexpr quint64 DefaultFileSize = 32 * GiB;
inline constexpr quint64 MaximumFileSize = 64 * GiB;
inline constexpr quint64 DefaultMemoryBudget = 64 * MiB;
inline constexpr std::array<quint64, 4> MemoryBudgets{64 * MiB, 128 * MiB,
                                                      256 * MiB, 512 * MiB};
inline constexpr quint64 MinimumDetailedRanges = 1'000;
inline constexpr quint64 DefaultDetailedRanges = 100'000;
inline constexpr quint64 MaximumDetailedRanges = 1'000'000;

constexpr bool isMemoryBudget(quint64 bytes) {
    for (const auto budget : MemoryBudgets) {
        if (bytes == budget) {
            return true;
        }
    }
    return false;
}

} // namespace Limits
} // namespace WingDiff

#endif
