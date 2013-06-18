/* Name: requests.h
 * Project: custom-class, a basic USB example
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-09
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 * This Revision: $Id: requests.h 692 2008-11-07 15:07:40Z cs $
 */

/* This header is shared between the firmware and the host software. It
 * defines the USB request numbers (and optionally data types) used to
 * communicate between the host and the device.
 */

#ifndef __REQUESTS_H_INCLUDED__
#define __REQUESTS_H_INCLUDED__

#define CUSTOM_RQ_ECHO          0
/* Request that the device sends back wValue and wIndex. This is used with
 * random data to test the reliability of the communication.
 */

#define CUSTOM_RQ_SET_LED_STATUS    1
/* Set the LED status. Control-OUT.
 * The requested status is passed in the "wValue" field of the control
 * transfer. No OUT data is sent. Bit 0 of the low byte of wValue controls
 * the LED.
 */

#define CUSTOM_RQ_GET_LED_STATUS    2
/* Get the current LED status. Control-IN.
 * This control transfer involves a 1 byte data phase where the device sends
 * the current status to the host. The status is in bit 0 of the byte.
 */

#define CUSTOM_RQ_SET_MEM_RD_OFFSET    3
/* Set the selected memory's read offset. Control-OUT.
 * The requested offset is passed in the "wValue" field of the control
 * transfer. No OUT data is sent.
 */

#define CUSTOM_RQ_GET_MEM_RD_OFFSET    4
/* Get the selected memory's current read offset. Control-IN.
 * This control transfer involves a 2 byte data phase where the device sends
 * the current offset to the host. The offset LSB is in the byte 0.
 */

#define CUSTOM_RQ_SET_MEM_WR_OFFSET    5
/* Set the selected memory's write offset. Control-OUT.
 * The requested offset is passed in the "wValue" field of the control
 * transfer. No OUT data is sent.
 */

#define CUSTOM_RQ_GET_MEM_WR_OFFSET    6
/* Get the selected memory's current write offset. Control-IN.
 * This control transfer involves a 2 byte data phase where the device sends
 * the current offset to the host. The offset LSB is in the byte 0.
 */

#define CUSTOM_RQ_GET_MEM_SIZE         7
/* Get the selected memory's size. Control-IN.
 * This control transfer involves a 2 byte data phase where the device sends
 * the selected memory's size to the host. The offset LSB is in the byte 0.
 */

#define CUSTOM_RQ_SET_MEM_TYPE         8
/* Set the memory to be accessed. Control-OUT.
 * The requested type is passed in the "wValue" field of the control
 * transfer. No OUT data is sent. Value of 0 indicates EEPROM and value of 1
 * indicates Flash. Other values are ignored. In case of successful setting,
 * all the offsets are reset.
 */

#define CUSTOM_RQ_GET_MEM_TYPE         9
/* Get the memory set to be accessed. Control-IN.
 * This control transfer involves a 1 byte data phase where the device sends
 * the current type to the host. The status is in bit 0 of the byte.
 */

#define CUSTOM_RQ_SET_REGISTER         10
/* Set the value of the specified register. Control-OUT.
 * The value is passed in the "wValue" field for the register specified by the
 * "wIndex" field of the control transfer. No OUT data is sent. Following values
 * of "wIndex" are supported:
 * 0 - Reserved
 * 1 - DDRA
 * 2 - DDRB
 * 3 - DDRC
 * 4 - DDRD
 * 5 - PORTA
 * 6 - PORTB
 * 7 - PORTC
 * 8 - PORTD
 * Other values are ignored.
 */

#define CUSTOM_RQ_GET_REGISTER         11
/* Get the memory set to be accessed. Control-IN.
 * This control transfer involves a 1 byte data phase where the device sends
 * the current value of the register specified by the "wIndex" field of the
 * control transfer, to the host. Following values of "wIndex" are supported:
 * 0 - Reserved
 * 1 - DDRA
 * 2 - DDRB
 * 3 - DDRC
 * 4 - DDRD
 * 5 - PINA
 * 6 - PINB
 * 7 - PINC
 * 8 - PIND
 * Other values are ignored.
 */

/* Defines for the register indices */
#define REG_RSVD 0
#define REG_DIRA 1
#define REG_DIRB 2
#define REG_DIRC 3
#define REG_DIRD 4
#define REG_PORTA 5
#define REG_PORTB 6
#define REG_PORTC 7
#define REG_PORTD 8

#define MASK_PORTA 0x00
#define MASK_PINA MASK_PORTA
#ifdef USE_CLCD
#define MASK_PORTB 0x97 /* PGM, DOWNLOAD, BUTTON, CLrs, CLen */
#define MASK_PINB 0x83 /* PGM, CLrs, CLen */
#else
#define MASK_PORTB 0x94 /* PGM, DOWNLOAD, BUTTON */
#define MASK_PINB 0x80 /* PGM */
#endif
#define MASK_PORTC 0x00
#define MASK_PINC MASK_PORTC
#ifdef USE_CLCD
#define MASK_PORTD 0xFC /* D7-D4, D-, D+ */
#else
#define MASK_PORTD 0x0C /* D-, D+ */
#endif
#define MASK_PIND MASK_PORTD

#endif /* __REQUESTS_H_INCLUDED__ */
