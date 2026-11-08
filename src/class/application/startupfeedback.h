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

#ifndef STARTUPFEEDBACK_H
#define STARTUPFEEDBACK_H

#include <QVariantList>
#include <algorithm>

namespace StartupFeedback {
// Leave time for widget creation/painting within the 200 ms feedback budget.
inline constexpr int ShowDelayMs = 150;
inline constexpr qint64 UnknownDurationMs = 250;
inline constexpr int HistoryLimit = 5;

inline QVariantList remember(const QVariantList &history, qint64 durationMs) {
    QVariantList result;
    for (const auto &sample : history) {
        bool ok = false;
        const auto value = sample.toLongLong(&ok);
        if (ok && value > 0 && value <= 60000) {
            result.append(value);
        }
    }
    if (durationMs > 0 && durationMs <= 60000) {
        result.append(durationMs);
    }
    while (result.size() > HistoryLimit) {
        result.removeFirst();
    }
    return result;
}

inline qint64 estimate(const QVariantList &history) {
    const auto valid = remember(history, 0);
    if (valid.isEmpty()) {
        return UnknownDurationMs;
    }
    QList<qint64> samples;
    for (const auto &sample : valid) {
        samples.append(sample.toLongLong());
    }
    std::sort(samples.begin(), samples.end());
    const auto count = samples.size();
    return (samples[(count - 1) / 2] + samples[count / 2]) / 2;
}
} // namespace StartupFeedback

#endif // STARTUPFEEDBACK_H
