#ifndef _BCML_CONFIG_H_
#define _BCML_CONFIG_H_

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Set the configuration data of a specific type (in JSON format).
 * @param type        Configuration type string (e.g., "wireless", "display", ...)
 * @param json_data   Configuration content (JSON string)
 * @return true on success, false on failure
 */
bool bcml_config_set(const char* type, const char* json_data);

/**
 * @brief Get the configuration data of a specific type (exported in JSON format).
 * @param type         Configuration type string
 * @param json_buffer  Buffer for output JSON
 * @param buffer_size  Buffer size in bytes
 * @return true on success, false on failure
 */
bool bcml_config_get(const char* type, char* json_buffer, size_t buffer_size);

#endif // _BCML_CONFIG_H_

