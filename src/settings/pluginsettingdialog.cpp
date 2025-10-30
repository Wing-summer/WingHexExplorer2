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
#include "class/eventfilter.h"
#include "class/pluginsystem.h"
#include "class/settingmanager.h"
#include "ui_pluginsettingdialog.h"
#include "utilities.h"

enum PLUGIN_INFO {
    PLUIGN_META = Qt::UserRole,
    PLUIGN_NAME,
    PLUIGN_COMMENT,
};

PluginSettingDialog::PluginSettingDialog(QWidget *parent)
    : WingHex::SettingPage(parent), ui(new Ui::PluginSettingDialog) {
    ui->setupUi(this);

    Utilities::addSpecialMark(ui->cbEnablePlugin);
    Utilities::addSpecialMark(ui->cbEnablePluginRoot);
    Utilities::addSpecialMark(ui->cbEnableManager);
    Utilities::addSpecialMark(ui->cbEnableHex);

    reload();

    bool ok = false;
    auto dis = qEnvironmentVariableIntValue("WING_DISABLE_PLUGIN_SYSTEM", &ok);
    if (dis && ok) {
        ui->groupBox->setEnabled(false);
        ui->tabPluginInfo->setEnabled(false);
        ui->tabAPIMon->setEnabled(false);
        ui->tabDevInfo->setEnabled(false);
        ui->tabHexEditorExt->setEnabled(false);
        return;
    } else {
        dis = qEnvironmentVariableIntValue("WING_DISABLE_PLUGIN", &ok);
        if (dis && ok) {
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

        dis = qEnvironmentVariableIntValue("WING_DISABLE_EXTDRV", &ok);
        if (dis && ok) {
            ui->tabDevInfo->setEnabled(false);
        }

        dis = qEnvironmentVariableIntValue("WING_DISABLE_MONITOR", &ok);
        if (dis && ok) {
            ui->cbEnableManager->setEnabled(false);
        } else {
            connect(ui->cbEnableManager,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
                    &QCheckBox::checkStateChanged,
#else
                    &QCheckBox::stateChanged,
#endif
                    this, &PluginSettingDialog::optionNeedRestartChanged);
        }

        dis = qEnvironmentVariableIntValue("WING_DISABLE_HEXEXT", &ok);
        if (dis && ok) {
            ui->cbEnableManager->setEnabled(false);
        } else {
            connect(ui->cbEnableHex,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
                    &QCheckBox::checkStateChanged,
#else
                    &QCheckBox::stateChanged,
#endif
                    this, &PluginSettingDialog::optionNeedRestartChanged);
        }
    }

    connect(ui->cbEnablePluginRoot,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this, &PluginSettingDialog::optionNeedRestartChanged);

    auto &plgsys = PluginSystem::instance();
    auto pico = ICONRES("plugin");
    ui->plglist->clear();

    for (auto &p : plgsys.plugins()) {
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
        ui->plglist->addItem(lwi);
    }

    pico = ICONRES("plugindis");
    auto blkplgs = plgsys.blockedPlugins();
    auto dblkplgs = blkplgs[PluginSystem::BlockReason::Disabled];
    for (auto &meta : dblkplgs) {
        auto lwi = new QListWidgetItem(pico, meta.id);
        auto flags = lwi->flags();
        flags.setFlag(Qt::ItemIsUserCheckable);
        lwi->setFlags(flags);
        lwi->setData(PLUIGN_META, QVariant::fromValue(meta));
        lwi->setCheckState(Qt::Unchecked);
        ui->plglist->addItem(lwi);
    }

    pico = getGrayIcon(NAMEICONRES("plugin"));
    dblkplgs = blkplgs[PluginSystem::BlockReason::BlockedByManager];
    for (auto &meta : dblkplgs) {
        auto lwi = new QListWidgetItem(pico, meta.id);
        auto flags = lwi->flags();
        flags.setFlag(Qt::ItemIsUserCheckable, false);
        lwi->setFlags(flags);
        auto font = lwi->font();
        font.setStrikeOut(true);
        lwi->setFont(font);
        lwi->setData(PLUIGN_META, QVariant::fromValue(meta));
        ui->plglist->addItem(lwi);
    }

    pico = ICONRES("devext");
    ui->devlist->clear();
    for (auto &d : plgsys.devices()) {
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
    auto blkdevs = plgsys.blockedDevPlugins();
    auto dblkdevs = blkdevs[PluginSystem::BlockReason::Disabled];
    for (auto &meta : dblkdevs) {
        auto lwi = new QListWidgetItem(pico, meta.id);
        auto flags = lwi->flags();
        flags.setFlag(Qt::ItemIsUserCheckable);
        lwi->setFlags(flags);
        lwi->setData(PLUIGN_META, QVariant::fromValue(meta));
        lwi->setCheckState(Qt::Unchecked);
        ui->devlist->addItem(lwi);
    }

    pico = getGrayIcon(NAMEICONRES("devext"));
    dblkdevs = blkdevs[PluginSystem::BlockReason::BlockedByManager];
    for (auto &meta : dblkdevs) {
        auto lwi = new QListWidgetItem(pico, meta.id);
        auto flags = lwi->flags();
        flags.setFlag(Qt::ItemIsUserCheckable, false);
        lwi->setFlags(flags);
        auto font = lwi->font();
        font.setStrikeOut(true);
        lwi->setFont(font);
        lwi->setData(PLUIGN_META, QVariant::fromValue(meta));
        ui->devlist->addItem(lwi);
    }

    auto minfo = plgsys.monitorManagerInfo();
    QString mcomment;
    auto mp = plgsys.monitorManager();
    if (mp) {
        mcomment = mp->comment();
    }
    loadPluginInfo(minfo, {}, mcomment, {}, ui->txtm);

    auto hinfo = plgsys.hexEditorExtensionInfo();
    QString hcomment;
    auto hp = plgsys.hexEditorExtension();
    if (hp) {
        hcomment = hp->comment();
    }
    loadPluginInfo(hinfo, {}, hcomment, {}, ui->txtext);

    auto set = &SettingManager::instance();
    connect(ui->cbEnablePlugin, &QCheckBox::toggled, set,
            &SettingManager::setEnablePlugin);
    connect(ui->cbEnablePluginRoot, &QCheckBox::toggled, set,
            &SettingManager::setEnablePlgInRoot);
    connect(ui->cbEnableManager, &QCheckBox::toggled, set,
            &SettingManager::setEnableMonitor);
    connect(ui->cbEnableHex, &QCheckBox::toggled, set,
            &SettingManager::setEnableHexExt);

    connect(ui->plglist, &QListWidget::itemChanged, this,
            [this](QListWidgetItem *item) {
                if (item == nullptr) {
                    return;
                }
                auto info = item->data(PLUIGN_META).value<PluginInfo>();
                auto id = info.id;
                switch (item->checkState()) {
                case Qt::Unchecked:
                    _plgChanged.pushRemoveItem(id);
                    break;
                case Qt::Checked:
                    _plgChanged.pushAddItem(id);
                    break;
                case Qt::PartiallyChecked:
                    break;
                }

                ui->btnplgSave->setEnabled(_plgChanged.containChanges());
            });
    connect(ui->plglist, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem *current, QListWidgetItem *) {
                if (current == nullptr) {
                    return;
                }

                auto info = current->data(PLUIGN_META).value<PluginInfo>();
                auto plgName = current->data(PLUIGN_NAME).toString();
                auto plgComment = current->data(PLUIGN_COMMENT).toString();

                loadPluginInfo(info, plgName, plgComment, info.dependencies,
                               ui->txtc);
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
        ui->cbEnableManager->setChecked(set.enableMonitor());
        ui->cbEnableHex->setChecked(set.enableHexExt());
    } else {
        ui->cbEnablePluginRoot->setEnabled(false);
        ui->cbEnableManager->setEnabled(false);
        ui->cbEnableHex->setEnabled(false);
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
        for (auto &item : widget->selectedItems()) {
            auto flags = item->flags();
            if (flags.testFlag(Qt::ItemIsUserCheckable)) {
                item->setCheckState(Qt::Checked);
            }
        }
    });
    widget->addAction(tr("SelectDisable"), widget, [widget]() {
        for (auto &item : widget->selectedItems()) {
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

QIcon PluginSettingDialog::getGrayIcon(const QString &path) {
    QImage dpico(path);
    for (int y = 0; y < dpico.height(); ++y) {
        auto row = reinterpret_cast<QRgb *>(dpico.scanLine(y));
        for (int x = 0; x < dpico.width(); ++x) {
            auto c = row[x];
            if (c) {
                auto p = qGray(c);
                row[x] = qRgba(p, p, p, qAlpha(c));
            }
        }
    }
    return QIcon(QPixmap::fromImage(dpico));
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
        if (!name.isEmpty()) {
            t->append(getWrappedText(tr("Name") + sep + name));
        }
        t->append(getWrappedText(tr("ID") + sep + info->id));
        t->append(getWrappedText(tr("License") + sep + info->license));
        t->append(getWrappedText(tr("Author") + sep + info->author));
        t->append(getWrappedText(tr("Vendor") + sep + info->vendor));
        t->append(
            getWrappedText(tr("Version") + sep + info->version.toString()));
        t->append(getWrappedText(
            tr("URL") + sep + QStringLiteral("<a href=\"") + info->url +
            QStringLiteral("\">") + info->url + QStringLiteral("</a>")));

        if (!dependencies.isEmpty()) {
            ui->txtc->append(getWrappedText(tr("Dependencies") + sep));
            for (auto &d : dependencies) {
                ui->txtc->append(getWrappedText(QString(4, ' ') + tr("PUID") +
                                                sep + d.puid));
                ui->txtc->append(getWrappedText(QString(4, ' ') +
                                                tr("Version") + sep +
                                                d.version.toString()));
            }
        }

        t->append(getWrappedText({}));
        t->append(getWrappedText(tr("Comment") + sep));
        t->append(getWrappedText(comment));
    } else {
        t->setText(tr("NoPluginLoaded"));
    }
}

QString PluginSettingDialog::getWrappedText(const QString &str) {
    return QStringLiteral("<a>") + str + QStringLiteral("</a>");
}
