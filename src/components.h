/*
 * Copyright 2026 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * cels-cli Components
 *
 * App-specific types and re-export of shared widget components.
 */

#ifndef CELS_CLI_COMPONENTS_H
#define CELS_CLI_COMPONENTS_H

#include <cels-widgets/compositions.h>

/* ============================================================================
 * Enums
 * ============================================================================ */

typedef enum {
    TAB_PROJECTS = 0,
    TAB_PACKAGES,
    TAB_BUILD,
    TAB_TESTS,
    TAB_COUNT
} CliTab;

#endif /* CELS_CLI_COMPONENTS_H */
