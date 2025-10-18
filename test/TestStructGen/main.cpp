/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** You can redistribute this file and/or modify it under the terms of the
** BSD 3-Clause.
**
** THIS FILE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
** =============================================================================
*/

#include "TestHeader.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QTextStream>

static QTextStream qout(stdout);

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    static_assert(std::is_pod_v<TestHeader>, "TestHeader must be a pod object");

    TestHeader header;
    std::memset(&header, 0, sizeof(header));
    constexpr auto av = INT_MAX / 2;
    header.u.a = av;
    header.u.c = '?';
    header.t = 1024;
    header.a = 1;
    header.b = 2;
    header.c = false;
    // header.d
    header.e = 24;
    header.f = TestEnum::Test3;
    constexpr auto str = "wing";
    qstrncpy(header.buffer, str, sizeof(str));

    QDir dir(QCoreApplication::applicationDirPath());

    {
        QFile f(dir.absoluteFilePath(QStringLiteral("testh_l.bin")));
        if (!f.open(QFile::WriteOnly)) {
            qout << QStringLiteral(">> can not open testh_l.bin") << Qt::endl;
        }

        QDataStream s(&f);
        s.setByteOrder(QDataStream::ByteOrder::LittleEndian);
        s.writeRawData(reinterpret_cast<const char *>(&header), sizeof(header));
        f.close();

        qout << QStringLiteral(">> generate testh_l.bin") << Qt::endl;
    }

    {
        QFile f(dir.absoluteFilePath(QStringLiteral("testh_b.bin")));
        if (!f.open(QFile::WriteOnly)) {
            qout << QStringLiteral("can not open testh_b.bin") << Qt::endl;
        }

        QDataStream s(&f);
        s.setByteOrder(QDataStream::ByteOrder::BigEndian);
        s.writeRawData(reinterpret_cast<const char *>(&header), sizeof(header));
        f.close();

        qout << QStringLiteral(">> generate testh_b.bin") << Qt::endl;
    }

    return 0;
}
