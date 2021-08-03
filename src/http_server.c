#include "config.h"
#include "max30102.h"
#include "ssl_root_cert.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"
#include <time.h>

#include "esp_http_client.h"

#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048
static const char *TAG = "HTTP_CLIENT";

const char *howsmyssl_com_root_cert_pem = SSL_ROOT_CERT;
extern int emergency;

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;  // Buffer to store response of http request from event handler
    static int output_len;       // Stores number of bytes read
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                if (evt->user_data) {
                    memcpy(evt->user_data + output_len, evt->data, evt->data_len);
                } else {
                    if (output_buffer == NULL) {
                        output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    memcpy(output_buffer + output_len, evt->data, evt->data_len);
                }
                output_len += evt->data_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                if (output_buffer != NULL) {
                    free(output_buffer);
                    output_buffer = NULL;
                }
                output_len = 0;
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
    }
    return ESP_OK;
}

void http_main_loop(void *pvParameters)
{
    struct max30102_data *pulse_data = pvParameters;
    char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    esp_http_client_config_t config = {
        .host = DB_URL,
        .path = API_WRITE_PATH,
        .query = QUERY,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .event_handler = _http_event_handler,
        .user_data = local_response_buffer,
        .cert_pem = howsmyssl_com_root_cert_pem,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // POST
    char post_data[200];
    while (1) {
        sprintf(post_data, "data,host=host1 BPM=%d\n"
                           "data,host=host1 SP02=%f\n"
                           "data,host=host1 TEMP=%f\n"
                           "data,host=host1 EMERGENCY=%d", 
                            pulse_data->BPM, pulse_data->Sp02, pulse_data->temp, emergency);

        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_header(client, "Authorization", "Token HSsa1xoxHFjMHTXmr_l0zh5Coa-KZHwmnl6UcvJO3gO4_w93QovneuRQYe2HHV1o238yEXQW5jQ71j8Z_rv8bA==");
        esp_http_client_set_post_field(client, post_data, strlen(post_data));
        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) 
        {
            //emergency = 0;           
        } 
        else 
        {
            ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
        }       
    }  
    esp_http_client_cleanup(client);
}