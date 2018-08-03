/*
 * mqtt_task.h
 *
 *  Created on: 09.03.2017
 *      Author: pcbreflux
 */

#ifndef MAIN_MQTT_TASK_H_
#define MAIN_MQTT_TASK_H_


void mqtt_connect();
void mqtt_disconnect();
void mqtt_publish(void *pvParameters);
void mqtt_subscribe();
void mqtt_unsubscribe();
void mqtt_yield();


#endif /* MAIN_MQTT_TASK_H_ */
