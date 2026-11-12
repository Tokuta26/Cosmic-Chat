
#include "serverpanel.h"
#include "ui_serverpanel.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QSqlQuery>
#include <QMessageBox>
#include <QModelIndex>

static QString cosmicControlLightStyle()
{
    return QStringLiteral(R"(
        QMainWindow, QWidget#centralwidget {
            background-color: #F5F0FF;
        }

        QLabel {
            color: #493271;
            background-color: transparent;
            border: none;
            font-size: 14px;
            font-weight: bold;
        }

        QTableView {
            background-color: #FFFFFF;
            alternate-background-color: #F0E9FC;
            color: #493271;
            gridline-color: #DDD0F4;
            border: 1px solid #D1BDED;
            border-radius: 10px;
            selection-background-color: #C5B5E9;
            selection-color: #352252;
            font-size: 13px;
        }

        QHeaderView::section {
            background-color: #DED2F4;
            color: #493271;
            padding: 8px;
            border: none;
            border-right: 1px solid #CBB9E9;
            font-weight: bold;
        }

        QPushButton {
            background-color: #C5B5E9;
            color: #493271;
            border: 1px solid #B7A2DF;
            border-radius: 9px;
            padding: 9px 14px;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: #B39DDF;
        }

        QPushButton#banButton {
            background-color: #E8BED1;
            color: #64344E;
            border-color: #D8A3BB;
        }

        QPushButton#banButton:hover {
            background-color: #DDA9C0;
        }
    )");
}

static QString cosmicControlDarkStyle()
{
    return QStringLiteral(R"(
        QMainWindow, QWidget#centralwidget {
            background-color: #0B1428;
        }

        QLabel {
            color: #E8ECFF;
            background-color: transparent;
            border: none;
            font-size: 14px;
            font-weight: bold;
        }

        QTableView {
            background-color: #142440;
            alternate-background-color: #1B2D4D;
            color: #E8ECFF;
            gridline-color: #344C70;
            border: 1px solid #526A9B;
            border-radius: 10px;
            selection-background-color: #6354A8;
            selection-color: white;
            font-size: 13px;
        }

        QHeaderView::section {
            background-color: #26395C;
            color: #E8ECFF;
            padding: 8px;
            border: none;
            border-right: 1px solid #344C70;
        }

        QPushButton {
            background-color: #514783;
            color: #FFFFFF;
            border: 1px solid #62578F;
            border-radius: 9px;
            padding: 9px 14px;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: #7160AB;
        }

        QPushButton#banButton {
            background-color: #79445E;
            border-color: #945772;
        }

        QPushButton#banButton:hover {
            background-color: #985570;
        }
    )");
}
static void styleCosmicMessageBox(QMessageBox &box)
{
    box.setStyleSheet(R"(
        QMessageBox {
            background-color: #F5F0FF;
        }

        QMessageBox QLabel {
            color: #3D2865;
            background-color: transparent;
            font-size: 14px;
            font-weight: bold;
        }

        QMessageBox QPushButton {
            background-color: #C5B5E9;
            color: #3D2865;
            border: none;
            border-radius: 10px;
            padding: 8px 18px;
            min-width: 65px;
            font-weight: bold;
        }

        QMessageBox QPushButton:hover {
            background-color: #AE98DE;
        }
    )");
}

static void applyCosmicDialogStyle(QWidget *parent)
{
    if (!parent)
        return;

    parent->setStyleSheet(
        parent->styleSheet() +
        R"(
            QMessageBox {
                background-color: #F5F0FF;
            }

            QMessageBox QLabel {
                color: #3D2865;
                background-color: transparent;
                font-size: 14px;
            }

            QMessageBox QPushButton {
                background-color: #C5B5E9;
                color: #3D2865;
                border: none;
                border-radius: 10px;
                padding: 8px 18px;
                min-width: 65px;
                font-weight: bold;
            }
        )"
        );
}


ServerPanel::ServerPanel(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::ServerPanel),
    usersModel(new QSqlQueryModel(this)),
    messagesModel(new QSqlQueryModel(this))
{
    ui->setupUi(this);
    applyTheme(false);
    setWindowTitle("Cosmic Control ✦");
applyCosmicDialogStyle(this);
    // Таблица пользователей
    ui->usersTableView->setModel(usersModel);
    ui->usersTableView->setSelectionBehavior(
        QAbstractItemView::SelectRows);
    ui->usersTableView->setSelectionMode(
        QAbstractItemView::SingleSelection);
    ui->usersTableView->horizontalHeader()
        ->setStretchLastSection(true);

    // Таблица всех сообщений
    ui->messagesTableView->setModel(messagesModel);
    ui->messagesTableView->setSelectionBehavior(
        QAbstractItemView::SelectRows);
    ui->messagesTableView->setSelectionMode(
        QAbstractItemView::SingleSelection);
    ui->messagesTableView->horizontalHeader()
        ->setStretchLastSection(true);

    // Убираем серые номера строк
    ui->usersTableView->verticalHeader()->hide();
    ui->messagesTableView->verticalHeader()->hide();

    // Запрещаем редактирование данных прямо в таблицах
    ui->usersTableView->setEditTriggers(
        QAbstractItemView::NoEditTriggers);
    ui->messagesTableView->setEditTriggers(
        QAbstractItemView::NoEditTriggers);

    // Растягиваем таблицы аккуратно
    ui->usersTableView->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::Stretch);

    ui->messagesTableView->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::Interactive);
                                                                  ui->messagesTableView->setColumnWidth(0, 45);
    ui->messagesTableView->setColumnWidth(1, 130);
    ui->messagesTableView->setColumnWidth(2, 130);
    ui->messagesTableView->setColumnWidth(3, 250);
    ui->messagesTableView->setColumnWidth(4, 145);

    loadUsers();
    updateStatistics();
    loadMessages();
    updateStatistics();
}

ServerPanel::~ServerPanel()
{
    delete ui;
}

void ServerPanel::loadUsers()
{
    QSqlDatabase db = QSqlDatabase::database();

    if (!db.isOpen()) {
        qWarning() << "PostgreSQL не подключён!";
        return;
    }

    usersModel->setQuery(
        "SELECT id, username, "
        "CASE WHEN is_banned THEN 'Забанен' "
        "ELSE 'Активен' END AS status "
        "FROM users ORDER BY id",
        db
        );

    if (usersModel->lastError().isValid()) {
        qWarning() << "Ошибка загрузки пользователей:"
                   << usersModel->lastError().text();
    }

    usersModel->setHeaderData(
        0, Qt::Horizontal, "ID");
    usersModel->setHeaderData(
        1, Qt::Horizontal, "Логин");
    usersModel->setHeaderData(
        2, Qt::Horizontal, "Статус");
    updateStatistics();
}

void ServerPanel::loadMessages()
{
    QSqlDatabase db = QSqlDatabase::database();

    if (!db.isOpen()) {
        qWarning() << "PostgreSQL не подключён!";
        return;
    }

    messagesModel->setQuery(
        "SELECT "
        "m.id, "
        "s.username AS sender, "
        "COALESCE(r.username, 'Общий чат') AS recipient, "
        "m.content, "
        "TO_CHAR(m.created_at AT TIME ZONE 'Asia/Almaty', "
        "'YYYY-MM-DD HH24:MI') AS sent_at "
        "FROM messages m "
        "JOIN users s ON s.id = m.sender_id "
        "LEFT JOIN users r ON r.id = m.receiver_id "
        "ORDER BY m.created_at DESC, m.id DESC",
        db
        );

    if (messagesModel->lastError().isValid()) {
        qWarning() << "Ошибка загрузки сообщений:"
                   << messagesModel->lastError().text();
    }

    messagesModel->setHeaderData(
        0, Qt::Horizontal, "ID");
    messagesModel->setHeaderData(
        1, Qt::Horizontal, "Отправитель");
    messagesModel->setHeaderData(
        2, Qt::Horizontal, "Получатель");
    messagesModel->setHeaderData(
        3, Qt::Horizontal, "Сообщение");
    messagesModel->setHeaderData(
        4, Qt::Horizontal, "Время");

    ui->messagesTableView->setColumnWidth(0, 45);
    ui->messagesTableView->setColumnWidth(1, 130);
    ui->messagesTableView->setColumnWidth(2, 130);
    ui->messagesTableView->setColumnWidth(3, 250);
    ui->messagesTableView->setColumnWidth(4, 145);
}

// Возвращение в Cosmic Chat
void ServerPanel::on_backToChatButton_clicked()
{
    if (parentWidget()) {
        parentWidget()->show();
        parentWidget()->raise();
        parentWidget()->activateWindow();
    }

    close();
}

void ServerPanel::on_banButton_clicked()
{
    // Проверяем, выбран ли пользователь
    QModelIndex index =
        ui->usersTableView->currentIndex();

    if (!index.isValid())
    {
        QMessageBox::warning(
            this,
            "Cosmic Control",
            "Сначала выбери пользователя в таблице 💜"
            );
        return;
    }

    // В первом столбце таблицы находится ID
    const int userId =
        usersModel->data(usersModel->index(index.row(), 0)).toInt();

    QSqlDatabase db = QSqlDatabase::database();

    if (!db.isOpen())
    {
        QMessageBox::warning(
            this,
            "Cosmic Control",
            "Нет подключения к PostgreSQL."
            );
        return;
    }

    // Не разрешаем забанить администратора
    QSqlQuery check(db);
    check.prepare(
        "SELECT is_admin FROM users WHERE id = :id"
        );
    check.bindValue(":id", userId);

    if (!check.exec() || !check.next())
    {
        QMessageBox::warning(
            this,
            "Cosmic Control",
            "Не удалось проверить пользователя."
            );
        return;
    }

    if (check.value(0).toBool())
    {
        QMessageBox::warning(
            this,
            "Cosmic Control",
            "Администратора банить нельзя 🔒"
            );
        return;
    }

    // Подтверждаем действие
    auto answer = QMessageBox::question(
        this,
        "Подтверждение",
        "Забанить выбранного пользователя?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (answer != QMessageBox::Yes)
        return;

    // Обновляем статус в PostgreSQL
    QSqlQuery query(db);

    query.prepare(
        "UPDATE users "
        "SET is_banned = TRUE "
        "WHERE id = :id AND is_admin = FALSE"
        );

    query.bindValue(":id", userId);

    if (!query.exec() || query.numRowsAffected() != 1)
    {
        QMessageBox::critical(
            this,
            "Ошибка",
            "Не удалось забанить пользователя: "
                + query.lastError().text()
            );
        return;
    }

    // Обновляем таблицу пользователей
    loadUsers();
    updateStatistics();
    QMessageBox::information(
        this,
        "Cosmic Control",
        "Пользователь успешно забанен! 🔒"
        );
}

void ServerPanel::on_unbanButton_clicked()
{
    QModelIndex index = ui->usersTableView->currentIndex();

    if (!index.isValid())
    {
        QMessageBox::warning(
            this,
            "Cosmic Control",
            "Сначала выбери пользователя 💜"
            );
        return;
    }

    int userId = usersModel->data(
                               usersModel->index(index.row(), 0)
                               ).toInt();

    QSqlDatabase db = QSqlDatabase::database();

    if (!db.isOpen())
    {
        QMessageBox::warning(
            this,
            "Ошибка",
            "Нет подключения к PostgreSQL."
            );
        return;
    }

    QSqlQuery query(db);

    query.prepare(
        "UPDATE users "
        "SET is_banned = FALSE "
        "WHERE id = :id AND is_admin = FALSE"
        );

    query.bindValue(":id", userId);

    if (!query.exec() || query.numRowsAffected() != 1)
    {
        QMessageBox::warning(
            this,
            "Ошибка",
            "Не удалось разбанить пользователя: "
                + query.lastError().text()
            );
        return;
    }

    loadUsers();
    updateStatistics();
    QMessageBox::information(
        this,
        "Cosmic Control",
        "Пользователь разбанен! 💜"
        );
}

void ServerPanel::on_disconnectButton_clicked()
{
    QModelIndex index = ui->usersTableView->currentIndex();

    if (!index.isValid())
    {
        QMessageBox::warning(
            this, "Cosmic Control",
            "Сначала выбери пользователя 💜");
        return;
    }

    const int userId = usersModel->data(
                                     usersModel->index(index.row(), 0)).toInt();

    QSqlDatabase db = QSqlDatabase::database();

    if (!db.isOpen())
    {
        QMessageBox::warning(
            this, "Ошибка",
            "Нет подключения к PostgreSQL.");
        return;
    }

    QSqlQuery check(db);
    check.prepare(
        "SELECT is_admin FROM users WHERE id = :id");
    check.bindValue(":id", userId);

    if (!check.exec() || !check.next())
    {
        QMessageBox::warning(
            this, "Ошибка",
            "Не удалось проверить пользователя.");
        return;
    }

    if (check.value(0).toBool())
    {
        QMessageBox::warning(
            this, "Cosmic Control",
            "Администратора отключать нельзя.");
        return;
    }


    QMessageBox confirmBox(this);

    confirmBox.setWindowTitle("Cosmic Control ✦");
    confirmBox.setText(
        "Отключить выбранного пользователя от чата?"
        );
    confirmBox.setIcon(QMessageBox::Question);
    confirmBox.setStandardButtons(
        QMessageBox::Yes | QMessageBox::No
        );
    confirmBox.setDefaultButton(QMessageBox::No);

    styleCosmicMessageBox(confirmBox);

    if (confirmBox.exec() != QMessageBox::Yes)
    {
        return;
    }


    if (!db.transaction())
    {
        QMessageBox::critical(
            this, "Ошибка", db.lastError().text());
        return;
    }

    QSqlQuery query(db);
    query.prepare(
        "UPDATE users "
        "SET forced_logout_at = clock_timestamp() "
        "WHERE id = :id AND is_admin = FALSE");
    query.bindValue(":id", userId);

    if (!query.exec() || query.numRowsAffected() != 1)
    {
        const QString error = query.lastError().text();
        db.rollback();
        QMessageBox::critical(this, "Ошибка", error);
        return;
    }

    // Удаляем сохранённые сеансы пользователя.
    QSqlQuery sessions(db);
    sessions.prepare(
        "DELETE FROM user_sessions WHERE user_id = :id");
    sessions.bindValue(":id", userId);

    if (!sessions.exec())
    {
        const QString error = sessions.lastError().text();
        db.rollback();
        QMessageBox::critical(this, "Ошибка", error);
        return;
    }

    if (!db.commit())
    {
        QMessageBox::critical(
            this, "Ошибка", db.lastError().text());
        return;
    }

    QMessageBox::information(
        this, "Cosmic Control",
        "Команда отключения отправлена! 🚪");
}

void ServerPanel::updateStatistics()
{
    QSqlDatabase db = QSqlDatabase::database();

    if (!db.isOpen())
    {
        qWarning() << "Cosmic Control: база не подключена";
        return;
    }

    QSqlQuery query(db);

    // Всего пользователей
    if (query.exec("SELECT COUNT(*) FROM users")
        && query.next())
    {
        ui->usersCountLabel->setText(
            query.value(0).toString()
            );
    }

    // Всего сообщений
    if (query.exec("SELECT COUNT(*) FROM messages")
        && query.next())
    {
        ui->messagesCountLabel->setText(
            query.value(0).toString()
            );
    }

    // Активные (незаблокированные) аккаунты
    if (query.exec(
            "SELECT COUNT(*) FROM users "
            "WHERE is_banned = FALSE"
            ) && query.next())
    {
        ui->activeCountLabel->setText(
            query.value(0).toString()
            );
    }

    // Заблокированные аккаунты
    if (query.exec(
            "SELECT COUNT(*) FROM users "
            "WHERE is_banned = TRUE"
            ) && query.next())
    {
        ui->bannedCountLabel->setText(
            query.value(0).toString()
            );
    }
}

void ServerPanel::applyTheme(bool darkTheme)
{
    // 1. Переключаем общую тему админки
    if (darkTheme)
    {
        setStyleSheet(cosmicControlDarkStyle());
    }
    else
    {
        setStyleSheet(cosmicControlLightStyle());
    }

    // 2. Цвета карточек
    const QString cardStyle = darkTheme
                                  ? "background-color: #172743;"
                                    "border: 1px solid #526A9B;"
                                    "border-radius: 18px;"
                                  : "background-color: #FFFFFF;"
                                    "border: 1px solid #D7C6F0;"
                                    "border-radius: 18px;";

    // 3. Цвета больших цифр
    const QString countStyle = darkTheme
                                   ? "color: #D6C3FF;"
                                     "font-size: 46px;"
                                     "font-weight: bold;"
                                     "background: transparent;"
                                     "border: none;"
                                   : "color: #493271;"
                                     "font-size: 46px;"
                                     "font-weight: bold;"
                                     "background: transparent;"
                                     "border: none;";

    // 4. Цвета подписей
    const QString titleStyle = darkTheme
                                   ? "color: #E8ECFF;"
                                     "font-size: 17px;"
                                     "font-weight: bold;"
                                     "background: transparent;"
                                     "border: none;"
                                   : "color: #493271;"
                                     "font-size: 17px;"
                                     "font-weight: bold;"
                                     "background: transparent;"
                                     "border: none;";

    // 5. Цвета значков
    const QString iconStyle = darkTheme
                                  ? "color: #C9B6FF;"
                                    "font-size: 23px;"
                                    "background: transparent;"
                                    "border: none;"
                                  : "color: #7153A5;"
                                    "font-size: 23px;"
                                    "background: transparent;"
                                    "border: none;";

    // 6. Применяем стиль ко всем четырём карточкам
    ui->usersCard->setStyleSheet(cardStyle);
    ui->messagesCard->setStyleSheet(cardStyle);
    ui->activeCard->setStyleSheet(cardStyle);
    ui->bannedCard->setStyleSheet(cardStyle);

    // 7. Большие цифры
    ui->usersCountLabel->setStyleSheet(countStyle);
    ui->messagesCountLabel->setStyleSheet(countStyle);
    ui->activeCountLabel->setStyleSheet(countStyle);
    ui->bannedCountLabel->setStyleSheet(countStyle);

    // 8. Подписи
    ui->usersTitleLabel->setStyleSheet(titleStyle);
    ui->messagesTitleLabel->setStyleSheet(titleStyle);
    ui->activeTitleLabel->setStyleSheet(titleStyle);
    ui->bannedTitleLabel->setStyleSheet(titleStyle);

    // 9. Значки
    ui->usersIconLabel->setStyleSheet(iconStyle);
    ui->messagesIconLabel->setStyleSheet(iconStyle);
    ui->activeIconLabel->setStyleSheet(iconStyle);
    ui->bannedIconLabel->setStyleSheet(iconStyle);
}
