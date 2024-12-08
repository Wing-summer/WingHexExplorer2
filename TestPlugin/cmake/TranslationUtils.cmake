# ==============================================================================
# Copyright (C) 2024-2027 WingSummer
#
# You can redistribute this file and/or modify it under the terms of the BSD
# 3-Clause.
#
# THIS FILE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# =============================================================================

if(NOT TARGET Qt${QT_VERSION_MAJOR}::lconvert)
    message(
        FATAL_ERROR
            "The package \"Qt${QT_VERSION_MAJOR}LinguistTools\" is required.")
endif()

set(Qt_LCONVERT_EXECUTABLE Qt${QT_VERSION_MAJOR}::lconvert)

function(ADD_PLUGIN_TRANSLATIONS_RESOURCE res_file target)
    set(_qm_files ${ARGN})
    set(_res_file ${CMAKE_CURRENT_BINARY_DIR}/app_translations.qrc)

    file(
        WRITE ${_res_file}
        "<!DOCTYPE RCC><RCC version=\"1.0\">\n <qresource prefix=\"/PLGLANG/${target}\">\n"
    )
    foreach(_lang ${_qm_files})
        get_filename_component(_filename ${_lang} NAME)
        file(APPEND ${_res_file} "  <file>${_filename}</file>\n")
    endforeach()
    file(APPEND ${_res_file} " </qresource>\n</RCC>\n")

    set(${res_file}
        ${_res_file}
        PARENT_SCOPE)
endfunction()

add_custom_target(
    lupdate
    COMMAND ${Qt${QT_VERSION_MAJOR}_LUPDATE_EXECUTABLE} -recursive
            ${PROJECT_SOURCE_DIR} -ts *.ts
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Updating translations")
