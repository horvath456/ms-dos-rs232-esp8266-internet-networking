#include <vector>
#include <cstring>
#include <string>
#include <dos.h>
#include <conio.h>
#include <iostream>

#include "driver.h"
#include "serial.h"

using namespace std;

std::vector<std::string> split(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    size_t last = 0;
    size_t next = 0;
    while ((next = s.find(delimiter, last)) != string::npos)
    {
        token = s.substr(last, next - last);
        last = next + 1;
        tokens.push_back(token);
    }
    token = s.substr(last);
    tokens.push_back(token);
    return tokens;
}

ESP8266_Driver::ESP8266_Driver(unsigned int base_io_port)
{
    io_port = base_io_port;
}

void ESP8266_Driver::delay(unsigned int ms)
{
}

bool ESP8266_Driver::init()
{
    RS232::setup(io_port, 115200);
    ESP8266_Driver::delay(100);
    RS232::send(io_port, "ATE0");
    ESP8266_Driver::delay(200);
    RS232::send(io_port, "AT");
    ESP8266_Driver::delay(200);

    bool ok = false;
    unsigned char far *data = RS232::receive(io_port);
    if (data[0] == 'O' && data[1] == 'K')
        ok = true;

    delete[] data;
    return ok;
}

vector<AP_List_Entry> ESP8266_Driver::get_available_APs()
{
    RS232::send(io_port, "AT+CWLAP");
    ESP8266_Driver::delay(1000);
    unsigned char far *data = RS232::receive(io_port);
    vector<AP_List_Entry> list;
    vector<string> lines;
    string data_string((const char *)data);
    lines = split(data_string, '\n');
    for (unsigned int i = 0; i < lines.size(); i++)
    {
        string line = lines[i].substr(8);
        vector<string> tokens = split(line, ',');
    }
    delete[] data;
    return list;
}