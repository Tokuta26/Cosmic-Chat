#pragma once
#include <vector>
#include "User.h"
#include "Message.h"
#include <string>
#include <unordered_map>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

class Database
{
	vector<User> _users;
	vector<Message> _messages;
	unordered_map<string, int> _usersMapByName;
	int searchUserByName(string);
    QSqlDatabase _db;
public:
  vector<string> getUserList() const;
  string getUserName(int userId) const;
	Database();
	int addUser(string username, string password);
	int checkPassword(string username, string password);
	void addChatMessage(string sender, string);
	bool addPrivateMessage(string sender, string target, string message);
	vector<string> getChatMessages();//показать все сообщения
  vector<Message> getPrivateMessage(int userID = -1);//показать личные сообщения пользователю username
    // Создать сессию после успешного входа
    bool createSession(int userId, const QString &tokenHash);

    // Проверить сохранённую сессию
    bool restoreSession(const QString &tokenHash,
                        int &userId,
                        QString &userName);

    // Завершить сессию
    bool revokeSession(const QString &tokenHash);

};
