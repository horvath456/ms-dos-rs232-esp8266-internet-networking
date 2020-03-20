#include "serial.h"

#include <stdio>
#include <conio.h>
#include <dos.h>

#define LCR 3
#define RBR 0
#define Latch_High 0x01
#define Latch_Low 0x00
#define THR 0
#define LSR 5

char get_divisor_low_byte(unsigned long baud)
{
    switch (baud)
    {
    case 50:
        return 0x00;
    case 110:
        return 0x17;
    case 220:
        return 0x0C;
    case 300:
        return 0x80;
    case 600:
        return 0xC0;
    case 1200:
        return 0x60;
    case 2400:
        return 0x30;
    case 4800:
        return 0x18;
    case 9600:
        return 0x0C;
    case 19200:
        return 0x06;
    case 38400:
        return 0x03;
    case 57600:
        return 0x02;
    case 115200:
        return 0x01;
    }

    return 0x0;
}

char get_divisor_high_byte(unsigned long baud)
{
    switch (baud)
    {
    case 50:
        return 0x09;
    case 110:
        return 0x04;
    case 220:
        return 0x02;
    case 300:
        return 0x01;
    case 600:
        return 0x00;
    case 1200:
        return 0x00;
    case 2400:
        return 0x00;
    case 4800:
        return 0x00;
    case 9600:
        return 0x00;
    case 19200:
        return 0x00;
    case 38400:
        return 0x00;
    case 57600:
        return 0x00;
    case 115200:
        return 0x00;
    }

    return 0x0;
}

void RS232::setup(unsigned int port_base_io, unsigned long baud)
{
    outp(port_base_io + LCR, 0x80);
    outp(port_base_io + Latch_High, get_divisor_high_byte(baud));
    outp(port_base_io + Latch_Low, get_divisor_low_byte(baud));
    outp(port_base_io + LCR, 0x03);
    return;
}

void RS232::send(unsigned int port_base_io, const char *data)
{
    while (*data != 0)
    {
        outp(port_base_io + THR, *data++);
    }
}

unsigned char far *RS232::receive(unsigned int port_base_io)
{
    unsigned char far *data = new far unsigned char[2048];
    unsigned int cnt = 0;
    while ((inp(port_base_io + LSR) & 0x01) > 0)
    {
        char c = inp(port_base_io + RBR);
        *data++ = c;
        cnt++;
        if (c == 0x0 || cnt >= 2047)
            break;
    }
    return data;
}