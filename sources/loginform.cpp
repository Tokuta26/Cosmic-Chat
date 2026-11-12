
#include "loginform.h"
#include "ui_loginform.h"
#include <QMessageBox>
#include <QSettings>
#include "SessionStorage.h"

LoginForm::LoginForm(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::LoginForm)
{
    ui->setupUi(this);

    // Загружаем сохранённые настройки
    QSettings settings(
        "CosmicChat",
        "CosmicChat"
        );

    bool remember = settings.value(
                                "login/remember",
                                false
                                ).toBool();

    ui->rememberMeCheckBox->setChecked(remember);

    if (remember)
    {
        QString savedLogin = settings.value(
                                         "login/username",
                                         ""
                                         ).toString();

        ui->loginEdit->setText(savedLogin);
    }
}

LoginForm::~LoginForm()
{
    delete ui;
}

void LoginForm::setDatabase(
    std::shared_ptr<Database> dbPtr)
{
    m_dbPtr = dbPtr;
}

void LoginForm::on_buttonBox_accepted()
{
    QString username = ui->loginEdit->text();
    QString password = ui->passwordEdit->text();

    if (!m_dbPtr)
    {
        QMessageBox::critical(
            this,
            tr("Error"),
            tr("Database is not available")
            );
        return;
    }

    int userId = m_dbPtr->checkPassword(
        username.toStdString(),
        password.toStdString()
        );

    if (userId == -2)
    {
        QMessageBox::warning(
            this,
            tr("Аккаунт заблокирован 🔒"),
            tr("Ваш аккаунт заблокирован администратором Cosmic Chat.")
            );
        return;
    }

    if (userId < 0)
    {
        QMessageBox::critical(
            this,
            tr("Ошибка входа"),
            tr("Неверный логин или пароль.")
            );
        return;
    }

    // Сохраняем настройки только после
    // успешного входа
    QSettings settings(
        "CosmicChat",
        "CosmicChat"
        );

    bool remember =
        ui->rememberMeCheckBox->isChecked();

    settings.setValue(
        "login/remember",
        remember
        );

    if (remember)
    {
        settings.setValue(
            "login/username",
            username
            );
    }
    else
    {
        settings.remove("login/username");
    }

    // Создаём сессию, если пользователь
    // выбрал "Запомнить меня"
    if (remember)
    {
        QString token = SessionStorage::generateToken();
        QString tokenHash = SessionStorage::hashToken(token);

        if (m_dbPtr->createSession(userId, tokenHash))
        {
            if (!SessionStorage::saveToken(token))
            {
                m_dbPtr->revokeSession(tokenHash);
                SessionStorage::clearToken();
            }
        }
    }
    else
    {
        // Отзываем предыдущую сессию
        QString oldToken = SessionStorage::loadToken();

        if (!oldToken.isEmpty())
        {
            QString oldHash =
                SessionStorage::hashToken(oldToken);

            if (m_dbPtr->revokeSession(oldHash))
            {
                SessionStorage::clearToken();
            }
        }
    }

    // Пароль не сохраняем
    ui->passwordEdit->clear();

    emit accepted(userId, username);
}

void LoginForm::on_buttonBox_rejected()
{
    emit rejected();
}

void LoginForm::on_registrationPushButton_clicked()
{
    emit registerRequested();
}
