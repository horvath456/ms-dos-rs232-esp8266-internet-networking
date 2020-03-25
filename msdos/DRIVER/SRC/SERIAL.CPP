#include "serial.h"

#include <stdio>
#include <conio.h>
#include <string>
#include <dos.h>
#include <time.h>
#include <iostream>

#define LCR 3
#define RBR 0
#define Latch_High 0x01
#define Latch_Low 0x00
#define THR 0
#define LSR 5

using namespace std;

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

int serial_received(unsigned int port_base_io)
{
    return inp(port_base_io + 5) & 1;
}

int is_transmit_empty(unsigned int port_base_io)
{
    return inp(port_base_io + 5) & 0x20;
}

bool RS232::is_available(unsigned int port_base_io)
{
    return (!(serial_received(port_base_io) == 0));
}

void RS232::setup(unsigned int port_base_io, unsigned long baud)
{
    outp(port_base_io + 1, 0x00); // Disable all interrupts

    outp(port_base_io + LCR, 0x80);
    outp(port_base_io + Latch_High, get_divisor_high_byte(baud));
    outp(port_base_io + Latch_Low, get_divisor_low_byte(baud));

    outp(port_base_io + LCR, 0x03); // 8 bits, no parity, one stop bit
    outp(port_base_io + 2, 0xC7);   // Enable FIFO, clear them, with 14-byte threshold
    outp(port_base_io + 4, 0x0B);   // IRQs enabled, RTS/DSR set
    return;
}

int RS232::wait_read(unsigned int port_base_io)
{
    clock_t time_start = clock();
    while (!RS232::is_available(port_base_io))
    {
        if (clock() >= time_start + 1000)
            return -1;
    }

    return inp(port_base_io);
}

int RS232::read(unsigned int port_base_io)
{
    if (!RS232::is_available(port_base_io))
    {
        return -1;
    }
    else
    {
        return inp(port_base_io);
    }
}

void RS232::write(unsigned int port_base_io, char a)
{
    while (is_transmit_empty(port_base_io) == 0)
        ;

    outp(port_base_io, a);
}

void RS232::write(unsigned int port_base_io, std::string s)
{
    RS232::write(port_base_io, s.c_str());
}

void RS232::write(unsigned int port_base_io, const char *b)
{
    while (true)
    {
        char c = *b++;
        if (c == 0x0)
            return;
        RS232::write(port_base_io, c);
    }
}

void RS232::read_bytes(unsigned int port_base_io, unsigned char far *buffer, size_t length)
{
    for (unsigned int i = 0; i < length; i++)
    {
        int j = RS232::wait_read(port_base_io);
        if (j == -1)
            return;
        char c = (char)j;
        *buffer++ = c;
    }
}

void RS232::read_bytes_until(unsigned int port_base_io, char c, unsigned char far *buffer)
{
    while (true)
    {
        int j = RS232::wait_read(port_base_io);
        if (j == -1)
            return;
        char ch = (char)j;
        *buffer++ = ch;
        if (ch == c)
            return;
    }
}

string RS232::read_string(unsigned int port_base_io)
{
    string s = "";
    while (true)
    {
        int j = RS232::wait_read(port_base_io);
        if (j == -1)
            return s;
        char c = (char)j;

        s = s + c;
    }
    return s;
}

string RS232::read_string_until(unsigned int port_base_io, char c)
{
    string s = "";
    while (true)
    {
        int j = RS232::wait_read(port_base_io);
        if (j == -1)
            return s;
        char ch = (char)j;

        s = s + ch;
        if (ch == c)
            return s;
    }
}