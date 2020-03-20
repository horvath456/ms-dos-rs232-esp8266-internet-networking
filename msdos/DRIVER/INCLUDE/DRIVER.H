/* driver.h */

#ifndef __driver_h_
#define __driver_h_

#include <vector>
#include "AP_ENTRY.H"

class ESP8266_Driver
{
public:
    ESP8266_Driver(unsigned int base_io_port);
    bool init();
    void reset();
    std::vector<AP_List_Entry> get_available_APs();
    void connect_to_AP(std::string ssid, std::string password);
    void start_tls_connection(std::string ip, unsigned int port);
    void send_bytes(unsigned char *data, size_t size);
    unsigned char far *receive_bytes();
    void close_tls_connection();

private:
    static void delay(unsigned int ms);
    unsigned long io_port;
};

#endif