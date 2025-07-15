#include "sb_ops.h"
#include "bcml_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "rest_client.h"
#include "bcml_log.h"

#define REST_API_BASE_URL "http://127.0.0.1:5566/v1/wlan/setting"
#define WIRELESS_JSON_BUF_SIZE 4096

// REST API PATCH for wireless config (multiple SSID)
static bool rest_set_wireless_config(const bcml_wireless_cfg_t* cfg) {
    BCML_LOG_DEBUG("rest_set_wireless_config: called with cfg=%p\n", cfg);
    if (!cfg) {
        BCML_LOG_WARN("rest_set_wireless_config: cfg is NULL\n");
        return false;
    }

    cJSON *body = cJSON_CreateObject();

    // Build SSID array
    cJSON *ssid_array = cJSON_CreateArray();
    for (int i = 0; i < MAX_SSID_NUM; ++i) {
        const bcml_wireless_ssid_t *ssid_cfg = &cfg->ssid[i];
        // Skip empty SSID entries
        if (ssid_cfg->ssid[0] == '\0') continue;

        cJSON *ssid_obj = cJSON_CreateObject();
        cJSON_AddStringToObject(ssid_obj, "ssid", ssid_cfg->ssid);
        cJSON_AddBoolToObject(ssid_obj, "hide", ssid_cfg->hide);
        cJSON_AddNumberToObject(ssid_obj, "security", ssid_cfg->security);
        cJSON_AddStringToObject(ssid_obj, "password", ssid_cfg->password);
        cJSON_AddBoolToObject(ssid_obj, "password-onscreen", ssid_cfg->password_onscreen);
        cJSON_AddBoolToObject(ssid_obj, "enable2g", ssid_cfg->enable2g);
        cJSON_AddBoolToObject(ssid_obj, "enable5g", ssid_cfg->enable5g);
        cJSON_AddBoolToObject(ssid_obj, "isolation", ssid_cfg->isolation);
        cJSON_AddBoolToObject(ssid_obj, "hopping", ssid_cfg->hopping);
        cJSON_AddItemToArray(ssid_array, ssid_obj);
        BCML_LOG_DEBUG("rest_set_wireless_config: ssid[%d] added: ssid='%s', hide=%d, security=%d, password='%s', password-onscreen=%d, enable2g=%d, enable5g=%d, isolation=%d, hopping=%d\n",
            i, ssid_cfg->ssid, ssid_cfg->hide, ssid_cfg->security, ssid_cfg->password, ssid_cfg->password_onscreen,
            ssid_cfg->enable2g, ssid_cfg->enable5g, ssid_cfg->isolation, ssid_cfg->hopping);
    }
    cJSON_AddItemToObject(body, "ssid", ssid_array);

    // If you need to PATCH radio settings, add similar code here.

    char *json_str = cJSON_PrintUnformatted(body);
    BCML_LOG_DEBUG("rest_set_wireless_config: sending json: %s\n", json_str);

    bool ret = rest_client_request(
        REST_PATCH,
        REST_API_BASE_URL,
        json_str,
        NULL, 0
    );
    BCML_LOG_DEBUG("rest_set_wireless_config: rest_client_request returned %d\n", ret);

    cJSON_Delete(body);
    free(json_str);

    return ret;
}

// REST API GET for wireless config (multiple SSID)
static bool rest_get_wireless_config(bcml_wireless_cfg_t* cfg) {
    BCML_LOG_DEBUG("rest_get_wireless_config: called with cfg=%p\n", cfg);
    if (!cfg) {
        BCML_LOG_WARN("rest_get_wireless_config: cfg is NULL\n");
        return false;
    }

    char response_buf[WIRELESS_JSON_BUF_SIZE] = {0};
    bool ok = rest_client_request(
        REST_GET,
        REST_API_BASE_URL,
        NULL,
        response_buf,
        sizeof(response_buf)
    );
    BCML_LOG_DEBUG("rest_get_wireless_config: rest_client_request returned %d, response_buf='%s'\n", ok, response_buf);
    if (!ok) {
        BCML_LOG_ERROR("rest_get_wireless_config: rest_client_request failed\n");
        return false;
    }

    cJSON *json = cJSON_Parse(response_buf);
    if (!json) {
        BCML_LOG_ERROR("rest_get_wireless_config: Failed to parse JSON response\n");
        return false;
    }

    // Parse SSID array
    cJSON *ssid_array = cJSON_GetObjectItem(json, "ssid");
    if (ssid_array && cJSON_IsArray(ssid_array)) {
        int count = cJSON_GetArraySize(ssid_array);
        BCML_LOG_DEBUG("rest_get_wireless_config: Found ssid array with %d elements\n", count);
        for (int i = 0; i < MAX_SSID_NUM; ++i) {
            if (i >= count) {
                memset(&cfg->ssid[i], 0, sizeof(cfg->ssid[i]));
                continue;
            }
            cJSON *item = cJSON_GetArrayItem(ssid_array, i);
            bcml_wireless_ssid_t *ssid_cfg = &cfg->ssid[i];

            cJSON *ssid = cJSON_GetObjectItem(item, "ssid");
            cJSON *hide = cJSON_GetObjectItem(item, "hide");
            cJSON *security = cJSON_GetObjectItem(item, "security");
            cJSON *password = cJSON_GetObjectItem(item, "password");
            cJSON *password_onscreen = cJSON_GetObjectItem(item, "password-onscreen");
            cJSON *enable2g = cJSON_GetObjectItem(item, "enable2g");
            cJSON *enable5g = cJSON_GetObjectItem(item, "enable5g");
            cJSON *isolation = cJSON_GetObjectItem(item, "isolation");
            cJSON *hopping = cJSON_GetObjectItem(item, "hopping");

            if (ssid && cJSON_IsString(ssid))
                strncpy(ssid_cfg->ssid, ssid->valuestring, sizeof(ssid_cfg->ssid));
            else
                ssid_cfg->ssid[0] = '\0';

            ssid_cfg->hide = (hide && cJSON_IsBool(hide)) ? cJSON_IsTrue(hide) : false;
            ssid_cfg->security = (security && cJSON_IsNumber(security)) ? security->valueint : 0;
            if (password && cJSON_IsString(password))
                strncpy(ssid_cfg->password, password->valuestring, sizeof(ssid_cfg->password));
            else
                ssid_cfg->password[0] = '\0';

            ssid_cfg->password_onscreen = (password_onscreen && cJSON_IsBool(password_onscreen)) ? cJSON_IsTrue(password_onscreen) : false;
            ssid_cfg->enable2g = (enable2g && cJSON_IsBool(enable2g)) ? cJSON_IsTrue(enable2g) : false;
            ssid_cfg->enable5g = (enable5g && cJSON_IsBool(enable5g)) ? cJSON_IsTrue(enable5g) : false;
            ssid_cfg->isolation = (isolation && cJSON_IsBool(isolation)) ? cJSON_IsTrue(isolation) : false;
            ssid_cfg->hopping = (hopping && cJSON_IsBool(hopping)) ? cJSON_IsTrue(hopping) : false;

            BCML_LOG_DEBUG("rest_get_wireless_config: ssid[%d] parsed: ssid='%s', hide=%d, security=%d, password='%s', password-onscreen=%d, enable2g=%d, enable5g=%d, isolation=%d, hopping=%d\n",
                i, ssid_cfg->ssid, ssid_cfg->hide, ssid_cfg->security, ssid_cfg->password, ssid_cfg->password_onscreen,
                ssid_cfg->enable2g, ssid_cfg->enable5g, ssid_cfg->isolation, ssid_cfg->hopping);
        }
    } else {
        BCML_LOG_DEBUG("rest_get_wireless_config: 'ssid' array not found, clearing entries\n");
        // If no ssid array, clear all entries
        for (int i = 0; i < MAX_SSID_NUM; ++i)
            memset(&cfg->ssid[i], 0, sizeof(cfg->ssid[i]));
    }

    // If you want to parse radio settings, add similar code here.

    cJSON_Delete(json);
    BCML_LOG_DEBUG("rest_get_wireless_config: parsing complete, returning true\n");
    return true;
}

sb_ops_t sb = {
    .set_wireless_config = rest_set_wireless_config,
    .get_wireless_config = rest_get_wireless_config,
};