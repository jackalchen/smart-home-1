#include "../HAL/hal.h"
#include "../ZNP/znp_interface.h"
#include "../ZNP/af_zdo.h"              //for pollingAndFindDevice()
#include "shp_utils.h"
#include <string.h> 

#include "../ZNP/znp_interface_spi.h"
#define TIMEOUTSEC 2
#define TIME_OUT_ERROR -1

extern unsigned char znpBuf[100];

extern signed int znpResult;                    //from znp_interface
struct Device allDevices[MAX_NUM_DEVICES];

void shpUtilInit() {
  for (int i=0; i<MAX_NUM_DEVICES; i++) {
    allDevices[i].active = 0;
  }
}

void addNewDevice(unsigned int shortAddr)
{
  for (int i=0; i < MAX_NUM_DEVICES; i++) {
    // TODO: if mac address already in table, update it
    if (allDevices[i].active == 0) {
      allDevices[i].shortAddr[0] = shortAddr & 0xff;
      allDevices[i].shortAddr[1] = (shortAddr >> 8) & 0xff;
      allDevices[i].active = 1;
      return;
    }
  }
}

void smartAck(unsigned char handle, unsigned char mode)
/*
mode == 0: read data and store it in znpBuf
mode == 1: read ack. ignore evreything else, except interrupt
*/
{
    unsigned long counter = 0;
    unsigned long timeout = (TIMEOUTSEC / 2) * XTAL; //since loop takes more than 1 clock cycle
    while (counter < timeout) {
      while (SRDY_IS_HIGH() && (counter != timeout))
          counter++;   
    
      spiPoll();
      if (znpBuf[SRSP_LENGTH_FIELD] > 0 && znpBuf[1] == MSB(ZB_RECEIVE_DATA_INDICATION) && znpBuf[2] == LSB(ZB_RECEIVE_DATA_INDICATION))
      {
        // TODO: check for interupt packet
        if (mode ==0) {
          znpResult = 0;
          return;
        }
        // interupt handling
      } else if (znpBuf[SRSP_LENGTH_FIELD] > 0 && znpBuf[1] == MSB(ZDO_END_DEVICE_ANNCE_IND) && znpBuf[2] == LSB(ZDO_END_DEVICE_ANNCE_IND)) {
        addNewDevice(256*znpBuf[5] + znpBuf[6]);
        printf("short address of new device %d %d",znpBuf[5],znpBuf[6]); 
      } else if (znpBuf[SRSP_LENGTH_FIELD] > 0 && znpBuf[1] == MSB(ZB_SEND_DATA_CONFIRM) && znpBuf[2] == LSB(ZB_SEND_DATA_CONFIRM)) {
        if (mode == 1 && znpBuf[3] == handle) {
          znpResult = 0;
          printf("Received ACK! YEAYYY!");
          return;
        }
      }
   }
   znpResult = TIME_OUT_ERROR;
}

void dataSend(unsigned int destinationShortAddress, unsigned int commandID, unsigned char handle, unsigned char* data, unsigned char dataLength)
{
    if (dataLength > MAXIMUM_PAYLOAD_LENGTH)
    {
        znpResult = -1;
        return;
    }
    
    znpBuf[0] = 11 + dataLength;
    znpBuf[1] = MSB(ZB_SEND_DATA_REQUEST);
    znpBuf[2] = LSB(ZB_SEND_DATA_REQUEST);      
    znpBuf[3] = MSB(destinationShortAddress); 
    znpBuf[4] = LSB(destinationShortAddress);
    znpBuf[5] = LSB(commandID);
    znpBuf[6] = MSB(commandID);
    znpBuf[7] = handle;
    znpBuf[8] = MAC_ACK; //Could also use AF_APS_ACK;
    znpBuf[9] = DEFAULT_RADIUS;
    znpBuf[10] = dataLength; 
    
    memcpy(znpBuf+11, data, dataLength);
    znpResult = sendMessage();
}