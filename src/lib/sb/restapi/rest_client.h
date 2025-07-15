#ifndef REST_CLIENT_H
#define REST_CLIENT_H

#include <stddef.h>
#include <stdbool.h>

// For PATCH, POST, PUT, GET, DELETE...
typedef enum {
    REST_GET,
    REST_POST,
    REST_PUT,
    REST_PATCH,
    REST_DELETE
} rest_method_t;

// Return true if HTTP code == 200
bool rest_client_request(
    rest_method_t method,
    const char *url,
    const char *json_body,   // For PATCH/POST/PUT
    char *response_buf,      // For GET/response, can be NULL
    size_t response_buf_size // Size of response_buf
);


#endif // REST_CLIENT_H