#pragma once

#include "requests.h"
#include"ThreadPool.h"
#include <sqltypes.h>
#include <sql.h>
#include <sqlext.h>
#include <list>
#include <map>

#pragma comment (lib, "Ws2_32.lib")

class server
{
public :

	server(ThreadPool*);

	server(const server&) = delete;
	server(server&&) = delete;

	~server();

	server& operator=(const server&) = delete;
	server& operator=(server&&) = delete;

	void Start();
	void Stop();

private :

	ThreadPool * tp;

	bool stop = false;

	SQLHANDLE sqlConnHandle = NULL;
	SQLHANDLE sqlEnvHandle = NULL;

	DWORD  flags = 0; //flags always 0

	std::mutex list_mtx, query_mtx; //list_mtx - mutex for connected_users list, query_mtx - mutex for making query requests

	HANDLE com_port = 0;

	SOCKET server_socket = 0;

	struct sock_info {
		OVERLAPPED Overlapped;
		WSABUF DataBuf;
		char Buffer[2048]{};
	};

	struct user {
		Student student;
		SOCKET sckt;
		sock_info sock_data;
		std::mutex buffer_mtx;
	};

	std::list<user*> connected_users;
	std::map<std::string,std::string> chats;

	void ConnectionAccepter(); //Accepts clients
	void DataHandler(); // Handles the data from clients
	void FindAndErase(user*); //FInds and erases user from connected users list
	void DataBaseConnect(); //Create the connection with database
	void DataBaseDissconnect(); // Destroys connection with database
	void HandleMessage(user*, sock_info*, std::vector<char>  ); //Handles text message
	void HandleConnectionRequest(user*,sock_info*,std::string); //Handles connection request
	void SendFile(std::string,std::string,std::string, unsigned long long,user*); //Sends file
	void NotifyAll(user* ,const std::string, std::vector<char>); //notifies all users about the new message
	void NotifyAll(user*, const unsigned int,const std::string, const std::string, const std::string, std::vector<char>); //notifies all users about the new file
	void RecvFile(user*, std::vector<char>); //receives the file
	void CheckFileName(user*,std::string&); //checks if file name is available ,if not returns available
	
	bool Connected(SOCKET); //checks if user is connected

	char* Append(char*,const char *, const unsigned long long); //appends char array with char array
	wchar_t* Append(wchar_t*, const wchar_t*, const unsigned long long); //appends wchar array with whcar array

	unsigned int LengthOfName (const std::string); //returns amount of words appeared in string
	unsigned int GetFirstNumber(std::vector<char>&); //Retrievs a vector and returns first number in it (erases this vector)

	std::string GetFileName(std::vector<char>&,const unsigned int); //Retrievs a vector and returns file name from it (erases this vector)
	std::string GetFileType(std::vector<char>&); //Retrievs a vector and returns file type from it (erases this vector)
	
};

