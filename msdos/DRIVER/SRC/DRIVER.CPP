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
    RS232::send(io_port, "AT\n");
    ESP8266_Driver::delay(200);

    bool ok = false;
    unsigned char far *data = RS232::receive(io_port);
    if (data[0] == 'O' && data[1] == 'K')
        ok = true;

    delete[] data;
    return ok;
}

void ESP8266_Driver::reset()
{
    RS232::send(io_port, "AT+RESET\n");
}

vector<AP_List_Entry> ESP8266_Driver::get_available_APs()
{
    RS232::send(io_port, "AT+LISTAP\n");
    ESP8266_Driver::delay(1000);
    unsigned char far *data = RS232::receive(io_port);
    vector<AP_List_Entry> list;
    vector<string> lines;
    string data_string((const char *)data);
    lines = split(data_string, '\n');
    for (unsigned int i = 0; i < lines.size(); i++)
    {
        string line = lines[i].substr(1);
        vector<string> tokens = split(line, ',');
    }
    delete[] data;
    return list;
}

void ESP8266_Driver::connect_to_AP(std::string ssid, std::string password)
{
    string s = "AT+CONNECT=" + ssid + "," + password + "\n";
    RS232::send(io_port, s.c_str());
}

void ESP8266_Driver::start_tls_connection(std::string ip, unsigned int port)
{
    char *x_str = new char[5];
    x_str = itoa(port, x_str, 10); // base 10
    string s = "AT+TLSSTART=" + ip + ":" + string(x_str) + "\n";
    RS232::send(io_port, s.c_str());
}

void ESP8266_Driver::send_bytes(unsigned char *data, size_t size)
{
    char *x_str = new char[5];
    x_str = itoa(size, x_str, 10); // base 10
    string s = "AT+SEND=" + string(x_str) + "\n";
    RS232::send(io_port, s.c_str());
    RS232::send(io_port, (const char *)data);
}

unsigned char far *ESP8266_Driver::receive_bytes()
{
    return RS232::receive(io_port);
}

void ESP8266_Driver::close_tls_connection()
{
    RS232::send(io_port, "AT+TLSCLOSE\n");
}