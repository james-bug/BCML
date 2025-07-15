#include "bcml_config.h"
#include "bcml_types.h"
#include "validator_wireless.h"
#include "parse_wireless_json.h"
#include "export_wireless_json.h"
// #include "bchal_display.h" // Include if display config type is supported
#include "sb_ops.h" // Include southbound interface
#include "bcml_log.h" // Include logging interface

#include <stdio.h>
#include <string.h>
#include <strings.h> // for strcasecmp

typedef struct {
    const char* type;
    bool (*validate)(const char* json, const char* schema_path);
    bool (*parse)(const char* json, void* sdata);
    bool (*export_json)(const void* sdata, char* json_buffer, size_t buffer_size); // Export config to JSON string
    void* cfg_instance;
    const char* schema_path;
} config_handler_t;

// Example: Wireless config handler
static bcml_wireless_cfg_t g_wireless_cfg;

static config_handler_t config_handlers[] = {
    {
        .type = "wireless",
        .validate = validate_wireless_json,
        .parse = parse_wireless_json,
        .export_json = export_wireless_json,
        .cfg_instance = &g_wireless_cfg,
        .schema_path = "schema/wireless_data_model_schema.json"
    },
    // Example for display config type
    // {
    //     .type = "display",
    //     .validate = validate_display_json,
    //     .parse = parse_display_json_adapter,
    //     .export_json = export_display_json_adapter,
    //     .cfg_instance = &g_display_cfg,
    //     .schema_path = "schema/display_data_model_schema.json"
    // },
};

static const config_handler_t* find_handler(const char* type) {
    size_t num = sizeof(config_handlers) / sizeof(config_handlers[0]);
    
    BCML_LOG_DEBUG("find_handler: searching type='%s' in %zu handlers.\n", type ? type : "(null)", num);
    for (size_t i = 0; i < num; ++i) {
        if (strcasecmp(type, config_handlers[i].type) == 0) {
            BCML_LOG_DEBUG("find_handler: FOUND handler for '%s' at index %zu\n", type, i);
            return &config_handlers[i];
        }
    }
    
    BCML_LOG_WARN("find_handler: No handler found for type: %s \n", type ? type : "(null)");
    return NULL;
}

bool bcml_config_set(const char* type, const char* json_data) {
    if (!type || !json_data)
        return false;

    const config_handler_t* handler = find_handler(type);
    if (!handler) {
        BCML_LOG_ERROR("Unknown config type: %s\n", type);
        return false;
    }

    if (!json_data || strlen(json_data) == 0) {
        BCML_LOG_WARN("bcml_config_set: JSON data is empty.\n");
        return false;
    }
    // Validate the JSON data against the schema
    if (handler->validate && !handler->validate(json_data, handler->schema_path)) {
        BCML_LOG_ERROR("%s JSON schema validation failed.\n", handler->type);
        return false;
    }
    // Parse the JSON data into the configuration instance
    if (handler->parse && !handler->parse(json_data, handler->cfg_instance)) {
        BCML_LOG_ERROR("%s JSON parsing failed.\n", handler->type);
        return false;
    }

    // Southbound: Dispatch to corresponding sb_ops by config type
    const sb_ops_entry_t* sb_entry = sb_ops_find(type);
    if (!sb_entry || !sb_entry->set) {
        BCML_LOG_ERROR("No southbound set for type: %s\n", type);
        return false;
    }
    if (!sb_entry->set(handler->cfg_instance)) {
        BCML_LOG_ERROR("Southbound set failed: %s\n", type);
        return false;
    }

    BCML_LOG_INFO("bcml_config_set: %s config applied via southbound.\n", handler->type);
    return true;
}

// Get config function: fetch from southbound and export to JSON
bool bcml_config_get(const char* type, char* json_buffer, size_t buffer_size) {
    if (!type || !json_buffer || buffer_size == 0) {
        BCML_LOG_WARN("bcml_config_get: Invalid input. %s %x %d \n", type, json_buffer, buffer_size);
        return false;
    }

    const config_handler_t* handler = find_handler(type);
    if (!handler) {
        BCML_LOG_ERROR("Unknown config type: %s\n", type);
        return false;
    }

    BCML_LOG_DEBUG("bcml_config_get: handler->type='%s' \n", handler->type);

    // 1. Southbound: retrieve current config structure
    BCML_LOG_DEBUG("bcml_config_get: finding sb_ops_entry for type '%s' \n", type);
    const sb_ops_entry_t* sb_entry = sb_ops_find(type);
    if (!sb_entry || !sb_entry->get) {
        BCML_LOG_ERROR("No southbound get for type: %s\n", type);
        return false;
    }
 
    BCML_LOG_DEBUG("bcml_config_get: calling sb_entry->get for type '%s' \n", type);
    if (!sb_entry->get(handler->cfg_instance)) {
        // If southbound get fails, we cannot export the config
        BCML_LOG_ERROR("Southbound get failed for type: %s\n", type);
        return false;
    }
    BCML_LOG_DEBUG("bcml_config_get: sb_entry->get succeeded for type '%s' \n", type);

    // 2. Export config structure to JSON string
    if (!handler->export_json) {
        BCML_LOG_ERROR("%s export_json not implemented.\n", handler->type);
        return false;
    }

    BCML_LOG_DEBUG("bcml_config_get: exporting JSON for type '%s' \n", handler->type);

    if (!handler->export_json(handler->cfg_instance, json_buffer, buffer_size)) {
        BCML_LOG_ERROR("%s export_json failed.\n", handler->type);
        return false;
    }

    // Optional: validate the exported JSON (for extra safety)
    if (handler->validate) {
        BCML_LOG_DEBUG("bcml_config_get: validating exported JSON for type '%s' \n", handler->type);
        if (!handler->validate(json_buffer, handler->schema_path)) {
            BCML_LOG_ERROR("%s exported JSON schema validation failed. \n", handler->type);
            return false;
        }
    }

    BCML_LOG_INFO("bcml_config_get: %s config exported to JSON.\n", handler->type);
    return true;
}