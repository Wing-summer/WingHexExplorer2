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

#include "asbuilder.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QObject>

asBuilder::asBuilder(asIScriptEngine *engine) : AsPreprocesser(engine) {}

int asBuilder::StartNewModule(const char *moduleName) {
    module = engine->GetModule(moduleName, asGM_ALWAYS_CREATE);
    if (module == nullptr)
        return -1;

    ClearAll();
    return 0;
}

asIScriptModule *asBuilder::GetModule() { return module; }

int asBuilder::Build() {
    Q_ASSERT(module);
    if (module == nullptr) {
        return -1;
    }

    for (auto &mod : modifiedScripts) {
        module->AddScriptSection(mod.section.toUtf8(), mod.script.data(),
                                 mod.script.size(), mod.lineOffset);
    }

    int r = module->Build();
    if (r < 0)
        return r;

    // After the script has been built, the metadata strings should be
    // stored for later lookup by function id, type id, and variable index
    for (int n = 0; n < (int)foundDeclarations.size(); n++) {
        SMetadataDecl &decl = foundDeclarations[n];
        module->SetDefaultNamespace(decl.nameSpace.toUtf8());
        if (decl.type == MDT_TYPE) {
            // Find the type id
            int typeId = module->GetTypeIdByDecl(decl.declaration.toUtf8());
            Q_ASSERT(typeId >= 0);
            if (typeId >= 0)
                typeMetadataMap.insert(typeId, decl.metadata);
        } else if (decl.type == MDT_FUNC) {
            if (decl.parentClass.isEmpty()) {
                // Find the function id
                asIScriptFunction *func =
                    module->GetFunctionByDecl(decl.declaration.toUtf8());
                Q_ASSERT(func);
                if (func)
                    funcMetadataMap.insert(func->GetId(), decl.metadata);
            } else {
                // Find the method id
                int typeId = module->GetTypeIdByDecl(decl.parentClass.toUtf8());
                Q_ASSERT(typeId > 0);

                auto it = classMetadataMap.find(typeId);
                if (it == classMetadataMap.end()) {
                    classMetadataMap.insert(typeId,
                                            SClassMetadata(decl.parentClass));
                    it = classMetadataMap.find(typeId);
                }

                asITypeInfo *type = engine->GetTypeInfoById(typeId);
                asIScriptFunction *func =
                    type->GetMethodByDecl(decl.declaration.toUtf8());
                Q_ASSERT(func);
                if (func)
                    it.value().funcMetadataMap.insert(func->GetId(),
                                                      decl.metadata);
            }
        } else if (decl.type == MDT_VIRTPROP) {
            if (decl.parentClass.isEmpty()) {
                // Find the global virtual property accessors
                asIScriptFunction *func = module->GetFunctionByName(
                    (QStringLiteral("get_") + decl.declaration).toUtf8());
                if (func)
                    funcMetadataMap.insert(func->GetId(), decl.metadata);
                func = module->GetFunctionByName(
                    (QStringLiteral("set_") + decl.declaration).toUtf8());
                if (func)
                    funcMetadataMap.insert(func->GetId(), decl.metadata);
            } else {
                // Find the method virtual property accessors
                int typeId = module->GetTypeIdByDecl(decl.parentClass.toUtf8());
                Q_ASSERT(typeId > 0);
                auto it = classMetadataMap.find(typeId);
                if (it == classMetadataMap.end()) {
                    classMetadataMap.insert(typeId,
                                            SClassMetadata(decl.parentClass));
                    it = classMetadataMap.find(typeId);
                }

                asITypeInfo *type = engine->GetTypeInfoById(typeId);
                asIScriptFunction *func = type->GetMethodByName(
                    (QStringLiteral("get_") + decl.declaration).toUtf8());
                if (func)
                    it.value().funcMetadataMap.insert(func->GetId(),
                                                      decl.metadata);
                func = type->GetMethodByName(
                    (QStringLiteral("set_") + decl.declaration).toUtf8());
                if (func)
                    it.value().funcMetadataMap.insert(func->GetId(),
                                                      decl.metadata);
            }
        } else if (decl.type == MDT_VAR) {
            if (decl.parentClass.isEmpty()) {
                // Find the global variable index
                int varIdx =
                    module->GetGlobalVarIndexByName(decl.declaration.toUtf8());
                Q_ASSERT(varIdx >= 0);
                if (varIdx >= 0)
                    varMetadataMap.insert(varIdx, decl.metadata);
            } else {
                int typeId = module->GetTypeIdByDecl(decl.parentClass.toUtf8());
                Q_ASSERT(typeId > 0);

                // Add the classes if needed
                auto it = classMetadataMap.find(typeId);
                if (it == classMetadataMap.end()) {
                    classMetadataMap.insert(typeId,
                                            SClassMetadata(decl.parentClass));
                    it = classMetadataMap.find(typeId);
                }

                // Add the variable to class
                asITypeInfo *objectType = engine->GetTypeInfoById(typeId);
                int idx = -1;

                // Search through all properties to get proper declaration
                for (asUINT i = 0; i < (asUINT)objectType->GetPropertyCount();
                     ++i) {
                    const char *name;
                    objectType->GetProperty(i, &name);
                    if (decl.declaration == name) {
                        idx = i;
                        break;
                    }
                }

                // If found, add it
                Q_ASSERT(idx >= 0);
                if (idx >= 0)
                    it.value().varMetadataMap.insert(idx, decl.metadata);
            }
        } else if (decl.type == MDT_FUNC_OR_VAR) {
            if (decl.parentClass.isEmpty()) {
                // Find the global variable index
                int varIdx =
                    module->GetGlobalVarIndexByName(decl.name.toUtf8());
                if (varIdx >= 0)
                    varMetadataMap.insert(varIdx, decl.metadata);
                else {
                    asIScriptFunction *func =
                        module->GetFunctionByDecl(decl.declaration.toUtf8());
                    Q_ASSERT(func);
                    if (func)
                        funcMetadataMap.insert(func->GetId(), decl.metadata);
                }
            } else {
                int typeId = module->GetTypeIdByDecl(decl.parentClass.toUtf8());
                Q_ASSERT(typeId > 0);

                // Add the classes if needed
                auto it = classMetadataMap.find(typeId);
                if (it == classMetadataMap.end()) {
                    classMetadataMap.insert(typeId,
                                            SClassMetadata(decl.parentClass));
                    it = classMetadataMap.find(typeId);
                }

                // Add the variable to class
                asITypeInfo *objectType = engine->GetTypeInfoById(typeId);
                int idx = -1;

                // Search through all properties to get proper declaration
                for (asUINT i = 0; i < (asUINT)objectType->GetPropertyCount();
                     ++i) {
                    const char *name;
                    objectType->GetProperty(i, &name);
                    if (decl.name == name) {
                        idx = i;
                        break;
                    }
                }

                // If found, add it
                if (idx >= 0)
                    it.value().varMetadataMap.insert(idx, decl.metadata);
                else {
                    // Look for the matching method instead
                    asITypeInfo *type = engine->GetTypeInfoById(typeId);
                    asIScriptFunction *func =
                        type->GetMethodByDecl(decl.declaration.toUtf8());
                    if (func)
                        it.value().funcMetadataMap.insert(func->GetId(),
                                                          decl.metadata);
                }
            }
        }
    }
    module->SetDefaultNamespace("");

    return 0;
}
