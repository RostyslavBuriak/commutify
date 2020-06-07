#pragma once

#include "requests.h"
#include"ThreadPool.h"
#include <sqltypes.h>
#include <sql.h>
#include <sqlext.h>
#include <list>
#include <map>

#pragma comment (lib, "Ws2_32.lib")

#define buff_size 2048

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

	unsigned int threads = 4;

	SQLHANDLE sqlConnHandle = NULL;
	SQLHANDLE sqlEnvHandle = NULL;

	DWORD  flags = 0; //flags always 0

	std::mutex list_mtx, query_mtx; //list_mtx - mutex for connected_users list, query_mtx - mutex for making query requests

	HANDLE com_port = 0;

	SOCKET server_socket = 0;

	struct sock_info {
		OVERLAPPED Overlapped;
		WSABUF DataBuf;
		char Buffer[buff_size];
	};

	struct user {
		Student student;
		SOCKET sckt;
		sock_info sock_data;
	};

	std::list<user*> connected_users;
	std::map<std::string,std::string> chats;

	void ConnectionAccepter(); //Accepts clients
	void DataHandler(); // Handles the data from clients
	void FindAndErase(user*); //FInds and erases user from connected users list
	void DataBaseConnect(); //Create the connection with database
	void DataBaseDissconnect(); // Destroys connection with database
	
	bool Connected(SOCKET); //checks if user is connected
	
};

