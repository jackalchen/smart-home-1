#include "../HAL/hal.h"
#include "../ZNP/znp_interface.h"
#include "../ZNP/af_zdo.h"              //for pollingAndFindDevice()
#include "shp_utils.h"
#include <string.h> 

#include "../ZNP/znp_interface_spi.h"

extern unsigned char znpBuf[100];
extern signed int znpResult;                    //from znp_interface

struct Device allDevices[MAX_NUM_DEVICES]; //store information about all devices on th network
struct Interrupt interruptQueue[MAX_NUM_INTERRUPT]; // store short address of all devices requested a poll
int numInterrupts;

void shpUtilInit() {
  for (int i=0; i<MAX_NUM_DEVICES; i++) {
    allDevices[i].active = 0;
  }
}

int is_equal_mac(unsigned char macAddr1[],unsigned char macAddr2[])
{
  for (int i=0; i<8; i++) {
    if (macAddr1[i] != macAddr2[i]) {
      return -1;
    }
  }
  return 0;
}

void addNewDevice(unsigned int shortAddr, unsigned char macAddr[])
{
  int index = -1;
  for (int i=0; i < MAX_NUM_DEVICES; i++) {
    // TODO: if mac address already in table, update it
    if (allDevices[i].active == 0 && index == -1) {
      index = i;
    } else if (allDevices[i].active == 1 && is_equal_mac(macAddr,allDevices[i].macAddr) == 0) {
      index = i;
    }
  }
  if (index != -1) {
    allDevices[index].shortAddr[0] = shortAddr & 0xff;
    allDevices[index].shortAddr[1] = (shortAddr >> 8) & 0xff;
    for (int i=0; i<8; i++) {
       allDevices[index].macAddr[i] = macAddr[i];
    }
    allDevices[index].active = 1;
  }
}

void smartAck(unsigned char handle, unsigned char mode)
/*
handle: a transaction ID for ACK. This is a unique ID to check if we get the correct ACK.
mode == SMART_ACK_READ: read data and store it in znpBuf
mode == SMART_ACK_ACK: read ack. ignore evreything else, except interrupt
mode == SMART_ACK_IDLE: only accepts interrupt.
*/
{
    unsigned long counter = 0;
    unsigned long timeout = (TIMEOUTSEC / 2) * XTAL; //since loop takes more than 1 clock cycle
    while (counter < timeout) {
      while (SRDY_IS_HIGH() && (counter != timeout))
          counter++;   
      spiPoll();
      printHexBytes(znpBuf,znpBuf[0]+3);
      if (znpBuf[SRSP_LENGTH_FIELD] > 0 && znpBuf[1] == MSB(ZB_RECEIVE_DATA_INDICATION) && znpBuf[2] == LSB(ZB_RECEIVE_DATA_INDICATION))
      {
        printf("I GOT DATA!\n");
        // Check if We got an interrupt
        if (znpBuf[5] == 0x55 && znpBuf[6] == 0x55) {
          znpResult = RECEIVED_INTERRUPT;
          if (numInterrupts < MAX_NUM_INTERRUPT) {
            interruptQueue[numInterrupts].shortAddr[0] = znpBuf[4];
            interruptQueue[numInterrupts].shortAddr[1] = znpBuf[3];
            numInterrupts++;
          }
          // If we are in idle mode, return to handle interrupt more promptly
          if (mode == SMART_ACK_IDLE) {
            return;
          }
        // if we are in READ mode, return. 
        } else if (mode == SMART_ACK_READ) {
          znpResult = RECEIVED_DATA;
          return;
        }
      // If new device joined, add it.
      } else if (znpBuf[SRSP_LENGTH_FIELD] > 0 && znpBuf[1] == MSB(ZDO_END_DEVICE_ANNCE_IND) && znpBuf[2] == LSB(ZDO_END_DEVICE_ANNCE_IND)) {
        unsigned char macAddr[8];
        for (int i=0; i<8; i++) {
          macAddr[i] = znpBuf[7+i];
        }
        addNewDevice(256*znpBuf[5] + znpBuf[6],macAddr);
        printf("New Device Joined With SHort Address: 0x%x%x\n",znpBuf[5],znpBuf[6]); 
      // If ack received return
      } else if (znpBuf[SRSP_LENGTH_FIELD] > 0 && znpBuf[1] == MSB(ZB_SEND_DATA_CONFIRM) && znpBuf[2] == LSB(ZB_SEND_DATA_CONFIRM)) {
        if (mode == SMART_ACK_ACK && znpBuf[3] == handle) {
          znpResult = RECEIVED_ACK;
          printf("Received ACK! With Handle %d\n",handle);
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