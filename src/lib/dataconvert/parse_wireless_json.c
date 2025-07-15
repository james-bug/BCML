#include "parse_wireless_json.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "bcml_types.h"
#include <cjson/cJSON.h>
#include "bcml_log.h"

#ifndef MAX_RADIO_NUM
#define MAX_RADIO_NUM 4
#endif

#ifndef MAX_SSID_NUM
#define MAX_SSID_NUM 8
#endif

static int get_json_int(const cJSON *obj, const char *key, int defval) {
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(obj, key);
    return (cJSON_IsNumber(item)) ? item->valueint : defval;
}

static bool get_json_bool(const cJSON *obj, const char *key, bool defval) {
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(obj, key);
    return (cJSON_IsBool(item)) ? cJSON_IsTrue(item) : defval;
}

static void get_json_string(const cJSON *obj, const char *key, char *dst, size_t dst_sz) {
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(obj, key);
    if (cJSON_IsString(item) && (item->valuestring != NULL)) {
        strncpy(dst, item->valuestring, dst_sz - 1);
        dst[dst_sz - 1] = '\0';
    } else {
        if (dst_sz > 0) dst[0] = '\0';
    }
}

bool parse_wireless_json(const char* json, void* sdata) {
    BCML_LOG_DEBUG("parse_wireless_json: called with json=%p, sdata=%p\n", json, sdata);

    bcml_wireless_cfg_t* cfg = (bcml_wireless_cfg_t*)sdata;

    if (!json || !cfg) {
        BCML_LOG_WARN("parse_wireless_json: Invalid input. json=%p, sdata=%p\n", json, sdata);
        return false;
    }
    memset(cfg, 0, sizeof(bcml_wireless_cfg_t));

    cJSON *root = cJSON_Parse(json);
    if (!root) {
        BCML_LOG_ERROR("parse_wireless_json: Failed to parse JSON.\n");
        return false;
    }

    // Get "wireless" object
    cJSON *wireless = cJSON_GetObjectItemCaseSensitive(root, "wireless");
    if (!cJSON_IsObject(wireless)) {
        BCML_LOG_ERROR("parse_wireless_json: 'wireless' object not found or not an object\n");
        cJSON_Delete(root);
        return false;
    }

    // Parse radio array
    cJSON *radios = cJSON_GetObjectItemCaseSensitive(wireless, "radio");
    if (cJSON_IsArray(radios)) {
        int radio_count = cJSON_GetArraySize(radios);
        BCML_LOG_DEBUG("parse_wireless_json: Found radio array with %d elements.\n", radio_count);
        for (int i = 0; i < radio_count && i < MAX_RADIO_NUM; ++i) {
            cJSON *radio_obj = cJSON_GetArrayItem(radios, i);
            if (!cJSON_IsObject(radio_obj)) continue;
            bcml_wireless_radio_t *radio = &cfg->radio[i];
            radio->power         = get_json_int(radio_obj, "power", 0);
            radio->channel2g     = get_json_int(radio_obj, "channel2g", 0);
            radio->channel5g     = get_json_int(radio_obj, "channel5g", 0);
            radio->bandwidth2g   = get_json_int(radio_obj, "bandwidth2g", 0);
            radio->bandwidth5g   = get_json_int(radio_obj, "bandwidth5g", 0);
            radio->dfs           = get_json_bool(radio_obj, "dfs", false);
            radio->atf           = get_json_bool(radio_obj, "atf", false);
            radio->bandsteering  = get_json_bool(radio_obj, "bandsteering", false);
            radio->zerowait      = get_json_bool(radio_obj, "zerowait", false);
            BCML_LOG_DEBUG("parse_wireless_json: radio[%d] parsed: power=%d, channel2g=%d, channel5g=%d, bandwidth2g=%d, bandwidth5g=%d, dfs=%d, atf=%d, bandsteering=%d, zerowait=%d\n",
                i, radio->power, radio->channel2g, radio->channel5g, radio->bandwidth2g, radio->bandwidth5g,
                radio->dfs, radio->atf, radio->bandsteering, radio->zerowait);
        }
    } else {
        BCML_LOG_ERROR("parse_wireless_json: 'radio' array not found in 'wireless'\n");
    }

    // Parse ssid array
    cJSON *ssids = cJSON_GetObjectItemCaseSensitive(wireless, "ssid");
    if (cJSON_IsArray(ssids)) {
        int ssid_count = cJSON_GetArraySize(ssids);
        BCML_LOG_DEBUG("parse_wireless_json: Found ssid array with %d elements.\n", ssid_count);
        for (int i = 0; i < ssid_count && i < MAX_SSID_NUM; ++i) {
            cJSON *ssid_obj = cJSON_GetArrayItem(ssids, i);
            if (!cJSON_IsObject(ssid_obj)) continue;
            bcml_wireless_ssid_t *ssid = &cfg->ssid[i];
            get_json_string(ssid_obj, "ssid", ssid->ssid, sizeof(ssid->ssid));
            ssid->hide              = get_json_bool(ssid_obj, "hide", false);
            ssid->security          = get_json_int(ssid_obj, "security", 0);
            get_json_string(ssid_obj, "password", ssid->password, sizeof(ssid->password));
            ssid->password_onscreen = get_json_bool(ssid_obj, "password_onscreen", false);
            ssid->enable2g          = get_json_bool(ssid_obj, "enable2g", false);
            ssid->enable5g          = get_json_bool(ssid_obj, "enable5g", false);
            ssid->isolation         = get_json_bool(ssid_obj, "isolation", false);
            ssid->hopping           = get_json_bool(ssid_obj, "hopping", false);
            BCML_LOG_DEBUG("parse_wireless_json: ssid[%d] parsed: ssid='%s', hide=%d, security=%d, password='%s', password_onscreen=%d, enable2g=%d, enable5g=%d, isolation=%d, hopping=%d\n",
                i, ssid->ssid, ssid->hide, ssid->security, ssid->password, ssid->password_onscreen,
                ssid->enable2g, ssid->enable5g, ssid->isolation, ssid->hopping);
        }
    } else {
        BCML_LOG_ERROR("parse_wireless_json: 'ssid' array not found in 'wireless'\n");
    }

    cJSON_Delete(root);
    BCML_LOG_DEBUG("parse_wireless_json: JSON parsed successfully.\n");
    return true;
}
