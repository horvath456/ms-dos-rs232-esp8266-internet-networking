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

unsigned int timeout = 1000;

void far __interrupt new_handler();
void far __interrupt (*old_handler)();

unsigned int intvect_port_base_io;

char buffer[1025]; /* storage for ring-buffer */
int bufferin = 0;  /* position for storing the next character */
int bufferout = 0; /* position for reading the next character */

int serial_received(unsigned int port_base_io)
{
    return inp(port_base_io + 5) & 1;
}

int is_transmit_empty(unsigned int port_base_io)
{
    return inp(port_base_io + 5) & 0x20;
}

unsigned char read_char(unsigned int port_base_io)
{
    return inp(port_base_io);
}

void setup_interrupt(unsigned char intvect)
{
    old_handler = _dos_getvect(intvect);
    _dos_setvect(intvect, new_handler);
}

void deinit_interrupt(unsigned char intvect)
{
    _dos_setvect(intvect, old_handler);
}

void far __interrupt new_handler()
{
    bool available;

    do
    {
        available = (!(serial_received(intvect_port_base_io) == 0));
        if (available)
        {
            buffer[bufferin] = read_char(intvect_port_base_io);
            bufferin++;
            if (bufferin == 1024)
            {
                bufferin = 0;
            } /* ring buffer */
        }
    } while (available); /* while data ready */

    outp(0x20, 0x20); /* clear the interrupt */
}

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

bool RS232::is_available()
{
    return bufferin != bufferout;
}

void RS232::setup(unsigned int port_base_io, unsigned char intvect, unsigned char int_num, unsigned long baud)
{
    outp(port_base_io + 1, 0x00); // Disable all interrupts

    intvect_port_base_io = port_base_io;
    setup_interrupt(intvect);
    /* Set Interrupt Vector Entry */
    /* COM1 - 0x0C, COM2 - 0x0B, COM3 - 0x0C, COM4 - 0x0B */

    outp(port_base_io + LCR, 0x80);
    outp(port_base_io + Latch_High, get_divisor_high_byte(baud));
    outp(port_base_io + Latch_Low, get_divisor_low_byte(baud));

    outp(port_base_io + LCR, 0x03); // 8 bits, no parity, one stop bit
    outp(port_base_io + 2, 0xC7);   // Enable FIFO, clear them, with 14-byte threshold
    outp(port_base_io + 4, 0x0B);   // IRQs enabled, RTS/DSR set

    /* Set Programmable Interrupt Controller */
    /* COM1, COM3 (IRQ4) - 0xEF  */
    /* COM2, COM4 (IRQ3) - 0xF7  */
    outp(0x21, (inp(0x21) & int_num));

    outp(port_base_io + 1, 0x01); // Disable all interrupts /* Interrupt when data received */

    return;
}

void RS232::deinit(unsigned int port_base_io, unsigned char intvect, unsigned char irqmask)
{
    deinit_interrupt(intvect);

    outp(port_base_io + 1, 0x00); // Disable all interrupts

    outp(0x21, (inp(0x21) | irqmask)); /* MASK IRQ using PIC */
                                       /* COM1 und COM3 (IRQ4) - 0x10  */
                                       /* COM2 unf COM4 (IRQ3) - 0x08  */

    deinit_interrupt(intvect);
}

int RS232::read()
{
    if (!RS232::is_available())
    {
        return -1;
    }
    else
    {
        char ch = buffer[bufferout];
        bufferout++;
        if (bufferout == 1024)
            bufferout = 0;
        return (int)ch;
    }
}

int RS232::wait_read()
{
    clock_t time_start = clock();
    while (!RS232::is_available())
    {
        if (clock() >= time_start + timeout)
            return -1;
    }

    return RS232::read();
}

void RS232::read_bytes(unsigned char far *buffer, size_t length)
{
    for (unsigned int i = 0; i < length; i++)
    {
        int j = RS232::wait_read();
        if (j == -1)
            return;
        char c = (char)j;
        *buffer++ = c;
    }
}

void RS232::read_bytes_until(char c, unsigned char far *buffer)
{
    while (true)
    {
        int j = RS232::wait_read();
        if (j == -1)
            return;
        char ch = (char)j;
        *buffer++ = ch;
        if (ch == c)
            return;
    }
}

string RS232::read_string()
{
    string s = "";
    while (true)
    {
        int j = RS232::wait_read();
        if (j == -1)
            return s;
        char c = (char)j;

        s = s + c;
    }
    return s;
}

string RS232::read_string_until(char c)
{
    string s = "";
    while (true)
    {
        int j = RS232::wait_read();
        if (j == -1)
            return s;
        char ch = (char)j;

        s = s + ch;
        if (ch == c)
            return s;
    }
}

void RS232::clear_recv_buffer()
{
    bufferin = 0;  /* position for storing the next character */
    bufferout = 0; /* position for reading the next character */
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

void RS232::set_timeout(unsigned int t)
{
    timeout = t;
}

void RS232::reset_timeout()
{
    timeout = 1000;
}