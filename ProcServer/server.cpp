#include "server.h"

#define SQL_RESULT_LEN 240
#define SQL_RETURN_CODE_LEN 1000


void server::Stop() {
    stop = true;
}


bool server::Connected(SOCKET sckt) {
    std::lock_guard<std::mutex> lg(list_mtx); //critical section

    for (auto it = connected_users.begin(); it != connected_users.end(); ++it) {

        if ((*it)->sckt == sckt) {
            return true;
        }

    }
    return false;
}


void server::ConnectionAccepter() {

    WSADATA wsaDataRcp;
    WSAStartup(MAKEWORD(2, 2), &wsaDataRcp);

    std::cout  << "worked\n";

    SOCKET client;

    sockaddr_in client_addr;
    int client_len = sizeof(client_addr);

    while (TRUE)
    {
        if ((client = WSAAccept(server_socket, NULL, NULL, NULL, 0)) == SOCKET_ERROR)
        {
            printf("WSAAccept() failed with error %d\n", WSAGetLastError());
            return;
        }

        user* nuser = new user{};
        nuser->sckt = client;

        if (CreateIoCompletionPort((HANDLE)client, com_port, (DWORD)nuser, 0) == NULL)
        {
            printf("CreateIoCompletionPort() failed with error %d\n", GetLastError());
            return ;
        }

        nuser->sock_data.DataBuf.buf = nuser->sock_data.Buffer;
        nuser->sock_data.DataBuf.len = buff_size;

        DWORD RBytes;

        if (WSARecv(client, &(nuser->sock_data.DataBuf), 1, &RBytes, &flags, &(nuser->sock_data.Overlapped), NULL) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != ERROR_IO_PENDING)
            {
                printf("WSARecv() failed with error %d\n", WSAGetLastError());
            }
        }

    }
}



void server::DataHandler() {
    std::cout << "Started handling data\n";

    user * usr = nullptr;
    sock_info * sock_data = nullptr;

    DWORD BytesTransferred, RecvBytes, Flags, SendBytes = 10;

    while (true) {

            if (GetQueuedCompletionStatus(com_port, &BytesTransferred,
                (LPDWORD)&usr, (LPOVERLAPPED*)&sock_data, INFINITE) == 0)
            {
                printf("GetQueuedCompletionStatus() failed with error %d\n", GetLastError());

                if (WSAGetLastError() != 64) //error code 64 indicates that client has disconnected
                    return;
            }

        if (!BytesTransferred) { //user disconnected

            FindAndErase(usr); //delete user from connected users list
            closesocket(usr->sckt); //close user socket
            delete usr; //clear all user data

        }

        else
        {

            std::string command = sock_data->Buffer; //copy buffer to the string for convenient work

            memset(sock_data->Buffer, 0, buff_size); //clear buffer

            if (command.substr(0,7) == "connect") {

                std::istringstream sstream(command);

                sstream >> usr->student.login >> usr->student.login; //double >> for skipping 'connect' word

                sstream >> usr->student.password;

                login lgn(&usr->student); //try to get user data
                lgn.GetPersonData();

                if (!usr->student.semester.empty()) { //if user exists

                    {
                        std::lock_guard<std::mutex>lg(list_mtx); //critical section
                        connected_users.push_back(usr); // add user to connected users list
                    }

                    std::string server_OK = "SERVER OK " + usr->student.name //create SERVER OK response with all user data
                        + "%" + usr->student.faculty 
                        + "%" + usr->student.department 
                        + "%" + usr->student.semester 
                        + "%" + usr->student.specialization;

                    WSABUF server_ok{ server_OK.size(), &server_OK[0] }; //server answer if everything is OK

                    if (WSASend(usr->sckt, &server_ok, 1, &SendBytes, 0, &sock_data->Overlapped, NULL) == SOCKET_ERROR) { //send response that everything is OK

                        if (WSAGetLastError() != ERROR_IO_PENDING)
                        {

                            printf("WSASend() failed with error %d\n", WSAGetLastError());
                            return;

                        }

                    }

                    SQLHANDLE sqlFacultyCheck = NULL; //query handle

                    if (0 != SQLAllocHandle(3, sqlConnHandle, &sqlFacultyCheck))
                        DataBaseDissconnect();

                    std::wstring faculty_check = L"select count(*) from chat where chat.faculty = '" + std::wstring(usr->student.faculty.begin(), usr->student.faculty.end()) + L"'";

                    if (0 != SQLExecDirectW(sqlFacultyCheck, (SQLWCHAR*)faculty_check.data(), -3)) { //try to query

                        DataBaseDissconnect();

                    }
                    else { //if query is OK

                        //declare output variable and pointer
                        SQLCHAR sqlVersion[SQL_RESULT_LEN];
                        SQLINTEGER ptrSqlVersion;
                        while (SQLFetch(sqlFacultyCheck) == 0) { //for every query result

                            SQLGetData(sqlFacultyCheck, 1, 1, sqlVersion, SQL_RESULT_LEN, &ptrSqlVersion);

                            std::string count = (char*)sqlVersion;

                            SQLFreeHandle(3, sqlFacultyCheck); //release query handle

                            if (count == "0") { //if threr is no faculty

                                SQLHANDLE sqlFacultyInsert = NULL; //query handle

                                if (0 != SQLAllocHandle(3, sqlConnHandle, &sqlFacultyInsert))
                                    DataBaseDissconnect();

                                faculty_check = L"insert into chat (faculty) values('" + std::wstring(usr->student.faculty.begin(), usr->student.faculty.end()) + L"')";

                                if (0 != SQLExecDirectW(sqlFacultyInsert, (SQLWCHAR*)faculty_check.data(), -3)) { //try to query

                                    DataBaseDissconnect();

                                }

                                SQLFreeHandle(3, sqlFacultyInsert); //release query handle

                            }

                            else
                            {
                                //NOW WE HAVE TO SEND ALL NEEDED MESSAGES TO CLIENT, CLIENT SENDS 'command login password meesage_id' , we have to send all NEW messages to client

                                std::string last_mess; //last message id which client has

                                sstream >> last_mess;

                                std::wstring request = L"select * from messages m inner join chat c on m.chat_id = c.chat_id where c.faculty = '" //create query 
                                    + std::wstring(usr->student.faculty.begin(), usr->student.faculty.end())
                                    + L"' and m.message_id > " + std::wstring(last_mess.begin(), last_mess.end())
                                    + L" and m.msg_from <> '" + std::wstring(usr->student.login.begin(), usr->student.login.end()) + L"'";

                                SQLHANDLE sqlStmtHandle = NULL; //query handle

                                if (0 != SQLAllocHandle(3, sqlConnHandle, &sqlStmtHandle))
                                    DataBaseDissconnect();

                                if (0 != SQLExecDirectW(sqlStmtHandle, (SQLWCHAR*)request.data(), -3)) { //try to query

                                    DataBaseDissconnect();
                                    SQLFreeHandle(3, sqlStmtHandle);

                                }
                                else { //if query is OK

                                    //declare output variable and pointer
                                    memset(sqlVersion, 0, SQL_RESULT_LEN);
                                    ptrSqlVersion = 0;
                                    while (SQLFetch(sqlStmtHandle) == 0) { //for every query result

                                        SQLGetData(sqlStmtHandle, 2, 1, sqlVersion, SQL_RESULT_LEN, &ptrSqlVersion);

                                        std::string message_data((char*)sqlVersion); //take sender name (first response column)

                                        message_data += " ";

                                        SQLGetData(sqlStmtHandle, 4, 1, sqlVersion, SQL_RESULT_LEN, &ptrSqlVersion);

                                        message_data += (char*)sqlVersion;

                                        server_ok.buf = &message_data[0];
                                        server_ok.len = message_data.size();

                                        if (WSASend(usr->sckt, &server_ok, 1, &SendBytes, 0, &sock_data->Overlapped, NULL) == SOCKET_ERROR) { //send messages which client needs

                                            if (WSAGetLastError() != ERROR_IO_PENDING)
                                            {

                                                printf("WSASend() failed with error %d\n", WSAGetLastError());
                                                return;

                                            }

                                        }
                                    }

                                    SQLFreeHandle(3, sqlStmtHandle); //release query handle

                                }
                            }
                        }

                    }

                    

                    RecvBytes = 0;

                    if (WSARecv(usr->sckt, &(sock_data->DataBuf), 1, &RecvBytes, &flags, &(sock_data->Overlapped), NULL) == SOCKET_ERROR) //start retrieving new data in overlapped mode
                    {
                        if (WSAGetLastError() != ERROR_IO_PENDING)
                        {
                            printf("WSARecv() failed with error %d\n", WSAGetLastError());
                            return;
                        }
                    }

                }
                else { //if user doesnt exist

                    closesocket(usr->sckt);
                    delete sock_data; //clear all user data
                    delete usr;

                }
                int a = 5;

            }
            else if (command.substr(0, 3) == "msg") {

                if (Connected(usr->sckt)) {
                    SQLHANDLE addMsg = NULL; //query handle

                    if (0 != SQLAllocHandle(3, sqlConnHandle, &addMsg))
                        DataBaseDissconnect();

                    command = command.substr(4, std::string::npos);

                    std::wstring chat_id = std::wstring(chats.find(usr->student.faculty)->second.begin(), chats.find(usr->student.faculty)->second.end()); //get chat id for the faculty

                    std::wstring insert_msg = L"insert into messages(msg_from,chat_id,msg_text) values('"
                        + std::wstring(usr->student.login.begin(), usr->student.login.end()) //user login ex. 'w12345'
                        + L"'," + chat_id //chat_id
                        + L",'" + std::wstring(command.begin(), command.end()) + L"')"; //message text

                    if (0 != SQLExecDirectW(addMsg, (SQLWCHAR*)insert_msg.data(), -3)) { //try to query

                        DataBaseDissconnect();

                    }
                    SQLFreeHandle(3, addMsg);
                }

            }

        }

    }
}


void server::FindAndErase(user* usr) {

    std::lock_guard<std::mutex> lg(list_mtx); //critical section

    for (auto it = connected_users.begin(); it != connected_users.end(); ++it) {

        if (*it == usr) {
            connected_users.erase(it);

            return;
        }

    }
}



void server::Start() {

    ThreadPool pool(8); //thread pool (2*systemthreads) , threads are used for better performance

    DataBaseConnect();

    WSADATA wsaDataRcp;
    WSAStartup(MAKEWORD(2, 2), &wsaDataRcp);

    com_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    server_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

    sockaddr_in server_addr;
    server_addr.sin_port = htons(9009);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_family = AF_INET;

    if (bind(server_socket, (PSOCKADDR)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("bind() failed with error %d\n", WSAGetLastError());
        return;
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("listen() failed with error %d\n", WSAGetLastError());
        return ;
    }
    
    pool.AddTask([this]() {this->ConnectionAccepter(); }); //start acception handler in thread

    for (int i = 0; i < 7; ++i) //7 is the threads count in the thread pool
        pool.AddTask([this]() {this->DataHandler(); }); //start data handler in thread

    while(!stop){} //while funtion Stop() is not called the server will be working

}



void server::DataBaseDissconnect() {

    SQLDisconnect(sqlConnHandle);
    SQLFreeHandle(2, sqlConnHandle);
    SQLFreeHandle(1, sqlEnvHandle);

}


void server::DataBaseConnect() {

    SQLWCHAR retconstring[SQL_RETURN_CODE_LEN];

    if (0 != SQLAllocHandle(1, 0L, &sqlEnvHandle))
        DataBaseDissconnect();
    if (0 != SQLSetEnvAttr(sqlEnvHandle, 200, (SQLPOINTER)3UL, 0))
        DataBaseDissconnect();
    if (0 != SQLAllocHandle(2, sqlEnvHandle, &sqlConnHandle))
        DataBaseDissconnect();
    
    SQLDriverConnectW(sqlConnHandle,
        NULL,
        (SQLWCHAR*)L"DRIVER={SQL Server};SERVER=DESKTOP-572EJVV;DATABASE=master;UID=;PWD=;",
        -3,
        retconstring,
        1024,
        NULL,
        0);

    SQLHANDLE sqlStmtHandle = NULL; //query handle

    if (0 != SQLAllocHandle(3, sqlConnHandle, &sqlStmtHandle))
        DataBaseDissconnect();

    if (0 != SQLExecDirectW(sqlStmtHandle, (SQLWCHAR*)L"select * from chat",-3)) { //try to query

        DataBaseDissconnect();

    }
    else { //if query is OK

        //declare output variable and pointer
        SQLCHAR sqlVersion[SQL_RESULT_LEN];
        SQLINTEGER ptrSqlVersion;
        while (SQLFetch(sqlStmtHandle) == 0) { //for every query result

            SQLGetData(sqlStmtHandle, 1, 1, sqlVersion, SQL_RESULT_LEN, &ptrSqlVersion);

            std::string chat_id = (char*)sqlVersion;

            SQLGetData(sqlStmtHandle, 2, 1, sqlVersion, SQL_RESULT_LEN, &ptrSqlVersion);

            std::string faculty = (char*)sqlVersion;

            chats.insert(std::make_pair(faculty,chat_id));
        }
    }

    SQLFreeHandle(3, sqlStmtHandle); //release the query

}


server::~server() {

    DataBaseDissconnect();

    closesocket(server_socket);

    for (auto x : connected_users) {
        delete x;
    }

}