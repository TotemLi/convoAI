#pragma once

#include "esp_http_server.h"

httpd_handle_t start_webserver(void);
esp_err_t webserver_register_handler(httpd_handle_t server, const char *uri, httpd_method_t method, esp_err_t (*handler)(httpd_req_t *r));