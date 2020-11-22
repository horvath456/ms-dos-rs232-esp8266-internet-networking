/* driver.h */

#ifndef __driver_h_
#define __driver_h_

#include <vector>
#include "AP_ENTRY.H"

class ESP8266_Driver
{
public:
    ESP8266_Driver(unsigned char comport_num = 0); // COM1 = 0, COM4 = 3
    bool test();
    void reset();
    bool data_available();
    bool is_connected_to_AP();
    std::string get_ip();
    std::string get_mac();
    AP_List_Entry get_connected_AP();
    std::vector<AP_List_Entry> get_available_APs();
    void connect_to_AP(const std::string &ssid, const std::string &password="");
    void init_tls();
    void start_tls_connection_to_ip(const std::string &ip, unsigned int port);
    void start_tls_connection_to_hostname(const std::string &hostname, unsigned int port);
    void send_string(const std::string &s);
    std::string read_string();
    void close_tls_connection();

private:
    unsigned long io_port;
    unsigned char intvect;
    unsigned char pic_irq;
    unsigned char pic_irq_mask;
};

#endif