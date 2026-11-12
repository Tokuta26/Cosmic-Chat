
#include "mainwindow.h"
#include "startscreen.h"
#include "ui_mainwindow.h"
#include "SessionStorage.h"
#include "serverpanel.h"

#include <QMessageBox>
#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTimer>
#include <QFile>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

static QString cosmicDayStyle()
{
    return R"(
        QMainWindow,
        QWidget#centralwidget {
            background-color: #F5F1FF;
        }

        QLabel {
            color: #493668;
            font-size: 13px;
        }

        QLineEdit {
            background-color: #FFFFFF;
            color: #302344;
            border: 1px solid #B9A5E2;
            border-radius: 9px;
            padding: 7px;
            selection-background-color: #C3ACF3;
        }

        QTextBrowser {
            background-color: #FFFFFF;
            color: #493668;
            border: 1px solid #D5C7ED;
            border-radius: 12px;
            padding: 10px;
        }

        QPushButton {
            background-color: #C7B6E6;
            color: #423456;
            border: 1px solid #B8A5D9;
            border-radius: 10px;
            padding: 8px 14px;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: #BEAADB;
            border-color: #A994CC;
        }

        QPushButton:pressed {
            background-color: #AE99CF;
        }

        QPushButton:focus {
            outline: none;
        }

        QPushButton#themeButton {
            min-width: 42px;
            max-width: 42px;
            min-height: 42px;
            max-height: 42px;
            border-radius: 21px;
            padding: 0px;
            font-size: 18px;
            font-weight: bold;
        }

        QMenuBar,
        QMenu,
        QStatusBar {
            background-color: #E8DFF8;
            color: #493668;
        }

        QMenuBar::item:selected,
        QMenu::item:selected {
            background-color: #D2BFF0;
        }

        QListWidget#chatsListWidget {
            background-color: #FFFFFF;
            color: #493668;
            border: 1px solid #D5C7ED;
            border-radius: 12px;
            padding: 8px;
            font-size: 14px;
            outline: none;
        }

        QListWidget#chatsListWidget::item {
            padding: 12px;
            border-radius: 8px;
        }

        QListWidget#chatsListWidget::item:selected {
            background-color: #C7B6E6;
            color: #302344;
        }

        QListWidget#chatsListWidget::item:hover {
            background-color: #E8DFF8;
        }
QLabel#label_2 {
    font-size: 20px;
    font-weight: bold;
    color: #493668;
}
    )";
}
static QString cosmicNightStyle()
{
    return R"(
        QMainWindow,
        QWidget#centralwidget {
            background-color: #0B1428;
        }

        QLabel {
            color: #E8ECFF;
            font-size: 13px;
        }

        QLineEdit {
            background-color: #1A2B4B;
            color: #FFFFFF;
            border: 1px solid #526A9B;
            border-radius: 9px;
            padding: 7px;
            selection-background-color: #8268CF;
        }

        QTextBrowser {
            background-color: #142440;
            color: #E8ECFF;
            border: 1px solid #344C70;
            border-radius: 12px;
            padding: 10px;
        }

        QPushButton {
            background-color: #514783;
            color: #E8E4F5;
            border: 1px solid #62578F;
            border-radius: 10px;
            padding: 8px 14px;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: #605394;
            border-color: #71649D;
        }

        QPushButton:pressed {
            background-color: #443B70;
        }

        QPushButton:focus {
            outline: none;
        }

        QPushButton#themeButton {
            min-width: 42px;
            max-width: 42px;
            min-height: 42px;
            max-height: 42px;
            border-radius: 21px;
            padding: 0px;
            font-size: 18px;
            font-weight: bold;
        }

        QMenuBar,
        QMenu,
        QStatusBar {
            background-color: #101D35;
            color: #E8ECFF;
        }

        QMenuBar::item:selected,
        QMenu::item:selected {
            background-color: #6354A8;
        }

        QListWidget#chatsListWidget {
            background-color: #142440;
            color: #E8ECFF;
            border: 1px solid #344C70;
            border-radius: 12px;
            padding: 8px;
            font-size: 14px;
            outline: none;
        }

        QListWidget#chatsListWidget::item {
            padding: 12px;
            border-radius: 8px;
        }

        QListWidget#chatsListWidget::item:selected {
            background-color: #514783;
            color: #FFFFFF;
        }

        QListWidget#chatsListWidget::item:hover {
            background-color: #293B60;
        }
QLabel#label_2 {
    font-size: 20px;
    font-weight: bold;
    color: #E8ECFF;
}
    )";
}

int MainWindow::kInstanceCount = 0;

MainWindow::MainWindow(
    int userId,
    QString userName,
    std::shared_ptr<Database> dbPtr,
    QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_userId(userId),
    m_userName(userName)
{
    ui->setupUi(this);
setWindowTitle("Cosmic Control ✦");
    // По умолчанию скрываем вход в админку
    ui->accountButton->hide();

    // Проверяем роль текущего пользователя
   QSqlDatabase adminDb = QSqlDatabase::database();

    if (adminDb.isOpen())
    {
        QSqlQuery query(adminDb);

        query.prepare(
            "SELECT is_admin, is_banned "
            "FROM users WHERE id = :id"
            );

        query.bindValue(":id", m_userId);

        if (query.exec() && query.next())
        {
            const bool isAdmin = query.value(0).toBool();
            const bool isBanned = query.value(1).toBool();

            // Показываем кнопку только администратору
            ui->accountButton->setVisible(
                isAdmin && !isBanned
                );
        }
    }

    ui->accountButton->setText("✦ Cosmic Control");

    kInstanceCount++;

    if (dbPtr)
        m_dbPtr = dbPtr;
    else
        m_dbPtr = std::make_shared<Database>();

    ui->themeButton->setFocusPolicy(Qt::NoFocus);

    m_isDarkTheme = false;
    setStyleSheet(cosmicDayStyle());

    ui->themeButton->setText("☾");


    connect(ui->themeButton, &QPushButton::clicked,
            this, [this]()
            {
                m_isDarkTheme = !m_isDarkTheme;

                if (m_isDarkTheme)
                {
                    setStyleSheet(cosmicNightStyle());
                    ui->themeButton->setText("☀");
                }
                else
                {
                    setStyleSheet(cosmicDayStyle());
                    ui->themeButton->setText("☾");
                }
            });

    ui->chatsListWidget->clear();

    // Первый элемент — общий чат
    ui->chatsListWidget->addItem("🌍 Общий чат");

    // Загружаем пользователей
    auto users = m_dbPtr->getUserList();

    for (const auto &user : users)
    {
        QString username =
            QString::fromStdString(user);

        // Не показываем собственный аккаунт
        if (username == m_userName)
            continue;

        ui->chatsListWidget->addItem(
            "👤 " + username
            );
    }

    // По умолчанию открыт общий чат
    m_selectedChat = "Общий чат";

    ui->chatsListWidget->setCurrentRow(0);

    connect(
        ui->chatsListWidget,
        &QListWidget::currentItemChanged,
        this,
        [this](
            QListWidgetItem *current,
            QListWidgetItem *)
        {
            if (!current)
                return;

            const QString selected =
                current->text();

            if (selected.startsWith("🌍 "))
            {
                m_selectedChat = "Общий чат";

                setWindowTitle(
                    "Cosmic Chat — Общий чат"
                    );
            }
            else if (selected.startsWith("👤 "))
            {
                // Убираем значок пользователя
                m_selectedChat = selected.mid(3);

                setWindowTitle(
                    "Cosmic Chat — " + m_selectedChat
                    );
            }
            else
            {
                return;
            }

            // Одна область отображения сообщений
            ui->commonChatBrowser->show();
            ui->privateChatBrowser->hide();

            // Сразу обновляем переписку
            updateChats();
        }
        );

    setWindowTitle("Cosmic Chat — Общий чат");

    ui->commonChatBrowser->show();
    ui->privateChatBrowser->hide();

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery startedQuery(db);

    if (startedQuery.exec("SELECT clock_timestamp()")
        && startedQuery.next())
    {
        m_sessionStartedAt =
            startedQuery.value(0).toDateTime();
    }
    else
    {
        qWarning()
        << "Не удалось получить время сервера:"
        << startedQuery.lastError().text();
    }

    auto timer = new QTimer(this);

    connect(
        timer,
        &QTimer::timeout,
        this,
        &MainWindow::updateChats
        );

    connect(
        timer,
        &QTimer::timeout,
        this,
        &MainWindow::checkForcedLogout
        );

    timer->start(1000);

    // Загружаем сообщения при открытии
    updateChats();
}

MainWindow::~MainWindow()
{
    delete ui;

    kInstanceCount--;

    if (kInstanceCount <= 0)
        qApp->exit(0);
}

MainWindow *MainWindow::createClient(
    std::shared_ptr<Database> dbPtr,
    bool allowAutoLogin)
{
    StartScreen s(dbPtr, nullptr, allowAutoLogin);

    auto result = s.exec();

    if (result == QDialog::Rejected)
        return nullptr;

    auto w = new MainWindow(
        s.userId(),
        s.userName(),
        s.getDatabase()
        );

    w->setAttribute(Qt::WA_DeleteOnClose);

    return w;
}

void MainWindow::on_messageLineEdit_returnPressed()
{
    on_sendMessageButton_clicked();
}

void MainWindow::on_sendMessageButton_clicked()
{
    if (!m_dbPtr)
        return;

    QString text = ui->messageLineEdit->text();

    // Не отправляем пустые сообщения
    if (text.trimmed().isEmpty())
        return;

    if (m_selectedChat == "Общий чат" ||
        m_selectedChat.isEmpty())
    {
        m_dbPtr->addChatMessage(
            m_userName.toStdString(),
            text.toStdString()
            );
    }

    else
    {
        bool saved = m_dbPtr->addPrivateMessage(
            m_userName.toStdString(),
            m_selectedChat.toStdString(),
            text.toStdString()
            );

        if (!saved)
        {
            QMessageBox::warning(
                this,
                "Cosmic Chat",
                "Не удалось отправить личное сообщение."
                );
            return;
        }
    }

    // Очищаем поле после отправки
    ui->messageLineEdit->clear();

    // Обновляем выбранный чат
    updateChats();
}

void MainWindow::on_privateMessageSendButton_clicked()
{
    QDialog dial(this);
    dial.setModal(true);

    auto layout = new QVBoxLayout();
    dial.setLayout(layout);

    auto userListWgt = new QListWidget(&dial);
    layout->addWidget(userListWgt);

    auto buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel,
        &dial
        );

    layout->addWidget(buttonBox);

    connect(
        buttonBox,
        &QDialogButtonBox::accepted,
        &dial,
        &QDialog::accept
        );

    connect(
        buttonBox,
        &QDialogButtonBox::rejected,
        &dial,
        &QDialog::reject
        );

    auto userList = m_dbPtr->getUserList();

    for (const auto &user : userList)
    {
        QString username =
            QString::fromStdString(user);

        // Не предлагаем отправлять сообщение себе
        if (username == m_userName)
            continue;

        userListWgt->addItem(username);
    }

    if (userListWgt->count() == 0)
    {
        QMessageBox::information(
            this,
            "Cosmic Chat",
            "Пока нет других пользователей."
            );
        return;
    }

    userListWgt->setCurrentRow(0);

    auto result = dial.exec();

    if (result == QDialog::Accepted &&
        userListWgt->currentItem())
    {
        QString text = ui->messageLineEdit->text();

        if (text.trimmed().isEmpty())
            return;

        bool saved = m_dbPtr->addPrivateMessage(
            m_userName.toStdString(),
            userListWgt->currentItem()
                ->text().toStdString(),
            text.toStdString()
            );

        if (saved)
        {
            ui->messageLineEdit->clear();
            updateChats();
        }
        else
        {
            QMessageBox::warning(
                this,
                "Cosmic Chat",
                "Не удалось отправить личное сообщение."
                );
        }
    }
}

void MainWindow::on_actionOpen_another_cliend_triggered()
{
    // Открываем новый клиент без автоматического входа
    MainWindow *newClient = createClient(m_dbPtr, false);

    if (newClient)
    {
        newClient->show();
    }
}

void MainWindow::on_actionCloseClient_triggered()
{
    close();
}

void MainWindow::updateChats()
{
    if (!m_dbPtr)
        return;

    QString chat;

    if (m_selectedChat == "Общий чат" ||
        m_selectedChat.isEmpty())
    {
        auto messages =
            m_dbPtr->getChatMessages();

        for (const auto &msg : messages)
        {
            chat +=
                QString::fromStdString(msg) + "\n";
        }
    }

    else
    {
        auto messages =
            m_dbPtr->getPrivateMessage(m_userId);

        for (const auto &msg : messages)
        {
            QString sender =
                QString::fromStdString(
                    msg.getSender()
                    );

            QString receiver =
                QString::fromStdString(
                    m_dbPtr->getUserName(
                        msg.getDest()
                        )
                    );

            // Я отправила сообщение
            // выбранному пользователю
            bool fromMe =
                sender == m_userName &&
                receiver == m_selectedChat;

            // Выбранный пользователь
            // отправил сообщение мне
            bool toMe =
                sender == m_selectedChat &&
                receiver == m_userName;

            // Сообщения других диалогов пропускаем
            if (!fromMe && !toMe)
                continue;

            QString time =
                QString::fromStdString(
                    msg.getTime()
                    );

            QString text =
                QString::fromStdString(
                    msg.getText()
                    );

            QString author =
                fromMe ? "Вы" : sender;

            chat +=
                "[" + time + "] " +
                author + ": " +
                text + "\n";
        }
    }

    if (ui->commonChatBrowser->toPlainText() != chat)
    {
        ui->commonChatBrowser->setPlainText(chat);
    }
}

void MainWindow::on_logoutButton_clicked()
{
    // Получаем сохранённый токен
    const QString token =
        SessionStorage::loadToken();

    // Если есть сохранённая сессия,
    // удаляем её из PostgreSQL
    if (!token.isEmpty())
    {
        const QString tokenHash =
            SessionStorage::hashToken(token);

        if (!m_dbPtr ||
            !m_dbPtr->revokeSession(tokenHash))
        {
            QMessageBox::warning(
                this,
                "Cosmic Chat",
                "Не удалось завершить сессию. "
                "Проверь подключение к PostgreSQL."
                );
            return;
        }
    }

    // Удаляем сохранённый вход
    SessionStorage::clearToken();

    // Скрываем текущий чат
    hide();

    // Открываем экран входа
    MainWindow *nextWindow =
        MainWindow::createClient(m_dbPtr);

    if (nextWindow)
    {
        nextWindow->show();
    }

    // Закрываем старое окно
    close();
}

void MainWindow::on_accountButton_clicked()
{
    QSqlDatabase db =
        QSqlDatabase::database();

    if (!db.isOpen())
    {
        QMessageBox::warning(
            this,
            "Cosmic Chat",
            "Нет подключения к PostgreSQL."
            );
        return;
    }

    // Проверяем права пользователя
    QSqlQuery query(db);

    query.prepare(
        "SELECT is_admin, is_banned "
        "FROM users "
        "WHERE id = :user_id"
        );

    query.bindValue(
        ":user_id",
        m_userId
        );

    if (!query.exec() || !query.next())
    {
        QMessageBox::warning(
            this,
            "Cosmic Chat",
            "Не удалось проверить права пользователя."
            );
        return;
    }

    const bool isAdmin =
        query.value(0).toBool();

    const bool isBanned =
        query.value(1).toBool();

    // Обычным пользователям вход запрещён
    if (!isAdmin || isBanned)
    {
        QMessageBox::warning(
            this,
            "Доступ запрещён 🔒",
            "Cosmic Control доступна "
            "только администратору."
            );
        return;
    }

    // Создаём административную панель
    auto *panel = new ServerPanel(this);
    panel->applyTheme(m_isDarkTheme);
    panel->setAttribute(
        Qt::WA_DeleteOnClose
        );

    panel->setWindowFlag(
        Qt::Window,
        true
        );

    // Открываем панель на месте чата
    panel->resize(size());
    panel->move(pos());

    panel->show();

    // Прячем главное окно
    hide();
}

void MainWindow::checkForcedLogout()
{
    // Если отключение уже обработано,
    // повторно ничего не делаем
    if (m_forcedLogoutHandled ||
        !m_sessionStartedAt.isValid())
    {
        return;
    }

    QSqlDatabase db =
        QSqlDatabase::database();

    if (!db.isOpen())
        return;

    QSqlQuery query(db);

    query.prepare(
        "SELECT is_banned, "
        "forced_logout_at > :started_at "
        "FROM users "
        "WHERE id = :id"
        );

    query.bindValue(
        ":started_at",
        m_sessionStartedAt
        );

    query.bindValue(
        ":id",
        m_userId
        );

    if (!query.exec() || !query.next())
        return;

    const bool banned =
        query.value(0).toBool();

    const bool disconnected =
        query.value(1).toBool();

    // Если пользователь не забанен
    // и не отключён, продолжаем работу
    if (!banned && !disconnected)
        return;

    m_forcedLogoutHandled = true;

    // Удаляем сохранённый вход
    SessionStorage::clearToken();

    // Прячем окно чата
    hide();

    // Показываем причину отключения
    QMessageBox::warning(
        this,
        banned
            ? "Аккаунт заблокирован 🔒"
            : "Отключение от чата 🚪",
        banned
            ? "Ваш аккаунт заблокирован "
              "администратором Cosmic Chat."
            : "Вы были отключены "
              "администратором. "
              "Вы можете войти снова."
        );

    // Возвращаем пользователя
    // на экран авторизации
    MainWindow *nextWindow =
        MainWindow::createClient(m_dbPtr);

    if (nextWindow)
    {
        nextWindow->show();
    }

    // Закрываем старый чат
    close();
}
