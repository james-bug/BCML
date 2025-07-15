#ifndef _BCML_TYPES_H_
#define _BCML_TYPES_H_

#include <stdbool.h>

#define MAX_RADIO_NUM 4  // Maximum number of radios
#define MAX_SSID_NUM 4   // Maximum number of SSID entries per wireless config

// Wireless radio settings
typedef struct {
    int power;
    int channel2g;
    int channel5g;
    int bandwidth2g;
    int bandwidth5g;
    bool dfs;
    bool atf;
    bool bandsteering;
    bool zerowait;
} bcml_wireless_radio_t;

// Wireless SSID settings
typedef struct {
    char ssid[64];
    bool hide;
    int security;
    char password[64];
    bool password_onscreen;
    bool enable2g;
    bool enable5g;
    bool isolation;
    bool hopping;
} bcml_wireless_ssid_t;

// Top-level wireless configuration
typedef struct {
    bcml_wireless_radio_t radio[MAX_RADIO_NUM];
    bcml_wireless_ssid_t ssid[MAX_SSID_NUM];
} bcml_wireless_cfg_t;

#endif // _BCML_TYPES_H_