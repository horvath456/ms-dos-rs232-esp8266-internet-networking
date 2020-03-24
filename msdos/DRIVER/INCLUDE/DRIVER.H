/* driver.h */

#ifndef __driver_h_
#define __driver_h_

#include <vector>
#include "AP_ENTRY.H"

class ESP8266_Driver
{
public:
    ESP8266_Driver(unsigned int base_io_port);
    bool test();
    void reset();
    bool data_available();
    bool is_connected_to_AP();
    std::string get_ip();
    std::string get_mac();
    AP_List_Entry get_connected_AP();
    std::vector<AP_List_Entry> get_available_APs();
    void connect_to_AP(std::string ssid, std::string password);
    void start_tls_connection(std::string ip, unsigned int port);
    void send_string(std::string s);
    std::string read_string();
    void close_tls_connection();

private:
    unsigned long io_port;
};

#endif