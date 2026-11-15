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

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <memory>
#include <vector>

#include "pluginsystem.h"

class ScriptPluginPackage;

class PluginLoader final {
public:
    PluginLoader(PluginSystem &system, bool devices, bool plugins);
    ~PluginLoader();

public:
    void load();
    static bool verifyCertificate(const QFileInfo &file,
                                  const QByteArray &fingerprint);

private:
    struct Candidate;
    void discover(bool device);
    bool validate(Candidate &candidate);
    void rejectDuplicates();
    bool dependenciesReady(const Candidate &candidate) const;
    void loadCandidate(Candidate &candidate);
    void persistEnabledPlugins();
    void block(const Candidate &candidate, PluginSystem::PluginStatus status);
    template <class Adapter>
    WingHex::IWingPluginBase *prepareAdapter(Candidate &candidate);

private:
    PluginSystem &m_system;
    bool m_devices;
    bool m_plugins;
    std::vector<std::unique_ptr<Candidate>> m_candidates;
};

#endif // PLUGINLOADER_H
