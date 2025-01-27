/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** You can redistribute this file and/or modify it under the terms of the
** BSD 3-Clause.
**
** THIS FILE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
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

#ifndef IWINGDEVICE_H
#define IWINGDEVICE_H

#include "iwingpluginbase.h"

#include <optional>

namespace WingHex {

class IWingDevice : public IWingPluginBase {
    Q_OBJECT
public:
    virtual QString supportedFileExtDisplayName() const = 0;

    virtual QIcon supportedFileIcon() const { return {}; };

public:
    virtual std::optional<QPair<QString, QVariantList>> onOpenFileBegin() {
        return std::nullopt;
    }

    virtual QIODevice *onOpenFile(const QString &path,
                                  const QVariantList &params) = 0;

    virtual bool onCloseFile(QIODevice *dev) = 0;
};

} // namespace WingHex

Q_DECLARE_INTERFACE(WingHex::IWingDevice, "com.wingsummer.iwingdevice")

#endif // IWINGDEVICE_H
