#include <iostream>
#include <QApplication>
#include "CommandLineInterface.h"
#include "mainwindow.h"
#include <QTranslator>
#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>
#include <QLoggingCategory>
#include <QSettings>
#include <QCoreApplication>
#include <QDir>

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QLoggingCategory::setFilterRules(
        "qt.sql.*=true\n"
        "qt.core.plugin.*=true"
        );

    qDebug() << "Available SQL drivers:"
             << QSqlDatabase::drivers();

    qDebug() << "PostgreSQL driver available:"
             << QSqlDatabase::isDriverAvailable("QPSQL");

    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");

    QSettings settings(
        QDir(QCoreApplication::applicationDirPath())
            .filePath("database.ini"),
        QSettings::IniFormat
        );

        db.setHostName(
            settings.value("Database/host", "127.0.0.1").toString()
            );

    db.setPort(
        settings.value("Database/port", 5432).toInt()
        );

    db.setDatabaseName(
        settings.value("Database/name", "cosmic_chat").toString()
        );

    db.setUserName(
        settings.value("Database/user", "postgres").toString()
        );

    db.setPassword(
        settings.value("Database/password").toString()
        );


    if (db.open()) {
        qDebug() << "Cosmic Chat: Database connected!";
    } else {
        qDebug() << "Database error:" << db.lastError().text();
    }

  QTranslator myappTranslator;
  myappTranslator.load("translations/my_ru.qm");
  a.installTranslator(&myappTranslator);

  QTranslator qtTranslator;
  qtTranslator.load("translations/qt_ru.qm");
  a.installTranslator(&qtTranslator);

  auto w = MainWindow::createClient();
  if(w)
    w->show();
  else
    return 0;
  return a.exec();
}


