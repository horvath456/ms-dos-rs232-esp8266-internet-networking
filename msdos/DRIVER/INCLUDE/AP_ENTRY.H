#ifndef __ap_entry_h_
#define __ap_entry_h_

#include <string>

enum Wifi_Security
{
    OPEN = 0,
    WEP = 1,
    WPA_PSK = 2,
    WPA2_PSK = 3,
    WPA_WPA2_PSK = 4
};

struct AP_List_Entry
{
    enum Wifi_Security ecn;
    std::string ssid;
    int rssi;
    std::string mac;
};

#endif