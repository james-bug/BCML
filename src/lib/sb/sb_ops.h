#ifndef SB_OPS_H
#define SB_OPS_H

#include "bcml_types.h"
#include <stdbool.h>

// sb_ops_t: Function pointers for all config types
typedef struct {
    bool (*set_wireless_config)(const bcml_wireless_cfg_t* cfg);
    bool (*get_wireless_config)(bcml_wireless_cfg_t* cfg);
    // Extend here for more config types
    // bool (*set_network_config)(const bcml_network_cfg_t* cfg);
    // bool (*get_network_config)(bcml_network_cfg_t* cfg);
} sb_ops_t;

// sb_ops_entry_t: Used for dispatching set/get by config type
typedef struct {
    const char* type;
    bool (*set)(const void* cfg);
    bool (*get)(void* cfg);
} sb_ops_entry_t;

// Global sb, implemented by backend (extern, decided by linker)
extern sb_ops_t sb;

// Lookup function, returns the set/get entry for the specified config type
const sb_ops_entry_t* sb_ops_find(const char* type);

#endif // SB_OPS_H