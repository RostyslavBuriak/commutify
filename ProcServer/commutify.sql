create table chats(
	chat_id int IDENTITY(1,1) NOT NULL PRIMARY KEY,
	faculty varchar(255) NOT NULL
)

create table students(
	login varchar(255) NOT NULL,
	fullname varchar(255) NOT NULL,
	department varchar(255) NOT NULL,
	faculty varchar(255) NOT NULL,
	specialization varchar(255) NOT NULL,
	semester varchar(255) NOT NULL
)

create table messages(
	msg_id int IDENTITY(1,1) NOT NULL PRIMARY KEY,
	msg_chatid int NOT NULL,
	msg_from varchar(255) NOT NULL,
	msg_type varchar(255) NOT NULL,
	msg_text nchar (2048) NOT NULL,
	msg_time datetime DEFAULT(getdate())
)
