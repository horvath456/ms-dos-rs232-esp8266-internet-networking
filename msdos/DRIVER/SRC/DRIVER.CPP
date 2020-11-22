#include <vector>
#include <cstring>
#include <string>
#include <dos.h>
#include <conio.h>
#include <iostream>
#include <time.h>

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

int stoi(const std::string &s)
{
    int x;
    sscanf(s.c_str(), "%d", &x);
    return x;
}

bool wait_for_ok(unsigned int timeout_ms)
{
    clock_t time_start = clock();
    while (true)
    {
        if (RS232::is_available())
        {
            string s = RS232::read_string();
            if (s == "OK\r\n")
                return true;
        }
        if (clock() >= time_start + timeout_ms)
            return false;
    }
}

ESP8266_Driver::ESP8266_Driver(unsigned char comport_num)
{
    io_port = RS232::PORT_BASE_IO[comport_num];
    intvect = RS232::INTVECT[comport_num];
    pic_irq = RS232::PIC_IRQ[comport_num];
    pic_irq_mask = RS232::PIC_IRQ_MASK[comport_num];
    RS232::setup(io_port, intvect, pic_irq, 115200);
}

bool ESP8266_Driver::test()
{
    RS232::write(io_port, "AT\n");
    return RS232::read_string() == "OK\r\n";
}

void ESP8266_Driver::reset()
{
    RS232::write(io_port, "AT+RESET\n");
    wait_for_ok(1000);
}

vector<AP_List_Entry> ESP8266_Driver::get_available_APs()
{
    RS232::write(io_port, "AT+LIST_AP\n");

    RS232::set_timeout(5000);
    string data = RS232::read_string();
    RS232::reset_timeout();

    vector<string> lines;
    lines = split(data, '\n');

    vector<AP_List_Entry> list;
    for (unsigned int i = 0; i < lines.size(); i++)
    {
        string line = lines[i];
        if (line.size() < 3)
            continue;
        vector<string> tokens = split(line, ',');

        AP_List_Entry entry;
        entry.ssid = tokens[1];
        entry.channel = stoi(tokens[2]);
        entry.rssi = stoi(tokens[3]);
        entry.ecn = (Wifi_Security)stoi(tokens[4]);

        list.push_back(entry);
    }

    return list;
}

void ESP8266_Driver::connect_to_AP(const std::string &ssid, const std::string &password)
{
    string s = "AT+CONNECT_TO_AP=\"" + ssid + "\",\"" + password + "\"\n";
    RS232::write(io_port, s);
    wait_for_ok(10000);
}

void ESP8266_Driver::init_tls()
{
    RS232::write(io_port, "AT+INIT_TLS\n");
    wait_for_ok(15000);
}

void ESP8266_Driver::start_tls_connection_to_ip(const std::string &ip, unsigned int port)
{
    char *x_str = new char[6];
    x_str = itoa(port, x_str, 10); // base 10
    string s = "AT+TLS_START_IP=\"" + ip + "\",\"" + string(x_str) + "\"\n";
    RS232::write(io_port, s);
}

void ESP8266_Driver::start_tls_connection_to_hostname(const std::string &hostname, unsigned int port)
{
    char *x_str = new char[6];
    x_str = itoa(port, x_str, 10); // base 10
    string s = "AT+TLS_START_HOSTNAME=\"" + hostname + "\",\"" + string(x_str) + "\"\n";
    RS232::write(io_port, s);
}

void ESP8266_Driver::send_string(const std::string &str)
{
    char *x_str = new char[5];
    x_str = itoa(str.length(), x_str, 10); // base 10
    string s = "AT+SEND=" + string(x_str) + "\n";
    RS232::write(io_port, s);
    RS232::write(io_port, str);
}

bool ESP8266_Driver::data_available()
{
    return RS232::is_available();
}

string ESP8266_Driver::read_string()
{
    return RS232::read_string();
}

void ESP8266_Driver::close_tls_connection()
{
    RS232::write(io_port, "AT+TLS_CLOSE\n");
    wait_for_ok(5000);
}

bool ESP8266_Driver::is_connected_to_AP()
{
    RS232::write(io_port, "AT+IS_CONNECTED_TO_AP\n");
    return RS232::read_string() == "YES\n";
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