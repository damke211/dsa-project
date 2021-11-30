#ifndef __WIFI_H
#define __WIFI_H 
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"



#include "debug.h"


/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


#define EXAMPLE_ESP_MAXIMUM_RETRY  4
typedef enum
{
	WIFI_STATE_DISCONNECTED = 0x00,
	WIFI_STATE_CONNECTING,
	WIFI_STATE_CONNECTED,
} WIFI_STATE;

typedef struct wifi_module_t
{
	uint8_t state;
    char ssid[32];
    char password[32];

} WIFI_INFO;


extern volatile WIFI_INFO g_WIFI_INFO;

void initWIFI(char * ssid,char * password);


#endif 

