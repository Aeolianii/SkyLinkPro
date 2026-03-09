#include <QApplication>
#include <QFont>
#include "app_style.h"
#include "login_dialog.h"
#include "admin_window.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    // Qt6 默认开启高分屏支持，移除了旧的属性设置以消除警告

    QApplication a(argc, argv);

    // 1. 字体设置
    QFont font;
    font.setFamily("Microsoft YaHei UI");
    if (font.exactMatch() == false) {
        font.setFamily("Microsoft YaHei");
    }
    font.setPixelSize(14);
    font.setStyleStrategy(QFont::PreferAntialias);
    a.setFont(font);

    // 2. 加载样式
    a.setStyleSheet(AppStyle::getQSS());

    // 3. 显示登录窗口
    LoginDialog login;
    if (login.exec() == QDialog::Accepted) {
        UserInfo user = login.getUserInfo();

        if (user.isAdmin) {
            AdminWindow* adminWin = new AdminWindow;
            adminWin->show();
        } else {
            MainWindow* w = new MainWindow(user);
            w->show();
        }
        return a.exec();
    }

    return 0;
}
