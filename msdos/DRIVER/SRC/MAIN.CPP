#include <stdio>
#include <dos.h>
#include <iostream>

#include "driver.h"

using namespace std;

unsigned char read_scancode()
{
	unsigned char res;
	_asm {
        in al, 60h
        mov res, al
        in al, 61h
        or al, 128
        out 61h, al
        xor al, 128
        out 61h, al
	}
	return res;
}

int main()
{
	ESP8266_Driver esp8266;

	if (!esp8266.test())
	{
		cerr << "ESP8266 does not respond. Maybe the baud rate is messed up or the serial port IO address is wrong" << endl;
		return 1;
	}
	else
	{
		cout << "ESP8266 responded." << endl;
	}

	esp8266.reset();
	esp8266.get_available_APs();
	esp8266.connect_to_AP("WLAN", "123456789!");
	esp8266.init_tls();
	esp8266.start_tls_connection_to_ip("192.168.0.23", 8000);
	esp8266.send_string("PING");

	while (true)
	{
		if (esp8266.data_available())
		{
			cout << esp8266.read_string().c_str();
		}

		if (read_scancode() == 0x81) // ESC keyup
			break;
	}

	return 0;
}