#!/bin/bash

# BCHAL V1.0 basic directory & file initializer

# Declare required directories and files
declare -A DIRS_AND_FILES=(
    ["include"]="api.h"
    ["lib/core"]="system.c"
    ["lib/mapping"]="mapping.c"
    ["lib/validator"]="validator.c"
    ["lib/adapters/vendor_a"]="system_adapter.c"
    ["utils"]="json_helper.c"
    ["cli"]="bchalcli.c"
)

# Create directories and files
for dir in "${!DIRS_AND_FILES[@]}"; do
    if [ ! -d "$dir" ]; then
        echo "Creating directory: $dir"
        mkdir -p "$dir"
    fi
    file="$dir/${DIRS_AND_FILES[$dir]}"
    if [ ! -f "$file" ]; then
        echo "Creating file: $file"
        case "$file" in
            *api.h)
cat <<'EOF' > "$file"
#ifndef _BCHAL_API_H_
#define _BCHAL_API_H_

#include <json-c/json.h>

// BCHAL API version
#define BCHAL_API_VERSION "1.0"

// BCHAL status codes
typedef enum {
    BCHAL_OK = 0,
    BCHAL_ERR_INVALID_PARAM = -1,
    BCHAL_ERR_UNSUPPORTED = -2,
    BCHAL_ERR_NOT_FOUND = -3,
    BCHAL_ERR_INTERNAL = -4,
    BCHAL_ERR_VALIDATE_FAIL = -5,
    // Add more as needed
} bchal_status_t;

// Optional: General response structure
typedef struct {
    bchal_status_t status;
    char message[128];             // Error description if any
    struct json_object *data;      // Data for get APIs
} bchal_response_t;

// Main BCHAL APIs

/**
 * bchal_set
 * Set the data for the specified module.
 * @param module   Module name (e.g. "system", "network", "wireless")
 * @param obj      Input data (json-c object)
 * @return         BCHAL_OK or error code
 */
int bchal_set(const char *module, struct json_object *obj);

/**
 * bchal_get
 * Get the data for the specified module.
 * @param module   Module name
 * @param result   Output pointer to json-c object
 * @return         BCHAL_OK or error code
 */
int bchal_get(const char *module, struct json_object **result);

/**
 * bchal_get_version
 * Get the BCHAL API version string.
 * @return         Version string (static, do not free)
 */
const char* bchal_get_version(void);

#endif // _BCHAL_API_H_
EOF
            ;;
            *system.c)
                echo 'void dummy_core_system() {}' > "$file"
            ;;
            *mapping.c)
                echo 'void dummy_mapping() {}' > "$file"
            ;;
            *validator.c)
                echo 'void dummy_validator() {}' > "$file"
            ;;
            *system_adapter.c)
                echo 'void dummy_adapter() {}' > "$file"
            ;;
            *json_helper.c)
                echo 'void dummy_utils_json() {}' > "$file"
            ;;
            *bchalcli.c)
                echo -e '#include <stdio.h>\nint main() { printf("BCHAL CLI demo\\n"); return 0; }' > "$file"
            ;;
            *)
                echo "// Dummy file for $file" > "$file"
            ;;
        esac
    else
        echo "File already exists: $file"
    fi
done

echo "BCHAL V1.0 base structure is ready."

