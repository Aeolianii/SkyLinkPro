#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <QDialog>
#include <QKeyEvent>
#include "db_conn.h"

// 引入 UI 命名空间
namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
    UserInfo getUserInfo() const { return m_userInfo; }

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupConnections(); // 将信号槽连接单独放一个函数，更整洁
    void doLogin();
    void doRegister();

private:
    Ui::LoginDialog *ui; // 指向 .ui 文件生成的类

    UserInfo m_userInfo;
};

#endif // LOGIN_DIALOG_H
