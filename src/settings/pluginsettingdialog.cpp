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

#include "pluginsettingdialog.h"
#include "ui_pluginsettingdialog.h"

#include "class/eventfilter.h"
#include "class/pluginsystem.h"
#include "class/settingmanager.h"
#include "class/showinshell.h"
#include "class/wingplugincert.h"
#include "utilities.h"

#include <QDesktopServices>

enum {
    PLUIGN_META = Qt::UserRole,
    PLUIGN_NAME,
    PLUIGN_COMMENT,
    PLUGIN_DEPENDENCY_IDX,
};

enum { CERT_CERT = Qt::UserRole };

PluginSettingDialog::PluginSettingDialog(QWidget *parent)
    : WingHex::SettingPage(parent), ui(new Ui::PluginSettingDialog) {
    ui->setupUi(this);

    Utilities::addSpecialMark(ui->cbEnablePlugin);
    Utilities::addSpecialMark(ui->cbEnablePluginRoot);

    reload();

    auto dis = qEnvironmentVariableBool("WINGHEX_DISABLE_PLUGIN_SYSTEM", false);
    if (dis) {
        ui->groupBox->setEnabled(false);
        ui->tabPluginInfo->setEnabled(false);
        ui->tabDevInfo->setEnabled(false);
        return;
    } else {
        dis = qEnvironmentVariableBool("WINGHEX_DISABLE_PLUGIN", false);
        if (dis) {
            ui->cbEnablePlugin->setEnabled(false);
        } else {
            connect(ui->cbEnablePlugin,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
                    &QCheckBox::checkStateChanged,
#else
                    &QCheckBox::stateChanged,
#endif
                    this, &PluginSettingDialog::optionNeedRestartChanged);
        }

        dis = qEnvironmentVariableBool("WINGHEX_DISABLE_EXTDRV", false);
        if (dis) {
            ui->tabDevInfo->setEnabled(false);
        }
    }

    dis = qEnvironmentVariableBool("WINGHEX_DISALLOW_ROOT_PLUGIN", true);
    if (dis) {
        ui->cbEnablePluginRoot->setEnabled(false);
    } else {
        connect(ui->cbEnablePluginRoot,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
                &QCheckBox::checkStateChanged,
#else
                &QCheckBox::stateChanged,
#endif
                this, &PluginSettingDialog::optionNeedRestartChanged);
    }

    auto &plgsys = PluginSystem::instance();
    auto pico = ICONRES("plugin");
    ui->plglist->clear();

    const auto &plugins = plgsys.plugins();
    auto total = plugins.size();

    const auto deps = plgsys.generatePluginsDepMap();
    host = deps.host;
    dep = deps.dep;

    for (qsizetype i = 0; i < total; ++i) {
        auto p = plugins.at(i);
        auto pco = p->pluginIcon();
        auto lwi =
            new QListWidgetItem(pco.isNull() ? pico : pco, p->pluginName());
        auto info = plgsys.getPluginInfo(p);
        auto flags = lwi->flags();
        if (Q_LIKELY(p != plgsys.angelApi())) {
            flags.setFlag(Qt::ItemIsUserCheckable);
            lwi->setFlags(flags);
            lwi->setCheckState(Qt::Checked);
        } else {
            flags.setFlag(Qt::ItemIsUserCheckable, false);
            lwi->setFlags(flags);
        }

        lwi->setData(PLUIGN_META, QVariant::fromValue(info));
        lwi->setData(PLUIGN_NAME, p->pluginName());
        lwi->setData(PLUIGN_COMMENT, p->pluginComment());
        lwi->setData(PLUGIN_DEPENDENCY_IDX, i);
        ui->plglist->addItem(lwi);
    }

    pico = ICONRES("plugindis");
    const auto &blkplgs = plgsys.blockedPlugins();
    const auto istart = total;
    total = blkplgs.size();
    for (int i = 0; i < total; ++i) {
        auto [status, m] = blkplgs.at(i);
        auto lwi = new QListWidgetItem(pico, m.id);
        lwi->setData(PLUIGN_META, QVariant::fromValue(m));
        if (status == PluginSystem::PluginStatus::Blocked) {
            auto flags = lwi->flags();
            flags.setFlag(Qt::ItemIsUserCheckable);
            lwi->setFlags(flags);
            lwi->setCheckState(Qt::Unchecked);
        } else if (status == PluginSystem::PluginStatus::InvalidPlugin) {
            auto font = lwi->font();
            font.setStrikeOut(true);
            lwi->setFont(font);
        }
        auto ridx = istart + i;
        lwi->setData(PLUGIN_DEPENDENCY_IDX, ridx);
        ui->plglist->addItem(lwi);
    }

    pico = ICONRES("devext");
    ui->devlist->clear();
    for (const auto &d : plgsys.devices()) {
        auto pco = d->pluginIcon();
        auto lwi =
            new QListWidgetItem(pco.isNull() ? pico : pco, d->pluginName());
        auto info = plgsys.getPluginInfo(d);
        auto flags = lwi->flags();
        flags.setFlag(Qt::ItemIsUserCheckable);
        lwi->setFlags(flags);
        lwi->setData(PLUIGN_META, QVariant::fromValue(info));
        lwi->setData(PLUIGN_NAME, d->pluginName());
        lwi->setData(PLUIGN_COMMENT, d->pluginComment());
        lwi->setCheckState(Qt::Checked);
        ui->devlist->addItem(lwi);
    }

    pico = ICONRES("devextdis");
    const auto blkdevs = plgsys.blockedDevPlugins();
    for (const auto &[status, m] : blkdevs) {
        auto lwi = new QListWidgetItem(pico, m.id);
        auto flags = lwi->flags();
        lwi->setFlags(flags);
        lwi->setData(PLUIGN_META, QVariant::fromValue(m));
        if (status == PluginSystem::PluginStatus::Blocked) {
            flags.setFlag(Qt::ItemIsUserCheckable);
            lwi->setCheckState(Qt::Unchecked);
        } else if (status == PluginSystem::PluginStatus::InvalidPlugin) {
            auto font = lwi->font();
            font.setStrikeOut(true);
            lwi->setFont(font);
        }
        ui->devlist->addItem(lwi);
    }

    auto set = &SettingManager::instance();
    connect(ui->cbEnablePlugin, &QCheckBox::toggled, set,
            &SettingManager::setEnablePlugin);
    connect(ui->cbEnablePluginRoot, &QCheckBox::toggled, set,
            &SettingManager::setEnablePlgInRoot);

    connect(ui->plglist, &QListWidget::itemChanged, this,
            [this](QListWidgetItem *item) {
                if (item == nullptr) {
                    return;
                }
                const auto info = item->data(PLUIGN_META).value<PluginInfo>();
                const auto id = info.id;

                switch (item->checkState()) {
                case Qt::Unchecked: {
                    const auto idx = item->data(PLUGIN_DEPENDENCY_IDX).toInt();
                    const auto deps = this->dep.at(idx);
                    for (const auto &idx : deps) {
                        // exclude WingAngelAPI
                        if (idx || !PluginSystem::instance().angelApi()) {
                            auto item = ui->plglist->item(idx);
                            Q_ASSERT(item);
                            item->setCheckState(Qt::Unchecked);
                        }
                    }
                    _plgChanged.pushRemoveItem(id);
                } break;
                case Qt::Checked: {
                    const auto idx = item->data(PLUGIN_DEPENDENCY_IDX).toInt();
                    const auto deps = this->host.at(idx);
                    for (const auto &idx : deps) {
                        if (idx || !PluginSystem::instance().angelApi()) {
                            auto item = ui->plglist->item(idx);
                            Q_ASSERT(item);
                            item->setCheckState(Qt::Checked);
                        }
                    }
                    _plgChanged.pushAddItem(id);
                } break;
                case Qt::PartiallyChecked:
                    break;
                }

                ui->btnplgSave->setEnabled(_plgChanged.containChanges());
            });
    connect(ui->plglist, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem *current, QListWidgetItem *previous) {
                ui->plglist->blockSignals(true);
                if (previous) {
                    const auto idx =
                        previous->data(PLUGIN_DEPENDENCY_IDX).toInt();
                    auto deps = this->host.at(idx);
                    for (const auto &idx : std::as_const(deps)) {
                        auto d = ui->plglist->item(idx);
                        if (d) {
                            auto font = d->font();
                            font.setUnderline(false);
                            font.setBold(false);
                            d->setFont(font);
                        }
                    }

                    deps = this->dep.at(idx);
                    for (const auto &idx : std::as_const(deps)) {
                        auto d = ui->plglist->item(idx);
                        if (d) {
                            auto font = d->font();
                            font.setItalic(false);
                            font.setBold(false);
                            d->setFont(font);
                        }
                    }
                }

                if (current == nullptr) {
                    ui->txtc->clear();
                    return;
                }

                auto info = current->data(PLUIGN_META).value<PluginInfo>();
                auto plgName = current->data(PLUIGN_NAME).toString();
                auto plgComment = current->data(PLUIGN_COMMENT).toString();

                loadPluginInfo(info, plgName, plgComment, info.dependencies,
                               ui->txtc);

                // highlight deps
                const auto idx = current->data(PLUGIN_DEPENDENCY_IDX).toInt();
                auto deps = this->host.at(idx);
                for (const auto &idx : std::as_const(deps)) {
                    auto d = ui->plglist->item(idx);
                    auto font = d->font();
                    font.setUnderline(true);
                    font.setBold(true);
                    d->setFont(font);
                }

                deps = this->dep.at(idx);
                for (const auto &idx : std::as_const(deps)) {
                    auto d = ui->plglist->item(idx);
                    auto font = d->font();
                    font.setItalic(true);
                    font.setBold(true);
                    d->setFont(font);
                }

                ui->plglist->blockSignals(false);
            });

    connect(ui->devlist, &QListWidget::itemChanged, this,
            [this](QListWidgetItem *item) {
                if (item == nullptr) {
                    return;
                }
                auto info = item->data(PLUIGN_META).value<PluginInfo>();
                auto id = info.id;
                switch (item->checkState()) {
                case Qt::Unchecked:
                    _devChanged.pushRemoveItem(id);
                    break;
                case Qt::Checked:
                    _devChanged.pushAddItem(id);
                    break;
                case Qt::PartiallyChecked:
                    break;
                }

                ui->btndevSave->setEnabled(_devChanged.containChanges());
            });
    connect(ui->devlist, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem *current, QListWidgetItem *) {
                if (current == nullptr) {
                    ui->txtd->clear();
                    return;
                }

                auto info = current->data(PLUIGN_META).value<PluginInfo>();
                auto plgName = current->data(PLUIGN_NAME).toString();
                auto plgComment = current->data(PLUIGN_COMMENT).toString();

                loadPluginInfo(info, plgName, plgComment, info.dependencies,
                               ui->txtd);
            });

    connect(ui->btnplgSave, &QPushButton::clicked, set, [this]() {
        SettingManager::instance().setEnabledExtPlugins(
            _plgChanged.getContents());
        _plgChanged.clear();
        ui->btnplgSave->setEnabled(false);
        Q_EMIT optionNeedRestartChanged();
    });
    connect(ui->btndevSave, &QPushButton::clicked, set, [this]() {
        SettingManager::instance().setEnabledDevPlugins(
            _devChanged.getContents());
        _devChanged.clear();
        ui->btndevSave->setEnabled(false);
        Q_EMIT optionNeedRestartChanged();
    });

    connect(ui->txtc, &QTextBrowser::anchorClicked, this,
            [this](const QUrl &url) {
                if (url.isLocalFile()) {
                    ShowInShell::showInGraphicalShell(this, url.toLocalFile(),
                                                      false);
                } else {
                    QDesktopServices::openUrl(url);
                }
            });

    auto cicon = ICONRES(QStringLiteral("cert"));
    auto &ct = WingPluginCert::instance();
    for (auto &&[c, loc] : ct.certificates().asKeyValueRange()) {
        auto lwi = new QListWidgetItem(cicon, c.subjectDisplayName() +
                                                  QStringLiteral(" (") +
                                                  loc.fileName() + ')');
        lwi->setData(CERT_CERT, QVariant::fromValue(c));
        if (!ct.isValidFingerPrint(c)) {
            auto font = lwi->font();
            font.setItalic(true);
            font.setBold(true);
            font.setStrikeOut(true);
            lwi->setFont(font);
        }
        ui->certlist->addItem(lwi);
    }

    connect(ui->certlist, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem *current, QListWidgetItem *) {
                if (current == nullptr) {
                    ui->txtcert->clear();
                    return;
                }
                auto c = current->data(CERT_CERT).value<QSslCertificate>();
                ui->txtcert->setText(c.toText());
            });

    auto ev = new EventFilter(QEvent::EnabledChange, this);
    connect(ev, &EventFilter::eventTriggered, this,
            [this](QObject *obj, QEvent *) {
                if (obj == ui->btnplgSave) {
                    auto tabName = QCoreApplication::translate(
                        "PluginSettingDialog", "PluginInfo", nullptr);
                    if (ui->btnplgSave->isEnabled()) {
                        tabName.append(QStringLiteral(" *"));
                    }
                    ui->tabWidget->setTabText(
                        ui->tabWidget->indexOf(ui->tabPluginInfo), tabName);
                } else if (obj == ui->btndevSave) {
                    auto tabName = QCoreApplication::translate(
                        "PluginSettingDialog", "DevExtInfo", nullptr);
                    if (ui->btndevSave->isEnabled()) {
                        tabName.append(QStringLiteral(" *"));
                    }
                    ui->tabWidget->setTabText(
                        ui->tabWidget->indexOf(ui->tabDevInfo), tabName);
                }
            });
    ui->btnplgSave->installEventFilter(ev);
    ui->btndevSave->installEventFilter(ev);

    resetChangedList();

    createPluginStandardMenu(ui->plglist);
    createPluginStandardMenu(ui->devlist);
}

PluginSettingDialog::~PluginSettingDialog() { delete ui; }

void PluginSettingDialog::reload() {
    this->blockSignals(true);
    auto &set = SettingManager::instance();
    auto ep = set.enablePlugin();
    ui->cbEnablePlugin->setChecked(ep);
    if (ep) {
        ui->cbEnablePluginRoot->setChecked(set.enablePlgInRoot());
    } else {
        ui->cbEnablePluginRoot->setEnabled(false);
    }
    this->blockSignals(false);
}

QIcon PluginSettingDialog::categoryIcon() const { return ICONRES("plugin"); }

QString PluginSettingDialog::name() const { return tr("Plugin"); }

QString PluginSettingDialog::id() const { return QStringLiteral("Plugin"); }

bool PluginSettingDialog::containUnsavedChanges() const {
    return ui->btndevSave->isEnabled() || ui->btnplgSave->isEnabled();
}

void PluginSettingDialog::highlightUnsavedChange() {
    if (ui->btnplgSave->isEnabled()) {
        ui->tabWidget->setCurrentWidget(ui->tabPluginInfo);
    } else if (ui->btndevSave->isEnabled()) {
        ui->tabWidget->setCurrentWidget(ui->tabDevInfo);
    }
}

void PluginSettingDialog::discard() {
    resetChangedList();
    resetUIChanged();
}

void PluginSettingDialog::restore() {
    SettingManager::instance().reset(SettingManager::SETTING::PLUGIN);
    reload();
}

void PluginSettingDialog::createPluginStandardMenu(QListWidget *widget) {
    widget->setSelectionMode(QListWidget::ExtendedSelection);
    widget->setContextMenuPolicy(Qt::ActionsContextMenu);
    widget->addAction(tr("SelectEnable"), widget, [widget]() {
        const auto sels = widget->selectedItems();
        for (const auto &item : sels) {
            auto flags = item->flags();
            if (flags.testFlag(Qt::ItemIsUserCheckable)) {
                item->setCheckState(Qt::Checked);
            }
        }
    });
    widget->addAction(tr("SelectDisable"), widget, [widget]() {
        const auto sels = widget->selectedItems();
        for (const auto &item : sels) {
            auto flags = item->flags();
            if (flags.testFlag(Qt::ItemIsUserCheckable)) {
                item->setCheckState(Qt::Unchecked);
            }
        }
    });
    auto a = new QAction(widget);
    a->setSeparator(true);
    widget->addAction(a);
    widget->addAction(tr("SelectAll"), widget, &QListWidget::selectAll);
    widget->addAction(tr("SelectClear"), widget, &QListWidget::clearSelection);
    a = new QAction(widget);
    a->setSeparator(true);
    widget->addAction(a);
    widget->addAction(tr("DiscardChanges"), this,
                      &PluginSettingDialog::discard);
}

void PluginSettingDialog::resetChangedList() {
    auto &set = SettingManager::instance();
    _plgChanged.setContents(set.enabledExtPlugins());
    _devChanged.setContents(set.enabledDevPlugins());
}

void PluginSettingDialog::resetUIChanged() {
    ui->plglist->blockSignals(true);
    ui->devlist->blockSignals(true);

    for (int i = 0; i < ui->plglist->count(); ++i) {
        auto item = ui->plglist->item(i);
        auto flags = item->flags();
        if (flags.testFlag(Qt::ItemIsUserCheckable)) {
            auto info = item->data(PLUIGN_META).value<PluginInfo>();
            auto id = info.id;
            if (_plgChanged.containChanges(id)) {
                item->setCheckState(Qt::Checked);
            } else {
                item->setCheckState(Qt::Unchecked);
            }
        }
    }

    for (int i = 0; i < ui->devlist->count(); ++i) {
        auto item = ui->devlist->item(i);
        auto flags = item->flags();
        if (flags.testFlag(Qt::ItemIsUserCheckable)) {
            auto info = item->data(PLUIGN_META).value<PluginInfo>();
            auto id = info.id;
            if (_devChanged.containChanges(id)) {
                item->setCheckState(Qt::Checked);
            } else {
                item->setCheckState(Qt::Unchecked);
            }
        }
    }

    ui->plglist->blockSignals(false);
    ui->devlist->blockSignals(false);

    ui->btnplgSave->setEnabled(false);
    ui->btndevSave->setEnabled(false);
}

void PluginSettingDialog::loadPluginInfo(
    const std::optional<WingHex::PluginInfo> &info, const QString &name,
    const QString &comment, const QList<WingHex::WingDependency> &dependencies,
    QTextBrowser *t) {
    if (info) {
        t->clear();
        static auto sep = QStringLiteral(" : ");
        appendWrappedText(t, tr("Name"), name);
        appendWrappedText(t, tr("ID"), info->id);
        appendWrappedText(t, tr("License"), info->license);
        appendWrappedText(t, tr("Author"), info->author);
        appendWrappedText(t, tr("Vendor"), info->vendor);
        appendWrappedText(t, tr("Version"), info->version.toString());

        auto url = info->url;
        if (!url.isEmpty()) {
            t->append(getWrappedText(
                tr("URL") + sep + QStringLiteral("<a href=\"") + url +
                QStringLiteral("\">") + url + QStringLiteral("</a>")));
        }

        auto certID = info->certID;
        if (!certID.isEmpty()) {
            auto &ct = WingPluginCert::instance();
            auto c = ct.cert(certID);
            auto finfo = ct.certLocation(c);
            if (finfo.exists()) {
                auto p = finfo.absoluteFilePath();
                t->append(getWrappedText(
                    tr("CertID") + sep + QStringLiteral("<a href=\"") +
                    Utilities::getUrlString(p) + QStringLiteral("\" title=\"") +
                    c.subjectDisplayName() + QStringLiteral("\">") +
                    finfo.fileName() + QStringLiteral("</a>")));
            } else {
                appendWrappedText(
                    t, tr("CertID"),
                    QString::fromLatin1(certID.toHex().toUpper()));
            }
        }

        if (!dependencies.isEmpty()) {
            ui->txtc->append(getWrappedText(tr("Dependencies") + sep));
            for (const auto &d : dependencies) {
                QString dep = QStringLiteral(" - ") + d.puid;
                if (!d.version.isNull()) {
                    dep.append(':').append(d.version.toString());
                }
                ui->txtc->append(getWrappedText(dep));
            }
        }

        t->append(getWrappedText({}));
        if (!comment.isEmpty()) {
            t->append(getWrappedText(tr("Comment") + sep));
            t->append(getWrappedText(comment));
        }
    } else {
        t->setText(tr("NoPluginLoaded"));
    }
}

QString PluginSettingDialog::getWrappedText(const QString &content) {
    return QStringLiteral("<a>") + content + QStringLiteral("</a>");
}

void PluginSettingDialog::appendWrappedText(QTextBrowser *t,
                                            const QString &header,
                                            const QString &content) {
    if (content.isEmpty()) {
        return;
    }
    t->append(QStringLiteral("<a>") + header + QStringLiteral(" : ") + content +
              QStringLiteral("</a>"));
}
