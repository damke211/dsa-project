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
#include "wifi.h"
#include "esp_http_client.h"
#include "protocol_examples_common.h"
#include "esp_tls.h"
#include "sensors.h"
#include "hw_timer.h"


#define WIFI_SSID "ESP32 SoftAP"
#define WIFI_PASS "Password"
#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048

void parseAndPerformActions(char* content){
    printDEBUG(DSYS,"napokon u funkciji");
    printDEBUG(DSYS,"%s CONTENT ZAVRSEN \n",content );
    int outputId =0, outputValue =0;
     char *str1, *str2, *str3, *token, *subtoken, *endtoken;
            char *saveptr1, *saveptr2, *saveptr3;
               token = strtok_r(content,"$", &saveptr1);
               if (token == NULL)
                   return 0;
               for (str2 = token; ; str2 = NULL) {
                   subtoken = strtok_r(str2, "#", &saveptr2);
                   if (subtoken == NULL)
                       break;
                    int i =0;
                    for(str3 = subtoken; ; str3 = NULL)
                    {  
                        endtoken = strtok_r(str3,"-",&saveptr3);
                        if(endtoken == NULL){
                            break;
                        }
                        if(i == 0){
                                outputId = atoi(endtoken);
                            }else{
                                outputValue = atoi(endtoken);
                         switch (outputId){
                	case 1:{
                	       //servo
                        // printDEBUG(DSYS,"servo prilagodjava %d \n", outputValue);
                		devices[0].value = outputValue;
                        // servo(outputValue);
                        break;
                	       }
                	case 2:{
                        // printDEBUG(DSYS,"sijalica 1 prilagodjava %d \n", outputValue);
                		devices[1].value = outputValue;
                		// LED_on(LED1);    
                        break;
                          }
                	case 3:{
                        // printDEBUG(DSYS,"sijalica 2 prilagodjava %d \n",outputValue);
                        devices[2].value = outputValue;
                		// LED_on(LED2);
                        break;  
                		  }
                	default:{
                        break;
                    }
                        }

                            }
                            
                            i++;
                    }
               }
}


esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;  // Buffer to store response of http request from event handler
    static int output_len;       // Stores number of bytes read
    switch(evt->event_id)
    {
        case HTTP_EVENT_ERROR:
        {
            printDEBUG(DHTTP, "HTTP_EVENT_ERROR\n");
            break;
        }
        case HTTP_EVENT_ON_CONNECTED:
        {
            printDEBUG(DHTTP, "HTTP_EVENT_ON_CONNECTED\n");
            break;
        }
        case HTTP_EVENT_HEADER_SENT:
        {
            printDEBUG(DHTTP, "HTTP_EVENT_HEADER_SENT\n");
            break;
        }
        case HTTP_EVENT_ON_HEADER:
        {
            printDEBUG(DHTTP, "HTTP_EVENT_ON_HEADER, key=%s, value=%s\n", evt->header_key, evt->header_value);
            break;
        }
        case HTTP_EVENT_ON_DATA:
        {
            printDEBUG(DHTTP, "HTTP_EVENT_ON_DATA, len=%d\n", evt->data_len);
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                if (evt->user_data)
                {
                    // printDEBUG(DHTTP, "printanje data direktno\n");
                    // printDEBUG(DHTTP,"%s",evt->data); 
                    printDEBUG(DHTTP, "prije ulaska u funkciju\n");
                    parseAndPerformActions(evt->data);
                    // memcpy(evt->user_data + output_len, evt->data, evt->data_len);
                } 
                else
                {
                    if (output_buffer == NULL)
                    {
                        output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
                        output_len = 0;
                        if (output_buffer == NULL)
                        {
                            printDEBUG(DHTTP | DERROR, "Failed to allocate memory for output buffer\n");
                            return ESP_FAIL;
                        }
                    }
                    memcpy(output_buffer + output_len, evt->data, evt->data_len);
                }
                output_len += evt->data_len;
            }
            break;
        }
        case HTTP_EVENT_ON_FINISH:
        {
            printDEBUG(DHTTP, "HTTP_EVENT_ON_FINISH\n");
            if (output_buffer != NULL) 
            {
                // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
                // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
                free(output_buffer);
                output_buffer = NULL;
                output_len = 0;
            }
            break;
        }
        case HTTP_EVENT_DISCONNECTED:
        {

            printDEBUG(DHTTP, "HTTP_EVENT_DISCONNECTED\n");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
            if (err != 0)
            {
                if (output_buffer != NULL)
                {
                    free(output_buffer);
                    output_buffer = NULL;
                    output_len = 0;
                }
                printDEBUG(DHTTP, "Last esp error code: 0x%x\n", err);
                printDEBUG(DHTTP, "Last mbedtls failure: 0x%x\n", mbedtls_err);
            }
            break;
        }
    }
    return ESP_OK;
}

void http_rest_with_url(char sendData[])
{
    char local_response_buffer[200] ={0};
    esp_http_client_config_t config = {
        .url = "http://192.168.0.1/devices",
        .event_handler = _http_event_handler,
        .user_data = local_response_buffer
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // const char *sendData = {"bilo sta"};
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, sendData, strlen(sendData));
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        printDEBUG(DHTTP, "HTTP GET Status = %d, content_length = %d\n",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        printDEBUG(DHTTP | DERROR, "HTTP GET request failed: %s\n", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}

void app_main()
{
    esp_err_t ret = nvs_flash_init();

    initDEBUG("", '5', 921600, "DSA - Debug example");
    initWIFI(WIFI_SSID,WIFI_PASS);

    // adc();
    // LDR_init();
    // hcsr_init();
    // motion_init();
    printDEBUG(DSYS,"problem ne nastavi ovdje");

    ser.id=1;
    strcpy(ser.device_details,"output-door-Garage_Door");
    // ser.value=servo();
    ser.value = 0;

    LED.id = 2;
    strcpy(LED.device_details,"output-light-Garden_Light");
    LED.value = 0;


    LLED.id = 3;
    strcpy(LLED.device_details,"output-light-Kitchen_Light");
    LLED.value = 0;
    // int motion=motion_detection();
    // printDEBUG(DSYS,"motion %d\n",motion);

    mmotion.id=4;
    strcpy(mmotion.device_details,"sensor-motion-Entry_door");
    // mmotion.value=motion;
    mmotion.value = 0;
    // int ldr=LDR_output();
    // printDEBUG(DSYS,"ldr %d\n",ldr);

    LLDR.id=5;
    strcpy(LLDR.device_details,"sensor-ldr-Roof");
    LLDR.value=0;

    // int distance=get_ultrasonic();
    ultrason.id=6;
    strcpy(ultrason.device_details,"sensor-ultrasonic-Garage_Door");
    ultrason.value=0;


    int numberOfDevices=6;
    devices[0]=ser;
    devices[1]=LED;
    devices[2]=LLED;
    devices[3]=mmotion;
    devices[4]=LLDR;
    devices[5]=ultrason;
    const char template[100] = {"%d-%d-%s#"};

    while(1){
        // devices[4].value = LDR_output();
        // devices[5].value = get_ultrasonic();

        if(devices[4].value == 0)
            {
                    devices[4].value = 1;
                    devices[5].value = 1;
                
            }
        else
            {
                devices[4].value = 0;
                devices[5].value = 0;
            }
            
        char sendBuffer [200]={""};
        for( int i = 0; i < numberOfDevices; i++)
        {
            if(i == 0)
            {
                strcat(sendBuffer,"0#");
            }
            char temp[50];
            snprintf(temp,sizeof temp,template,devices[i].id,devices[i].value,devices[i].device_details);
            strcat(sendBuffer,temp);
        }

        printDEBUG(DSYS,"Priprema podataka\n");
        printDEBUG(DSYS,"%s\n",sendBuffer);

        // memset(sendBuffer,0,200);
        http_rest_with_url(sendBuffer);
        vTaskDelay(1000);
        // 
    }
}