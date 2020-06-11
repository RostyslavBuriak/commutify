#include "server.h"
#include <filesystem>
#include <fstream>

#define SQL_RESULT_LEN 240
#define SQL_RETURN_CODE_LEN 1000

namespace fs = std::filesystem;


void server::Stop() {
    stop = true;
}


server::server(ThreadPool* _tp) :tp(_tp) {}


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

    SOCKET client;

    sockaddr_in client_addr;
    int client_len = sizeof(client_addr);

    while (!stop)
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

        if (!GetQueuedCompletionStatus(com_port, &BytesTransferred,
            (LPDWORD)&usr, (LPOVERLAPPED*)&sock_data, 5000)) //wait 5 second for event and return false if no event occured
        {
            if (stop) //if no event occured check if we should stop the thread otherwise wait some more
                break;
        }
        else
        {
            if (!BytesTransferred) { //user disconnected

                FindAndErase(usr); //delete user from connected users list
                closesocket(usr->sckt); //close user socket
                delete usr; //clear all user data

            }

            else
            {

                std::string command = sock_data->Buffer; //copy buffer to the string for convenient work

                memset(sock_data->Buffer, 0, buff_size); //clear buffer

                if (command.substr(0, 7) == "connect") {

                    HandleConnectionRequest(usr, sock_data, std::move(command));

                }
                else if (command.substr(0, 3) == "msg") {

                    HandleMessage(usr, sock_data, std::move(command));

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

    DataBaseConnect();

    fs::create_directory("FILES"); //create directory for file storage

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
    
    tp->AddTask([this]() {this->ConnectionAccepter(); }); //start acception handler in thread

    for (unsigned int i = 1; i < (tp->ThreadsNumber()/2); ++i) //7 is the threads count in the thread pool
        tp->AddTask([this]() {this->DataHandler(); }); //start data handler in thread

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
        (SQLWCHAR*)L"DRIVER={SQL Server};SERVER=DESKTOP-BGNLAUK;DATABASE=master;UID=;PWD=;",
        -3,
        retconstring,
        1024,
        NULL,
        0);

    SQLHANDLE sqlStmtHandle = NULL; //query handle

    if (0 != SQLAllocHandle(3, sqlConnHandle, &sqlStmtHandle))
        DataBaseDissconnect();

    if (0 != SQLExecDirectW(sqlStmtHandle, (SQLWCHAR*)L"select * from chats",-3)) { //try to query

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
        closesocket(x->sckt);
        delete x;
    }

}


void server::HandleFile() {



}


void server::HandleMessage(user* usr, sock_info* sock_data, std::string command) {

    DWORD BytesTransferred, RecvBytes, Flags, SendBytes = 10;

    if (Connected(usr->sckt)) {
        SQLHANDLE addMsg = NULL; //query handle

        if (0 != SQLAllocHandle(3, sqlConnHandle, &addMsg))
            DataBaseDissconnect();

        command = command.substr(4, std::string::npos);

        std::wstring chat_id = std::wstring(chats.find(usr->student.faculty)->second.begin(), chats.find(usr->student.faculty)->second.end()); //get chat id for the faculty


        std::wstring insert_msg = L"insert into messages(msg_from,chat_id,msg_type,msg_text) values('"
            + std::wstring(usr->student.login.begin(), usr->student.login.end()) //user login ex. 'w12345'
            + L"'," + chat_id //chat_id
            + L",'" + std::wstring(command.begin(), command.end()) + L"')"; //message text

        query_mtx.lock();

        if (0 != SQLExecDirectW(addMsg, (SQLWCHAR*)insert_msg.data(), -3)) { //try to query

            DataBaseDissconnect();

        }

        query_mtx.unlock();


        if (WSARecv(usr->sckt, &(sock_data->DataBuf), 1, &RecvBytes, &flags, &(sock_data->Overlapped), NULL) == SOCKET_ERROR) //start retrieving new data in overlapped mode
        {
            if (WSAGetLastError() != ERROR_IO_PENDING)
            {
                printf("WSARecv() failed with error %d\n", WSAGetLastError());
                return;
            }
        }

        SQLFreeHandle(3, addMsg);
    }

}


void server::SendFile(std::string filename,std::string filetype,std::string filepath, user* usr) {

    std::ifstream file(filepath.data(), std::ios::in | std::ios::binary);

    char buffer[1025]{};

    std::string send_data(filename + "." + filetype +" ");

    while (!file.eof()) {

        file.read(buffer, 1024);

        send_data += buffer;

        send(usr->sckt, buffer, 1024, 0);

        send_data = filename + "." + filetype + " ";
        memset(buffer, 0, 1024);

    }

}


void server::HandleConnectionRequest(user* usr, sock_info* sock_data, std::string command) {

    std::wstring str(std::wstring(usr->student.login.begin(), usr->student.login.end()));

    DWORD BytesTransferred, RecvBytes, Flags, SendBytes = 10;

    std::istringstream sstream(command);

    sstream >> usr->student.login >> usr->student.login; //double >> for skipping 'connect' word

    sstream >> usr->student.password;

    login lgn(&usr->student); //try to get user data
    lgn.GetPersonData();

    if (!usr->student.semester.empty()) { //if user exists

        std::wstring _login(usr->student.login.begin(), usr->student.login.end()),
            fullname(usr->student.name.begin(), usr->student.name.end()),
            department(usr->student.department.begin(), usr->student.department.end()),
            faculty(usr->student.faculty.begin(), usr->student.faculty.end()),
            specialization(usr->student.specialization.begin(), usr->student.specialization.end()),
            semester(usr->student.semester.begin(), usr->student.semester.end());

        {
            std::lock_guard<std::mutex>lg(list_mtx); //critical section
            connected_users.push_back(usr); // add user to connected users list
        }

        SQLHANDLE sqlAddUser = NULL; //query handle

        if (0 != SQLAllocHandle(3, sqlConnHandle, &sqlAddUser))
            DataBaseDissconnect();

        std::wstring adduser = L"if not exists(select * from students where login = '"
            + _login
            + L"')"
            + L" insert into students(login,fullname,department,faculty,specialization,semester) "
            + L"values('"
            + _login
            + L"','"
            + fullname
            + L"','"
            + department
            + L"','"
            + faculty
            + L"','"
            + specialization
            + L"',"
            + semester
            + L")";

        if (0 != SQLExecDirectW(sqlAddUser, (SQLWCHAR*)adduser.data(), -3)) { //try to query

            DataBaseDissconnect();

        }

        SQLFreeHandle(3, sqlAddUser); //release query handle


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

        std::wstring faculty_check = L"select count(*) from chats where chats.faculty = '" + std::wstring(usr->student.faculty.begin(), usr->student.faculty.end()) + L"'";

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

                if (count == "0") { //if there is no faculty

                    SQLHANDLE sqlFacultyInsert = NULL; //query handle

                    if (0 != SQLAllocHandle(3, sqlConnHandle, &sqlFacultyInsert))
                        DataBaseDissconnect();

                    faculty_check = L"insert into chats (faculty) values('" + std::wstring(usr->student.faculty.begin(), usr->student.faculty.end()) + L"')";

                    if (0 != SQLExecDirectW(sqlFacultyInsert, (SQLWCHAR*)faculty_check.data(), -3)) { //try to query

                        DataBaseDissconnect();

                    }
                    else
                    {
                        chats.insert(std::make_pair(usr->student.faculty, std::to_string(chats.size() + 1))); //add newly created chat and its id to our list
                    }

                    SQLFreeHandle(3, sqlFacultyInsert); //release query handle

                }

                else
                {
                    //NOW WE HAVE TO SEND ALL NEEDED MESSAGES TO CLIENT, CLIENT SENDS 'command login password meesage_id' , we have to send all NEW messages to client

                    std::string last_mess; //last message id which client has

                    sstream >> last_mess;

                    std::wstring request(L"select m.msg_type,m.msg_time,u.login,u.fullname, m.msg_text from messages m "
                         L"inner join chats c on m.msg_chatid = c.chat_id "
                         L"inner join students u on m.msg_from = u.login "
                         L"where c.faculty = '" //create query 
                        + faculty
                        + L"' and m.msg_id > " + std::wstring(last_mess.begin(), last_mess.end()));

                    SQLHANDLE sqlStmtHandle = NULL; //query handle

                    if (0 != SQLAllocHandle(3, sqlConnHandle, &sqlStmtHandle))
                        DataBaseDissconnect();

                    query_mtx.lock();

                    if (0 != SQLExecDirectW(sqlStmtHandle, (SQLWCHAR*)request.data(), -3)) { //try to query

                        DataBaseDissconnect();
                        SQLFreeHandle(3, sqlStmtHandle);

                    }
                    else { //if query is OK

                        query_mtx.unlock();

                        //declare output variable and pointer
                        memset(sqlVersion, 0, SQL_RESULT_LEN);
                        ptrSqlVersion = 0;

                        std::future<int> ft;

                        while (SQLFetch(sqlStmtHandle) == 0) { //for every query result

                            SQLGetData(sqlStmtHandle, 1, 1, sqlVersion, SQL_RESULT_LEN, &ptrSqlVersion);

                            if (std::strcmp((char*)sqlVersion, "text") == 0) {

                                std::string message_data("SERVER SYNCH "); 

                                message_data += (char*)sqlVersion; //message type
                                message_data += " ";

                                SQLGetData(sqlStmtHandle, 2, 1, sqlVersion, SQL_RESULT_LEN, &ptrSqlVersion); //message  time

                                message_data += (char*)sqlVersion;
                                message_data += " ";

                                SQLGetData(sqlStmtHandle, 3, 1, sqlVersion, SQL_RESULT_LEN, &ptrSqlVersion); //sender login

                                message_data += (char*)sqlVersion;
                                message_data += " ";

                                SQLGetData(sqlStmtHandle, 4, 1, sqlVersion, SQL_RESULT_LEN, &ptrSqlVersion); //sender fullname

                                message_data += (char*)sqlVersion;
                                message_data += " ";

                                SQLGetData(sqlStmtHandle, 5, 1, sqlVersion, SQL_RESULT_LEN, &ptrSqlVersion); //message text

                                message_data += (char*)sqlVersion;

                                if (send(usr->sckt, message_data.data(), message_data.size(), 0) == SOCKET_ERROR) { //send messages which client needs in BLOCKING CALL

                                    continue;

                                }
                            }

                            else {

                                std::string message_data("SERVER SYNCH "),
                                    file_type((char*)sqlVersion),
                                    file_name;

                                message_data += (char*)sqlVersion; //message type
                                message_data += " ";

                                SQLGetData(sqlStmtHandle, 2, 1, sqlVersion, SQL_RESULT_LEN, &ptrSqlVersion); //message  time

                                message_data += (char*)sqlVersion;
                                message_data += " ";

                                SQLGetData(sqlStmtHandle, 3, 1, sqlVersion, SQL_RESULT_LEN, &ptrSqlVersion); //sender login

                                message_data += (char*)sqlVersion;
                                message_data += " ";

                                SQLGetData(sqlStmtHandle, 4, 1, sqlVersion, SQL_RESULT_LEN, &ptrSqlVersion); //sender fullname

                                message_data += (char*)sqlVersion;
                                message_data += " ";

                                SQLGetData(sqlStmtHandle, 5, 1, sqlVersion, SQL_RESULT_LEN, &ptrSqlVersion); //file name



                                message_data += (char*)sqlVersion;
                                file_name = (char*)sqlVersion;

                                std::string file_path("FILES/" + file_name + "." + file_type); //file name

                                long long file_size = fs::file_size(file_path);
                                message_data += " " + std::to_string(file_size);

                                send(usr->sckt, message_data.data(), message_data.size(), 0); //send notification

                                tp->AddTask( //Start file sending in NEW thread
                                    [this, file_name, file_type,file_path, usr]() {
                                        this->SendFile(file_name, file_type,file_path, usr);
                                    }
                                );

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
        delete usr;

    }

}