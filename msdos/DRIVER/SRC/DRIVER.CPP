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

bool is_ip(const std::string &s)
{
    vector<string> tokens = split(s, '.');
    if (tokens.size() != 4)
        return false;
    for (int i = 0; i < tokens.size(); i++)
    {
        int x;
        int sscanf_val = sscanf(s.c_str(), "%d", &x);
        if (sscanf_val != 1)
            return false;
    }
    return true;
}

ESP8266_Driver::ESP8266_Driver(unsigned char comport_num)
{
    io_port = RS232::PORT_BASE_IO[comport_num];
    intvect = RS232::INTVECT[comport_num];
    pic_irq = RS232::PIC_IRQ[comport_num];
    pic_irq_mask = RS232::PIC_IRQ_MASK[comport_num];
    RS232::setup(io_port, intvect, pic_irq, 115200);

    device_info = "";
}

bool ESP8266_Driver::is_emulated_esp8266()
{
    return ESP8266_Driver::retrieve_device_info() == "ESP8266_EMULATOR_FOR_DOSBOX\r\n";
}

bool ESP8266_Driver::test()
{
    RS232::write(io_port, "AT\n");
    return RS232::read_string() == "OK\r\n";
}

std::string ESP8266_Driver::retrieve_device_info()
{
    if (device_info == NULL || device_info == "")
    {
        RS232::write(io_port, "AT+INFO\n");
        device_info = RS232::read_string();
    }
    return device_info; // contains \r\n (!)
}

void ESP8266_Driver::reset()
{
    RS232::write(io_port, "AT+RESET\n");
    wait_for_ok(1000);
}

void ESP8266_Driver::delay(unsigned int milliseconds)
{
    clock_t time_start = clock();
    while (true)
    {
        if (clock() >= time_start + milliseconds)
            return;
    }
}

vector<AP_List_Entry> ESP8266_Driver::get_available_APs()
{
    RS232::write(io_port, "AT+LIST_AP\n");

    string data = ESP8266_Driver::read_string(5000);

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

bool ESP8266_Driver::connect_to_AP(const std::string &ssid, const std::string &password)
{
    string s = "AT+CONNECT_TO_AP=\"" + ssid + "\",\"" + password + "\"\n";
    RS232::write(io_port, s);
    bool success = wait_for_ok(10000);
    ESP8266_Driver::delay(5000); // wait 5s
    return success && ESP8266_Driver::is_connected_to_AP();
}

bool ESP8266_Driver::init_tls()
{
    RS232::write(io_port, "AT+INIT_TLS\n");
    return wait_for_ok(25000); // wait 25s for OK
}

bool ESP8266_Driver::start_tls_connection(const std::string &hostname, unsigned int port)
{
    char *x_str = new char[6];
    x_str = itoa(port, x_str, 10); // base 10

    string s;

    if (is_ip(hostname))
    {
        s = "AT+TLS_START_IP=\"" + hostname + "\",\"" + string(x_str) + "\"\n";
    }
    else
    {
        s = "AT+TLS_START_HOSTNAME=\"" + hostname + "\",\"" + string(x_str) + "\"\n";
    }

    RS232::write(io_port, s);

    string data = ESP8266_Driver::read_string(15000);

    return (strncmp(data.c_str(), "CONNECTED", 9) == 0);
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

std::string ESP8266_Driver::read_string()
{
    return RS232::read_string();
}

std::string ESP8266_Driver::read_string(unsigned int timeout_ms)
{
    string s = "";

    clock_t time_start = clock();
    while (true)
    {
        if (RS232::is_available())
        {
            s = RS232::read_string();
            return s;
        }
        if (clock() >= time_start + timeout_ms)
            return s;
    }
    return s;
}

void ESP8266_Driver::close_tls_connection()
{
    RS232::write(io_port, "AT+TLS_CLOSE\n");
    wait_for_ok(5000);
}

bool ESP8266_Driver::is_connected_to_AP()
{
    RS232::write(io_port, "AT+IS_CONNECTED_TO_AP\n");
    return RS232::read_string() == "YES\r\n";
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

std::string ESP8266_Driver::get_connected_AP()
{
    RS232::write(io_port, "AT+CONNECTED_AP\n");
    return ESP8266_Driver::read_string();
}