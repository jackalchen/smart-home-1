/**
* @ingroup znpCommunications
* @{
*
* @file example_basic_comms_coordinator_sapi.c
*
* @brief Resets Radio, configures this device to be a Zigbee Coordinator, and displays any messages that are received.
* Uses the SIMPLE API.
*
* This matches example_basic_comms_ROUTER.c
*
* Basic Coordinator Startup:
* - Reset Radio
* - Set Startup Options = CLEAR_STATE and CLEAR_CONFIG - This will restore the ZNP to "factory" configuration
* - Reset Radio
* - Set Zigbee DeviceType to COORDINATOR
* - If you want to set a custom PANID or channel list, do that here and then reset the radio
* - Register Application (Configure the ZNP for our application)
* - Start Application
* - Wait for the Start Confirm
*
* @see http://processors.wiki.ti.com/index.php/Tutorial_on_the_Examples and http://e2e.ti.com/support/low_power_rf/default.aspx
*
* $Rev: 624 $
* $Author: dsmith $
* $Date: 2010-07-02 10:58:43 -0700 (Fri, 02 Jul 2010) $
*
* YOU ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS” WITHOUT WARRANTY 
* OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, 
* TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL TEXAS INSTRUMENTS 
* OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, 
* BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
* INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, 
* LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY 
* CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
*/
#include "../HAL/hal_cc2530znp_target_board.h"
#include "../ZNP/znp_interface.h"
#include "../ZNP/simple_api.h"
#include "znp_example_utils.h"   //for handleReturnValue() and polling()
#include "shp_utils.h"
//uncomment only ONE of the two options below:
//#define USE_SECURITY_MODE_PRECONFIGURED_KEYS
//#define USE_SECURITY_MODE_COORD_DIST_KEYS

extern struct Device allDevices[MAX_NUM_DEVICES];
extern struct Interrupt interruptQueue[MAX_NUM_INTERRUPT];
extern int numInterrupts;
extern signed int znpResult;


unsigned char key[16] = {0x44, 0x65, 0x72, 0x65, 0x6B, 0x53, 0x6D, 0x69, 0x74, 0x68, 0x44, 0x65, 0x73, 0x69, 0x67, 0x6E};    //encryption key used in security

int main( void )
{
    numInterrupts = 0;
    halInit(); 
    printf("\r\n++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");        
    printf("\r\nBasic Communications Example - COORDINATOR - using Simple API\r\n");
    HAL_ENABLE_INTERRUPTS();
    setLed(0);
    //Simple check to ensure that both security options weren't #defined.
#if defined(USE_SECURITY_MODE_PRECONFIGURED_KEYS) && defined(USE_SECURITY_MODE_COORD_DIST_KEYS)
    printf("ERROR\r\n");
    while (1);
#endif 
    
    /* Initialize the ZNP */
    printf("Initializing the ZNP\r\n");    
    znpInit(); 
    handleReturnValue();
    
    /* Set Startup Options (will restore the ZNP to default values on reset) */
    printf("Setting StartupOptions\r\n");
    setStartupOptions(STARTOPT_CLEAR_CONFIG + STARTOPT_CLEAR_STATE);
    handleReturnValue();
    
    /* Reset the ZNP */
    printf("Reset the ZNP\r\n");    
    znpReset();
    handleReturnValue();
    
    /* Set Zigbee Device Type to be COORDINATOR */
    printf("Setting Zigbee Device Type\r\n"); 
    setZigbeeDeviceType(COORDINATOR);
    handleReturnValue();
    
    /* Enabling Callbacks (required to receive ZDO_IEEE_ADDR_RSP)  */
    printf("Enabling Callbacks\r\n"); 
    setCallbacks(CALLBACKS_ENABLED);
    handleReturnValue();    

    /* Configure security mode, if it is being used */
#ifdef USE_SECURITY_MODE_PRECONFIGURED_KEYS
    printf("SECURITY ON WITH PRECONFIGURED KEYS\r\n");
    
    /* Turn security ON with pre-configured keys */
    setSecurityMode(SECURITY_MODE_PRECONFIGURED_KEYS);
    handleReturnValue();
    
    /* All devices on the network must be loaded with the same key */    
    setSecurityKey(key);
    handleReturnValue();    
#endif
    
#ifdef USE_SECURITY_MODE_COORD_DIST_KEYS
    printf("SECURITY ON WITH COORDINATOR DISTRIBUTING KEYS\r\n");
    
    /* Turn security ON with the coordinator distributing keys. */
    setSecurityMode(SECURITY_MODE_COORD_DIST_KEYS);
    handleReturnValue();
    
    /* This is the key that will be distributed to other devices when they attempt to join */
    setSecurityKey(key);
    handleReturnValue();
#endif   
    
#if !defined(USE_SECURITY_MODE_PRECONFIGURED_KEYS) && !defined(USE_SECURITY_MODE_COORD_DIST_KEYS)
    printf("SECURITY OFF\r\n");
#endif 
    
    /* Configure the ZNP for our application: */
    printf("Registering Application\r\n");
    sapiRegisterGenericApplication();    
    handleReturnValue();
    
    /* Now, start the application. We will receive a START_REQUEST_SRSP, and then if it is successful, a START_CONFIRM. */
    printf("Starting the Application\r\n");      
    sapiStartApplication();
    handleReturnValue();
    
    printf("On Network!\r\n");
    setLed(1);
    
    /* On network, display info about this network */
#ifdef DISPLAY_NETWORK_INFORMATION      
    getNetworkConfigurationParameters();                
    getDeviceInformation();
#endif    
    
    // Main While Loop
    unsigned char i = 0;
    while (1) {
      smartAck(0,SMART_ACK_IDLE);
      if (numInterrupts > 0) {
        dataSend(interruptQueue[numInterrupts].shortAddr[0] + 256*interruptQueue[numInterrupts].shortAddr[1], i, 10, "hello", 0);
        smartAck(10,SMART_ACK_ACK);
        i++;
        i = i % 4;
        numInterrupts--;
      }
    }
    /*
    while (allDevices[0].active == 0) {
      smartAck(0,SMART_ACK_ACK);
    }
    printf("done fill in\n");
    char message[] = "hello\0";
    unsigned char i = 0;
    while(1) {
      smartAck(0,SMART_ACK_ACK);
      if (znpResult == RECEIVED_INTERRUPT) {
        dataSend(allDevices[0].shortAddr[0] + 256*allDevices[0].shortAddr[1], i, 10, message, 5);
        smartAck(10,SMART_ACK_ACK);
        i++;
        i = i % 4;
      }
    }
    */
}

/* @} */