// TS_Serwer_UDP.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
//

#include "pch.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <cstdio>
#include <cstdlib>
#include <winsock2.h>
#include <string>
#include <bitset>
#include <iostream>
#include <ctime>
#include <vector>
#include <algorithm> 
#include <functional>

#pragma comment(lib, "Ws2_32.lib")

// Zmienne globalne
std::bitset<2> poleOperacji;
std::bitset<16> poleLiczb_1;
std::bitset<16> poleLiczb_2;
std::bitset<3> poleStatusu;
std::bitset<8> poleID;
std::bitset<3> poleInstrukcji;
std::bitset<1> ACK;
std::bitset<7> poleDopelnienia;
char pakiet[7];


// Funkcje
int bit_to_int(const std::string &s) {
	int liczba = 0;
	for (int i = s.size() - 1, p = 1; i >= 0; i--, p *= 2)
	{
		if (s[i] == '1')
			liczba += p;
	}
	return liczba;
}

std::bitset<16> int_to_bit(int liczba) {
	std::bitset<16> bity;
	int i = 0;
	while (liczba > 0)
	{
		if (liczba == 1)
		{
			bity.set(i, true);
			break;
		}
		if (!(liczba % 2))
		{
			bity.set(i, false);
			liczba /= 2;
		}
		else
		{
			bity.set(i, true);
			liczba = liczba / 2;
		}
		i++;
	}
	return bity;
}

void czysc_pakiet()
{
	poleOperacji.reset();
	poleLiczb_1.reset();
	poleLiczb_2.reset();
	poleStatusu.reset();
	poleInstrukcji.reset();
	ACK.reset(); 
}

void zapakuj_pakiet() {
	std::string temp;

	temp = poleOperacji.to_string();
	temp += poleLiczb_1.to_string();
	temp += poleLiczb_2.to_string();
	temp += poleStatusu.to_string();
	temp += poleID.to_string();
	temp += poleInstrukcji.to_string();
	temp += ACK.to_string();
	temp += poleDopelnienia.to_string();

	std::string bajt_1 = temp.substr(0, 8);
	std::string bajt_2 = temp.substr(8, 8);
	std::string bajt_3 = temp.substr(16, 8);
	std::string bajt_4 = temp.substr(24, 8);
	std::string bajt_5 = temp.substr(32, 8);
	std::string bajt_6 = temp.substr(40, 8);
	std::string bajt_7 = temp.substr(48, 8);

	pakiet[0] = bit_to_int(bajt_1);
	pakiet[1] = bit_to_int(bajt_2);
	pakiet[2] = bit_to_int(bajt_3);
	pakiet[3] = bit_to_int(bajt_4);
	pakiet[4] = bit_to_int(bajt_5);
	pakiet[5] = bit_to_int(bajt_6);
	pakiet[6] = bit_to_int(bajt_7); 
}

void odpakuj_pakiet() {
	std::string temp;
	std::bitset<8> bajt;

	for (int i = 0; i < 7; i++)
	{
		bajt = pakiet[i];
		temp += bajt.to_string();
	}

	poleOperacji = bit_to_int(temp.substr(0, 2));
	poleLiczb_1 = bit_to_int(temp.substr(2, 16));
	poleLiczb_2 = bit_to_int(temp.substr(18, 16));
	poleStatusu = bit_to_int(temp.substr(34, 3)); 
	poleID = bit_to_int(temp.substr(37, 8));
	poleInstrukcji = bit_to_int(temp.substr(45, 3));
	ACK = bit_to_int(temp.substr(48, 1));
	poleDopelnienia = bit_to_int(temp.substr(49, 7));

	//std::cout << "SERVER: UNPACKAGING FINISHED" << std::endl;
}

void pokaz_pakiet() {
	std::cout << "========== PAKIET ==========" << std::endl;
	std::cout << "Pole operacji: " << poleOperacji << std::endl;
	std::cout << "Pole liczby 1: " << poleLiczb_1 << std::endl;
	std::cout << "Pole liczby 2: " << poleLiczb_2 << std::endl;
	std::cout << "Pole numeru ID: " << poleID << std::endl;
	std::cout << "Pole statusu: " << poleStatusu << std::endl;
	std::cout << "Pole instrukcji: " << poleInstrukcji << std::endl;
	std::cout << "Pole ACK: " << ACK << std::endl;
	std::cout << "Pole dopelnienia: " << poleDopelnienia << std::endl;
	std::cout << "============================" << std::endl;
}

int wylosuj(int a, int b) { 
	return ((std::rand() % (b-a)) + (a+1));
}

bool rand_bit()
{
	return ((std::rand() % 2) + 0);
}


void blad(SOCKET &mainSocket, sockaddr_in &addr) {
	poleInstrukcji.reset();
	poleStatusu.set(0, 1);
	poleStatusu.set(1, 1);
	poleStatusu.set(2, 0);
	poleLiczb_1.reset();
	poleLiczb_2.reset();
	zapakuj_pakiet();
	sendto(mainSocket, pakiet, sizeof(pakiet), 0, (SOCKADDR *)& addr, sizeof(struct sockaddr));
	czysc_pakiet();
}

/*****************************************************/
int main()
{
	srand(time(NULL));
	WSADATA wsaData;
	std::vector<int> wektor_liczb;

	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != NO_ERROR)
		printf("Initialization error.\n");

	// Utworzenie deskryptora gniazda - socket(AF_INET - rodzina adresow IPv4, typ gniazda, protokol)
	SOCKET mainSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (mainSocket == INVALID_SOCKET)
	{
		printf("Error creating socket: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Struktura sockaddr_in - reprezentuje adres IP
	sockaddr_in my_addr;
	memset(&my_addr, 0, sizeof(my_addr)); // Wyzeruj reszte struktury
	my_addr.sin_family = AF_INET; // host byte order
	my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	my_addr.sin_port = htons(27016); // short, network byte order

	sockaddr_in their_addr;
	memset(&their_addr, 0, sizeof(their_addr)); // Wyzeruj reszte struktury	their_addr.sin_family = AF_INET; // host byte order
	their_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	their_addr.sin_port = htons(27016); // short, network byte order

	int addr_len = sizeof(struct sockaddr);

	// Przypisanie gniazda do adresu
	if (bind(mainSocket, (SOCKADDR *)& my_addr, sizeof(my_addr)) == SOCKET_ERROR)
	{
		printf("bind() failed.\n");
		closesocket(mainSocket);
		return 1;
	}
	printf("Oczekiwanie na polaczenie z klientem...\n");

	//Serwer otrzymuje od klienta pusty pakiet z polem statusu = 100 - chec nawiazania polaczenia
	recvfrom(mainSocket, pakiet, sizeof(pakiet), 0, (SOCKADDR *)& their_addr, &addr_len);
	std::cout << "Odebrano pakiet" << std::endl;
	odpakuj_pakiet();
	pokaz_pakiet();
	std::cout << "Rozpakowano pakiet" << std::endl;
	if (ACK == 0)
	{
		poleStatusu = poleInstrukcji;
		ACK.set(0, 1);
		zapakuj_pakiet();
		sendto(mainSocket, pakiet, sizeof(pakiet), 0, (SOCKADDR *)& their_addr, sizeof(struct sockaddr)); // wyslanie potwierdzenia otrzymania prosby
		std::cout << "Wyslano potwierdzenie ACK" << std::endl;
	}

	if ((bit_to_int(poleInstrukcji.to_string())) == 4) // przydzielenie losowego nr ID
	{
		for (int i = 0; i < 8; i++)
		{
			//poleID.set(i, ((std::rand() % 2) + 0));
			poleID.set(i, rand_bit());
		}

	}
	std::cout << "Przydzielono ID: " << poleID << std::endl;
	poleStatusu = poleInstrukcji;
	poleInstrukcji.reset();
	ACK.reset();
	zapakuj_pakiet();
	//wyslanie do klienta nr ID + ACK
	sendto(mainSocket, pakiet, sizeof(pakiet), 0, (SOCKADDR *)& their_addr, sizeof(struct sockaddr));
	std::cout << "Wyslano pakiet z przydzielonym ID" << std::endl;

	while (1) {
		czysc_pakiet();
		recvfrom(mainSocket, pakiet, sizeof(pakiet), 0, (SOCKADDR *)& their_addr, &addr_len);
		odpakuj_pakiet();
		if (ACK == 0)
		{
			std::cout << "Odebrano pakiet" << std::endl;
			pokaz_pakiet();
			ACK.set(0, 1);
			poleStatusu.set(2, 1);
			zapakuj_pakiet();
			sendto(mainSocket, pakiet, sizeof(pakiet), 0, (SOCKADDR *)& their_addr, sizeof(struct sockaddr));
			std::cout << "Wyslano potwierdzenie ACK" << std::endl;
			ACK.reset();
		}
		else std::cout << "Odebrano potwierdzenie ACK" << std::endl;
		if (bit_to_int(poleInstrukcji.to_string()) == 1) {

			switch (bit_to_int(poleOperacji.to_string()))
			{
			case 0: // random
			{
				if (bit_to_int(poleLiczb_1.to_string()) >= bit_to_int(poleLiczb_2.to_string()))
				{
					blad(mainSocket, their_addr);
					break;
				}
				else
				{
					int wylosowana = wylosuj(bit_to_int(poleLiczb_1.to_string()), bit_to_int(poleLiczb_2.to_string()));
					poleLiczb_2.reset();
					poleLiczb_1 = int_to_bit(wylosowana);
					poleStatusu = poleInstrukcji;
					poleInstrukcji.reset();
					ACK.reset();
					zapakuj_pakiet();
					sendto(mainSocket, pakiet, sizeof(pakiet), 0, (SOCKADDR *)& their_addr, sizeof(struct sockaddr));
					pokaz_pakiet();
					czysc_pakiet();
					break;
				}
			}
			case 1: //modulo
			{ 
				if (bit_to_int(poleLiczb_2.to_string()) == 0)
				{
					blad(mainSocket, their_addr);
				}
				else {
					int mod = bit_to_int(poleLiczb_1.to_string()) % bit_to_int(poleLiczb_2.to_string());
					poleLiczb_2.reset();
					poleLiczb_1 = int_to_bit(mod);
					poleStatusu = poleInstrukcji;
					poleInstrukcji.reset();
					ACK.reset();
					zapakuj_pakiet();
					sendto(mainSocket, pakiet, sizeof(pakiet), 0, (SOCKADDR *)& their_addr, sizeof(struct sockaddr));
					pokaz_pakiet();
					czysc_pakiet();
				}
				break;
			}
			case 2: //suma
			{
				int suma = bit_to_int(poleLiczb_1.to_string()) + bit_to_int(poleLiczb_2.to_string());
				if (suma > 65535)
				{
					blad(mainSocket, their_addr);
				}
				else
				{
					poleLiczb_2.reset();
					poleLiczb_1 = int_to_bit(suma);
					poleStatusu = poleInstrukcji;
					poleInstrukcji.reset();
					ACK.reset();
					zapakuj_pakiet();
					sendto(mainSocket, pakiet, sizeof(pakiet), 0, (SOCKADDR *)& their_addr, sizeof(struct sockaddr));
					czysc_pakiet();
				}

				break;
			}
			case 3: //roznica
			{
				int roznica = bit_to_int(poleLiczb_1.to_string()) - bit_to_int(poleLiczb_2.to_string());
				if (roznica < 0)
				{
					blad(mainSocket, their_addr);
				}
				else
				{
					poleLiczb_2.reset();
					poleLiczb_1 = int_to_bit(roznica);
					poleStatusu = poleInstrukcji;
					poleInstrukcji.reset();
					zapakuj_pakiet();
					ACK.reset();
					sendto(mainSocket, pakiet, sizeof(pakiet), 0, (SOCKADDR *)& their_addr, sizeof(struct sockaddr));
					czysc_pakiet();
				}

				break;
			}
			}
		} //sortowanie
		if (bit_to_int(poleInstrukcji.to_string()) == 2) { //malejaco
			int liczba = bit_to_int(poleLiczb_1.to_string());
			//std::cout << "vec size" << wektor_liczb.size() << std::endl;
			wektor_liczb.push_back(liczba);
			czysc_pakiet();
			//std::cout << poleOperacji << std::endl;
		}
		if (bit_to_int(poleInstrukcji.to_string()) == 3) //jest ostatnim pakietem
		{
			wektor_liczb.push_back(bit_to_int(poleLiczb_1.to_string()));
			//std::cout << "vec size" << wektor_liczb.size() << std::endl;
			czysc_pakiet();
			std::sort(wektor_liczb.begin(), wektor_liczb.end(), std::greater <int>());
			//std::cout << "vec size - po sort" << wektor_liczb.size() << std::endl;
			for (auto &e : wektor_liczb)
			{
				//std::cout << e << " ";
				poleLiczb_1 = int_to_bit(e);
				poleStatusu.set(0, 0);
				poleStatusu.set(1, 0);
				poleStatusu.set(2, 0);
				poleInstrukcji.reset();
				zapakuj_pakiet();
				sendto(mainSocket, pakiet, sizeof(pakiet), 0, (SOCKADDR *)& their_addr, sizeof(struct sockaddr));
			}
			wektor_liczb.clear();

		}
		if (bit_to_int(poleInstrukcji.to_string()) == 7) { //rosnaco
			int liczba = bit_to_int(poleLiczb_1.to_string());
			wektor_liczb.push_back(liczba);
			czysc_pakiet();
			//std::cout << poleOperacji << std::endl;
		}
		if (bit_to_int(poleInstrukcji.to_string()) == 6) //jest ostatnim pakietem
		{
			wektor_liczb.push_back(bit_to_int(poleLiczb_1.to_string()));
			std::cout << "vec size" << wektor_liczb.size() << std::endl;
			czysc_pakiet();
			std::sort(wektor_liczb.begin(), wektor_liczb.end());
			std::cout << "vec size - po sort" << wektor_liczb.size() << std::endl;
			for (auto &e : wektor_liczb)
			{
				//std::cout << e << " ";
				poleLiczb_1 = int_to_bit(e);
				poleStatusu.set(0, 0);
				poleStatusu.set(1, 0);
				poleStatusu.set(2, 0);
				poleInstrukcji.reset();
				zapakuj_pakiet();
				sendto(mainSocket, pakiet, sizeof(pakiet), 0, (SOCKADDR *)& their_addr, sizeof(struct sockaddr));
			}
			wektor_liczb.clear();
		}
		if (bit_to_int(poleInstrukcji.to_string()) == 5)
		{
			std::cout << "Zakonczenie polaczenia" << std::endl;
			closesocket(mainSocket);
			WSACleanup();
			system("pause");
			return 0;
			break;
		}
	}

}