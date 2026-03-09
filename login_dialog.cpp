#include "login_dialog.h"
#include "ui_login_dialog.h"
#include "toast.h"
#include <QGraphicsDropShadowEffect>
#include <QSqlError>
#include <QTimer>
#include <QDebug>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LoginDialog) {
    // 1. 初始化 UI
    ui->setupUi(this);

    // 2. 窗口基础属性
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 3. 阴影效果 (注意：加在 UI 文件里名为 Card 的容器上)
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(25);
    shadow->setOffset(0, 5);
    shadow->setColor(QColor(0, 0, 0, 40));
    ui->Card->setGraphicsEffect(shadow);

    // 4. 信号槽连接
    setupConnections();

    // 5. 初始聚焦
    ui->m_loginUser->setFocus();
}

LoginDialog::~LoginDialog() {
    delete ui;
}

void LoginDialog::setupConnections() {
    // 关闭按钮
    connect(ui->BtnClose, &QPushButton::clicked, this, &QDialog::reject);

    // 登录按钮
    connect(ui->btnLogin, &QPushButton::clicked, this, &LoginDialog::doLogin);

    // 去注册
    connect(ui->btnToReg, &QPushButton::clicked, [=](){
        ui->m_regUser->clear(); ui->m_regPass->clear(); ui->m_regName->clear();
        ui->m_stack->setCurrentIndex(1); // 切换到第二页
        ui->m_regUser->setFocus();
    });

    // 注册按钮
    connect(ui->btnReg, &QPushButton::clicked, this, &LoginDialog::doRegister);

    // 返回登录
    connect(ui->btnBack, &QPushButton::clicked, [=](){
        ui->m_loginUser->clear(); ui->m_loginPass->clear();
        ui->m_stack->setCurrentIndex(0); // 切换回第一页
        ui->m_loginUser->setFocus();
    });
}

void LoginDialog::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {

        // 1. 登录页
        if (ui->m_stack->currentIndex() == 0) {
            if (ui->m_loginUser->hasFocus()) {
                ui->m_loginPass->setFocus();
            } else if (ui->m_loginPass->hasFocus()) {
                doLogin();
            }
        }
        // 2. 注册页
        else {
            if (ui->m_regUser->hasFocus()) {
                ui->m_regPass->setFocus();
            } else if (ui->m_regPass->hasFocus()) {
                ui->m_regName->setFocus();
            } else if (ui->m_regName->hasFocus()) {
                doRegister();
            }
        }
        return; // 拦截成功
    }

    QDialog::keyPressEvent(event);
}

void LoginDialog::doLogin() {
    QString u = ui->m_loginUser->text().trimmed();
    QString p = ui->m_loginPass->text();

    if(u.isEmpty() || p.isEmpty()) {
        Toast::showError("请输入用户名和密码", this);
        if(u.isEmpty()) ui->m_loginUser->setFocus(); else ui->m_loginPass->setFocus();
        return;
    }

    QSqlDatabase db = DbManager::getConn();
    if(db.open()) {
        QSqlQuery q(db);
        q.prepare("SELECT id, username, real_name, balance, is_admin FROM users WHERE username=:u AND password=:p");
        q.bindValue(":u", u);
        q.bindValue(":p", p);

        if(q.exec() && q.next()) {
            m_userInfo.id = q.value("id").toInt();
            m_userInfo.username = q.value("username").toString();
            m_userInfo.realName = q.value("real_name").toString();
            m_userInfo.balance = q.value("balance").toDouble();
            m_userInfo.isAdmin = (q.value("is_admin").toInt() == 1);

            Toast::showSuccess("欢迎回来，" + m_userInfo.realName, this);
            QTimer::singleShot(500, this, [=](){
                accept();
            });

        } else {
            Toast::showError("用户名或密码错误", this);
            ui->m_loginPass->clear();
            ui->m_loginPass->setFocus();
        }
        db.close();
    } else {
        Toast::showError("数据库连接失败: " + db.lastError().text(), this);
    }
}

void LoginDialog::doRegister() {
    QString u = ui->m_regUser->text().trimmed();
    QString p = ui->m_regPass->text();
    QString n = ui->m_regName->text().trimmed();

    if(u.isEmpty() || p.isEmpty()) {
        Toast::showError("用户名和密码不能为空", this);
        return;
    }

    QSqlDatabase db = DbManager::getConn();
    if(db.open()) {
        QSqlQuery q(db);
        QSqlQuery checkQ(db);
        checkQ.prepare("SELECT username FROM users WHERE username = :u");
        checkQ.bindValue(":u", u);
        if(checkQ.exec() && checkQ.next()) {
            Toast::showError("该用户名已被使用，请换一个", this);
            return;
        }

        q.prepare("INSERT INTO users (username, password, real_name, balance, is_admin) VALUES (:u, :p, :n, 0.00, 0)");
        q.bindValue(":u", u);
        q.bindValue(":p", p);
        q.bindValue(":n", n.isEmpty() ? u : n);

        if(q.exec()) {
            Toast::showSuccess("注册成功！已自动填入账号", this);
            ui->m_stack->setCurrentIndex(0); // 跳回登录页
            ui->m_loginUser->setText(u);
            ui->m_loginPass->setFocus();
        } else {
            Toast::showError("注册写入失败: " + q.lastError().text(), this);
        }
        db.close();
    }
}
