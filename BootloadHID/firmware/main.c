/* Name: main.c
 * Project: AVR bootloader HID
 * Author: Christian Starkjohann
 * Creation Date: 2007-03-19
 * Tabsize: 4
 * Copyright: (c) 2007 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt)
 * This Revision: $Id: main.c 788 2010-05-30 20:54:41Z cs $
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/boot.h>
#include <string.h>
#include <util/delay.h>

static void leaveBootloader() __attribute__((__noreturn__));

#include "bootloaderconfig.h"
#include "usbdrv.c"
#include "oddebug.h"

#include "fwb.h"

/* ------------------------------------------------------------------------ */

#ifndef ulong
#   define ulong    unsigned long
#endif
#ifndef uint
#   define uint     unsigned int
#endif

#if (FLASHEND) > 0xffff /* we need long addressing */
#   define addr_t           ulong
#else
#   define addr_t           uint
#endif

static addr_t           currentAddress; /* in bytes */
static uchar            offset;         /* data already processed in current transfer */
#if BOOTLOADER_CAN_EXIT
static uchar            exitMainloop;
#endif

const PROGMEM char usbHidReportDescriptor[33] = {
    0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                    // USAGE (Vendor Usage 1)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)

    0x85, 0x01,                    //   REPORT_ID (1)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x09, 0x00,                    //   USAGE (Undefined)
    0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)

    0x85, 0x02,                    //   REPORT_ID (2)
    0x95, 0x83,                    //   REPORT_COUNT (131)
    0x09, 0x00,                    //   USAGE (Undefined)
    0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)
    0xc0                           // END_COLLECTION
};

/* allow compatibility with avrusbboot's bootloaderconfig.h: */
#ifdef BOOTLOADER_INIT
#   define bootLoaderInit()         BOOTLOADER_INIT
#endif
#ifdef BOOTLOADER_CONDITION
#   define bootLoaderCondition()    BOOTLOADER_CONDITION
#endif

/* compatibility with ATMega88 and other new devices: */
#ifndef TCCR0
#define TCCR0   TCCR0B
#endif
#ifndef GICR
#define GICR    MCUCR
#endif

static void (*nullVector)(void) __attribute__((__noreturn__));

static void leaveBootloader()
{
    uint8_t gicr;

    DBG1(0x01, 0, 0);
    cli();
    boot_rww_enable();
    USB_INTR_ENABLE = 0;
    USB_INTR_CFG = 0;       /* also reset config bits */
#if F_CPU == 12800000
    TCCR0 = 0;              /* default value */
#endif
    gicr = GICR;
    GICR = gicr | (1 << IVCE);   /* enable change of interrupt vectors */
    GICR = gicr & ~(1 << IVSEL); /* move interrupts to application flash section */
    bootLoaderShut();
/* We must go through a global function pointer variable instead of writing
 *  ((void (*)(void))0)();
 * because the compiler optimizes a constant 0 to "rcall 0" which is not
 * handled correctly by the assembler.
 */
    nullVector();
}

uchar   usbFunctionSetup(uchar data[8])
{
usbRequest_t    *rq = (void *)data;
static uchar    replyBuffer[7] = {
        1,                              /* report ID */
        SPM_PAGESIZE & 0xff,
        SPM_PAGESIZE >> 8,
        ((long)FLASHEND + 1) & 0xff,
        (((long)FLASHEND + 1) >> 8) & 0xff,
        (((long)FLASHEND + 1) >> 16) & 0xff,
        (((long)FLASHEND + 1) >> 24) & 0xff
    };

    if(rq->bRequest == USBRQ_HID_SET_REPORT){
        if(rq->wValue.bytes[0] == 2){
            offset = 0;
            return USB_NO_MSG;
        }
#if BOOTLOADER_CAN_EXIT
        else{
            exitMainloop = 1;
        }
#endif
    }else if(rq->bRequest == USBRQ_HID_GET_REPORT){
        usbMsgPtr = replyBuffer;
        return 7;
    }
    return 0;
}

uchar usbFunctionWrite(uchar *data, uchar len)
{
union {
    addr_t  l;
    uint    s[sizeof(addr_t)/2];
    uchar   c[sizeof(addr_t)];
}       address;
uchar   isLast;

    address.l = currentAddress;
    if(offset == 0){
        DBG2(0xB0, data, 3);
        address.c[0] = data[1];
        address.c[1] = data[2];
#if (FLASHEND) > 0xffff /* we need long addressing */
        address.c[2] = data[3];
        address.c[3] = 0;
#endif
        data += 4;
        len -= 4;
    }
    DBG2(0xB1, (void *)&currentAddress, 4);
    offset += len;
    isLast = offset & 0x80; /* != 0 if last block received */
    do{
#ifndef TEST_MODE
        addr_t prevAddr;
#endif
#if SPM_PAGESIZE > 256
        uint pageAddr;
#else
        uchar pageAddr;
#endif
        //DBG2(0xB2, 0, 0);
        pageAddr = address.s[0] & (SPM_PAGESIZE - 1);
        if(pageAddr == 0){              /* if page start: erase */
            DBG2(0xB3, 0, 0);
#ifndef TEST_MODE
            cli();
            boot_page_erase(address.l); /* erase page */
            sei();
            boot_spm_busy_wait();       /* wait until page is erased */
#endif
        }
        cli();
        boot_page_fill(address.l, *(short *)data);
        sei();
#ifndef TEST_MODE
        prevAddr = address.l;
#endif
        address.l += 2;
        data += 2;
        /* write page when we cross page boundary */
        pageAddr = address.s[0] & (SPM_PAGESIZE - 1);
        if(pageAddr == 0){
            DBG2(0xB4, 0, 0);
#ifndef TEST_MODE
            cli();
            boot_page_write(prevAddr);
            sei();
            boot_spm_busy_wait();
#endif
        }
        len -= 2;
    }while(len);
    currentAddress = address.l;
    DBG1(0xB5, (void *)&currentAddress, 4);
    return isLast;
}

static void initForUsbConnectivity(void)
{
uchar   i = 0;

#if F_CPU == 12800000
    TCCR0 = 3;          /* 1/64 prescaler */
#endif
    usbInit();
    //DBG2(0xA0, 0, 0);
    /* enforce USB re-enumerate: */
    usbDeviceDisconnect();  /* do this while interrupts are disabled */
    //DBG2(0xA1, 0, 0);
    do{             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    //DBG2(0xA2, 0, 0);
    }while(--i);
    //DBG2(0xA3, 0, 0);
    usbDeviceConnect();
    DBG1(0xA4, 0, 0);
    sei();
    DBG1(0xA5, 0, 0);
}

int __attribute__((noreturn)) main(void)
{
    unsigned int blink_cnt = 0;

    /* initialize hardware */
    bootLoaderInit();
    /*
     * Calling the function below for it to get included in the bootloader.
     * Also, making sure that it actually doesn't do anything. And hence,
     * passing an unaligned address 0x0001, for the function to fail.
     */
#if (FLASHEND) > 0xFFFF
    flash_write_block(0x00000001, NULL);
#else
    flash_write_block(0x0001, NULL);
#endif
    odDebugInit();
    DBG1(0x00, 0, 0);
    /* jump to application if jumper is set */
    if(bootLoaderCondition()){
#ifndef TEST_MODE
        uint8_t gicr;

        gicr = GICR;
        GICR = gicr | (1 << IVCE);  /* enable change of interrupt vectors */
        GICR = gicr | (1 << IVSEL); /* move interrupts to boot flash section */
#endif
        DBG1(0x02, 0, 0);
        initForUsbConnectivity();
        do{ /* main event loop */
            //DBG2(0x03, 0, 0);
            wdt_reset();
            usbPoll();

            /* The following piece of code indicates being in bootloader */
            if (++blink_cnt == 60000)
            {
                blink_cnt = 0;
                toggleLED();
                //DBG2(0x04, 0, 0);
            }

            //DBG2(0x05, 0, 0);
#if BOOTLOADER_CAN_EXIT
            if(exitMainloop){
                break;
            }
#endif
        }while(1);
    }
    leaveBootloader();
}

