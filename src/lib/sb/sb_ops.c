#include "sb_ops.h" // Include southbound interface
#include "bcml_log.h" // Include logging interface
#include <string.h>
#include <stdio.h>

extern sb_ops_t sb;

// Adapter for wireless set
static bool sb_wireless_set(const void* cfg) {
    const bcml_wireless_cfg_t* wcfg = (const bcml_wireless_cfg_t*)cfg;
    if (!sb.set_wireless_config) {
        printf("[SB] set_wireless_config is NULL!\n");
        return false;
    }
    return sb.set_wireless_config(wcfg);
}

// Adapter for wireless get
static bool sb_wireless_get(void* cfg) {
    bcml_wireless_cfg_t* wcfg = (bcml_wireless_cfg_t*)cfg;
    if (!sb.get_wireless_config) {
        printf("[SB] get_wireless_config is NULL!\n");
        return false;
    }
    return sb.get_wireless_config(wcfg);
}

const sb_ops_entry_t sb_ops_table[] = {
    { "wireless", sb_wireless_set, sb_wireless_get },
    // { "network", sb_network_set, sb_network_get },
    // { "display", sb_display_set, sb_display_get },
};

const size_t sb_ops_table_size = sizeof(sb_ops_table) / sizeof(sb_ops_table[0]);

const sb_ops_entry_t* sb_ops_find(const char* type) {

    BCML_LOG_DEBUG("sb_ops_find: searching for type='%s' in %zu entries\n", type ? type : "(null)", sb_ops_table_size);
   
    for (size_t i = 0; i < sb_ops_table_size; ++i) {
        if (strcasecmp(type, sb_ops_table[i].type) == 0)
            return &sb_ops_table[i];
    }

    BCML_LOG_WARN("sb_ops_find: No entry found for type '%s' \n", type ? type : "(null)");
    return NULL;
}