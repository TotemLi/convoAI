#include "esp_http_server.h"
#include "esp_err.h"
#include "esp_log.h"
#include "cjson/cJSON.h"

static char *TAG = "webserver";

static esp_err_t handle_404(httpd_req_t *req, httpd_err_code_t error)
{
    httpd_resp_set_status(req, "404 Not Found");
    httpd_resp_set_type(req, HTTPD_TYPE_JSON);
    cJSON *data_json = cJSON_CreateObject();
    cJSON_AddNumberToObject(data_json, "ret", 404);
    cJSON_AddStringToObject(data_json, "msg", "not found");
    char *data = cJSON_Print(data_json);
    httpd_resp_send(req, data, strlen(data));
    return ESP_OK;
}

httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    if (httpd_start(&server, &config) == ESP_OK)
    {
        ESP_LOGI(TAG, "server start on port: %d", config.server_port);
        // 处理404
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, handle_404);
        return server;
    }
    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

esp_err_t webserver_register_handler(httpd_handle_t server, const char *uri, httpd_method_t method, esp_err_t (*handler)(httpd_req_t *r))
{
    const httpd_uri_t tmp = {
        .uri = uri,
        .method = method,
        .handler = handler,
    };
    esp_err_t err = httpd_register_uri_handler(server, &tmp);
    return err;
}
