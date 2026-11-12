
#pragma once

#include <QString>
#include <QByteArray>
#include <QSettings>
#include <QRandomGenerator>
#include <QCryptographicHash>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define byte win_byte
#include <windows.h>
#include <wincrypt.h>
#undef byte

class SessionStorage
{
public:
    // Создаём случайный токен (256 бит)
    static QString generateToken()
    {
        QByteArray bytes;
        bytes.reserve(32);

        for (int i = 0; i < 8; ++i)
        {
            quint32 number =
                QRandomGenerator::system()->generate();

            for (int j = 0; j < 4; ++j)
            {
                bytes.append(
                    static_cast<char>((number >> (j * 8)) & 0xFF)
                    );
            }
        }

        return QString::fromLatin1(bytes.toHex());
    }

    // Хеш для хранения в PostgreSQL
    static QString hashToken(const QString &token)
    {
        QByteArray hash = QCryptographicHash::hash(
            token.toUtf8(),
            QCryptographicHash::Sha256
            );

        return QString::fromLatin1(hash.toHex());
    }

    // Защищённое сохранение токена в Windows
    static bool saveToken(const QString &token)
    {
        QByteArray plain = token.toUtf8();

        DATA_BLOB input{};
        input.pbData = reinterpret_cast<BYTE*>(plain.data());
        input.cbData = static_cast<DWORD>(plain.size());

        DATA_BLOB encrypted{};

        if (!CryptProtectData(
                &input,
                L"Cosmic Chat Session",
                nullptr,
                nullptr,
                nullptr,
                0,
                &encrypted))
        {
            return false;
        }

        QByteArray protectedBytes(
            reinterpret_cast<const char*>(encrypted.pbData),
            static_cast<int>(encrypted.cbData)
            );

        LocalFree(encrypted.pbData);

        QSettings settings("CosmicChat", "CosmicChat");
        settings.setValue(
            "session/protectedToken",
            protectedBytes.toBase64()
            );
        settings.sync();

        return settings.status() == QSettings::NoError;
    }

    // Читаем и расшифровываем токен
    static QString loadToken()
    {
        QSettings settings("CosmicChat", "CosmicChat");

        QByteArray encoded = settings.value(
                                         "session/protectedToken"
                                         ).toByteArray();

        if (encoded.isEmpty())
            return {};

        QByteArray protectedBytes =
            QByteArray::fromBase64(encoded);

        if (protectedBytes.isEmpty())
            return {};

        DATA_BLOB input{};
        input.pbData =
            reinterpret_cast<BYTE*>(protectedBytes.data());
        input.cbData =
            static_cast<DWORD>(protectedBytes.size());

        DATA_BLOB decrypted{};

        if (!CryptUnprotectData(
                &input,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                0,
                &decrypted))
        {
            return {};
        }

        QByteArray plain(
            reinterpret_cast<const char*>(decrypted.pbData),
            static_cast<int>(decrypted.cbData)
            );

        LocalFree(decrypted.pbData);

        return QString::fromUtf8(plain);
    }

    // Удаляем локальную сессию
    static void clearToken()
    {
        QSettings settings("CosmicChat", "CosmicChat");
        settings.remove("session/protectedToken");
        settings.sync();
    }
};
