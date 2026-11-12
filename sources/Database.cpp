#include "Database.h"
#include "Parsing.h"
#include "sha1.h"
#include <sstream>
#include <iomanip>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
//'Ανθρωπος

string hashToString(const Hash& hash)
{
    stringstream ss;

    for (int i = 0; i < SHA1HASHLENGTHUINTS; ++i)
    {
        ss << hex
           << setw(8)
           << setfill('0')
           << hash.c[i];
    }

    return ss.str();
}

int Database::searchUserByName(string username)
{
    if (!_db.isOpen()) {
        qDebug() << "User search: PostgreSQL not connected!";
        return -1;
    }

    QSqlQuery query(_db);

    query.prepare(
        "SELECT id FROM users WHERE username = :username"
        );

    query.bindValue(
        ":username",
        QString::fromStdString(username)
        );

    if (!query.exec()) {
        qDebug() << "User search error:"
                 << query.lastError().text();
        return -1;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return -1;
}



vector<string> Database::getUserList() const
{
    vector<string> userList;

    if (!_db.isOpen()) {
        qDebug() << "Cannot load users: PostgreSQL not connected!";
        return userList;
    }

    QSqlQuery query(_db);

    if (!query.exec(
            "SELECT username FROM users ORDER BY username"
            ))
    {
        qDebug() << "Users loading error:"
                 << query.lastError().text();

        return userList;
    }

    while (query.next())
    {
        userList.push_back(
            query.value(0).toString().toStdString()
            );
    }

    return userList;
}



string Database::getUserName(int userId) const
{
    if (!_db.isOpen()) {
        return "";
    }

    QSqlQuery query(_db);

    query.prepare(
        "SELECT username FROM users WHERE id = :id"
        );

    query.bindValue(":id", userId);

    if (!query.exec()) {
        qDebug() << "Username loading error:"
                 << query.lastError().text();
        return "";
    }

    if (query.next()) {
        return query.value(0).toString().toStdString();
    }

    return "";
}


Database::Database()
    : _users(),
    _messages(),
    _db(QSqlDatabase::database())
{
    if (_db.isOpen())
    {
        qDebug() << "Database class: PostgreSQL connected!";
    }
    else
    {
        qDebug() << "Database class: PostgreSQL NOT connected!";
    }
}

int Database::addUser(string username, string password)
{
    if (!correctName(username))
        return -1;

    if (!_db.isOpen()) {
        qDebug() << "PostgreSQL is not connected!";
        return -3;
    }

    Hash passHash = sha1(password);
    string hashString = hashToString(passHash);

    QSqlQuery query(_db);

    query.prepare(
        "INSERT INTO users (username, password_hash) "
        "VALUES (:username, :password_hash) "
        "ON CONFLICT (username) DO NOTHING "
        "RETURNING id"
        );

    query.bindValue(":username", QString::fromStdString(username));
    query.bindValue(":password_hash", QString::fromStdString(hashString));

    if (!query.exec()) {
        qDebug() << "Registration error:" << query.lastError().text();
        return -3;
    }

    if (!query.next()) {
        return -2;
    }

    int userId = query.value(0).toInt();

    User newUser(userId, username, passHash);

    _users.push_back(newUser);
    _usersMapByName[username] = userId;

    qDebug() << "User registered in PostgreSQL:"
             << QString::fromStdString(username);

    return userId;
}


int Database::checkPassword(string username, string password)
{
    if (!_db.isOpen()) {
        qDebug() << "Login error: PostgreSQL not connected!";
        return -1;
    }

    QSqlQuery query(_db);

    query.prepare(
        "SELECT id, password_hash, is_banned "
        "FROM users "
        "WHERE username = :username"
        );

    query.bindValue(
        ":username",
        QString::fromStdString(username)
        );

    if (!query.exec()) {
        qDebug() << "Login SQL error:"
                 << query.lastError().text();
        return -1;
    }

    if (!query.next()) {
        qDebug() << "User not found!";
        return -1;
    }

    int userId = query.value(0).toInt();
    QString savedHash = query.value(1).toString();
    bool isBanned = query.value(2).toBool();

    if (isBanned) {
        qDebug() << "User is banned!";
        return -2;
    }

    Hash passHash = sha1(password);

    QString enteredHash = QString::fromStdString(
        hashToString(passHash)
        );

    if (savedHash != enteredHash) {
        qDebug() << "Incorrect password!";
        return -1;
    }

    qDebug() << "Login successful!"
             << QString::fromStdString(username);

    return userId;
}


void Database::addChatMessage(string sender, string text)
{
    if (!_db.isOpen()) {
        qDebug() << "Message error: PostgreSQL not connected!";
        return;
    }

    int senderId = searchUserByName(sender);

    if (senderId < 0) {
        qDebug() << "Message error: Sender not found!";
        return;
    }

    QSqlQuery query(_db);

    query.prepare(
        "INSERT INTO messages "
        "(sender_id, receiver_id, content) "
        "VALUES (:sender_id, NULL, :content)"
        );

    query.bindValue(":sender_id", senderId);
    query.bindValue(
        ":content",
        QString::fromStdString(text)
        );

    if (!query.exec()) {
        qDebug() << "Message saving error:"
                 << query.lastError().text();
        return;
    }

    qDebug() << "Message saved to PostgreSQL!";
}



bool Database::addPrivateMessage(
    string sender,
    string target,
    string message)
{
    if (!_db.isOpen()) {
        qDebug() << "Private message: PostgreSQL not connected!";
        return false;
    }

    int senderId = searchUserByName(sender);
    int targetId = searchUserByName(target);

    if (senderId < 0 || targetId < 0) {
        qDebug() << "Private message: User not found!";
        return false;
    }

    QSqlQuery query(_db);

    query.prepare(
        "INSERT INTO messages "
        "(sender_id, receiver_id, content) "
        "VALUES (:sender_id, :receiver_id, :content)"
        );

    query.bindValue(":sender_id", senderId);
    query.bindValue(":receiver_id", targetId);
    query.bindValue(
        ":content",
        QString::fromStdString(message)
        );

    if (!query.exec()) {
        qDebug() << "Private message saving error:"
                 << query.lastError().text();
        return false;
    }

    qDebug() << "Private message saved!";
    return true;
}


vector<string> Database::getChatMessages()
{
    vector<string> messages;

    if (!_db.isOpen()) {
        qDebug() << "Cannot load messages: PostgreSQL not connected!";
        return messages;
    }

    QSqlQuery query(_db);

    if (!query.exec(
            "SELECT u.username, m.content, "
            "TO_CHAR(m.created_at AT TIME ZONE 'Asia/Almaty', 'HH24:MI') "
            "FROM messages m "
            "JOIN users u ON u.id = m.sender_id "
            "WHERE m.receiver_id IS NULL "
            "ORDER BY m.created_at ASC, m.id ASC"
            )) {
        qDebug() << "Messages loading error:"
                 << query.lastError().text();
        return messages;
    }

    while (query.next()) {
        string username =
            query.value(0).toString().toStdString();

        string text =
            query.value(1).toString().toStdString();

        string time =
            query.value(2).toString().toStdString();

        messages.push_back(
            "[" + time + "] <" + username + ">: " + text
            );
    }

    return messages;
}


vector<Message> Database::getPrivateMessage(int userID)
{
    vector<Message> messages;

    if (!_db.isOpen()) {
        qDebug() << "Private messages: PostgreSQL not connected!";
        return messages;
    }

    // Не разрешаем загружать все личные сообщения без указания пользователя.
    if (userID < 0) {
        qDebug() << "Private messages: Invalid user ID!";
        return messages;
    }

    QSqlQuery query(_db);

    query.prepare(
        "SELECT u.username, m.receiver_id, m.content, "
        "TO_CHAR(m.created_at AT TIME ZONE 'Asia/Almaty', 'HH24:MI') "
        "FROM messages m "
        "JOIN users u ON u.id = m.sender_id "
        "WHERE m.receiver_id IS NOT NULL "
        "AND (m.sender_id = :sender_id "
        "     OR m.receiver_id = :receiver_id) "
        "ORDER BY m.created_at ASC, m.id ASC"
        );

    query.bindValue(":sender_id", userID);
    query.bindValue(":receiver_id", userID);

    if (!query.exec()) {
        qDebug() << "Private messages loading error:"
                 << query.lastError().text();
        return messages;
    }

    while (query.next()) {
        string sender =
            query.value(0).toString().toStdString();

        int receiverId = query.value(1).toInt();

        string content =
            query.value(2).toString().toStdString();


        string time =
            query.value(3).toString().toStdString();

        Message msg(sender, receiverId, content);
        msg.setTime(time);

        messages.push_back(msg);

    }

    return messages;
}

bool Database::createSession(
    int userId,
    const QString &tokenHash)
{
    if (!_db.isOpen() ||
        userId <= 0 ||
        tokenHash.isEmpty())
    {
        return false;
    }

    QSqlQuery query(_db);

    query.prepare(
        "INSERT INTO user_sessions "
        "(user_id, token_hash, expires_at) "
        "SELECT id, :token_hash, "
        "NOW() + INTERVAL '30 days' "
        "FROM users "
        "WHERE id = :user_id "
        "AND is_banned = FALSE"
        );

    query.bindValue(":user_id", userId);
    query.bindValue(":token_hash", tokenHash);

    if (!query.exec())
    {
        qDebug() << "Session creation error:"
                 << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() == 1;
}


bool Database::restoreSession(
    const QString &tokenHash,
    int &userId,
    QString &userName)
{
    userId = -1;
    userName.clear();

    if (!_db.isOpen() || tokenHash.isEmpty())
    {
        return false;
    }

    QSqlQuery query(_db);

    query.prepare(
        "SELECT u.id, u.username "
        "FROM user_sessions s "
        "JOIN users u ON u.id = s.user_id "
        "WHERE s.token_hash = :token_hash "
        "AND s.expires_at > NOW() "
        "AND u.is_banned = FALSE"
        );

    query.bindValue(":token_hash", tokenHash);

    if (!query.exec())
    {
        qDebug() << "Session restoration error:"
                 << query.lastError().text();
        return false;
    }

    if (!query.next())
    {
        return false;
    }

    userId = query.value(0).toInt();
    userName = query.value(1).toString();

    return true;
}


bool Database::revokeSession(
    const QString &tokenHash)
{
    if (!_db.isOpen() || tokenHash.isEmpty())
    {
        return false;
    }

    QSqlQuery query(_db);

    query.prepare(
        "DELETE FROM user_sessions "
        "WHERE token_hash = :token_hash"
        );

    query.bindValue(":token_hash", tokenHash);

    if (!query.exec())
    {
        qDebug() << "Session revocation error:"
                 << query.lastError().text();
        return false;
    }

    return true;
}
