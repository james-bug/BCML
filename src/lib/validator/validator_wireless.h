#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <stdbool.h>

// Validate wireless JSON data, return true if valid, false otherwise
bool validate_wireless_json(const char* json, const char* schema_path);

#endif // VALIDATOR_H