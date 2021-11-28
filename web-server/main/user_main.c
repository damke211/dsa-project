#include "string.h"
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <esp_http_server.h>
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "driver/gpio.h"

#include "debug.h"

#define SOFT_AP_SSID "ESP32 SoftAP"
#define SOFT_AP_PASSWORD "Password"

#define SOFT_AP_IP_ADDRESS_1 192
#define SOFT_AP_IP_ADDRESS_2 168
#define SOFT_AP_IP_ADDRESS_3 0
#define SOFT_AP_IP_ADDRESS_4 1

#define SOFT_AP_GW_ADDRESS_1 192
#define SOFT_AP_GW_ADDRESS_2 168
#define SOFT_AP_GW_ADDRESS_3 0
#define SOFT_AP_GW_ADDRESS_4 2

#define SOFT_AP_NM_ADDRESS_1 255
#define SOFT_AP_NM_ADDRESS_2 255
#define SOFT_AP_NM_ADDRESS_3 255
#define SOFT_AP_NM_ADDRESS_4 0

#define SERVER_PORT 80 

#define GPIO_LED										16

static httpd_handle_t server = NULL;

/* An HTTP GET handler */
esp_err_t home_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;


    char resp[512];
    snprintf(resp, sizeof(resp),req->user_ctx,
            req->uri,
            xTaskGetTickCount() * portTICK_PERIOD_MS / 1000);
    httpd_resp_send(req, resp, strlen(resp));

    return ESP_OK;
}

httpd_uri_t home = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = home_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = {
        "HTTP/1.1 200 OK\r\n"
            "Content-type: text/html\r\n\r\n"
            "<html><head><title>HTTP Server</title>"
            "<style> div.main {"
            "font-family: Arial;"
            "padding: 0.01em 16px;"
            "box-shadow: 2px 2px 1px 1px #d2d2d2;"
            "background-color: #f1f1f1;}"
            "</style></head>"
            "<body><div class='main'>"
            "<h3>HTTP Server</h3>"
            "<p>URL: %s</p>"
            "<p>Uptime: %d seconds</p>"
            "<button onclick=\"location.href='/on'\" type='button'>"
            "LED On</button></p>"
            "<button onclick=\"location.href='/off'\" type='button'>"
            "LED Off</button></p>"
            "</div></body></html>"
    },
};

/* An HTTP POST handler */
esp_err_t on_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;


    gpio_set_level(GPIO_LED, 0);
    char resp[512];
    snprintf(resp, sizeof(resp),req->user_ctx,
            req->uri,
            xTaskGetTickCount() * portTICK_PERIOD_MS / 1000);
    httpd_resp_send(req, resp, strlen(resp));

    return ESP_OK;
}

httpd_uri_t on = 
{
    .uri       = "/on",
    .method    = HTTP_GET,
    .handler   = on_get_handler,
    .user_ctx  = {
        "HTTP/1.1 200 OK\r\n"
            "Content-type: text/html\r\n\r\n"
            "<html><head><title>HTTP Server</title>"
            "<style> div.main {"
            "font-family: Arial;"
            "padding: 0.01em 16px;"
            "box-shadow: 2px 2px 1px 1px #d2d2d2;"
            "background-color: #f1f1f1;}"
            "</style></head>"
            "<body><div class='main'>"
            "<h3>HTTP Server</h3>"
            "<p>URL: %s</p>"
            "<p>Uptime: %d seconds</p>"
            "<button onclick=\"location.href='/on'\" type='button'>"
            "LED On</button></p>"
            "<button onclick=\"location.href='/off'\" type='button'>"
            "LED Off</button></p>"
            "</div></body></html>"
    }
};

esp_err_t off_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;


    gpio_set_level(GPIO_LED, 1);
    char resp[512];
    snprintf(resp, sizeof(resp),req->user_ctx,
            req->uri,
            xTaskGetTickCount() * portTICK_PERIOD_MS / 1000);
    httpd_resp_send(req, resp, strlen(resp));

    return ESP_OK;
}

httpd_uri_t off = {
    .uri       = "/off",
    .method    = HTTP_GET,
    .handler   = off_get_handler,
    .user_ctx  = {
        "HTTP/1.1 200 OK\r\n"
            "Content-type: text/html\r\n\r\n"
            "<html><head><title>HTTP Server</title>"
            "<style> div.main {"
            "font-family: Arial;"
            "padding: 0.01em 16px;"
            "box-shadow: 2px 2px 1px 1px #d2d2d2;"
            "background-color: #f1f1f1;}"
            "</style></head>"
            "<body><div class='main'>"
            "<h3>HTTP Server</h3>"
            "<p>URL: %s</p>"
            "<p>Uptime: %d seconds</p>"
            "<button onclick=\"location.href='/on'\" type='button'>"
            "LED On</button></p>"
            "<button onclick=\"location.href='/off'\" type='button'>"
            "LED Off</button></p>"
            "</div></body></html>"
    }
};


httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    printDEBUG(DSYS, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Set URI handlers
        printDEBUG(DSYS, "Registering URI handlers");
        httpd_register_uri_handler(server, &home);
        httpd_register_uri_handler(server, &on);
        httpd_register_uri_handler(server, &off);
        return server;
    }

    printDEBUG(DSYS, "Error starting server!");
    return NULL;
}

void stop_webserver()
{
    // Stop the httpd server
    httpd_stop(server);
    //     
}


static esp_err_t wifiEventHandler(void* userParameter, system_event_t *event)
{
    switch(event->event_id)
    {
        case (SYSTEM_EVENT_AP_STACONNECTED):
        {
            printDEBUG(DSYS," Start http server\n");
            start_webserver();
            break;
        }
        case (SYSTEM_EVENT_AP_STADISCONNECTED):
        {
            printDEBUG(DSYS," Stop http server\n");
            stop_webserver();
            break;
        }
        default:
        {

            break;
        }
    }
    return ESP_OK;
}


static void launchSoftAp()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    tcpip_adapter_init();
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
    tcpip_adapter_ip_info_t ipAddressInfo;
    memset(&ipAddressInfo, 0, sizeof(ipAddressInfo));
    IP4_ADDR(
            &ipAddressInfo.ip,
            SOFT_AP_IP_ADDRESS_1,
            SOFT_AP_IP_ADDRESS_2,
            SOFT_AP_IP_ADDRESS_3,
            SOFT_AP_IP_ADDRESS_4);
    IP4_ADDR(
            &ipAddressInfo.gw,
            SOFT_AP_GW_ADDRESS_1,
            SOFT_AP_GW_ADDRESS_2,
            SOFT_AP_GW_ADDRESS_3,
            SOFT_AP_GW_ADDRESS_4);
    IP4_ADDR(
            &ipAddressInfo.netmask,
            SOFT_AP_NM_ADDRESS_1,
            SOFT_AP_NM_ADDRESS_2,
            SOFT_AP_NM_ADDRESS_3,
            SOFT_AP_NM_ADDRESS_4);
    ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &ipAddressInfo));
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));
    ESP_ERROR_CHECK(esp_event_loop_init(wifiEventHandler, NULL));
    wifi_init_config_t wifiConfiguration = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifiConfiguration));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    wifi_config_t apConfiguration =
    {
        .ap = {
            .ssid = SOFT_AP_SSID,
            .password = SOFT_AP_PASSWORD,
            .ssid_len = 0,
            //.channel = default,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .ssid_hidden = 0,
            .max_connection = 1,
            .beacon_interval = 150,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &apConfiguration));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main(void)
{
    initDEBUG("", '5', 921600, "DSA - Debug example");

    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask =  (1 << GPIO_LED);
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);


    launchSoftAp();
    while(1) 
    {
        vTaskDelay(10);
    }
}



