/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: portserial.c,v 1.1 2006/08/22 21:35:13 wolti Exp $
 */

/* ----------------------- System includes ----------------------------------*/
#include "mbed.h"                   // Cam

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#if MBED_MAJOR_VERSION >= 6
#include "MyUnbufferedSerial.h"
#endif

/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR( void );
static void prvvUARTRxISR( void );
static void prvvUARTISR( void );

/* ----------------------- System Variables ---------------------------------*/
#if MBED_CONF_NU_MODBUS_ENABLE_SERIAL_RS485
    #include "nvt_rs485.h"
    // RS485 TX, RX, RTS pins
    #if defined(TARGET_NUMAKER_PFM_NUC472)  // for NUC472 board
        NvtRS485  pc(PF_13, PF_14, PF_11);
    #elif defined(TARGET_NUMAKER_PFM_M453)  // for M453 board
        NvtRS485  pc(PE_8, PE_9, PE_11);
    #elif defined(TARGET_NUMAKER_PFM_M487)  || defined(TARGET_NUMAKER_IOT_M487) // for M487 board
        NvtRS485  pc(PB_3, PB_2, PB_8);
        #warning "Notice: It has no RS485 port on NUMAKER-PFM-M487 board."
        #warning "But, you can connect with a RS485 daughter board to (D1, D0, A2) PB_3(TX), PB_2(RX) and PB_8(RTS) pin."
    #elif defined(TARGET_NUMAKER_IOT_M467)  // for M467 board
        NvtRS485  pc(PB_3, PB_2, PB_8);
        #warning "Notice: It has no RS485 port on NUMAKER-IOT-M467 board."
        #warning "But, you can connect with a RS485 daughter board to (D1, D0, A2) PB_3(TX), PB_2(RX) and PB_8(RTS) pin."
    #else
        #error "The demo code can't be executed on this board."
    #endif
#else
    //UART TX, RX
    #if defined(TARGET_NUMAKER_PFM_NUC472)  // for NUC472 board
    UnbufferedSerial pc(PG_2, PG_1);
    #elif defined(TARGET_NUMAKER_PFM_M453)  // for M453 board
    UnbufferedSerial pc(PD_1, PD_6);
    #elif defined(TARGET_NUMAKER_PFM_M487)  // for M487 board
    UnbufferedSerial pc(PC_12, PC_11);
    #elif defined(TARGET_NUMAKER_IOT_M467)  // for M467 board
    UnbufferedSerial pc(PC_12, PC_11);
    #else
        #error "The demo code can't be executed on this board."    
    #endif
#endif

static volatile BOOL RxEnable, TxEnable;     // Cam - keep a static copy of the RxEnable and TxEnable
                                    // status for the simulated ISR (ticker)


/* ----------------------- Start implementation -----------------------------*/
// Cam - This is called every 1mS to simulate Rx character received ISR and
// Tx buffer empty ISR.
static void
prvvUARTISR( void )
{
    if ( TxEnable )
        if(pc.writeable())
            prvvUARTTxReadyISR();

    if ( RxEnable )
        if(pc.readable())
            prvvUARTRxISR();
}

void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
    RxEnable = xRxEnable;
    TxEnable = xTxEnable;
    
    //printf("\r\nRx: %d, TX:%d\r\n", RxEnable, TxEnable);
}

/* ----------------------- Start implementation -----------------------------*/
BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
    pc.baud(ulBaudRate);
#if MBED_CONF_NU_MODBUS_ENABLE_SERIAL_RS485
    #if defined(TARGET_NUMAKER_PFM_NUC472)      // for NUC472 board
    pc.set_rs485_mode(PF_11);
    #elif defined(TARGET_NUMAKER_PFM_M453)  // for M453 board
    pc.set_rs485_mode(PE_11);
    #elif defined(TARGET_NUMAKER_PFM_M487)  // for M487 board
    pc.set_rs485_mode(PB_8);    
    #elif defined(TARGET_NUMAKER_IOT_M467)  // for M467 board
    pc.set_rs485_mode(PB_8);    
    #endif
#endif
//#if 0 //MBED_MAJOR_VERSION >= 6
	SerialBase::Parity parity;
	if (eParity == MB_PAR_NONE)
		parity = SerialBase::None;
	else if ( eParity == MB_PAR_ODD )
		parity = SerialBase::Odd;
	else if ( eParity == MB_PAR_EVEN)
		parity = SerialBase::Even;
	pc.format(
        /* bits */ 8,
        /* parity */ parity, 
        /* stop bit */ 1
		);
//#endif
    return TRUE;
}

void xMBPortSerialPolling( void )
{
    prvvUARTISR( );
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */
    //printf("[%02x]", ucByte );
#if MBED_MAJOR_VERSION >= 6
    pc.write(&ucByte, 1);
#else
    pc.putc( ucByte);
#endif
    return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */
#if MBED_MAJOR_VERSION >= 6
	pc.read(pucByte,1);
#else
    *pucByte = pc.getc();
#endif
    //printf("<%02x>", *pucByte );
    return TRUE;
}

/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
static void prvvUARTTxReadyISR( void )
{
    pxMBFrameCBTransmitterEmpty(  );
}

/* Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
static void prvvUARTRxISR( void )
{
    vMBPortTimersDisable();
    pxMBFrameCBByteReceived(  );
}
