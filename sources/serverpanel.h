
#ifndef SERVERPANEL_H
#define SERVERPANEL_H

#include <QMainWindow>
#include <QSqlQueryModel>

namespace Ui {
class ServerPanel;
}

class ServerPanel : public QMainWindow
{
    Q_OBJECT

public:
    explicit ServerPanel(QWidget *parent = nullptr);
    ~ServerPanel();

 void applyTheme(bool darkTheme);

private slots:
    void on_backToChatButton_clicked();
    void on_banButton_clicked();
    void on_unbanButton_clicked();
    void on_disconnectButton_clicked();

private:
    Ui::ServerPanel *ui;

    QSqlQueryModel *usersModel;
    QSqlQueryModel *messagesModel;

    void loadUsers();
    void loadMessages();
    void updateStatistics();
};
#endif // SERVERPANEL_H
