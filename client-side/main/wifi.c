#include "wifi.h"


volatile WIFI_INFO g_WIFI_INFO;
/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
        esp_wifi_connect();
    }
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
		{
            esp_wifi_connect();
            s_retry_num++;
            printDEBUG(DWIFI | DWARNING, "retry to connect to the AP\n");
        }
		else
		{
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        printDEBUG(DWIFI,"connect to the AP fail\n");
    }
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        printDEBUG(DWIFI, "got ip:%s\n",ip4addr_ntoa(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}


void initWIFI(char * ssid,char * password)
{
    s_wifi_event_group = xEventGroupCreate();
	g_WIFI_INFO.state = WIFI_STATE_DISCONNECTED;

    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config =
	{
        .sta =
		{
            .ssid = {""},
            .password = {""}
        },
    };

    strcpy((char *) wifi_config.sta.ssid,ssid);
    strcpy((char *) wifi_config.sta.password,password);
    strcpy((char *) g_WIFI_INFO.ssid,ssid);
    strcpy((char *) g_WIFI_INFO.password,password);
    /* Setting a password implies station will connect to all security modes including WEP/WPA.
    * However these modes are deprecated and not advisable to be used. Incase your Access point
    * doesn't support WPA2, these mode can be enabled by commenting below line */
    if (strlen((char *)wifi_config.sta.password))
	{
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

	g_WIFI_INFO.state = WIFI_STATE_CONNECTING;
    printDEBUG(DWIFI, "init completed\n");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,	
            pdFALSE,	// don't clear bits on exit
            pdFALSE,	// wait for any bit
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
	{
        printDEBUG(DWIFI, "connected to ap SSID:%s password:%s\n",g_WIFI_INFO.ssid, g_WIFI_INFO.password);
		g_WIFI_INFO.state = WIFI_STATE_CONNECTED;
	}
	else if (bits & WIFI_FAIL_BIT)
	{
        printDEBUG(DWIFI, "Failed to connect to SSID:%s, password:%s\n",g_WIFI_INFO.ssid, g_WIFI_INFO.password);
		g_WIFI_INFO.state = WIFI_STATE_DISCONNECTED;
    }
	else
	{
		g_WIFI_INFO.state = WIFI_STATE_DISCONNECTED;
        printDEBUG(DWIFI, "UNEXPECTED EVENT");
    }
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    vEventGroupDelete(s_wifi_event_group);
}



