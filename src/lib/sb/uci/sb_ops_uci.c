#include "sb_ops.h"
#include <stdio.h>

static bool uci_set_wireless_config(const bcml_wireless_cfg_t* cfg) {
    printf("[UCI] set_wireless_config, ssid0=%s\n", cfg->ssid[0].ssid);
    // real backend logic here
    return true;
}

static bool uci_get_wireless_config(bcml_wireless_cfg_t* cfg) {
    snprintf(cfg->ssid[0].ssid, sizeof(cfg->ssid[0].ssid), "uci_ssid");
    return true;
}

// Global sb_ops_t instance, linker will resolve "sb"
sb_ops_t sb = {
    .set_wireless_config = uci_set_wireless_config,
    .get_wireless_config = uci_get_wireless_config,
};