/* Name: main.c
 * Project: custom-class, a basic USB example
 * Author: Christian Starkjohann
 * Modified by: Anil Kumar Pugalia
 * Creation Date: 2008-04-09
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 * This Revision: $Id: main.c 692 2008-11-07 15:07:40Z cs $
 */

/*
 * Tested with ATmega32
 * Fuse settings: H: 0x89; L: 0xFF
 */

/*
This example should run on most AVRs with only little changes. No special
hardware resources except INT0 are used. You may have to change usbconfig.h for
different I/O pins for USB. Please note that USB D+ must be the INT0 pin, or
at least be connected to INT0 as well.
*/

/* PD4-7 (DATA4-7) & PB0 (EN) & PB1 (RS) are used for Character LCD */
//#define USE_WD /* Enabling this, resets it pretty often, say even when controlling the LEDs */

#include <avr/io.h>
#ifdef USE_WD
#include <avr/wdt.h>
#endif
#include <avr/interrupt.h>  /* for sei() */
#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include <util/delay.h>     /* for _delay_ms() */
#include <avr/eeprom.h>
#include <string.h>

#include "usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */
#include "requests.h"       /* The custom request numbers we use */
#ifdef USE_CLCD
#include "clcd.h"           /* clcd for display, debugging */
#endif
#include "serial.h"         /* serial communication */
#ifdef MEM_FLASH
#include "flash.h"
#endif

/*
We assume that an active high LED is connected to port B bit 7. If you connect
it with different polarity change it in the usbFunctionSetup below. If you
connect it to a different port or bit, change the macros below:
*/
#define LED_PORT_DDR        DDRB
#define LED_PORT_OUTPUT     PORTB
#define LED_BIT             7
#define SW_PORT_DDR         DDRB
#define SW_PORT_INPUT       PINB
#define SW_BIT              2

#ifdef MEM_EEPROM
#define MEM_START           EEPROM_START
#define MEM_SIZE            (EEPROM_SIZE - EEPROM_START)
#define MEM_OFFSET_MASK     (MEM_SIZE - 1)
#define mem_read_word       eeprom_read_word
#define mem_write_word      eeprom_write_word
#else
#ifdef MEM_FLASH
#define MEM_START           FLASH_START
#define MEM_SIZE            (FLASH_SIZE - FLASH_START)
#define MEM_OFFSET_MASK     (MEM_SIZE - 1)
#define mem_read_word       flash_read_word
#define mem_write_word      flash_write_word
#else
#error No memory type defined
#endif
#endif

static unsigned mem_rd_off = -8;
static unsigned mem_wr_off = 0;

#ifdef USE_CLCD
void println1(char *str)
{
    uint8_t i;

    clcd_move_to(0);
    for (i = 0; (i < 16) && str[i]; i++)
    {
        clcd_data_wr(str[i]);
    }
    for (; i < 16; i++)
    {
        clcd_data_wr(' ');
    }
}
void println2(char *str)
{
    uint8_t i;

    clcd_move_to(16);
    for (i = 0; (i < 16) && str[i]; i++)
    {
        clcd_data_wr(str[i]);
    }
    for (; i < 16; i++)
    {
        clcd_data_wr(' ');
    }
}
#if (DEBUG_LEVEL > 0)
#define printlnd(str) println2(str)
#else
#define printlnd(str)
#endif
#else
#define printlnd(str)
#endif

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
    usbRequest_t *rq = (void *)data;
    static uchar dataBuffer[4]; /* buffer must stay valid when usbFunctionSetup returns */
    uint16_t mem_buf[4];
    uint8_t mem_i;

    if (rq->bRequest == CUSTOM_RQ_ECHO) { /* echo -- used for reliability tests */
        dataBuffer[0] = rq->wValue.bytes[0];
        dataBuffer[1] = rq->wValue.bytes[1];
        dataBuffer[2] = rq->wIndex.bytes[0];
        dataBuffer[3] = rq->wIndex.bytes[1];
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 4;
    } else if (rq->bRequest == CUSTOM_RQ_SET_LED_STATUS) {
        if (rq->wValue.bytes[0] & 1){    /* set LED */
            LED_PORT_OUTPUT |= _BV(LED_BIT); /* active high */
            printlnd("LED Status: ON");
        } else {                          /* clear LED */
            LED_PORT_OUTPUT &= ~_BV(LED_BIT); /* inactive low */
            printlnd("LED Status: OFF");
        }
    } else if(rq->bRequest == CUSTOM_RQ_GET_LED_STATUS) {
        dataBuffer[0] = ((LED_PORT_OUTPUT & _BV(LED_BIT)) != 0); /* active high */
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 1;                       /* tell the driver to send 1 byte */
    } else if (rq->bRequest == CUSTOM_RQ_SET_MEM_RD_OFFSET) {
        mem_rd_off = rq->wValue.word;
        if (mem_rd_off > MEM_SIZE)
        {
            mem_rd_off = MEM_SIZE;
        }
        printlnd("Mem Rd Off: SET");
        /*
         * Whether usbInterruptIsReady or not, let's set the Interrupt Endpoint data.
         * Basically overwriting the previous one.
         */
        for (mem_i = 0; mem_i < 8; mem_i += 2)
        {
            if (mem_rd_off + mem_i >= MEM_SIZE)
            {
                break;
            }
            mem_buf[mem_i >> 1] = mem_read_word((uint16_t *)(MEM_START + mem_rd_off + mem_i));
        }
        usbSetInterrupt((uchar *)mem_buf, mem_i);
    } else if(rq->bRequest == CUSTOM_RQ_GET_MEM_RD_OFFSET) {
        printlnd("Mem Rd Off: GET");
        dataBuffer[0] = mem_rd_off & 0xFF;
        dataBuffer[1] = (mem_rd_off >> 8) & 0xFF;
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 2;                       /* tell the driver to send 2 bytes */
    } else if (rq->bRequest == CUSTOM_RQ_SET_MEM_WR_OFFSET) {
        mem_wr_off = rq->wValue.word;
        if (mem_wr_off > MEM_SIZE)
        {
            mem_wr_off = MEM_SIZE;
        }
        printlnd("Mem Wr Off: SET");
    } else if(rq->bRequest == CUSTOM_RQ_GET_MEM_WR_OFFSET) {
        printlnd("Mem Wr Off: GET");
        dataBuffer[0] = mem_wr_off & 0xFF;
        dataBuffer[1] = (mem_wr_off >> 8) & 0xFF;
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 2;                       /* tell the driver to send 2 bytes */
    } else if(rq->bRequest == CUSTOM_RQ_GET_MEM_SIZE) {
        printlnd("Mem Get Size");
        dataBuffer[0] = MEM_SIZE & 0xFF;
        dataBuffer[1] = (MEM_SIZE >> 8) & 0xFF;
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 2;                       /* tell the driver to send 2 bytes */
    }

    return 0;   /* default for not implemented requests: return no data back to host */
}

USB_PUBLIC void usbFunctionWriteOut(uchar *data, uchar len)
{
    uchar mem_i;

    switch (usbRxToken)
    {
        case 1: // Save in Memory
            if (mem_wr_off + len > MEM_SIZE)
            {
                len = MEM_SIZE - mem_wr_off;
            }
            len &= ~0x01; // Keep length even
            for (mem_i = 0; mem_i < len; mem_i += 2)
            {
                mem_write_word((uint16_t *)(MEM_START + mem_wr_off + mem_i),
                                *(uint16_t *)(data + mem_i));
            }
            mem_wr_off += len;
            printlnd("Memory written");
            break;
        case 2: // Direct serial transfer
            data[len] = 0;
            usart_tx((char *)data);
            printlnd("Serial written");
            break;
        default:
            break;
    }
}

/* ------------------------------------------------------------------------- */

int main(void)
{
    uint16_t mem_buf[4];
    uint8_t mem_i;
    char ser_buf[8];
    uint8_t ser_data_cnt = 0;
    uchar i;

    //odDebugInit();
    usart_init(9600);
    usart_tx("DDK fw v" FW_VER "\r\n");
#ifdef USE_CLCD
    clcd_init();
    println1("Dev Drv Kit v1.1");
#endif

#ifdef USE_WD
    wdt_enable(WDTO_1S);
#endif
    /* Even if you don't use the watchdog, turn it off here. On newer devices,
     * the status of the watchdog (on/off, period) is PRESERVED OVER RESET!
     */
    DBG2(0x00, (uchar *)"A", 1);
    /* RESET status: all port bits are inputs without pull-up.
     * That's the way we need D+ and D-. Therefore we don't need any
     * additional hardware initialization.
     */
    usbInit();
    DBG2(0x00, (uchar *)"B", 1);
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    DBG2(0x00, (uchar *)"C", 1);
    i = 0;
    while(--i) {            /* fake USB disconnect for > 250 ms */
#ifdef USE_WD
        wdt_reset();
#endif
        _delay_ms(1);
    }
    DBG2(0x00, (uchar *)"D", 1);
    usbDeviceConnect();
    DBG2(0x00, (uchar *)"E", 1);
    LED_PORT_OUTPUT |= _BV(LED_BIT);  /* Switch on LED to start with */
    LED_PORT_DDR |= _BV(LED_BIT);     /* Make the LED bit an output */
    //SW_PORT_DDR &= ~_BV(SW_BIT);      /* Make the switch bit an input */
    printlnd("Enabling intrs");
    DBG2(0x00, (uchar *)"F", 1);
    sei();
    DBG2(0x01, (uchar *)"a", 1);
    printlnd("Entering inf");
    for(;;) {                /* main event loop */
        DBG2(0x02, (uchar *)"Z1", 2);
#ifdef USE_WD
        wdt_reset();
#endif
        usbPoll();
        DBG2(0x02, (uchar *)"Z2", 2);
        if (SW_PORT_INPUT & _BV(SW_BIT)) /* Clear LCD on switch press */
        {
#ifdef USE_CLCD
            clcd_cls();
            println1("Dev Drv Kit v1.1");
#endif
        }
        DBG2(0x02, (uchar *)"Z3", 2);
        while (usart_byte_available() && (ser_data_cnt < 8))
        {
            ser_buf[ser_data_cnt++] = usart_byte_rx();
        }
        DBG2(0x02, (uchar *)"Z4", 2);
        if (usbInterruptIsReady())
        {
            /* called after every poll of the interrupt endpoint */
            DBG2(0x03, 0, 0);   /* debug output: interrupt data prepared */
            if (mem_rd_off + 8 < MEM_SIZE)
            {
                mem_rd_off += 8;
            }
            else
            {
                mem_rd_off = MEM_SIZE;
            }
            for (mem_i = 0; mem_i < 8; mem_i += 2)
            {
                if (mem_rd_off + mem_i >= MEM_SIZE)
                {
                    break;
                }
                mem_buf[mem_i >> 1] = mem_read_word((uint16_t *)(MEM_START + mem_rd_off + mem_i));
            }
            usbSetInterrupt((uchar *)mem_buf, mem_i);
        }
        if (usbInterruptIsReady3() && (ser_data_cnt))
        {
            /* called after every poll of the interrupt endpoint */
            DBG2(0x04, 0, 0);   /* debug output: interrupt data prepared */
            usbSetInterrupt3((uchar *)ser_buf, ser_data_cnt);
            ser_data_cnt = 0;
        }
        DBG2(0x02, (uchar *)"Z5", 2);
    }
    DBG2(0x05, 0, 0);
    return 0;
}

/* ------------------------------------------------------------------------- */
