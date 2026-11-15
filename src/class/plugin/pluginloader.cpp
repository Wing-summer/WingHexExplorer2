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

#include "pluginloader.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QPluginLoader>
#include <QScopeGuard>

#include "class/application/logger.h"
#include "class/file/fileaccesscheck.h"
#include "class/security/wingcert.h"
#include "class/settings/settingmanager.h"
#include "luau/plugin/scriptplugin.h"
#include "utilities.h"

struct PluginLoader::Candidate {
    QFileInfo file;
    PluginInfo meta;
    bool device = false;
    bool processed = false;
    std::shared_ptr<ScriptPluginPackage> script;
    std::unique_ptr<QPluginLoader> native;
};

namespace {
bool requiresCertificate(const QFileInfo &file) {
    return !qEnvironmentVariableBool("WINGHEX_CERT_SKIP", false) &&
           (qEnvironmentVariableBool("WINGHEX_CERT_FORCE_ALL", false) ||
            FileAccessCheck::canStandardUserWriteFile(file) !=
                FileAccessCheck::Status::Deny);
}
} // namespace

PluginLoader::PluginLoader(PluginSystem &system, bool devices, bool plugins)
    : m_system(system), m_devices(devices), m_plugins(plugins) {}

PluginLoader::~PluginLoader() = default;

bool PluginLoader::verifyCertificate(const QFileInfo &file,
                                     const QByteArray &fingerprint) {
    return !requiresCertificate(file) ||
           WingCert::instance().verify(file, fingerprint);
}

void PluginLoader::block(const Candidate &candidate,
                         PluginSystem::PluginStatus status) {
    (candidate.device ? m_system.m_blkdevs : m_system.m_blkplgs)
        .append({status, candidate.meta});
}

void PluginLoader::discover(bool device) {
    const auto folder =
        device ? QStringLiteral("devdrv") : QStringLiteral("plugin");
    const auto suffix =
        device ? QStringLiteral("*.wingdrv") : QStringLiteral("*.wingplg");
    for (const auto &root : {QCoreApplication::applicationDirPath(),
                             Utilities::getAppDataPath()}) {
        const auto directory = QDir(root).filePath(folder);
        const auto files = m_system.findPlugins(directory, suffix);
        for (const auto &file : files) {
            auto candidate = std::make_unique<Candidate>();
            candidate->file = file;
            candidate->device = device;
            candidate->meta = {file.baseName(), -1};
            QFile content(file.absoluteFilePath());
            if (!content.open(QIODevice::ReadOnly)) {
                block(*candidate, PluginSystem::PluginStatus::InvalidPlugin);
                continue;
            }
            const bool archive =
                ScriptPluginPackage::hasZipSignature(content.peek(2));
            content.close();
            if (archive) {
                candidate->script = std::make_shared<ScriptPluginPackage>();
                const auto loaded =
                    candidate->script->load(file.absoluteFilePath());
                if (!loaded) {
                    qCritical().noquote() << file.fileName() << loaded.message;
                    block(*candidate,
                          PluginSystem::PluginStatus::InvalidPlugin);
                    continue;
                }
                candidate->meta =
                    m_system.parsePluginMetadata(candidate->script->manifest());
            } else {
#ifdef Q_OS_UNIX
                if (!file.isExecutable()) {
                    block(*candidate,
                          PluginSystem::PluginStatus::InvalidPlugin);
                    continue;
                }
#endif
                candidate->native =
                    std::make_unique<QPluginLoader>(file.absoluteFilePath());
                const auto meta =
                    m_system.readPluginMetaHeader(candidate->native);
                if (!meta) {
                    block(*candidate,
                          PluginSystem::PluginStatus::InvalidPlugin);
                    continue;
                }
                candidate->meta = *meta;
            }
            if (validate(*candidate)) {
                m_candidates.push_back(std::move(candidate));
            }
        }
    }
}

bool PluginLoader::validate(Candidate &candidate) {
    const auto status = m_system.checkPluginMetadata(candidate.meta);
    if (status != PluginSystem::PluginStatus::Valid) {
        block(candidate, status);
        return false;
    }
    bool verified = true;
    if (requiresCertificate(candidate.file)) {
        auto &certificates = WingCert::instance();
        if (candidate.script) {
            QBuffer content;
            content.setData(candidate.script->archiveData());
            verified = content.open(QIODevice::ReadOnly) &&
                       certificates.verify(
                           content, candidate.meta.certID,
                           WingCert::signatureFilePath(candidate.file));
        } else {
            verified =
                certificates.verify(candidate.file, candidate.meta.certID);
        }
    }
    if (!verified) {
        block(candidate, PluginSystem::PluginStatus::CertNotPassed);
        return false;
    }
    const auto &enabled =
        candidate.device ? m_system.m_enabledDevIDs : m_system.m_enabledExtIDs;
    if (!enabled.contains(candidate.meta.id)) {
        block(candidate, PluginSystem::PluginStatus::Blocked);
        return false;
    }
    return true;
}

void PluginLoader::rejectDuplicates() {
    QHash<QString, int> counts;
    for (const auto &candidate : m_candidates) {
        ++counts[candidate->meta.id.toCaseFolded()];
    }
    for (auto &candidate : m_candidates) {
        if (counts.value(candidate->meta.id.toCaseFolded()) > 1 ||
            m_system.isPluginLoaded(candidate->meta.id)) {
            candidate->processed = true;
            block(*candidate, PluginSystem::PluginStatus::DupID);
        }
    }
}

bool PluginLoader::dependenciesReady(const Candidate &candidate) const {
    for (const auto &dependency : candidate.meta.dependencies) {
        if (!m_system.isPluginLoaded(dependency)) {
            return false;
        }
    }
    return true;
}

template <class Adapter>
IWingPluginBase *PluginLoader::prepareAdapter(Candidate &candidate) {
    auto adapter = std::make_unique<Adapter>(candidate.script);
    m_system.m_pinfos.insert(adapter.get(), candidate.meta);
    const auto prepared = adapter->prepare();
    if (!prepared) {
        qCritical().noquote() << candidate.meta.id << prepared.message;
        auto *object = adapter.get();
        m_system.m_pinfos.remove(object);
        adapter.reset();
        m_system.m_inactivePlugins.remove(object);
        return nullptr;
    }
    return adapter.release();
}

void PluginLoader::loadCandidate(Candidate &candidate) {
    PluginLoadGuard guard(candidate.file);
    m_system.m_curLoadingPlg = candidate.file.fileName();
    const auto clearLoading =
        qScopeGuard([this] { m_system.m_curLoadingPlg.clear(); });
    IWingPluginBase *object = nullptr;
    if (candidate.script) {
        object = candidate.device
                     ? prepareAdapter<ScriptDevicePlugin>(candidate)
                     : prepareAdapter<ScriptPlugin>(candidate);
    } else {
        auto *instance = candidate.native->instance();
        object = candidate.device ? static_cast<IWingPluginBase *>(
                                        qobject_cast<IWingDevice *>(instance))
                                  : static_cast<IWingPluginBase *>(
                                        qobject_cast<IWingPlugin *>(instance));
    }
    QDir settings(Utilities::getAppDataPath());
    const auto folder =
        candidate.device ? QStringLiteral("drvset") : QStringLiteral("plgset");
    const auto directory = settings.mkpath(folder) && settings.cd(folder)
                               ? std::make_optional(settings)
                               : std::nullopt;
    bool accepted = false;
    if (object) {
        m_system.m_pinfos.insert(object, candidate.meta);
        m_system.retranslateMetadata(object, candidate.meta);
        accepted = candidate.device
                       ? m_system.loadPlugin(static_cast<IWingDevice *>(object),
                                             candidate.meta, directory)
                       : m_system.loadPlugin(static_cast<IWingPlugin *>(object),
                                             candidate.meta, directory);
    }
    if (accepted) {
        m_system.m_loadedPlugins.append(object);
        if (candidate.native) {
            candidate.native->setLoadHints(QLibrary::PreventUnloadHint);
        }
    } else {
        Logger::critical(
            PluginSystem::tr("Plugin [%1] initialization failed: %2")
                .arg(candidate.meta.id, candidate.file.fileName()));
        block(candidate, PluginSystem::PluginStatus::InitFailed);
        m_system.m_pinfos.remove(object);
        if (candidate.script) {
            delete object;
            m_system.m_inactivePlugins.remove(object);
        }
    }
}

void PluginLoader::persistEnabledPlugins() {
    auto &settings = SettingManager::instance();
    QStringList ordinary, devices;
    for (auto *plugin : m_system.plugins()) {
        ordinary.append(m_system.m_pinfos.value(plugin).id);
    }
    for (auto *device : m_system.devices()) {
        devices.append(m_system.m_pinfos.value(device).id);
    }
    if (m_plugins) {
        settings.setEnabledExtPlugins(ordinary);
    }
    if (m_devices) {
        settings.setEnabledDevPlugins(devices);
    }
}

void PluginLoader::load() {
    if (m_devices) {
        discover(true);
    }
    if (m_plugins) {
        discover(false);
    }
    rejectDuplicates();
    bool progress;
    do {
        progress = false;
        for (auto &candidate : m_candidates) {
            if (candidate->processed || !dependenciesReady(*candidate)) {
                continue;
            }
            candidate->processed = true;
            progress = true;
            loadCandidate(*candidate);
        }
    } while (progress);
    for (const auto &candidate : m_candidates) {
        if (!candidate->processed) {
            block(*candidate, PluginSystem::PluginStatus::LackDependencies);
        }
    }
    persistEnabledPlugins();
    m_system.printErrorSummary(m_system.m_blkplgs);
    m_system.printErrorSummary(m_system.m_blkdevs);
}
