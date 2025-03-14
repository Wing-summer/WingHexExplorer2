// Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
// SPDX-License-Identifier: BSD-3-Clause

#include "qtlockedfile.h"

#ifdef Q_OS_WIN

#include <qt_windows.h>

#define MUTEX_PREFIX "QtLockedFile mutex "
// Maximum number of concurrent read locks. Must not be greater than
// MAXIMUM_WAIT_OBJECTS
#define MAX_READERS MAXIMUM_WAIT_OBJECTS

#if QT_VERSION >= 0x050000
#define QT_WA(unicode, ansi) unicode
#endif

#include <QFileInfo>

#else
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#endif

using namespace QtLP_Private;

#ifdef Q_OS_WIN
Qt::HANDLE QtLockedFile::getMutexHandle(int idx, bool doCreate) {
    if (mutexname.isEmpty()) {
        QFileInfo fi(*this);
        mutexname =
            QString::fromLatin1(MUTEX_PREFIX) + fi.absoluteFilePath().toLower();
    }
    QString mname(mutexname);
    if (idx >= 0)
        mname += QString::number(idx);

    Qt::HANDLE mutex;
    if (doCreate) {
        QT_WA(
            { mutex = CreateMutexW(NULL, FALSE, LPCWSTR(mname.utf16())); },
            {
                mutex =
                    CreateMutexA(NULL, FALSE, mname.toLocal8Bit().constData());
            });
        if (!mutex) {
            qErrnoWarning("QtLockedFile::lock(): CreateMutex failed");
            return 0;
        }
    } else {
        QT_WA(
            {
                mutex = OpenMutexW(SYNCHRONIZE | MUTEX_MODIFY_STATE, FALSE,
                                   LPCWSTR(mname.utf16()));
            },
            {
                mutex = OpenMutexA(SYNCHRONIZE | MUTEX_MODIFY_STATE, FALSE,
                                   mname.toLocal8Bit().constData());
            });
        if (!mutex) {
            if (GetLastError() != ERROR_FILE_NOT_FOUND)
                qErrnoWarning("QtLockedFile::lock(): OpenMutex failed");
            return 0;
        }
    }
    return mutex;
}

bool QtLockedFile::waitMutex(Qt::HANDLE mutex, bool doBlock) {
    Q_ASSERT(mutex);
    DWORD res = WaitForSingleObject(mutex, doBlock ? INFINITE : 0);
    switch (res) {
    case WAIT_OBJECT_0:
    case WAIT_ABANDONED:
        return true;
        break;
    case WAIT_TIMEOUT:
        break;
    default:
        qErrnoWarning("QtLockedFile::lock(): WaitForSingleObject failed");
    }
    return false;
}
#endif

/*!
    \class QtLockedFile

 \brief The QtLockedFile class extends QFile with advisory locking
 functions.

 A file may be locked in read or write mode. Multiple instances of
 \e QtLockedFile, created in multiple processes running on the same
 machine, may have a file locked in read mode. Exactly one instance
 may have it locked in write mode. A read and a write lock cannot
 exist simultaneously on the same file.

 The file locks are advisory. This means that nothing prevents
 another process from manipulating a locked file using QFile or
 file system functions offered by the OS. Serialization is only
 guaranteed if all processes that access the file use
 QLockedFile. Also, while holding a lock on a file, a process
 must not open the same file again (through any API), or locks
 can be unexpectedly lost.

 The lock provided by an instance of \e QtLockedFile is released
 whenever the program terminates. This is true even when the
 program crashes and no destructors are called.
*/

/*! \enum QtLockedFile::LockMode

 This enum describes the available lock modes.

 \value ReadLock A read lock.
 \value WriteLock A write lock.
 \value NoLock Neither a read lock nor a write lock.
*/

/*!
    Constructs an unlocked \e QtLockedFile object. This constructor
    behaves in the same way as \e QFile::QFile().

 \sa QFile::QFile()
*/
QtLockedFile::QtLockedFile() : QFile() {
#ifdef Q_OS_WIN
    wmutex = nullptr;
    rmutex = nullptr;
#endif
    m_lock_mode = NoLock;
}

/*!
    Constructs an unlocked QtLockedFile object with file \a name. This
    constructor behaves in the same way as \e QFile::QFile(const
    QString&).

 \sa QFile::QFile()
*/
QtLockedFile::QtLockedFile(const QString &name) : QFile(name) {
#ifdef Q_OS_WIN
    wmutex = nullptr;
    rmutex = nullptr;
#endif
    m_lock_mode = NoLock;
}

bool QtLockedFile::lock(LockMode mode, bool block) {
    if (!isOpen()) {
        qWarning("QtLockedFile::lock(): file is not opened");
        return false;
    }

    if (mode == NoLock)
        return unlock();

    if (mode == m_lock_mode)
        return true;

    if (m_lock_mode != NoLock)
        unlock();

#ifdef Q_OS_WIN
    if (!wmutex && !(wmutex = getMutexHandle(-1, true)))
        return false;

    if (!waitMutex(wmutex, block))
        return false;

    if (mode == ReadLock) {
        int idx = 0;
        for (; idx < MAX_READERS; idx++) {
            rmutex = getMutexHandle(idx, false);
            if (!rmutex || waitMutex(rmutex, false))
                break;
            CloseHandle(rmutex);
        }
        bool ok = true;
        if (idx >= MAX_READERS) {
            qWarning("QtLockedFile::lock(): too many readers");
            rmutex = 0;
            ok = false;
        } else if (!rmutex) {
            rmutex = getMutexHandle(idx, true);
            if (!rmutex || !waitMutex(rmutex, false))
                ok = false;
        }
        if (!ok && rmutex) {
            CloseHandle(rmutex);
            rmutex = 0;
        }
        ReleaseMutex(wmutex);
        if (!ok)
            return false;
    } else {
        Q_ASSERT(rmutexes.isEmpty());
        for (int i = 0; i < MAX_READERS; i++) {
            Qt::HANDLE mutex = getMutexHandle(i, false);
            if (mutex)
                rmutexes.append(mutex);
        }
        if (rmutexes.size()) {
            DWORD res =
                WaitForMultipleObjects(rmutexes.size(), rmutexes.constData(),
                                       TRUE, block ? INFINITE : 0);
            if (res != WAIT_OBJECT_0 && res != WAIT_ABANDONED) {
                if (res != WAIT_TIMEOUT)
                    qErrnoWarning(
                        "QtLockedFile::lock(): WaitForMultipleObjects failed");
                m_lock_mode =
                    WriteLock; // trick unlock() to clean up - semiyucky
                unlock();
                return false;
            }
        }
    }
#else
    struct flock fl;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_type = (mode == ReadLock) ? F_RDLCK : F_WRLCK;
    int cmd = block ? F_SETLKW : F_SETLK;
    int ret = fcntl(handle(), cmd, &fl);

    if (ret == -1) {
        if (errno != EINTR && errno != EAGAIN)
            qWarning("QtLockedFile::lock(): fcntl: %s", strerror(errno));
        return false;
    }
#endif

    m_lock_mode = mode;
    return true;
}

bool QtLockedFile::unlock() {
    if (!isOpen()) {
        qWarning("QtLockedFile::unlock(): file is not opened");
        return false;
    }

    if (!isLocked())
        return true;

#ifdef Q_OS_WIN
    if (m_lock_mode == ReadLock) {
        ReleaseMutex(rmutex);
        CloseHandle(rmutex);
        rmutex = 0;
    } else {
        foreach (Qt::HANDLE mutex, rmutexes) {
            ReleaseMutex(mutex);
            CloseHandle(mutex);
        }
        rmutexes.clear();
        ReleaseMutex(wmutex);
    }
#else
    struct flock fl;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_type = F_UNLCK;
    int ret = fcntl(handle(), F_SETLKW, &fl);

    if (ret == -1) {
        qWarning("QtLockedFile::lock(): fcntl: %s", strerror(errno));
        return false;
    }
#endif

    m_lock_mode = NoLock;
    return true;
}

QtLockedFile::~QtLockedFile() {
    if (isOpen())
        unlock();

#ifdef Q_OS_WIN
    if (wmutex)
        CloseHandle(wmutex);
#endif
}

/*!
    Returns \e true if this object has a in read or write lock;
    otherwise returns \e false.

 \sa lockMode()
*/
bool QtLockedFile::isLocked() const { return m_lock_mode != NoLock; }

/*!
    Returns the type of lock currently held by this object, or \e
    QtLockedFile::NoLock.

 \sa isLocked()
*/
QtLockedFile::LockMode QtLockedFile::lockMode() const { return m_lock_mode; }

/*!
  Opens the file in OpenMode \a mode.

This is identical to QFile::open(), with the one exception that the
Truncate mode flag is disallowed. Truncation would conflict with the
advisory file locking, since the file would be modified before the
write lock is obtained. If truncation is required, use resize(0)
after obtaining the write lock.

Returns true if successful; otherwise false.

\sa QFile::open(), QFile::resize()
*/
bool QtLockedFile::open(OpenMode mode) {
    if (mode & QIODevice::Truncate) {
        qWarning("QtLockedFile::open(): Truncate mode not allowed.");
        return false;
    }
    return QFile::open(mode);
}
