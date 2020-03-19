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
    std::vector<AP_List_Entry> get_available_APs();

    private:
    static void delay(unsigned int ms);
    unsigned long io_port;
};

#endif