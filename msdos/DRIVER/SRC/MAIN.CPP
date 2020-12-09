#include <stdio>
#include <dos.h>
#include <iostream>
#include <string>

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
	cout << "Reset ESP8266." << endl;

	cout << "Getting device info..." << endl;
	esp8266.retrieve_device_info();
	if (esp8266.is_emulated_esp8266())
	{
		cout << "The device is an emulated device." << endl;
	}
	else
	{
		cout << "The device is a real esp8266 device." << endl;

		vector<AP_List_Entry> APs = esp8266.get_available_APs();
		for (unsigned int i = 0; i < APs.size(); i++)
		{
			cout << "(" << i + 1 << ") " << APs[i].ssid.c_str() << " ";
			cout << ((APs[i].ecn == OPEN) ? "(open)" : "(encrypted)") << endl;
		}

		if (APs.size() == 0)
		{
			cout << "Could not find APs. Quitting application." << endl;
			return 1;
		}

		unsigned int number;
		do
		{
			cout << "Enter AP number to connect (1-" << APs.size() << "): " << flush;
			cin >> number;

			if (std::cin.fail())
			{
				std::cin.clear();
				std::cin.ignore();
			}
		} while (number < 1 || number > APs.size());

		AP_List_Entry access_point = APs[number - 1];

		string passphrase;
		if (access_point.ecn != OPEN)
		{
			cout << "Enter passphrase: " << flush;
			{
				char passphrase_cstring[64];
				cin >> passphrase_cstring;
				passphrase = passphrase_cstring;
			}
		}

		cout << "Connecting to " << access_point.ssid.c_str() << " ..." << endl;
		bool connected_to_AP = esp8266.connect_to_AP(access_point.ssid, passphrase);
		if (!connected_to_AP)
		{
			cout << "Could not connect to AP." << endl;
			return 1;
		}
		cout << "Connected to WIFI." << endl;
	}

	cout << "Init TLS ..." << endl;
	bool initialized_tls = esp8266.init_tls();
	if (!initialized_tls)
	{
		cout << "Could not initialize TLS." << endl;
		return 1;
	}
	cout << "Initialized TLS." << endl;

	string server_ip;
	cout << "Enter server IP: " << flush;
	{
		char server_ip_cstring[64];
		cin >> server_ip_cstring;
		server_ip = server_ip_cstring;
	}

	unsigned int server_port;
	cout << "Enter server port: " << flush;
	cin >> server_port;

	cout << "Connecting to " << server_ip.c_str() << ":" << server_port << " ..." << endl;
	bool connected = esp8266.start_tls_connection(server_ip, server_port);
	if (!connected)
	{
		cout << "Could not connect to server." << endl;
		return 1;
	}

	string nickname;
	cout << "Enter a nickname: " << flush;
	{
		char nickname_cstring[64];
		cin >> nickname_cstring;
		nickname = nickname_cstring;
	}

	string welcome_msg = "Hello, my name is ";
	welcome_msg += nickname;
	welcome_msg += "!";

	esp8266.send_string(welcome_msg);

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