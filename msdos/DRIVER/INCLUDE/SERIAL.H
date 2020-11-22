/* serial.h */

#ifndef __serial_h_
#define __serial_h_

#include <string>

namespace RS232
{
    const unsigned int COM1_PORT_BASE_IO = 0x3F8;
    const unsigned int COM2_PORT_BASE_IO = 0x2F8;
    const unsigned int COM3_PORT_BASE_IO = 0x3E8;
    const unsigned int COM4_PORT_BASE_IO = 0x2E8;

    const unsigned char COM1_INTVECT = 0x0C;
    const unsigned char COM2_INTVECT = 0x0B;
    const unsigned char COM3_INTVECT = 0x0C;
    const unsigned char COM4_INTVECT = 0x0B;

    const unsigned char COM1_PIC_IRQ = 0xEF;
    const unsigned char COM2_PIC_IRQ = 0xF7;
    const unsigned char COM3_PIC_IRQ = 0xEF;
    const unsigned char COM4_PIC_IRQ = 0xF7;

    const unsigned char COM1_PIC_IRQ_MASK = 0x10;
    const unsigned char COM2_PIC_IRQ_MASK = 0x08;
    const unsigned char COM3_PIC_IRQ_MASK = 0x10;
    const unsigned char COM4_PIC_IRQ_MASK = 0x08;

    const unsigned int PORT_BASE_IO[4] = {COM1_PORT_BASE_IO, COM2_PORT_BASE_IO, COM3_PORT_BASE_IO, COM4_PORT_BASE_IO};
    const unsigned int INTVECT[4] = {COM1_INTVECT, COM2_INTVECT, COM3_INTVECT, COM4_INTVECT};
    const unsigned int PIC_IRQ[4] = {COM1_PIC_IRQ, COM2_PIC_IRQ, COM3_PIC_IRQ, COM4_PIC_IRQ};
    const unsigned int PIC_IRQ_MASK[4] = {COM1_PIC_IRQ_MASK, COM2_PIC_IRQ_MASK, COM3_PIC_IRQ_MASK, COM4_PIC_IRQ_MASK};

    void setup(unsigned int port_base_io, unsigned char intvect, unsigned char int_num, unsigned long baud);
    void deinit(unsigned int port_base_io, unsigned char intvect, unsigned char irqmask);
    void clear_recv_buffer();
    bool is_available();
    int wait_read();
    int read();
    void read_bytes(unsigned char far *buffer, size_t length);
    void read_bytes_until(char c, unsigned char far *buffer);
    std::string read_string();
    std::string read_string_until(char c);
    void write(unsigned int port_base_io, char a);
    void write(unsigned int port_base_io, std::string s);
    void write(unsigned int port_base_io, const char *b);
    void set_timeout(unsigned int t);
    void reset_timeout();
} // namespace RS232

#endif