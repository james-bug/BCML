#ifndef EXPORT_WIRELESS_JSON_H
#define EXPORT_WIRELESS_JSON_H

#include <stddef.h>
#include <stdbool.h>

// Export the wireless config structure to a JSON string.
// Returns true on success, false on failure.
// sdata: pointer to bcml_wireless_cfg_t
// json_buffer: output buffer to store the JSON string
// buffer_size: size of the output buffer
bool export_wireless_json(const void* sdata, char* json_buffer, size_t buffer_size);

#endif // EXPORT_WIRELESS_JSON_H