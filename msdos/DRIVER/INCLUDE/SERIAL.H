/* serial.h */

#ifndef __serial_h_
#define __serial_h_

#include <string>

namespace RS232
{
void setup(unsigned int port_base_io, unsigned long baud);
bool is_available(unsigned int port_base_io);
char wait_read(unsigned int port_base_io);
int read(unsigned int port_base_io);
void write(unsigned int port_base_io, char a);
void write(unsigned int port_base_io, std::string s);
void write(unsigned int port_base_io, const char *b);
void read_bytes(unsigned int port_base_io, unsigned char far *buffer, size_t length);
void read_bytes_until(unsigned int port_base_io, char c, unsigned char far *buffer);
std::string read_string(unsigned int port_base_io, size_t length);
std::string read_string_until(unsigned int port_base_io, char c);
} // namespace RS232

#endif