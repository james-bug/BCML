#include "export_wireless_json.h"
#include "bcml_types.h"
#include "bcml_log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cjson/cJSON.h>

/**
 * @brief Serialize a radio config structure to a cJSON object.
 * @param radio_cfg Pointer to the radio config structure.
 * @return cJSON object pointer or NULL if input is invalid.
 */
static cJSON* export_radio_json(const bcml_wireless_radio_t* radio_cfg) {
    if (!radio_cfg) {
        BCML_LOG_WARN("export_radio_json: radio_cfg is NULL");
        return NULL;
    }
    BCML_LOG_DEBUG("export_radio_json: serializing radio at %p", radio_cfg);
    cJSON* obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "power", radio_cfg->power);
    cJSON_AddNumberToObject(obj, "channel2g", radio_cfg->channel2g);
    cJSON_AddNumberToObject(obj, "channel5g", radio_cfg->channel5g);
    cJSON_AddNumberToObject(obj, "bandwidth2g", radio_cfg->bandwidth2g);
    cJSON_AddNumberToObject(obj, "bandwidth5g", radio_cfg->bandwidth5g);
    cJSON_AddBoolToObject(obj, "dfs", radio_cfg->dfs);
    cJSON_AddBoolToObject(obj, "atf", radio_cfg->atf);
    cJSON_AddBoolToObject(obj, "bandsteering", radio_cfg->bandsteering);
    cJSON_AddBoolToObject(obj, "zerowait", radio_cfg->zerowait);
    return obj;
}

/**
 * @brief Serialize an ssid config structure to a cJSON object.
 * @param ssid_cfg Pointer to the ssid config structure.
 * @return cJSON object pointer or NULL if input is invalid.
 */
static cJSON* export_ssid_json(const bcml_wireless_ssid_t* ssid_cfg) {
    if (!ssid_cfg) {
        BCML_LOG_WARN("export_ssid_json: ssid_cfg is NULL\n");
        return NULL;
    }
    BCML_LOG_DEBUG("export_ssid_json: serializing ssid at %p (ssid: %s)\n", ssid_cfg, ssid_cfg->ssid);
    cJSON* obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "ssid", ssid_cfg->ssid);
    cJSON_AddBoolToObject(obj, "hide", ssid_cfg->hide);
    cJSON_AddNumberToObject(obj, "security", ssid_cfg->security);
    cJSON_AddStringToObject(obj, "password", ssid_cfg->password);
    cJSON_AddBoolToObject(obj, "password_onscreen", ssid_cfg->password_onscreen);
    cJSON_AddBoolToObject(obj, "enable2g", ssid_cfg->enable2g);
    cJSON_AddBoolToObject(obj, "enable5g", ssid_cfg->enable5g);
    cJSON_AddBoolToObject(obj, "isolation", ssid_cfg->isolation);
    cJSON_AddBoolToObject(obj, "hopping", ssid_cfg->hopping);
    return obj;
}

/**
 * @brief Export wireless config to JSON string.
 *        Only non-empty ssid entries will be exported (to support multiple SSID).
 * @param sdata Pointer to wireless config.
 * @param json_buffer Output buffer for JSON string.
 * @param buffer_size Size of output buffer.
 * @return true if exported successfully, false otherwise.
 */
bool export_wireless_json(const void* sdata, char* json_buffer, size_t buffer_size) {
    BCML_LOG_DEBUG("export_wireless_json: called with sdata=%p, json_buffer=%p, buffer_size=%zu\n", sdata, json_buffer, buffer_size);

    if (!sdata || !json_buffer || buffer_size == 0) {
        BCML_LOG_ERROR("export_wireless_json: Invalid input. sdata=%p, json_buffer=%p, buffer_size=%zu\n", sdata, json_buffer, buffer_size);
        return false;
    }

    const bcml_wireless_cfg_t* cfg = (const bcml_wireless_cfg_t*)sdata;

    cJSON* wireless_obj = cJSON_CreateObject();

    /* Serialize radio array */
    cJSON* radio_array = cJSON_CreateArray();
    for (int i = 0; i < MAX_RADIO_NUM; ++i) {
        // Export all radios (assuming all are valid, if not, add additional check here)
        BCML_LOG_DEBUG("export_wireless_json: serializing radio[%d]\n", i);
        cJSON* radio_obj = export_radio_json(&cfg->radio[i]);
        if (radio_obj) {
            cJSON_AddItemToArray(radio_array, radio_obj);
        } else {
            BCML_LOG_WARN("export_wireless_json: radio_obj is NULL for radio[%d]\n", i);
        }
    }
    cJSON_AddItemToObject(wireless_obj, "radio", radio_array);

    /* Serialize ssid array: only add ssid with non-empty string */
    cJSON* ssid_array = cJSON_CreateArray();
    int ssid_count = 0;
    for (int i = 0; i < MAX_SSID_NUM; ++i) {
        if (cfg->ssid[i].ssid[0] != '\0') {
            BCML_LOG_DEBUG("export_wireless_json: serializing ssid[%d] (ssid: %s)\n", i, cfg->ssid[i].ssid);
            cJSON* ssid_obj = export_ssid_json(&cfg->ssid[i]);
            if (ssid_obj) {
                cJSON_AddItemToArray(ssid_array, ssid_obj);
                ssid_count++;
            } else {
                BCML_LOG_WARN("export_wireless_json: ssid_obj is NULL for ssid[%d]\n", i);
            }
        } else {
            BCML_LOG_DEBUG("export_wireless_json: skip empty ssid[%d]\n", i);
        }
    }
    cJSON_AddItemToObject(wireless_obj, "ssid", ssid_array);

    /* Create root and add "wireless" object */
    cJSON* root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "wireless", wireless_obj);

    /* Print JSON to string */
    char* json_str = cJSON_PrintUnformatted(root);
    if (!json_str) {
        BCML_LOG_ERROR("export_wireless_json: cJSON_PrintUnformatted failed\n");
        cJSON_Delete(root);
        return false;
    }

    BCML_LOG_DEBUG("export_wireless_json: generated JSON string length=%zu\n", strlen(json_str));
    bool ok = (strlen(json_str) < buffer_size);
    if (ok) {
        strcpy(json_buffer, json_str);
        BCML_LOG_INFO("export_wireless_json: JSON exported successfully (length=%zu)\n", strlen(json_str));
    } else {
        json_buffer[0] = '\0';
        BCML_LOG_ERROR("export_wireless_json: Buffer too small (required=%zu, given=%zu)\n", strlen(json_str) + 1, buffer_size);
    }

    free(json_str);
    cJSON_Delete(root);
    return ok;
}