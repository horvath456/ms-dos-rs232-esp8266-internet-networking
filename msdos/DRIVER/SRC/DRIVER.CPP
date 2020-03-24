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
    RS232::setup(io_port, 115200);
}

bool ESP8266_Driver::test()
{
    RS232::write(io_port, "AT\n");
    return (RS232::read(io_port) == 'O' && RS232::read(io_port) == 'K');
}

void ESP8266_Driver::reset()
{
    RS232::write(io_port, "AT+RESET\n");
}

vector<AP_List_Entry> ESP8266_Driver::get_available_APs()
{
    RS232::write(io_port, "AT+LIST_AP\n");
    vector<AP_List_Entry> list;

    string data = ESP8266_Driver::read_string();
    vector<string> lines;
    lines = split(data, '\n');
    for (unsigned int i = 0; i < lines.size(); i++)
    {
        string line = lines[i].substr(1);
        vector<string> tokens = split(line, ',');
    }

    return list;
}

void ESP8266_Driver::connect_to_AP(std::string ssid, std::string password)
{
    string s = "AT+CONNECT=" + ssid + "," + password + "\n";
    RS232::write(io_port, s);
}

void ESP8266_Driver::start_tls_connection(std::string ip, unsigned int port)
{
    char *x_str = new char[5];
    x_str = itoa(port, x_str, 10); // base 10
    string s = "AT+TLS_START=" + ip + ":" + string(x_str) + "\n";
    RS232::write(io_port, s);
}

void ESP8266_Driver::send_string(string str)
{
    char *x_str = new char[5];
    x_str = itoa(str.length(), x_str, 10); // base 10
    string s = "AT+SEND=" + string(x_str) + "\n";
    RS232::write(io_port, s);
    RS232::write(io_port, str);
}

bool ESP8266_Driver::data_available()
{
    return RS232::is_available(io_port);
}

string ESP8266_Driver::read_string()
{
    return RS232::read_string_until(io_port, 0x0);
}

void ESP8266_Driver::close_tls_connection()
{
    RS232::write(io_port, "AT+TLS_CLOSE\n");
}

bool ESP8266_Driver::is_connected_to_AP()
{
    RS232::write(io_port, "AT+IS_CONNECTED_TO_AP\n");
    return RS232::read(io_port) == 'Y';
}

std::string ESP8266_Driver::get_ip()
{
    RS232::write(io_port, "AT+GET_IP\n");
    return ESP8266_Driver::read_string();
}

std::string ESP8266_Driver::get_mac()
{
    RS232::write(io_port, "AT+GET_MAC\n");
    return ESP8266_Driver::read_string();
}

AP_List_Entry ESP8266_Driver::get_connected_AP()
{
    RS232::write(io_port, "AT+CONNECTED_AP\n");
    AP_List_Entry ap;
    return ap;
}