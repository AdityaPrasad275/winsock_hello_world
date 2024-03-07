#include<winsock2.h>
#include<ws2tcpip.h>
#include<iostream>
#include<string>

#pragma comment(lib, "Ws2_32.lib")

using std::cout;
using std::cin;
using std::string;
constexpr auto newLine = '\n';
constexpr auto port = "27015";
constexpr auto serverName = "localhost";

SOCKET createAndConnectSocket();
struct addrinfo* getServerInfo();
void chat(SOCKET activeSocket);

int main()
{
	// Initialize Winsock
	WSADATA wsaData; // Structure to hold details about the Windows Socket implementation
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	// Check for errors
	if (iResult != 0)
	{
		cout << "WSAStartup failed: " << iResult << newLine;
		return 1;
	}
	else
	{
		cout << "WSAStartup successful\n";
		cout << "The status: " << wsaData.szSystemStatus << newLine;
	}

	auto clientSocket = createAndConnectSocket();

	chat(clientSocket);

	// cleanup
	closesocket(clientSocket);
	WSACleanup();

	return 0;
}

void chat(SOCKET activeSocket)
{
	constexpr auto DEFAULT_BUFLEN = 512;

	char receiveBuffer[DEFAULT_BUFLEN] = { ' ' };
	int iResult = 0, iSendResult = 0;
	string sendBuffer = "this is a test";

	// Send an initial test buffer
	iSendResult = send(activeSocket, sendBuffer.c_str(), (int)sendBuffer.size(), 0);

	if (iSendResult == SOCKET_ERROR)
	{
		cout << "send failed: " << WSAGetLastError() << newLine;
		closesocket(activeSocket);
		WSACleanup();
		return;
	}
	cout << "Test Message sent: " << sendBuffer << newLine;

	//reset the buffer
	ZeroMemory(receiveBuffer, DEFAULT_BUFLEN);

	int n = 10;
	while (n--)
	{
		string message;
		cout << "Client: ";
		std::getline(cin, message);
		

		iSendResult = send(activeSocket, message.c_str(), (int)message.size(), 0);
		if (iSendResult == SOCKET_ERROR)
		{
			cout << "send failed: " << WSAGetLastError() << newLine;
			closesocket(activeSocket);
			WSACleanup();
			return;
		}

		//wait for response
		iResult = recv(activeSocket, receiveBuffer, DEFAULT_BUFLEN, 0);
		if (iResult > 0)
		{
			cout << "Server: " << string(receiveBuffer) << newLine;
		}
		else
		{
			cout << "recv failed: " << WSAGetLastError() << newLine;
		}

		//reset buffer
		ZeroMemory(receiveBuffer, DEFAULT_BUFLEN);
	}

	// shutdown the connection for sending since no more data will be sent
	iResult = shutdown(activeSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		cout << "shutdown failed: " << WSAGetLastError() << newLine;
		closesocket(activeSocket);
		WSACleanup();
		return;
	}
	return;
}

SOCKET createAndConnectSocket()
{
	SOCKET ConnectSocket = INVALID_SOCKET;

	//use getaddrinfo to get the address of the server and create socket
	auto addressInfo = getServerInfo();

	if (addressInfo == NULL)
	{
		return 1;
	}
	//create a socket
	ConnectSocket = socket(addressInfo->ai_family, addressInfo->ai_socktype, addressInfo->ai_protocol);

	if (ConnectSocket == INVALID_SOCKET)
	{
		cout << "Error at createSocket(): " << WSAGetLastError() << newLine;
		WSACleanup();
		return 1;
	}
	
	cout << "Socket created!\n";

	// Attempt to connect to an address until one succeeds
	for (auto ptr = addressInfo; ptr != NULL; ptr = ptr->ai_next) 
	{
		// Create a SOCKET for connecting to the server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		if (ConnectSocket == INVALID_SOCKET) 
		{
			std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
			freeaddrinfo(addressInfo);
			WSACleanup();
			return 1;
		}

		// Connect to the server
		auto iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

		if (iResult == SOCKET_ERROR) 
		{
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue; // couldn't connect, lets go to other results of getaddrinfo
		}

		break;//connected, break loop
	}
	freeaddrinfo(addressInfo);

	if (ConnectSocket == INVALID_SOCKET) 
	{
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}
	cout << "Connected to server!\n";
	return ConnectSocket;
}

struct addrinfo* getServerInfo()
{
	struct addrinfo* result = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	int iResult = getaddrinfo(serverName, port, &hints, &result);
	if (iResult != 0)
	{
		cout << "getaddrinfo failed: " << iResult << newLine;
		WSACleanup();
		return NULL;
	}
	else
	{
		cout << "getaddrinfo successful" << newLine;
		return result;
	}
}
