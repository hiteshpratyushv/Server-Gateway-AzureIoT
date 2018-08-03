#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "iothub_client.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "iothubtransportmqtt.h"

#ifdef MBED_BUILD_TIMESTAMP
#include "certs.h"
#endif // MBED_BUILD_TIMESTAMP

static const char* connectionString = "HostName=IoT-Hub-ESP32.azure-devices.net;DeviceId=ESP32;SharedAccessKey=QAgVyAp8tckHiRvCnhnztqAQvWgs0S842OCTM4myOPw=";

static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

typedef struct EVENT_INSTANCE_TAG
{
    IOTHUB_MESSAGE_HANDLE messageHandle;
    size_t messageTrackingId;  // For tracking the messages within the user callback.
} EVENT_INSTANCE;

static const char *TAG = "subpub";

extern EventGroupHandle_t wifi_event_group;

const int WIFI_CONNECTED_BIT = BIT0;

int ret;
int done=0;
static unsigned char* bytearray_to_str(const unsigned char *buffer, size_t len)
{
    unsigned char* ret = (unsigned char*)malloc(len+1);
    memcpy(ret, buffer, len);
    ret[len] = '\0';
    return ret;
}

static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
    const char* buffer;
    size_t size;
    done = 1;
    if (IoTHubMessage_GetByteArray(message, (const unsigned char**)&buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        (void)printf("unable to retrieve the message data\r\n");
    }
    else
    {
        unsigned char* message_string = bytearray_to_str((const unsigned char *)buffer, size);
        (void)printf("IoTHubMessage_GetByteArray received message: \"%s\" \n", message_string);
        free(message_string);
    }
    return IOTHUBMESSAGE_ACCEPTED;
}

static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    EVENT_INSTANCE* eventInstance = (EVENT_INSTANCE*)userContextCallback;
    (void)printf("Confirmation received for message\r\n");
    IoTHubMessage_Destroy(eventInstance->messageHandle);
}

void mqtt_connect()
{
    int receiveContext = 0;
    if (platform_init() != 0)
    {
        (void)printf("Failed to initialize the platform.\r\n");
    }
    else
    {
        if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol)) == NULL)
        {
            (void)printf("ERROR: iotHubClientHandle is NULL!\r\n");
        }
        else
        {
            bool traceOn = true;
            //(void)printf("size before IoTHubClient_LL_SetOption: %d \n", system_get_free_heap_size());
            IoTHubClient_LL_SetOption(iotHubClientHandle, "logtrace", &traceOn);

#ifdef MBED_BUILD_TIMESTAMP
            // For mbed add the certificate information
            if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
            {
                printf("failure to set option \"TrustedCerts\"\r\n");
            }
#endif // MBED_BUILD_TIMESTAMP

            /* Setting Message call back, so we can receive Commands. */
            //(void)printf("size before IoTHubClient_LL_SetMessageCallback: %d \n", system_get_free_heap_size());
            if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, &receiveContext) != IOTHUB_CLIENT_OK)
            {
                (void)printf("ERROR: IoTHubClient_LL_SetMessageCallback..........FAILED!\r\n");
            }
            else
            {
                (void)printf("IoTHubClient_LL_SetMessageCallback...successful.\r\n");                
            }
        }
    } 
    vTaskDelete(NULL);

}


void mqtt_publish(void *param)
{
    EVENT_INSTANCE message;
    char *TOPIC = "locdata/topic";
    char *buffer = (char*)param;

    printf("Ready to Send String:%s\n",(const char*)buffer);
    if ((message.messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char*)buffer, strlen(buffer))) == NULL)
    {
        (void)printf("ERROR: iotHubMessageHandle is NULL!\r\n");
    }
    else
    {
        message.messageTrackingId = 0;
        MAP_HANDLE propMap = IoTHubMessage_Properties(message.messageHandle);
        if (Map_AddOrUpdate(propMap, "topic", TOPIC) != MAP_OK)
        {
            (void)printf("ERROR: Map_AddOrUpdate Failed!\r\n");
        }
                //(void)printf("free heap size before IoTHubClient_LL_SendEventAsync: %d \n", system_get_free_heap_size());
        if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, message.messageHandle, SendConfirmationCallback, &message) != IOTHUB_CLIENT_OK)
        {
            (void)printf("ERROR: IoTHubClient_LL_SendEventAsync..........FAILED!\r\n");
        }
        else
        {
            (void)printf("IoTHubClient_LL_SendEventAsync Completed\r\n");
        }
        while(!done)
        	IoTHubClient_LL_DoWork(iotHubClientHandle);
        done=0;
        ThreadAPI_Sleep(10);
    } 
    vTaskDelete(NULL);
}
