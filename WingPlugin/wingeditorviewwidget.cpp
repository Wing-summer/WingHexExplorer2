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

#include "wingeditorviewwidget.h"

WingHex::WingEditorViewWidget::WingEditorViewWidget(IWingPlugin *plg,
                                                    QWidget *parent)
    : WingPluginWidget(plg, parent) {}

void WingHex::WingEditorViewWidget::raiseView() {
    constexpr auto VIEW_PROPERTY = "__VIEW__";
    constexpr auto VIEW_ID_PROPERTY = "__ID__";

    auto rawptr =
        reinterpret_cast<QObject *>(property(VIEW_PROPERTY).value<quintptr>());
    auto ptr = qobject_cast<QWidget *>(rawptr);
    auto id = property(VIEW_ID_PROPERTY).toString();
    if (ptr && !id.isEmpty()) {
        QMetaObject::invokeMethod(ptr, "raiseAndSwitchView",
                                  Qt::DirectConnection, WINGAPI_ARG(id));
    }
}

void WingHex::WingEditorViewWidget::loadState(const QByteArray &state) {
    Q_UNUSED(state);
}

bool WingHex::WingEditorViewWidget::hasUnsavedState() { return false; }

QByteArray WingHex::WingEditorViewWidget::saveState() { return {}; }

bool WingHex::WingEditorViewWidget::onClosing() { return true; }

void WingHex::WingEditorViewWidget::onWorkSpaceNotify(bool isWorkSpace) {
    Q_UNUSED(isWorkSpace);
}
