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

#ifndef ASBUILDER_H
#define ASBUILDER_H

#include "aspreprocesser.h"

#include <QMap>
#include <QSet>
#include <QVector>

class asBuilder : public AsPreprocesser {
public:
    explicit asBuilder(asIScriptEngine *engine);
    virtual ~asBuilder();

    // Start a new module
    virtual int StartNewModule(const char *moduleName);

    // Build the added script sections
    virtual int Build();

    // Returns the current module
    asIScriptModule *GetModule();

protected:
    asIScriptModule *module = nullptr;
};

#endif // ASBUILDER_H
