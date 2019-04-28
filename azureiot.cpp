#include <ArduinoJson.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <arduino.h>

#include <AzureIoTUtility.h>
#include <AzureIoTHub.h>

IOTHUB_CLIENT_LL_HANDLE myIotHubClientHandle;
IOTHUB_CLIENT_STATUS clientStatus;
 
int receiveContext = 0;
 /*String containing Hostname, Device Id & Device Key in the format:             */
  /*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"    */
 
static const char* connectionString = "insertconnectionstringhere";

void sendCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    unsigned int messageTrackingId = (unsigned int)(uintptr_t)userContextCallback;

    //Serial.print("Message Id: %u Sent.\r\n", messageTrackingId);
    Serial.print("Message Sent! : ");
    Serial.println(messageTrackingId);

  
}
static void sendMessage(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, const unsigned char* buffer, size_t size)
{
    static unsigned int messageTrackingId;
    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray(buffer, size);
    if (messageHandle == NULL)
    {
        printf("unable to create a new IoTHubMessage\r\n");
    }
    else
    {
        if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, sendCallback, (void*)(uintptr_t)messageTrackingId) != IOTHUB_CLIENT_OK)
        {
            printf("failed to hand over the message to IoTHubClient");
        }
        else
        {
           // printf("IoTHubClient accepted the message for delivery\r\n");
        }
        IoTHubMessage_Destroy(messageHandle);
    }
    messageTrackingId++;
}
void sendPayload(char* jsonChar){
 
  sendMessage(myIotHubClientHandle, (const unsigned char*)jsonChar, strlen(jsonChar));

  //static char msgText[1024];
  
  //sprintf_s(msgText, sizeof(msgText), jsonChar);

  //sendMessage(myIotHubClientHandle, (const unsigned char*)msgText, strlen(msgText));
  
 
   
  }


static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
    const unsigned char* buffer;
    size_t size;
    
    IoTHubMessage_GetByteArray(message, &buffer, &size);
    
    char* temp = (char*)malloc(size + 1);
    
    (void)memcpy(temp, buffer, size);
    temp[size] = '\0';
    Serial.print("Message Received! : ");
    Serial.print(temp);
    Serial.println("");
    
    String msg = temp;

    if(msg == String("0"))
       digitalWrite(5, LOW);
    if(msg == String("1"))
       digitalWrite(5, HIGH);
    
 }

void azureIotClientInit()
{
 
  myIotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol);

  if(!myIotHubClientHandle)
  {
  Serial.print("Failed to connect to IoT Hub\r\n");
  return;
  }
  Serial.print("Connected to Azure IoT Hub!\r\n");

  //set callback
  if (IoTHubClient_LL_SetMessageCallback(myIotHubClientHandle, ReceiveMessageCallback, &receiveContext) != IOTHUB_CLIENT_OK)
        {
           (void)printf("ERROR: Failed to set call back!\r\n");
          
         }
 //destroy handle (not required for us as we will keep looping
 // IoTHubClient_LL_Destroy(iotHubClientHandle);
}

void azureIotClientRun()
{
  //worker loop
  
  if(!myIotHubClientHandle)
  {
  Serial.print("Not connected to the Azure IoT Hub!");
  return;
  }

 // Serial.print("Start Work\r\n");

  IoTHubClient_LL_DoWork(myIotHubClientHandle);
  ThreadAPI_Sleep(1000);



}






