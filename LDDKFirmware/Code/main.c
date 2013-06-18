/* Name: main.c
 * Project: custom-class, a basic USB example
 * Author: Christian Starkjohann
 * Modified by: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 * Creation Date: 2008-04-09
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 * This Revision: $Id: main.c 692 2008-11-07 15:07:40Z cs $
 */

/*
 * Tested with ATmega16/32
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
#include "flash.h"

/*
We assume that an active high LED is connected to port B bit 7. If you connect
it with different polarity change it in the usbFunctionSetup below. If you
connect it to a different port or bit, change the macros below:
*/
#define LED_PORT_DDR        DDRB
#define LED_PORT_OUTPUT     PORTB
#define LED_BIT             7
#define SW_PORT_DDR         DDRB
#define SW_PORT_PULLUP      PORTB
#define SW_PORT_INPUT       PINB
#define BT_BIT              2 /* Button switch */
#define DL_BIT              4 /* Download switch */

typedef enum
{
    eeprom,
    flash,
    total_mem_type
} mem_type_t;
static mem_type_t mem_type;
static unsigned mem_start;
static unsigned mem_size;
static unsigned mem_offset_mask;
static uint8_t (*mem_read_byte)(const uint8_t *);
static unsigned mem_rd_off;
static unsigned mem_wr_off;
static unsigned fwp_buf_off;
static unsigned char flash_write_page_buffer[SPM_PAGESIZE];

#ifdef USE_CLCD
static void println1(char *str)
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
#if (DEBUG_LEVEL > 0)
static void println2(char *str)
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
#define printlnd(str) println2(str)
#else
#define printlnd(str)
#endif
#else
#define printlnd(str)
#endif

static void pre_load_mem_data(void)
{
    uint8_t mem_buf[8];
    uint8_t mem_i;

    for (mem_i = 0; mem_i < 8; mem_i++)
    {
        if (mem_rd_off + mem_i >= mem_size)
        {
            break;
        }
        mem_buf[mem_i] = mem_read_byte((uint8_t *)(mem_start + mem_rd_off + mem_i));
    }
    usbSetInterrupt((uchar *)mem_buf, mem_i);
}

static void set_mem_type(mem_type_t mt)
{
    if (mt == eeprom)
    {
        mem_type = eeprom;
        mem_start = EEPROM_START;
        mem_size = EEPROM_SIZE - EEPROM_START;
        mem_offset_mask = (mem_size - 1);
        mem_read_byte = eeprom_read_byte;
        mem_rd_off = 0;
        mem_wr_off = 0;
        fwp_buf_off = 0;
    }
    else if (mt == flash)
    {
        mem_type = flash;
        mem_start = FLASH_START;
        mem_size = FLASH_SIZE - FLASH_START;
        mem_offset_mask = (mem_size - 1);
        mem_read_byte = flash_read_byte;
        mem_rd_off = 0;
        mem_wr_off = 0;
        fwp_buf_off = 0;
    }
    /*
     * Whether usbInterruptIsReady or not, let's set the Interrupt Endpoint data.
     * Basically overwriting the previous one.
     */
    pre_load_mem_data();
}
/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
    usbRequest_t *rq = (void *)data;
    static uchar dataBuffer[4]; /* buffer must stay valid when usbFunctionSetup returns */
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
        printlnd("Mem Rd Off: SET");
        mem_rd_off = rq->wValue.word;
        if (mem_rd_off > mem_size)
        {
            mem_rd_off = mem_size;
        }
        /*
         * Whether usbInterruptIsReady or not, let's set the Interrupt Endpoint data.
         * Basically overwriting the previous one.
         */
        pre_load_mem_data();
    } else if(rq->bRequest == CUSTOM_RQ_GET_MEM_RD_OFFSET) {
        printlnd("Mem Rd Off: GET");
        dataBuffer[0] = mem_rd_off & 0xFF;
        dataBuffer[1] = (mem_rd_off >> 8) & 0xFF;
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 2;                       /* tell the driver to send 2 bytes */
    } else if (rq->bRequest == CUSTOM_RQ_SET_MEM_WR_OFFSET) {
        printlnd("Mem Wr Off: SET");
        mem_wr_off = rq->wValue.word;
        if (mem_wr_off > mem_size)
        {
            mem_wr_off = mem_size;
        }
        if (mem_type == flash)
        {
            fwp_buf_off = mem_wr_off & (SPM_PAGESIZE - 1);
            if (fwp_buf_off) // Page non-aligned offset - read the page till fwp_buf_off
            {
                for (mem_i = 0; mem_i < fwp_buf_off; mem_i++)
                    flash_write_page_buffer[mem_i] =
                        mem_read_byte((uint8_t *)(mem_start + (mem_wr_off & ~(SPM_PAGESIZE - 1)) + mem_i));
            }
        }
    } else if(rq->bRequest == CUSTOM_RQ_GET_MEM_WR_OFFSET) {
        printlnd("Mem Wr Off: GET");
        dataBuffer[0] = mem_wr_off & 0xFF;
        dataBuffer[1] = (mem_wr_off >> 8) & 0xFF;
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 2;                       /* tell the driver to send 2 bytes */
    } else if(rq->bRequest == CUSTOM_RQ_GET_MEM_SIZE) {
        printlnd("Mem Get Size");
        dataBuffer[0] = mem_size & 0xFF;
        dataBuffer[1] = (mem_size >> 8) & 0xFF;
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 2;                       /* tell the driver to send 2 bytes */
    } else if (rq->bRequest == CUSTOM_RQ_SET_MEM_TYPE) {
        printlnd("Mem Set Type");
        if (rq->wValue.bytes[0] < total_mem_type)
        {
            set_mem_type(rq->wValue.bytes[0]);
        }
    } else if(rq->bRequest == CUSTOM_RQ_GET_MEM_TYPE) {
        printlnd("Mem Get Type");
        dataBuffer[0] = mem_type;
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 1;                       /* tell the driver to send 1 byte */
    } else if (rq->bRequest == CUSTOM_RQ_SET_REGISTER) {
        printlnd("Reg Set");
        switch (rq->wIndex.bytes[0])
        {
            case REG_RSVD:
                break;
            case REG_DIRA:
                DDRA = (rq->wValue.bytes[0] & ~MASK_PORTA) | (DDRA & MASK_PORTA);
                break;
            case REG_DIRB:
                DDRB = (rq->wValue.bytes[0] & ~MASK_PORTB) | (DDRB & MASK_PORTB);
                break;
            case REG_DIRC:
                DDRC = (rq->wValue.bytes[0] & ~MASK_PORTC) | (DDRC & MASK_PORTC);
                break;
            case REG_DIRD:
				if ((rq->wValue.bytes[0] & ~MASK_PORTD) & 0b11) // PD0 & PD1 - are going to be used
					usart_disable(); // Serial needs to be disabled
                DDRD = (rq->wValue.bytes[0] & ~MASK_PORTD) | (DDRD & MASK_PORTD);
				if (!((rq->wValue.bytes[0] & ~MASK_PORTD) & 0b11)) // PD0 & PD1 - no longer being used
					usart_enable(); // Serial can be re-enabled
                break;
            case REG_PORTA:
                PORTA = (rq->wValue.bytes[0] & ~MASK_PORTA) | (PORTA & MASK_PORTA);
                break;
            case REG_PORTB:
                PORTB = (rq->wValue.bytes[0] & ~MASK_PORTB) | (PORTB & MASK_PORTB);
                break;
            case REG_PORTC:
                PORTC = (rq->wValue.bytes[0] & ~MASK_PORTC) | (PORTC & MASK_PORTC);
                break;
            case REG_PORTD:
                PORTD = (rq->wValue.bytes[0] & ~MASK_PORTD) | (PORTD & MASK_PORTD);
                break;
            default:
                break;
        }
    } else if(rq->bRequest == CUSTOM_RQ_GET_REGISTER) {
        printlnd("Reg Get");
        switch (rq->wIndex.bytes[0])
        {
            case REG_RSVD:
                dataBuffer[0] = 0;
                break;
            case REG_DIRA:
                dataBuffer[0] = DDRA & ~MASK_PINA;
                break;
            case REG_DIRB:
                dataBuffer[0] = DDRB & ~MASK_PINB;
                break;
            case REG_DIRC:
                dataBuffer[0] = DDRC & ~MASK_PINC;
                break;
            case REG_DIRD:
                dataBuffer[0] = DDRD & ~MASK_PIND;
                break;
            case REG_PORTA:
                dataBuffer[0] = PINA & ~MASK_PINA;
                break;
            case REG_PORTB:
                dataBuffer[0] = PINB & ~MASK_PINB;
                break;
            case REG_PORTC:
                dataBuffer[0] = PINC & ~MASK_PINC;
                break;
            case REG_PORTD:
                dataBuffer[0] = PIND & ~MASK_PIND;
                break;
            default:
                dataBuffer[0] = 0xFF;
                break;
        }
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 1;                       /* tell the driver to send 1 byte */
    }

    return 0;   /* default for not implemented requests: return no data back to host */
}

USB_PUBLIC void usbFunctionWriteOut(uchar *data, uchar len)
{
    uchar mem_i;

    switch (usbRxToken)
    {
        case 1: // Save in Memory
            if (mem_wr_off + len > mem_size)
            {
                len = mem_size - mem_wr_off;
            }
            if (len == 0)
            {
                break;
            }
            for (mem_i = 0; mem_i < len; mem_i++)
            {
                if (mem_type == eeprom)
                {
                    eeprom_write_byte((uint8_t *)(mem_start + mem_wr_off + mem_i),
                            *(uint8_t *)(data + mem_i));
                }
                else if (mem_type == flash)
                {
                    flash_write_page_buffer[fwp_buf_off + mem_i] = *(uint8_t *)(data + mem_i);
                    if (fwp_buf_off + mem_i == (SPM_PAGESIZE - 1))
                    {
                        flash_write_block((uint8_t *)(mem_start + mem_wr_off + mem_i - (SPM_PAGESIZE - 1)),
                                flash_write_page_buffer);
                        fwp_buf_off = -(mem_i + 1);
                    }
                }
            }
            fwp_buf_off += len;
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
    char ser_buf[8];
    uint8_t ser_data_cnt = 0;
    uchar i;

    //odDebugInit();
    usart_init(9600);
    usart_tx("LDDK fw v" FW_VER "\r\n");
#ifdef USE_CLCD
    clcd_init();
    println1("LDDK fw v" FW_VER);
#endif

#ifdef USE_WD
    /* Even if you don't use the watchdog, turn it off here. On newer devices,
     * the status of the watchdog (on/off, period) is PRESERVED OVER RESET!
     */
    wdt_enable(WDTO_1S);
#endif
    DBG2(0x00, (uchar *)"A", 1);
    /* Default memory access setting is for EEPROM */
    set_mem_type(eeprom);
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
     /* Make default state pulled-up for the BT & DL switches */
    SW_PORT_PULLUP |= (_BV(BT_BIT) | _BV(DL_BIT));
    /* Make the BT & DL switch bits as input */
    SW_PORT_DDR &= ~(_BV(BT_BIT) | _BV(DL_BIT));
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
        if (!(SW_PORT_INPUT & _BV(BT_BIT)))
        {
            usart_tx("Dev Drv Kit v2.1\r\n");
#ifdef USE_CLCD
            clcd_cls(); /* Clear LCD on switch press */
            println1("Dev Drv Kit v2.1");
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
            if (mem_rd_off + 8 < mem_size)
            {
                mem_rd_off += 8;
            }
            else
            {
                mem_rd_off = mem_size;
            }
            pre_load_mem_data();
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
