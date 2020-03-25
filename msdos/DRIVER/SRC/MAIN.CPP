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

	//esp8266.reset();
	//esp8266.get_available_APs();

	return 0;
}