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


/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR( void );
static void prvvUARTRxISR( void );
static void prvvUARTISR( void );

//#define DEF_RS485_PORT 1
/* ----------------------- System Variables ---------------------------------*/

#if defined(DEF_RS485_PORT) // mbed serial port
#include "nvt_rs485.h"
// RS485 TX, RX, RTS pins
NvtRS485  pc(PF_13, PF_14, PF_11);
#else
//UART TX, RX
Serial pc(PG_2, PG_1);
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
#if defined(DEF_RS485_PORT) // mbed serial port
    pc.set_rs485_mode(PF_11);
#endif
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
    pc.putc( ucByte);
    return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */
    *pucByte = pc.getc();
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


