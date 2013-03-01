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
#define USB_CLCD
//#define USE_WD /* Enabling this, resets it pretty often, say even when controlling the LEDs */
#define MEM_EEPROM

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
#ifdef USB_CLCD
#include "clcd.h"           /* clcd for debugging */
#endif
#include "serial.h"         /* serial communication */

/*
We assume that an active low LED is connected to port C bit 0. If you connect it
with different polarity change it in the usbFunctionSetup below. If you connect
it to a different port or bit, change the macros below:
*/
#define LED_PORT_DDR        DDRB
#define LED_PORT_OUTPUT     PORTB
#define LED_BIT             7
#define SW_PORT_DDR         DDRB
#define SW_PORT_INPUT       PINB
#define SW_BIT              2

/* TODO: The following macros could change for another AVR uC */
#define EEPROM_START        0x0
#define EEPROM_SIZE         0x400
#define EEPROM_OFFSET_MASK  (EEPROM_SIZE - 1)
#ifdef MEM_EEPROM
#define MEM_START           EEPROM_START
#define MEM_SIZE            EEPROM_SIZE
#define MEM_OFFSET_MASK     EEPROM_OFFSET_MASK
#define mem_read_byte       eeprom_read_byte
#define mem_write_byte      eeprom_write_byte
#else
#define MEM_START           FLASH_START
#define MEM_SIZE            FLASH_SIZE
#define MEM_OFFSET_MASK     FLASH_OFFSET_MASK
#define mem_read_byte       flash_read_byte
#define mem_write_byte      flash_write_byte
#endif

// TODO: Hack by Pugs. Why is INT1_vect raised for INT0?
// NB This is NOT the case when in bootloader section. Checked with BootloadHID
ISR_ALIAS(INT1_vect, INT0_vect);

static unsigned mem_rd_off = -8;
static unsigned mem_wr_off = 0;

#ifdef USB_CLCD
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
#if DEBUG_LEVEL > 0
#define printlnd(str) println2(str)
#else
#define printlnd(str)
#endif
#endif

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
    usbRequest_t    *rq = (void *)data;
    static uchar    dataBuffer[4];  /* buffer must stay valid when usbFunctionSetup returns */
    char mem_buf[8];
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
#ifdef USB_CLCD
            println2("LED Status: ON");
#endif
        } else {                          /* clear LED */
            LED_PORT_OUTPUT &= ~_BV(LED_BIT); /* inactive low */
#ifdef USB_CLCD
            println2("LED Status: OFF");
#endif
        }
    } else if(rq->bRequest == CUSTOM_RQ_GET_LED_STATUS) {
        dataBuffer[0] = ((LED_PORT_OUTPUT & _BV(LED_BIT)) != 0); /* active high */
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 1;                       /* tell the driver to send 1 byte */
    } else if (rq->bRequest == CUSTOM_RQ_SET_MEM_RD_OFFSET) {
        mem_rd_off = rq->wValue.word & MEM_OFFSET_MASK;
#ifdef USB_CLCD
        println2("Mem Rd Off:");
#endif
        /*
         * Whether usbInterruptIsReady or not, let's set the Interrupt Endpoint data.
         * Basically overwriting the previous one.
         */
        for (mem_i = 0; mem_i < 8; mem_i++)
        {
            if (mem_rd_off + mem_i >= MEM_SIZE)
            {
                break;
            }
            mem_buf[mem_i] = mem_read_byte((uint8_t *)(MEM_START + mem_rd_off + mem_i));
        }
        usbSetInterrupt((void *)mem_buf, mem_i);
    } else if(rq->bRequest == CUSTOM_RQ_GET_MEM_RD_OFFSET) {
        dataBuffer[0] = mem_rd_off & 0xFF;
        dataBuffer[1] = (mem_rd_off >> 8) & 0xFF;
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 2;                       /* tell the driver to send 2 bytes */
    } else if (rq->bRequest == CUSTOM_RQ_SET_MEM_WR_OFFSET) {
        mem_wr_off = rq->wValue.word & MEM_OFFSET_MASK;
#ifdef USB_CLCD
        println2("Mem Wr Off:");
#endif
    } else if(rq->bRequest == CUSTOM_RQ_GET_MEM_WR_OFFSET) {
        dataBuffer[0] = mem_wr_off & 0xFF;
        dataBuffer[1] = (mem_wr_off >> 8) & 0xFF;
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 2;                       /* tell the driver to send 2 bytes */
    }

    return 0;   /* default for not implemented requests: return no data back to host */
}

USB_PUBLIC void usbFunctionWriteOut(uchar *data, uchar len)
{
    int mem_i;

    switch (usbRxToken)
    {
        case 1: // Save in Memory
            if (mem_wr_off + len > MEM_SIZE)
            {
                len = MEM_SIZE - mem_wr_off;
            }
            for (mem_i = 0; mem_i < len; mem_i++)
            {
                mem_write_byte((uint8_t *)(MEM_START + mem_wr_off + mem_i), data[mem_i]);
            }
            mem_wr_off += len;
            println2("Memory written");
            break;
        case 2: // Direct serial transfer
            data[len] = 0;
            usart_tx((char *)data);
            println2("Serial written");
            break;
        default:
            break;
    }
}

/* ------------------------------------------------------------------------- */

int main(void)
{
    char mem_buf[8];
    uint8_t mem_i;
    char ser_buf[8];
    uint8_t ser_data_cnt = 0;
    uchar i;

	_delay_ms(1000);
    //odDebugInit();
    usart_init(9600);
#ifdef USB_CLCD
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
    LED_PORT_OUTPUT &= ~_BV(LED_BIT); /* Switch off LED to start with */
    LED_PORT_DDR |= _BV(LED_BIT);     /* Make the LED bit an output */
    //SW_PORT_DDR &= ~_BV(SW_BIT);      /* Make the switch bit an input */
#ifdef USB_CLCD
    printlnd("Enabling intrs");
#endif
    DBG2(0x00, (uchar *)"F", 1);
    sei();
    DBG2(0x01, (uchar *)"a", 1);
#ifdef USB_CLCD
    printlnd("Entering inf");
#endif
    for(;;) {                /* main event loop */
        DBG2(0x02, (uchar *)"Z1", 2);
#ifdef USE_WD
        wdt_reset();
#endif
        usbPoll();
        DBG2(0x02, (uchar *)"Z2", 2);
        if (SW_PORT_INPUT & _BV(SW_BIT)) /* Clear LCD on switch press */
        {
            clcd_cls();
            println1("Dev Drv Kit v1.1");
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
            for (mem_i = 0; mem_i < 8; mem_i++)
            {
                if (mem_rd_off + mem_i >= MEM_SIZE)
                {
                    break;
                }
                mem_buf[mem_i] = mem_read_byte((uint8_t *)(MEM_START + mem_rd_off + mem_i));
            }
            usbSetInterrupt((void *)mem_buf, mem_i);
        }
        if (usbInterruptIsReady3())
        {
            /* called after every poll of the interrupt endpoint */
            DBG2(0x04, 0, 0);   /* debug output: interrupt data prepared */
            usbSetInterrupt3((void *)ser_buf, ser_data_cnt);
            ser_data_cnt = 0;
        }
        DBG2(0x02, (uchar *)"Z5", 2);
    }
    DBG2(0x05, 0, 0);
    return 0;
}

/* ------------------------------------------------------------------------- */
