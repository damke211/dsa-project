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
#include<stdlib.h>
#include<stdio.h>


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
#define MAX_CONNECTIONS 10

static httpd_handle_t server = NULL;

void append(char* s, char c) {
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
}


struct action{
    int boardId;
    int outputId;
    int newState;
};

struct action actions[50];
int numOfActions = 0;

void generateAction(int boardId, int outputId, int newState){
    struct action temp;
    temp.outputId = outputId;
    temp.boardId = boardId;
    temp.newState = newState;
    actions[numOfActions] = temp;
    numOfActions++;
}

void deleteFromActions(int index){
    for(int i = index;i < numOfActions;i++)
    {
        actions[i] = actions[i+1]; 
    }
    numOfActions--;
}

struct data{
int id;
int board_id;
int state;
char sensor_data[200];
int connections[MAX_CONNECTIONS];
int numOfCon;
};


int SENSOR_COUNT = 0;
struct data board[20];
struct data test_data [7]={
{
        id:2,
	board_id:1,
	state:1,
        sensor_data:"sensor-motion-Main_door"
    },
    {
        id:3,
	board_id:2,
	state:1,
        sensor_data:"output-light-Main_door"
    },
    {
        id:4,
	board_id:1,
   	state:0,
        sensor_data:"output-light-Garden"
    },
    {
        id:5,
	board_id:2,
        state:1,
        sensor_data:"output-light-Main_door"
    }, 
    {
        id:6,
	board_id:1,
        state:0,
        sensor_data:"output-light-Garden"
    },
    {
        id:1,
	board_id:1,
        state:1,
        sensor_data:"sensor-ultrasonic-Garage_door"
    },
    {
        id:7,
	board_id:2,
 	state:0,
        sensor_data:"sensor-ldr-Roof"
    }
};

esp_err_t devices_get_handler(httpd_req_t *req)
{
   char data_set_parsed[500]={""};
	// strcat(data_set_parsed,"#");
    if(SENSOR_COUNT == 0)
        strcpy(data_set_parsed,"null");
    else{
    for(int i=0;i<SENSOR_COUNT;i++){
            char temp [200] ={""};
            memset(temp,0,sizeof(temp));
            int length=snprintf(NULL,0,"%d",board[i].id);
            char* str=malloc(length+1);
            snprintf(str,length+1,"%d",board[i].id);
            strcat(temp,str);
            free(str);
            strcat(temp,"-");
            length = snprintf( NULL, 0, "%d", board[i].state);
            str = malloc( length + 1 );
            snprintf( str, length + 1, "%d", board[i].state);
            strcat(temp,str);
            free(str);
            strcat(temp,"-");
            strcat(temp,board[i].sensor_data);
            strcat(data_set_parsed,temp);
            strcat(data_set_parsed,"#");
	}
    }

    // printDEBUG(DSYS,"%s",data_set_parsed);
	
    httpd_resp_send(req,data_set_parsed, strlen(data_set_parsed));

	memset(data_set_parsed,0,sizeof(data_set_parsed)+1);

    return ESP_OK;
}

httpd_uri_t devicesGet = {
    .uri       = "/devices",
    .method    = HTTP_GET,
    .handler   = devices_get_handler,
};

esp_err_t devices_post_handler(httpd_req_t *req)
{
    char content[200]={""};
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {

            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }

    printDEBUG(DSYS,"CONTENT ZA PARSIRANJE\n");
    printDEBUG(DSYS,"%s\n",content);   
    printDEBUG(DSYS,"////////////////////////////////\n");   
    char *end_str;
    char *token = strtok_r(content, "#", &end_str);
    int boardId = -1;
    short breakOuter = 1;
    for(int i =0; token!= NULL  ;i++)
    {
        printDEBUG(DSYS,"PARSING VELIKI\n");   
        
        char *end_token;
        if(boardId == -1){
            printDEBUG(DSYS,"PARSING BOARD ID \n");   
            boardId = atoi(token);
            token = strtok_r(NULL, "#", &end_str);
            continue;
        }
        breakOuter = 1;
        char details[50]={""};
        struct data temp;
        temp.board_id = boardId;
        char *token2 = strtok_r(token, "-", &end_token);
        for(int j =0; (token2!= NULL) && breakOuter == 1; j++)
        {
            if(j == 0){
                temp.id = atoi(token2);
            }else if(j ==1){
                printDEBUG(DSYS,"parsiranje value\n");   
                temp.state = atoi(token2);

                for(int k = 0;k<SENSOR_COUNT;k++)
                {
                    if(temp.id == board[k].id){
                        if(temp.state != board[k].state){
                            board[k].state = temp.state;
                            for(int l =0;l < board[k].numOfCon ;l++)
                            {                                
                                int connId = board[k].connections[l];
                                generateAction(boardId,connId,temp.state);
                                printDEBUG(DSYS,"Generisana akcija sa ouput id : %d i sa state : %d\n",connId,temp.state);   
                            }
                        }
                        breakOuter = 0;
                        break;
                    }
                }
            }else 
            {   
                strcat(details,token2);
                if(j!=4){
                char delim = '-';
                append(details,delim);
                }
            }
            token2 = strtok_r(NULL, "-", &end_token);
        }
        if(breakOuter == 1)
        {
            strcpy(temp.sensor_data,details);
            temp.numOfCon = 0;
            board[i-1] = temp;
            SENSOR_COUNT++;
            printDEBUG(DSYS,"Novi senzor je dodan - broj senzora: %d\n",SENSOR_COUNT);   
        }
        token = strtok_r(NULL, "#", &end_str);
    }

    printDEBUG(DSYS,"Zavrsene akcije salje se response************\n");   
    
    // const char resp[]= {"URI RESPONSE"};
    const char resp[200]={""};
    char template[] = {"%d-%d#"};
    int numOfDeletions = 0;
    int deletions[10];
    for(int i =0;i < numOfActions;i++)
    {
        if(actions[i].boardId == boardId)
        {
            char temp[10];
            snprintf(temp,sizeof(temp),template,actions[i].outputId,actions[i].newState);
            strcat(resp,temp);
            // deleteFromActions(i);
            deletions[numOfDeletions] = i;
            numOfDeletions++;
        }
    }

    for(int i =0;i<numOfDeletions;i++){
        deleteFromActions(deletions[i]);
    }

    httpd_resp_send(req, resp, -1);
    return ESP_OK;  
}

httpd_uri_t devicesPost = {
    .uri = "/devices",
    .method = HTTP_POST,
    .handler = devices_post_handler,
};

esp_err_t devices_connect_handler(httpd_req_t *req)
{
    char content[200];

    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {

            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }

    int sensorId = -1;
    int outputId = 0;
    char * token = strtok(content, "-");
    while( token != NULL ) {
        if(sensorId == -1)
            sensorId = atoi(token);
        else
            outputId = atoi(token);
        token = strtok(NULL, " ");
    }
    
    for(int i =0; i < SENSOR_COUNT;i++)
    {   
        if(board[i].id == sensorId)
        {
            if(board[i].numOfCon > MAX_CONNECTIONS)
            {
                printDEBUG(DSYS,"MAXIMUM NUMBER OF CONNECTIONS");
                // return;
            }
                // printDEBUG(DSYS,"konektovalo je nesto");
            int exists = 0;
            for(int j = 0;j < board[i].numOfCon;j++){
                if(board[i].connections[j] == outputId)
                {
                    exists = 1;
                    break;
                }
            }
                    
            if(exists == 1)
                break;
            board[i].connections[board[i].numOfCon] = outputId;
            board[i].numOfCon++;
            printDEBUG(DSYS,"Izvrsilo konekciju %d sa %d",board[i].id,outputId);
        
                // printDEBUG(DSYS,"%d",sensorId);
                // printDEBUG(DSYS,"%d",outputId);
        }
    } 

    // for(int i =0;i<sensors_counter;i++){
    //     printDEBUG(DSYS,"%d",board[i].id);
    //     printDEBUG(DSYS,"%d",board[i].state);
    //     printDEBUG(DSYS,"%s",board[i].sensor_data);
    // }

    const char resp[] = "konektovalo ";
    httpd_resp_send(req, resp, -1);
    return ESP_OK;
}

httpd_uri_t connectPost = {
    .uri = "/connect",
    .method = HTTP_POST,
    .handler = devices_connect_handler,
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
        httpd_register_uri_handler(server, &devicesGet);
        httpd_register_uri_handler(server, &devicesPost);
        httpd_register_uri_handler(server, &connectPost);

        // httpd_register_uri_handler(server, &off);
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
            .max_connection = 4,
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



