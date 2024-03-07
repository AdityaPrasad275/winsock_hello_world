#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

using std::cout;
using std::cin;
using std::string;

constexpr auto newLine = "\n";
constexpr auto DEFAULT_PORT = "27015";

SOCKET createAndBindSocket();
void startListening(SOCKET ServerSocket);
std::string giveIPV4address(sockaddr* addr);
struct addrinfo* getServerInfo();
void chat(SOCKET ClientSocket);

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
		cout << "WSAStartup successful" << newLine;
		cout << "The status: " << wsaData.szSystemStatus << newLine;
	}

	SOCKET ServerSocket = createAndBindSocket();

	if (ServerSocket == 1)
	{
		cout << "Error creating and binding socket" << newLine;
		return 1;
	}

	//listen for incoming connections on created socket
	startListening(ServerSocket);

	// Accept a client socket, this is where the code will block until a client connects
	SOCKET ClientSocket = INVALID_SOCKET;
	ClientSocket = accept(ServerSocket, NULL, NULL);

	if (ClientSocket == INVALID_SOCKET)
	{
		cout << "accept failed: " << WSAGetLastError() << newLine;
		closesocket(ServerSocket);
		WSACleanup();
		return 1;
	}
	
	cout << "Client connected successfully!\n";

	chat(ClientSocket);

	// cleanup
	closesocket(ClientSocket);
	closesocket(ServerSocket);
	WSACleanup();
	return 0;
}

void chat(SOCKET ClientSocket)
{
	constexpr auto DEFAULT_BUFLEN = 512;

	char receiveBuffer[DEFAULT_BUFLEN] = { ' ' };
	int iResult = 0, iSendResult = 0;

	//receive inital test buffer
	iResult = recv(ClientSocket, receiveBuffer, DEFAULT_BUFLEN, 0);
	if (iResult > 0)
	{
		cout << "Test Message received: " << string(receiveBuffer) << newLine;
	}
	else
	{
		cout << "receive failed: " << WSAGetLastError() << newLine;
		closesocket(ClientSocket);
		WSACleanup();
		return;
	}

	//rest buffer
	ZeroMemory(receiveBuffer, DEFAULT_BUFLEN);

	do
	{
		int iResult = recv(ClientSocket, receiveBuffer, DEFAULT_BUFLEN, 0);
		if (iResult > 0)
		{
			cout << "Client: " << string(receiveBuffer) << newLine;

			string message;
			cout << "Server: ";
			std::getline(cin, message);
			

			//send message
			iSendResult = send(ClientSocket, message.c_str(), (int)message.length(), 0);
			if (iSendResult == SOCKET_ERROR)
			{
				cout << "send failed: " << WSAGetLastError() << newLine;
				closesocket(ClientSocket);
				WSACleanup();
				return;
			}
		}
		else
		{
			cout << "receive failed: " << WSAGetLastError() << newLine;
			closesocket(ClientSocket);
			WSACleanup();
			return;
		}

		ZeroMemory(receiveBuffer, DEFAULT_BUFLEN);
	} 
	while (iResult > 0);
}

SOCKET createAndBindSocket()
{
	auto result = getServerInfo();

	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol); // Create a socket

	if (ListenSocket == INVALID_SOCKET) 
	{
		cout << "Error at socket(): " << WSAGetLastError() << newLine;
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	else
	{
		cout << "Socket created successfully" << newLine;
		cout << "ipv4 address: " << giveIPV4address(result->ai_addr) << newLine;

		auto iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);

		if (iResult == SOCKET_ERROR) 
		{
			cout << "bind failed with error: " << WSAGetLastError() << newLine;
			freeaddrinfo(result);
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}
		else
		{
			cout << "Socket bound successfully\n";
		}
	}
	freeaddrinfo(result); // Free the memory allocated by getaddrinfo
	return ListenSocket;
}

void startListening(SOCKET ServerSocket)
{
	//listen for incoming connections on created socket
	if (listen(ServerSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		cout << "Listen failed with error: " << WSAGetLastError() << newLine;
		closesocket(ServerSocket);
		WSACleanup();
		return;
	}
	else
	{
		cout << "Listening for incoming connections\n";
	}
} 

// make a function giveIPV4address that takes a sockaddr pointer and returns a string containing the ipv4 address
std::string giveIPV4address(sockaddr* addr)
{
	char ipstr[INET_ADDRSTRLEN];
	struct sockaddr_in* sa = (struct sockaddr_in*)addr;
	inet_ntop(AF_INET, &sa->sin_addr, ipstr, sizeof(ipstr));
	return ipstr;
}

struct addrinfo* getServerInfo()
{
	struct addrinfo* result = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_protocol = IPPROTO_TCP; // TCP
	hints.ai_flags = AI_PASSIVE; // Server

	// Resolve the local address and port to be used by the server using hints and store it in result
	auto iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		cout << "getaddrinfo failed: " << iResult << newLine;
		WSACleanup();
		return NULL;
	}
	return result;
}	
