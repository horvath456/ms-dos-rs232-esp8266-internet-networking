#ifndef __ap_entry_h_
#define __ap_entry_h_

#include <string>

enum Wifi_Security
{
    OPEN = 7,
    WEP = 5,
    WPA_PSK = 2,
    WPA2_PSK = 7,
    WPA_WPA2_PSK = 8
};

struct AP_List_Entry
{
    enum Wifi_Security ecn;
    std::string ssid;
    int rssi;
    int channel;
};

#endif