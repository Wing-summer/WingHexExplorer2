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
    connect(ui->cbEnablePlugin,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this, &PluginSettingDialog::optionNeedRestartChanged);
    connect(ui->cbEnablePluginRoot,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this, &PluginSettingDialog::optionNeedRestartChanged);

    reload();

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

    ui->txtc->clear();

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

    ui->txtd->clear();

    auto minfo = plgsys.monitorManagerInfo();
    if (minfo) {
        auto sep = QStringLiteral(" : ");

        ui->txtm->append(getWrappedText(tr("ID") + sep + minfo->id));
        ui->txtm->append(getWrappedText(tr("License") + sep + minfo->license));
        ui->txtm->append(getWrappedText(tr("Author") + sep + minfo->author));
        ui->txtm->append(getWrappedText(tr("Vendor") + sep + minfo->vendor));
        ui->txtm->append(
            getWrappedText(tr("Version") + sep + minfo->version.toString()));
        ui->txtm->append(getWrappedText(
            tr("URL") + sep + QStringLiteral("<a href=\"") + minfo->url +
            QStringLiteral("\">") + minfo->url + QStringLiteral("</a>")));
        ui->txtm->append(getWrappedText(tr("Comment") + sep));
        auto p = plgsys.monitorManager();
        if (p) {
            ui->txtm->append(getWrappedText(p->comment()));
        }
    } else {
        ui->txtm->setText(tr("NoMonitorPlugin"));
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
    connect(
        ui->plglist, &QListWidget::currentItemChanged, this,
        [this](QListWidgetItem *current, QListWidgetItem *) {
            if (current == nullptr) {
                return;
            }

            auto info = current->data(PLUIGN_META).value<PluginInfo>();
            auto plgName = current->data(PLUIGN_NAME).toString();
            auto plgComment = current->data(PLUIGN_COMMENT).toString();

            ui->txtc->clear();
            static auto sep = QStringLiteral(" : ");
            ui->txtc->append(getWrappedText(tr("ID") + sep + info.id));
            ui->txtc->append(getWrappedText(tr("Name") + sep + plgName));
            ui->txtc->append(
                getWrappedText(tr("License") + sep + info.license));
            ui->txtc->append(getWrappedText(tr("Author") + sep + info.author));
            ui->txtc->append(getWrappedText(tr("Vendor") + sep + info.vendor));
            ui->txtc->append(
                getWrappedText(tr("Version") + sep + info.version.toString()));
            ui->txtc->append(getWrappedText(tr("Comment") + sep + plgComment));
            if (!info.dependencies.isEmpty()) {
                ui->txtc->append(getWrappedText(tr("pluginDependencies:")));
                for (auto &d : info.dependencies) {
                    ui->txtc->append(
                        getWrappedText(QString(4, ' ') + tr("PUID:") + d.puid));
                    ui->txtc->append(getWrappedText(QString(4, ' ') +
                                                    tr("Version:") +
                                                    d.version.toString()));
                }
            }
            ui->txtc->append(getWrappedText(
                tr("URL") + sep + QStringLiteral("<a href=\"") + info.url +
                QStringLiteral("\">") + info.url + QStringLiteral("</a> ")));
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
    connect(
        ui->devlist, &QListWidget::currentItemChanged, this,
        [this](QListWidgetItem *current, QListWidgetItem *) {
            if (current == nullptr) {
                return;
            }

            auto info = current->data(PLUIGN_META).value<PluginInfo>();
            auto plgName = current->data(PLUIGN_NAME).toString();
            auto plgComment = current->data(PLUIGN_COMMENT).toString();

            ui->txtd->clear();
            static auto sep = QStringLiteral(" : ");
            ui->txtd->append(getWrappedText(tr("ID") + sep + info.id));
            ui->txtd->append(getWrappedText(tr("Name") + sep + plgName));
            ui->txtd->append(
                getWrappedText(tr("License") + sep + info.license));
            ui->txtd->append(getWrappedText(tr("Author") + sep + info.author));
            ui->txtd->append(getWrappedText(tr("Vendor") + sep + info.vendor));
            ui->txtd->append(
                getWrappedText(tr("Version") + sep + info.version.toString()));
            ui->txtd->append(getWrappedText(tr("Comment") + sep + plgComment));
            ui->txtd->append(getWrappedText(
                tr("URL") + sep + QStringLiteral("<a href=\"") + info.url +
                QStringLiteral("\">") + info.url + QStringLiteral("</a>")));
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
    ui->cbEnablePlugin->setChecked(set.enablePlugin());
    ui->cbEnablePluginRoot->setChecked(set.enablePlgInRoot());
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
    resetUIChagned();
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

void PluginSettingDialog::resetUIChagned() {
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

QString PluginSettingDialog::getWrappedText(const QString &str) {
    return QStringLiteral("<a>") + str + QStringLiteral("</a>");
}
