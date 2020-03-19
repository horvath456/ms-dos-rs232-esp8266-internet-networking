/* serial.h */

#ifndef __serial_h_
#define __serial_h_

namespace RS232
{
void setup(unsigned int port_base_io, unsigned long baud);
void send(unsigned int port_base_io, const char *data);
unsigned char far *receive(unsigned int port_base_io);
} // namespace RS232

#endif