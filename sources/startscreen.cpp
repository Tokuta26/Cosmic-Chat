#include "startscreen.h"
#include "ui_startscreen.h"
#include "SessionStorage.h"
#include <QTimer>

StartScreen::StartScreen(
    std::shared_ptr<Database> dbPtr,
    QWidget *parent,
    bool allowAutoLogin) :
  QDialog(parent),
  ui(new Ui::StartScreen)
{
  ui->setupUi(this);

  if(dbPtr)
    m_dbPtr = dbPtr;
  else
    m_dbPtr = make_shared<Database>();

  ui->loginWidget->setDatabase(m_dbPtr);
  ui->registerWidget->setDatabase(m_dbPtr);

  connect(ui->loginWidget, &LoginForm::registerRequested, this, &StartScreen::setRegistrationForm);
  connect(ui->loginWidget, &LoginForm::accepted, this, &StartScreen::onLoggedIn);
  connect(ui->loginWidget, &LoginForm::rejected, this, &StartScreen::onRejectRequested);
  connect(ui->registerWidget, &RegistrationForm::loginRequested, this, &StartScreen::setLoginForm);
  connect(ui->registerWidget, &RegistrationForm::accepted, this, &StartScreen::onLoggedIn);
  connect(ui->registerWidget, &RegistrationForm::rejected, this, &StartScreen::onRejectRequested);

  // При обычном запуске пробуем восстановить сессию.
  // При открытии второго клиента показываем форму входа.
  if (allowAutoLogin)
  {
      QTimer::singleShot(0, this, [this]()
                         {
                             QString token = SessionStorage::loadToken();

                             if (token.isEmpty())
                                 return;

                             QString tokenHash =
                                 SessionStorage::hashToken(token);

                             int savedUserId = -1;
                             QString savedUserName;

                             if (m_dbPtr->restoreSession(
                                     tokenHash,
                                     savedUserId,
                                     savedUserName))
                             {
                                 onLoggedIn(savedUserId, savedUserName);
                             }
                             else
                             {
                                 // Сессия недействительна или истекла
                                 SessionStorage::clearToken();
                             }
                         });
  }
}

StartScreen::~StartScreen()
{
  delete ui;
}

void StartScreen::setLoginForm()
{
  ui->stackedWidget->setCurrentIndex(0);
}

void StartScreen::setRegistrationForm()
{
  ui->stackedWidget->setCurrentIndex(1);
}

void StartScreen::onLoggedIn(int userId, QString userName)
{
  m_userId = userId;
  m_userName = userName;
  accept();
}

void StartScreen::onRejectRequested()
{
  reject();
}

std::shared_ptr<Database> StartScreen::getDatabase() const
{
  return m_dbPtr;
}

QString StartScreen::userName() const
{
  return m_userName;
}

int StartScreen::userId() const
{
  return m_userId;
}
