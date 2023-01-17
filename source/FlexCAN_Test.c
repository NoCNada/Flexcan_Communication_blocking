/*
 * Copyright 2016-2023 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file    FlexCAN_Test.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"
/* TODO: insert other include files here. */

/* TODO: insert other definitions and declarations here. */
void CAN0_CAN_ORED_MB_IRQHANDLER(void);


#define BOARD_LED_GPIO     BOARD_LED_RED_GPIO
#define BOARD_LED_GPIO_PIN BOARD_LED_RED_PIN
#define RX_MESSAGE_BUFFER_NUM (0)
#define TX_MESSAGE_BUFFER_NUM (1)


flexcan_frame_t txFrame, rxFrame;
flexcan_mb_transfer_t txXfer, rxXfer;
//flexcan_frame_t frame;
flexcan_rx_mb_config_t mbConfig;
uint8_t rxComplete = 0,counter = 0, txFlag = 1, counterTime = 50;
uint16_t waitTX = 1000;
/* CAN0_ORed_Message_buffer_IRQn interrupt handler */
void CAN0_CAN_ORED_MB_IRQHANDLER(void) {
  /*  Place your code here */
	if (0U != FLEXCAN_GetMbStatusFlags(CAN0, 1 << 0)){
		FLEXCAN_ClearMbStatusFlags(CAN0, 1 << 0);
		FLEXCAN_ReadRxMb(CAN0, 0, &rxFrame); //leermos datos
		//if(rxFrame.length == 2){ // 2 bytes == transmision completada.
			rxComplete = 1;
		//}
	}

  /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F
     Store immediate overlapping exception return operation might vector to incorrect interrupt. */
  #if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
  #endif
}

/* PIT0_IRQn interrupt handler */
void PIT_CHANNEL_0_IRQHANDLER(void) {
  uint32_t intStatus;
  /* Reading all interrupt flags of status register */
  intStatus = PIT_GetStatusFlags(PIT_PERIPHERAL, PIT_CHANNEL_0);
  PIT_ClearStatusFlags(PIT_PERIPHERAL, PIT_CHANNEL_0, intStatus);

  /* Place your code here */
  if(counter)
	  counter--;
  if(waitTX)
	  waitTX--;
  /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F
     Store immediate overlapping exception return operation might vector to incorrect interrupt. */
  #if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
  #endif
}


/*
 * @brief   Application entry point.
 */
int main(void) {

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    FLEXCAN_EnableMbInterrupts(CAN0, 1 << 0);
    //EnableIRQ(CAN0_CAN_ORED_MB_IRQHANDLER);
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif

    PRINTF("Hello World\n");
    printf("%d",CAN0_rx_mb_config_0.id);
    LED_RED_INIT(0);
    LED_GREEN_INIT(0);
    LED_BLUE_INIT(0);
    txFrame.format = (uint8_t)kFLEXCAN_FrameFormatStandard;
    txFrame.id = FLEXCAN_ID_STD(0x123);
	txFrame.type = (uint8_t)kFLEXCAN_FrameTypeData;
	txFrame.length = (uint8_t)2;
	txFrame.dataByte0 = (uint8_t)30;
	txFrame.dataByte1 = (uint8_t)5;
	mbConfig.format = kFLEXCAN_FrameFormatStandard;
	mbConfig.type   = kFLEXCAN_FrameTypeData;
	mbConfig.id = FLEXCAN_ID_STD(0x10);
	FLEXCAN_SetRxMbConfig(CAN0, 0, &mbConfig, true);
/*    txFrame.format = (uint8_t)kFLEXCAN_FrameFormatStandard;
    txFrame.type   = (uint8_t)kFLEXCAN_FrameTypeData;
    txFrame.id     = FLEXCAN_ID_STD(0x123);
    txFrame.length = (uint8_t)DLC;*/

	//ENVIO DATOS
	//FLEXCAN_TransferSendBlocking(CAN0, 2, &txFrame);

	//FLEXCAN_TransferSendNonBlocking(base, handle, pMbXfer);
    /* Force the counter to be placed into memory. */
    volatile static int i = 0 ;
    /* Enter an infinite loop, just incrementing a counter. */
    while(1) {
        i++ ;
        /*if(FLEXCAN_ReadRxMb(CAN0, 0, &rxFrame) == kStatus_Success){
        	rxComplete = 1;
        }*/
        if(txFlag && !waitTX){
        	txFrame.dataByte0 = (uint8_t)((counterTime+10)/10);
        	txFrame.dataByte1 = (uint8_t)5;
        	if(FLEXCAN_TransferSendBlocking(CAN0, 1, &txFrame) == kStatus_Success){
        		txFlag = 0;
        	}
        	waitTX = 1000;
        }
        if(rxComplete){
        	counterTime=rxFrame.dataByte0*10;
        	txFlag = 1;
        	rxComplete = 0;
        }
        if(!counter){
        	GPIO_PortToggle(BOARD_LED_GPIO, 1u << BOARD_LED_GPIO_PIN);
        	LED_BLUE_TOGGLE();
        	LED_GREEN_TOGGLE();
        	counter=counterTime;
        }
   }
        /* 'Dummy' NOP to allow source level single stepping of
            tight while() loop */
        __asm volatile ("nop");
    return 0 ;
}
