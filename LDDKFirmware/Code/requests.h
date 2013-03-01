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
/* Set the EEPROM read offset. Control-OUT.
 * The requested offset is passed in the "wValue" field of the control
 * transfer. No OUT data is sent.
 */

#define CUSTOM_RQ_GET_MEM_RD_OFFSET    4
/* Get the current EEPROM read offset. Control-IN.
 * This control transfer involves a 2 byte data phase where the device sends
 * the current offset to the host. The offset LSB is in the byte 0.
 */

#define CUSTOM_RQ_SET_MEM_WR_OFFSET    5
/* Set the EEPROM write offset. Control-OUT.
 * The requested offset is passed in the "wValue" field of the control
 * transfer. No OUT data is sent.
 */

#define CUSTOM_RQ_GET_MEM_WR_OFFSET    6
/* Get the current EEPROM write offset. Control-IN.
 * This control transfer involves a 2 byte data phase where the device sends
 * the current offset to the host. The offset LSB is in the byte 0.
 */

#endif /* __REQUESTS_H_INCLUDED__ */
