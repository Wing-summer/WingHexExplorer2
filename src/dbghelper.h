#ifndef DBGHELPER_H
#define DBGHELPER_H

/** @brief This class is used for defensive programming
 *  and performance testing.
 *  These functions and macros will do nothing in debug mode,
 *  except for log modules.
 *  @author wingsummer
 */

#include <QDebug>
#include <QFileInfo>
#include <ctime>
#include <qnamespace.h>

#ifdef QT_DEBUG
static clock_t m_clock;
#endif

#ifdef QT_DEBUG
#define ASSERT_SINGLETON                                                       \
    {                                                                          \
        static bool inited = false;                                            \
        Q_ASSERT(!inited);                                                     \
        inited = true;                                                         \
    }
#else
#define ASSERT_SINGLETON
#endif

#define DECLARE_CHECK_FAILED_FUNC                                              \
    bool checkFailed(bool con, const char *fmt, ...) const {                   \
        if (!con) {                                                            \
            va_list params;                                                    \
            va_start(params, fmt);                                             \
            CCritical(fmt, params);                                            \
            va_end(params);                                                    \
            return true;                                                       \
        }                                                                      \
        return false;                                                          \
    }

class DBGHelper {
public:
    // start the timer for performance testing
    static inline void tic() {
#ifdef QT_DEBUG
        m_clock = clock();
#endif
    }

    // stop the timer for performance testing and print a log
    static inline double toc(const char *title = nullptr) {
#ifdef QT_DEBUG
        auto now = clock();
        auto sec = double(now - m_clock) / CLOCKS_PER_SEC;
        if (title == nullptr) {
            qDebug("%f s passed", sec);
        } else {
            qDebug("%s : %f s passed", title, sec);
        }
        return sec;
#endif
        return 0;
    }
};

#endif
