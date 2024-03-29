/**
*  @file znp_examples_utilities.h
*
*  @brief  public methods for znp_examples_utilities.c
*
* $Rev: 585 $
* $Author: dsmith $
* $Date: 2010-06-09 18:26:07 -0700 (Wed, 09 Jun 2010) $
*
* YOU ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE PROVIDED �AS IS� WITHOUT WARRANTY 
* OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, 
* TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL TEXAS INSTRUMENTS 
* OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, 
* BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
* INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, 
* LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY 
* CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
*/
#ifndef SHP_UTILS_H
#define SHP_UTILS_H

#define MAX_NUM_DEVICES 8
#define MAX_NUM_INTERRUPT 8
#define TIMEOUTSEC 2

#define TIME_OUT_ERROR -1
#define RECEIVED_DATA 0
#define RECEIVED_ACK 1
#define RECEIVED_INTERRUPT 2

#define SMART_ACK_READ 0
#define SMART_ACK_ACK 1
#define SMART_ACK_IDLE 2

void smartAck(unsigned char handle, unsigned char mode);
void dataSend(unsigned int destinationShortAddress, unsigned int commandID, unsigned char handle, unsigned char* data, unsigned char dataLength);
void shpUtilInit();

struct Device {
  unsigned char macAddr[8]; // mac address of device
  unsigned char shortAddr[2]; // short address of device
  unsigned char appId[2]; // the ID for the app for this device
  unsigned char active;  // whether this entry is active (available) 
};

struct Interrupt {
    unsigned char shortAddr[2]; // short address of intterupting device
};
#endif
