#include "validator_wireless.h"
#include "bcml_log.h"
#include <stdio.h>
#include <string.h>
#include <cjson/cJSON.h>

static int validate_radio(const cJSON *radio_item) {
    if (!cJSON_IsObject(radio_item)) {
        BCML_LOG_WARN("validate_radio: radio_item is not an object\n");
        return 0;
    }

    // Validate integer fields with range
    const cJSON *power = cJSON_GetObjectItemCaseSensitive(radio_item, "power");
    if (!cJSON_IsNumber(power) || power->valueint < 0 || power->valueint > 100) {
        BCML_LOG_WARN("validate_radio: invalid power value\n");
        return 0;
    }

    const cJSON *channel2g = cJSON_GetObjectItemCaseSensitive(radio_item, "channel2g");
    if (!cJSON_IsNumber(channel2g) || channel2g->valueint < 0) {
        BCML_LOG_WARN("validate_radio: invalid channel2g value\n");
        return 0;
    }

    const cJSON *channel5g = cJSON_GetObjectItemCaseSensitive(radio_item, "channel5g");
    if (!cJSON_IsNumber(channel5g) || channel5g->valueint < 0) {
        BCML_LOG_WARN("validate_radio: invalid channel5g value\n");
        return 0;
    }

    const cJSON *bandwidth2g = cJSON_GetObjectItemCaseSensitive(radio_item, "bandwidth2g");
    if (!cJSON_IsNumber(bandwidth2g) || bandwidth2g->valueint < 0) {
        BCML_LOG_WARN("validate_radio: invalid bandwidth2g value\n");
        return 0;
    }

    const cJSON *bandwidth5g = cJSON_GetObjectItemCaseSensitive(radio_item, "bandwidth5g");
    if (!cJSON_IsNumber(bandwidth5g) || bandwidth5g->valueint < 0) {
        BCML_LOG_WARN("validate_radio: invalid bandwidth5g value\n");
        return 0;
    }

    // Validate boolean fields
    const char* bool_fields[] = {"dfs", "atf", "bandsteering", "zerowait"};
    for (int i = 0; i < 4; ++i) {
        const cJSON *field = cJSON_GetObjectItemCaseSensitive(radio_item, bool_fields[i]);
        if (!cJSON_IsBool(field)) {
            BCML_LOG_WARN("validate_radio: invalid boolean field %s\n", bool_fields[i]);
            return 0;
        }
    }

    // Check for additionalProperties == false
    int known_fields = 9; // total number of known fields for radio
    if (cJSON_GetArraySize(radio_item) > known_fields) {
        BCML_LOG_WARN("validate_radio: radio_item has unknown fields\n");
        return 0;
    }

    return 1;
}

static int validate_ssid(const cJSON *ssid_item) {
    if (!cJSON_IsObject(ssid_item)) {
        BCML_LOG_WARN("validate_ssid: ssid_item is not an object\n");
        return 0;
    }

    // Validate string fields
    const cJSON *ssid = cJSON_GetObjectItemCaseSensitive(ssid_item, "ssid");
    if (!cJSON_IsString(ssid) || strlen(ssid->valuestring) == 0 || strlen(ssid->valuestring) > 64) {
        BCML_LOG_WARN("validate_ssid: invalid ssid string\n");
        return 0;
    }

    const cJSON *password = cJSON_GetObjectItemCaseSensitive(ssid_item, "password");
    if (!cJSON_IsString(password) || strlen(password->valuestring) > 64) {
        BCML_LOG_WARN("validate_ssid: invalid password string\n");
        return 0;
    }

    // Validate integer field
    const cJSON *security = cJSON_GetObjectItemCaseSensitive(ssid_item, "security");
    if (!cJSON_IsNumber(security) || security->valueint < 0) {
        BCML_LOG_WARN("validate_ssid: invalid security value\n");
        return 0;
    }

    // Validate boolean fields
    const char* bool_fields[] = {
        "hide", "password_onscreen", "enable2g", "enable5g", "isolation", "hopping"
    };
    for (int i = 0; i < 6; ++i) {
        const cJSON *field = cJSON_GetObjectItemCaseSensitive(ssid_item, bool_fields[i]);
        if (!cJSON_IsBool(field)) {
            BCML_LOG_WARN("validate_ssid: invalid boolean field %s\n", bool_fields[i]);
            return 0;
        }
    }

    // Check for additionalProperties == false
    int known_fields = 9; // total number of known fields for ssid
    if (cJSON_GetArraySize(ssid_item) > known_fields) {
        BCML_LOG_WARN("validate_ssid: ssid_item has unknown fields\n");
        return 0;
    }

    return 1;
}

bool validate_wireless_json(const char* json, const char* schema_path) {
    (void)schema_path; // Not used, as validation is hardcoded for now

    BCML_LOG_DEBUG("validate_wireless_json: called. json=%p, schema_path=%s\n", json, schema_path ? schema_path : "(null)\n");
    if (!json) {
        BCML_LOG_ERROR("validate_wireless_json: json is NULL\n");
        return false;
    }

    cJSON *root = cJSON_Parse(json);
    if (!root) {
        BCML_LOG_ERROR("validate_wireless_json: JSON parse error\n");
        return false;
    }

    // Top-level: required "wireless"
    const cJSON *wireless = cJSON_GetObjectItemCaseSensitive(root, "wireless");
    if (!cJSON_IsObject(wireless)) {
        BCML_LOG_ERROR("validate_wireless_json: Missing or invalid 'wireless' object\n");
        cJSON_Delete(root);
        return false;
    }

    // wireless: required "radio" (array)
    const cJSON *radio = cJSON_GetObjectItemCaseSensitive(wireless, "radio");
    if (!cJSON_IsArray(radio)) {
        BCML_LOG_ERROR("validate_wireless_json: Missing or invalid 'radio' array\n");
        cJSON_Delete(root);
        return false;
    }
    int radio_count = cJSON_GetArraySize(radio);
    if (radio_count < 1 || radio_count > 4) {
        BCML_LOG_ERROR("validate_wireless_json: 'radio' array size out of bounds: %d\n", radio_count);
        cJSON_Delete(root);
        return false;
    }
    for (int i = 0; i < radio_count; ++i) {
        if (!validate_radio(cJSON_GetArrayItem(radio, i))) {
            BCML_LOG_ERROR("validate_wireless_json: Invalid 'radio' item at index %d\n", i);
            cJSON_Delete(root);
            return false;
        }
    }

    // wireless: required "ssid" (array)
    const cJSON *ssid = cJSON_GetObjectItemCaseSensitive(wireless, "ssid");
    if (!cJSON_IsArray(ssid)) {
        BCML_LOG_ERROR("validate_wireless_json: Missing or invalid 'ssid' array\n");
        cJSON_Delete(root);
        return false;
    }
    int ssid_count = cJSON_GetArraySize(ssid);
    if (ssid_count < 1 || ssid_count > 4) {
        BCML_LOG_ERROR("validate_wireless_json: 'ssid' array size out of bounds: %d\n", ssid_count);
        cJSON_Delete(root);
        return false;
    }
    for (int i = 0; i < ssid_count; ++i) {
        if (!validate_ssid(cJSON_GetArrayItem(ssid, i))) {
            BCML_LOG_ERROR("validate_wireless_json: Invalid 'ssid' item at index %d\n", i);
            cJSON_Delete(root);
            return false;
        }
    }

    // AdditionalProperties: wireless object
    int wireless_fields = 2; // radio, ssid
    if (cJSON_GetArraySize(wireless) > wireless_fields) {
        BCML_LOG_WARN("validate_wireless_json: 'wireless' object has unknown fields\n");
        cJSON_Delete(root);
        return false;
    }

    // AdditionalProperties: root object
    int root_fields = 1; // wireless
    if (cJSON_GetArraySize(root) > root_fields) {
        BCML_LOG_WARN("validate_wireless_json: Root object has unknown fields\n");
        cJSON_Delete(root);
        return false;
    }

    cJSON_Delete(root);
    BCML_LOG_INFO("validate_wireless_json: validation successful\n");
    return true;
}
