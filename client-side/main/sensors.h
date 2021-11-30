#ifndef __SENSORS_H
#define __SENSORS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "driver/pwm.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include <time.h>
#include "hw_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "debug.h"

#define TRIGGER_PIN    4 //D2
#define ECHO_PIN       0 //D3
#define MOTION         12  //D6 can be PWM pin 
#define LDR            13  //D7
#define SERVO          14  //D5
#define PWM_PERIOD     20000 //20ms ili 20000us
#define LED1           2 //D11
#define LED2           13 //D12
#define BUZZER_period  20000
#define BUZZER_pin     15
//Servo, buzzer, dioda,hcsr04 za garazu,2xLED,ldr
//dioda,ldr1 i motion napolje
//7 outputa
struct device{
int id;
int value;
char device_details[50];

};

struct device ser;
struct device LED;
struct device LLED;
struct device mmotion;
struct device LLDR;
struct device ultrason;
struct device devices[10];
void adc         (void);
void LDR_init    (void);
int LDR_output   (void);
void servo_init  (void);
void buzzer_init(void);
void servo	  (int);

void hcsr_init   (void);
uint8_t get_ultrasonic(void);
void motion_init (void);
int motion_detection(void);
void LED_init   (void);
void LED_on     (int LED_number);
void LED_off    (int LED_number);
#endif
