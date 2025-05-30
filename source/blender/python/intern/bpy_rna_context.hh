/* SPDX-FileCopyrightText: 2023 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup pythonintern
 */

#pragma once

#include <Python.h>

extern PyMethodDef BPY_rna_context_temp_override_method_def;

void bpy_rna_context_types_init();
