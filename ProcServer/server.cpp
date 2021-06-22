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
        nuser->sock_data.DataBuf.len = 1024;

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

    DWORD BytesTransferred=0;

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
                continue;
            }

            else
            {

                std::string command = sock_data->Buffer; //copy buffer to the string for convenient work


                if (command.substr(0, 7) == "connect") {

					if (Connected(usr->sckt))
						continue;

                    memset(sock_data->Buffer, 0, 2048); //clear buffer

                    HandleConnectionRequest(usr, sock_data, std::move(command));

                }
                else if (command.substr(0, 3) == "msg") {

                    std::vector<char> message(&sock_data->Buffer[4], &sock_data->Buffer[BytesTransferred]);

                    memset(sock_data->Buffer, 0, 2048); //clear buffer

                    BytesTransferred = 0;

                    HandleMessage(usr, sock_data, std::move(message));

                }
                else if (command.substr(0, 4) == "file") {

                    std::vector<char> message(&sock_data->Buffer[5], &sock_data->Buffer[BytesTransferred]);

                    memset(sock_data->Buffer, 0, 2048); //clear buffer

                    BytesTransferred = 0;

                    RecvFile(usr, std::move(message)); //recv file

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
        (SQLWCHAR*)L"DRIVER={SQL Server};SERVER=DESKTOP-4863CP9;DATABASE=master;UID=;PWD=;",
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

        fs::create_directory("FILES"); //create directory for file storage

        //declare output variable and pointer
        SQLCHAR sqlVersion[SQL_RESULT_LEN];
        SQLINTEGER ptrSqlVersion;
        while (SQLFetch(sqlStmtHandle) == 0) { //for every query result

            SQLGetData(sqlStmtHandle, 1, 1, sqlVersion, SQL_RESULT_LEN, &ptrSqlVersion);

            std::string chat_id = (char*)sqlVersion;

            SQLGetData(sqlStmtHandle, 2, 1, sqlVersion, SQL_RESULT_LEN, &ptrSqlVersion);

            std::string faculty = (char*)sqlVersion;

            std::string dirname("FILES/");
            dirname += (char*)sqlVersion;

            fs::create_directory(dirname.c_str()); //for every faculty create its own directory for files

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


void server::NotifyAll(user* usr,const std::string msg_time,std::vector<char> message) {

    std::string message_data("msg " + msg_time + " " + usr->student.login + " " + std::to_string(usr->student.name.length()) + " " +  usr->student.name + " ");

	message_data = std::to_string(message.size() + message_data.size()) + " " + message_data;

    std::unique_ptr<char> up(new char[1]{});

    up.reset(Append(up.get(), message_data.c_str(), message_data.size()));
    up.reset(Append(up.get(), &message[0], message.size()));

    for (auto user : connected_users) {

        if (user->student.faculty == usr->student.faculty && user->sckt != usr->sckt) {
            std::unique_lock<std::mutex>ul(user->buffer_mtx);
            send(user->sckt, up.get(), message_data.size() + message.size(), 0);

        }

    }

}


void server::NotifyAll(user* usr, const unsigned int namelength, const std::string time, const std::string filename, const std::string filetype, std::vector<char> filedata) {

    std::string message_data("file " + time + " " + usr->student.login + " " + std::to_string(usr->student.name.length()) + " " + 
        usr->student.name + " " + std::to_string(namelength) + " " + filename + " " + filetype);

    for (auto  u : connected_users) {

        if (u->student.faculty == usr->student.faculty && u->sckt != usr->sckt) {

            std::unique_lock<std::mutex>ul(u->buffer_mtx);
            send(u->sckt, message_data.data(), message_data.size(), 0);

            std::string filepath("FILES/" + usr->student.faculty + "/" + filename + "." + filetype);

            unsigned long long filesize = fs::file_size(filepath);

            tp->AddTask(
                [this, usr, namelength, time, filename, filetype, filedata, filesize, filepath]() {
                    this->SendFile(filename, filetype, filepath, filesize, usr);
                }
            );

        }

    }

}


void server::RecvFile(user* usr, std::vector<char> file) {

    unsigned int namelength = GetFirstNumber(file);

    std::string filename = GetFileName(file, namelength);

    std::string filetype = GetFileType(file);

    CheckFileName(usr, filename,filetype);

    std::ofstream ofile("FILES/" + usr->student.faculty + "/" + filename + "." + filetype, std::ios::out | std::ios::binary);

    ofile.write(&file[0], file.size());

    if (file.size() < 1024) { //indicates that end of file was reached

        std::wstring chat_id = std::wstring(chats.find(usr->student.faculty)->second.begin(), chats.find(usr->student.faculty)->second.end()); //get chat id for the faculty

        SQLCHAR sqlVersion[1024];
        SQLINTEGER ptrSqlVersion;

        SQLHANDLE sqlAddFile = NULL, selectMsg = NULL; //query handle

        if (0 != SQLAllocHandle(3, sqlConnHandle, &sqlAddFile))
            DataBaseDissconnect();

        std::wstring insert_req(
            L"insert into messages (msg_type,msg_from,msg_chatid,msg_text) values ('" +
            std::wstring(filetype.begin(), filetype.end()) + L"','" +
            std::wstring(usr->student.login.begin(), usr->student.login.end()) + L"'," +
            chat_id + L",'" +
            std::wstring(filename.begin(), filename.end()) + L"')"
        );

        if (0 != SQLExecDirectW(sqlAddFile, insert_req.data(), insert_req.length())) {   //try to query

            DataBaseDissconnect();

        }

        std::wstring select_msg = L"select top 1 msg_time from messages order by msg_id desc";

        std::string msg_time;

        SQLFreeHandle(3, sqlAddFile);

        if (0 != SQLAllocHandle(3, sqlConnHandle, &selectMsg))
            DataBaseDissconnect();

        if (0 != SQLExecDirectW(selectMsg, (SQLWCHAR*)select_msg.data(), select_msg.length())) {   //try to query

            DataBaseDissconnect();

        }
        else
        {
            SQLFetch(selectMsg);

            SQLGetData(selectMsg, 1, 1, sqlVersion, 1024, &ptrSqlVersion);

            msg_time = (char*)sqlVersion;

            NotifyAll(usr, namelength, msg_time, filename, filetype, std::move(file));

        }

    }

    ofile.close();

}


void server::HandleMessage(user* usr, sock_info* sock_data, std::vector<char> message) {

    if (Connected(usr->sckt)) {

        SQLCHAR sqlVersion[1025];
        SQLINTEGER ptrSqlVersion;

        DWORD RecvBytes;

        SQLHANDLE addMsg = NULL,selectMsg = NULL; //query handle

        if (0 != SQLAllocHandle(3, sqlConnHandle, &addMsg))
           DataBaseDissconnect();

        std::wstring chat_id = std::wstring(chats.find(usr->student.faculty)->second.begin(), chats.find(usr->student.faculty)->second.end()); //get chat id for the faculty


        std::wstring insert_msg = L"insert into messages(msg_from,msg_chatid,msg_type,msg_text) values('"
            + std::wstring(usr->student.login.begin(), usr->student.login.end())     //user login ex. 'w12345'
            + L"'," + chat_id                                                       //chat_id
            +L",'%text'"
            + L",'";
        
        std::unique_ptr<wchar_t> up(new wchar_t[1]{});

        up.reset(Append(up.get(), insert_msg.c_str(), insert_msg.length())); 
        up.reset(Append(up.get(), std::wstring(message.begin(),message.end()).c_str(), message.size()));//message text from 4th element in array(after 'msg ') and next messagelength bytes
        up.reset(Append(up.get(),L"')",3));

        std::unique_lock<std::mutex> ul(query_mtx);

        unsigned long long query_size = insert_msg.length() + message.size() + 2;

        if (0 != SQLExecDirectW(addMsg, (SQLWCHAR*)up.get(), query_size)) {   //try to query

           DataBaseDissconnect();

        }

        std::wstring select_msg = L"select top 1 msg_time from messages order by msg_id desc";

        std::string msg_time;

        SQLFreeHandle(3, addMsg);

        if (0 != SQLAllocHandle(3, sqlConnHandle, &selectMsg))
            DataBaseDissconnect();

        if (0 != SQLExecDirectW(selectMsg, (SQLWCHAR*)select_msg.data(), select_msg.length())) {   //try to query

            DataBaseDissconnect();

        }
        else
        {
            SQLFetch(selectMsg);

            SQLGetData(selectMsg, 1, 1, sqlVersion, 1024, &ptrSqlVersion);

            msg_time = (char*)sqlVersion;


        }

        ul.unlock();

        SQLFreeHandle(3, selectMsg);

        NotifyAll(usr,std::move(msg_time),std::move(message));

        if (WSARecv(usr->sckt, &(sock_data->DataBuf), 1, &RecvBytes, &flags, &(sock_data->Overlapped), NULL) == SOCKET_ERROR) //start retrieving new data in overlapped mode
        {
            if (WSAGetLastError() != ERROR_IO_PENDING)
            {
                printf("WSARecv() failed with error %d\n", WSAGetLastError());
                return;
            }
        }

    }
}


void server::SendFile(std::string filename,std::string filetype,std::string filepath,unsigned long long filesize, user* usr) {

    std::ifstream file(filepath.data(), std::ios::in | std::ios::binary);

    char buffer[1024];

    std::string send_data("file " + std::to_string(filename.length()) + " " + filename + " " + filetype + " ");

    while (!file.eof()) {

        std::unique_ptr<char> up(new char[1]{});

        file.read(buffer, 1024);

        up.reset(Append(up.get(), send_data.c_str(), send_data.size()));
        up.reset(Append(up.get(), buffer, file.gcount()));

        std::unique_lock<std::mutex>ul(usr->buffer_mtx);
        send(usr->sckt, up.get(), send_data.size() + file.gcount(), 0);

        memset(buffer, 0, 1024);

    }

    std::unique_lock<std::mutex>ul(usr->buffer_mtx);
    send(usr->sckt, send_data.data(), send_data.size(), 0);

    file.close();

}

std::string toUtf8(const std::wstring& str);


void server::HandleConnectionRequest(user* usr, sock_info* sock_data, std::string command) {

    DWORD BytesTransferred, RecvBytes, Flags, SendBytes = 10;

    std::istringstream sstream(command);

    sstream >> usr->student.login >> usr->student.login; //double >> for skipping 'connect' word

    sstream >> usr->student.password;

    std::mutex request_mtx;
    {
        std::unique_lock<std::mutex>ul(request_mtx);
        login lgn(&usr->student); //try to get user data
        lgn.GetPersonData(); 
    }


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

		server_OK = std::to_string(server_OK.length()) + " " + server_OK;

        send(usr->sckt, server_OK.data(), server_OK.length(), 0);

        SQLHANDLE sqlFacultyCheck = NULL; //query handle

        if (0 != SQLAllocHandle(3, sqlConnHandle, &sqlFacultyCheck))
            DataBaseDissconnect();

        std::wstring faculty_check = L"select count(*) from chats where chats.faculty = '" + std::wstring(usr->student.faculty.begin(), usr->student.faculty.end()) + L"'";

        if (0 != SQLExecDirectW(sqlFacultyCheck, (SQLWCHAR*)faculty_check.data(), -3)) { //try to query

            DataBaseDissconnect();

        }
        else { //if query is OK

            //declare output variable and pointer
            SQLCHAR sqlVersion[1024];
            SQLINTEGER ptrSqlVersion;

            while (SQLFetch(sqlFacultyCheck) == 0) { //for every query result

                SQLGetData(sqlFacultyCheck, 1, 1, sqlVersion, 1024, &ptrSqlVersion);

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

                    std::unique_lock<std::mutex> ul(query_mtx);
                    if (0 != SQLExecDirectW(sqlStmtHandle, (SQLWCHAR*)request.data(), -3)) { //try to query

                        DataBaseDissconnect();
                        SQLFreeHandle(3, sqlStmtHandle);
                        ul.unlock();

                    }
                    else { //if query is OK

                        ul.unlock();

                        //declare output variable and pointer
                        memset(sqlVersion, 0, 1024);
                        ptrSqlVersion = 0;

                        while (SQLFetch(sqlStmtHandle) == 0) { //for every query result

                            SQLGetData(sqlStmtHandle, 1, 1, sqlVersion, 1024, &ptrSqlVersion); //1025 is the size of sqlVersion buffer

                            if (std::strcmp((char*)sqlVersion, "%text") == 0) {

                                std::string message_data("SERVERSYNC ");

                                message_data += (char*)sqlVersion; //message type
                                message_data += " ";

                                SQLGetData(sqlStmtHandle, 2, 1, sqlVersion, 1024, &ptrSqlVersion); //message  time

                                message_data += (char*)sqlVersion;
                                message_data += " ";

                                SQLGetData(sqlStmtHandle, 3, 1, sqlVersion, 1024, &ptrSqlVersion); //sender login

                                message_data += (char*)sqlVersion;
                                message_data += " ";

                                SQLGetData(sqlStmtHandle, 4, 1, sqlVersion, 1024, &ptrSqlVersion); //sender fullname

                                message_data+= std::to_string(ptrSqlVersion);//fullname amount of chars
                                message_data += " ";
                                message_data += (char*)sqlVersion;
                                message_data += " ";

                                SQLGetData(sqlStmtHandle, 5, 1, sqlVersion, 1024, &ptrSqlVersion); //message text

                                std::unique_ptr<char> up(new char[1]{});

                                message_data += std::to_string(ptrSqlVersion); //message length without '\0'
                                message_data += " ";

								message_data = std::to_string(message_data.size() + ptrSqlVersion) + " " + message_data;

                                up.reset(Append(up.get(), message_data.c_str(), message_data.size()));
                                up.reset(Append(up.get(), (char*)sqlVersion, ptrSqlVersion + 1));

                                std::unique_lock<std::mutex>ul(usr->buffer_mtx);

								std::this_thread::sleep_for(std::chrono::milliseconds(50));

                                send(usr->sckt, up.get(), message_data.size() + ptrSqlVersion, 0);

                                memset(sqlVersion, 0, 1024);

                            }
                            else {

                                std::string message_data("SERVERSYNC "),
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

                                std::string file_path("FILES/" + usr->student.faculty + "/" + file_name + "." + file_type); //file name

                                const unsigned long long file_size = fs::file_size(file_path);
                                message_data += " " + std::to_string(file_size);

                                send(usr->sckt, message_data.data(), message_data.size(), 0); //send notification

                                tp->AddTask( //Start file sending in NEW thread
                                    [this, file_name, file_type,file_path,file_size, usr]() {
                                        this->SendFile(file_name, file_type, file_path, file_size, usr);
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
            }
        }

    }
    else { //if user doesnt exist

        closesocket(usr->sckt);
        delete usr;

    }

}


char* server::Append(char* dst, const char* src, const unsigned long long srclen) { 

    char* b = new char[srclen + std::strlen(dst) + 1]{};
    
    memcpy(b, dst, std::strlen(dst));
    memcpy(&b[std::strlen(dst)], src, srclen);

    return b;

}


wchar_t* server::Append(wchar_t* dst, const wchar_t* src, const unsigned long long srclen) {

    wchar_t* b = new wchar_t[srclen*2 + std::wcslen(dst) + 1]{}; //multiply size because wchar is 2 times bigger than char

    memcpy(b, dst, std::wcslen(dst)*2);
    memcpy(&b[std::wcslen(dst)], src, srclen*2);

    return b;

}


unsigned int server::LengthOfName(const std::string name) {

    unsigned int counter(1);

    for (auto c : name) {

        if (c == ' ') {

            ++counter;

        }

    }

    return counter;

}


unsigned int server::GetFirstNumber(std::vector<char>& vec) {

    std::string number;


    for (auto c = vec.begin(); c != vec.end(); c = vec.begin()) {

        if (*c == ' ') {

            vec.erase(vec.begin());

            break;

        }

        number += *c;

        vec.erase(vec.begin());

    }


    return std::stoi(number);

}


std::string server::GetFileName(std::vector<char>& vec, const unsigned int namelength) {

    std::string filename(vec.begin(), vec.begin() + namelength);

    vec.erase(vec.begin(), vec.begin() + namelength + 1);

    return filename;

}


std::string server::GetFileType(std::vector<char>& vec) {

    std::string filetype;

    for (auto c = vec.begin(); c != vec.end();  c = vec.begin()) {

        if (*c == ' ') {

            vec.erase(vec.begin());

            break;

        }

        filetype += *c;

        vec.erase(vec.begin());

    }

    return filetype;

}


void server::CheckFileName(user * usr,std::string& filename,std::string filetype) {

    unsigned long long counter = 0;

    std::string scounter;

    for (;; counter++) {

        std::string filepath = "FILES/" + usr->student.faculty + "/" + filename + scounter + "." + filetype;

        if (!fs::exists(filepath)) {
            filename += scounter;
            break;
        }

        scounter = "(" + std::to_string(counter) + ")";

    }

}