#include "rest_client.h"
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "bcml_log.h"

struct memory {
    char *response;
    size_t size;
};

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct memory *mem = (struct memory *)userp;
    mem->response = realloc(mem->response, mem->size + realsize + 1);
    if (mem->response == NULL) return 0; // out of memory
    memcpy(&(mem->response[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;
    return realsize;
}

bool rest_client_request(
    rest_method_t method,
    const char *url,
    const char *json_body,
    char *response_buf,
    size_t response_buf_size
) {
    BCML_LOG_DEBUG("rest_client_request: called. method=%d, url=%s, json_body=%p, response_buf=%p, response_buf_size=%zu\n", 
        method, url ? url : "(null)", json_body, response_buf, response_buf_size);

    CURL *curl = curl_easy_init();
    if (!curl) {
        BCML_LOG_ERROR("rest_client_request: curl_easy_init failed!\n");
        return false;
    }

    struct curl_slist *headers = NULL;
    CURLcode res;
    bool ret = false;
    long http_code = 0;

    // cURL Method
    const char *method_str = NULL;
    switch (method) {
        case REST_GET:    method_str = "GET"; break;
        case REST_POST:   method_str = "POST"; break;
        case REST_PUT:    method_str = "PUT"; break;
        case REST_PATCH:  method_str = "PATCH"; break;
        case REST_DELETE: method_str = "DELETE"; break;
        default:          method_str = "GET"; break;
    }
    BCML_LOG_DEBUG("rest_client_request: HTTP method set to %s\n", method_str);

    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method_str);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    if (json_body && (method == REST_POST || method == REST_PUT || method == REST_PATCH)) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_body);
        BCML_LOG_DEBUG("rest_client_request: Sending JSON body: %s\n", json_body);
    }

    struct memory chunk = {0};
    if (response_buf && response_buf_size > 0) {
        // set write callback for response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        BCML_LOG_DEBUG("rest_client_request: Write callback set for response.\n");
    }

    res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        BCML_LOG_DEBUG("rest_client_request: curl_easy_perform OK, HTTP code: %ld\n", http_code);
        ret = (http_code == 200);
        // Copy response to buffer if needed
        if (response_buf && chunk.response) {
            strncpy(response_buf, chunk.response, response_buf_size-1);
            response_buf[response_buf_size-1] = '\0';
            BCML_LOG_DEBUG("rest_client_request: Response copied to buffer. response=[%s]\n", response_buf);
        }
    } else {
        BCML_LOG_ERROR("rest_client_request: curl_easy_perform failed! CURLcode=%d, %s\n", res, curl_easy_strerror(res));
    }
    free(chunk.response);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    BCML_LOG_DEBUG("rest_client_request: done. ret=%d\n", ret);
    return ret;
}
