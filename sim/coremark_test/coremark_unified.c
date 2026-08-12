// Copyright 2026 Google LLC
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// coremark_unified.c

#define COMPILING_COREMARK_UNIFIED
#define HAS_PRINTF 1
#define HAS_STDIO 1
#define HAS_FLOAT 1

#include "core_portme.h"

// Include all Coremark C files
#include "core_main.c"
#include "core_list_join.c"
#include "core_matrix.c"
#include "core_state.c"
#include "core_util.c"
#include "core_portme.c"
