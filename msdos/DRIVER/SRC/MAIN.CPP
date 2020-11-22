#include <stdio>
#include <dos.h>
#include <iostream>

#include "driver.h"

using namespace std;

int main()
{
	ESP8266_Driver esp8266(0x3F8);

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
	esp8266.start_tls_connection_to_ip("192.168.0.23", 8000);
	esp8266.send_string("PING");

	while (true)
	{
		if (esp8266.data_available())
		{
			cout << esp8266.read_string().c_str() << endl;
		}
	}

	return 0;
}