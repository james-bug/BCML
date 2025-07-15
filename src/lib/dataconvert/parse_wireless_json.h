#ifndef PARSE_WIRELESS_JSON_H
#define PARSE_WIRELESS_JSON_H

#include "bcml_types.h"
#include <stdbool.h>

bool parse_wireless_json(const char* json, void* sdata);

#endif // PARSE_WIRELESS_JSON_H