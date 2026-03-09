#include "passenger_confirm_dialog.h"
#include "ui_passenger_confirm_dialog.h"
#include "flight_service.h"
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>

PassengerConfirmDialog::PassengerConfirmDialog(const UserInfo& user, const QString& orderNumber, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::PassengerConfirmDialog)
    , m_currentUser(user)
    , m_orderNumber(orderNumber) // 初始化
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect* eff = new QGraphicsDropShadowEffect(this);
    eff->setBlurRadius(20); eff->setColor(QColor(0, 0, 0, 50)); eff->setOffset(0, 5);
    ui->mainFrame->setGraphicsEffect(eff);

    ui->rbSelf->setText("给自己买 (" + m_currentUser.realName + ")");

    // === 动态添加订单号显示 ===
    QLabel* lblOrder = new QLabel(QString("订单号：%1").arg(m_orderNumber));
    lblOrder->setStyleSheet("color: #666; font-size: 13px; font-family: 'Arial'; margin-bottom: 10px;");
    // 将其插入到标题 (index 0) 之后，即 index 1 的位置
    ui->verticalLayout_Main->insertWidget(1, lblOrder);

    connect(ui->rbOther, &QRadioButton::toggled, ui->formContainer, &QWidget::setVisible);
    connect(ui->btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->btnOk, &QPushButton::clicked, this, &PassengerConfirmDialog::onConfirm);
}

PassengerConfirmDialog::~PassengerConfirmDialog() {
    delete ui;
}

void PassengerConfirmDialog::onConfirm() {
    QString pwd = ui->leMyPwd->text();
    if (pwd.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入密码进行验证");
        ui->leMyPwd->setFocus(); return;
    }
    if (!FlightService::verifyPassword(m_currentUser.id, pwd)) {
        QMessageBox::critical(this, "错误", "密码错误，无法完成支付！");
        ui->leMyPwd->clear(); ui->leMyPwd->setFocus(); return;
    }
    if (ui->rbOther->isChecked()) {
        QString targetUser = ui->leOtherUser->text().trimmed();
        QString targetName = ui->leOtherName->text().trimmed();
        if (targetUser.isEmpty() || targetName.isEmpty()) {
            QMessageBox::warning(this, "提示", "请填写完整的他人账号和姓名"); return;
        }
        int pid = FlightService::findUserByName(targetUser);
        if (pid != -1) {
            m_finalPassengerId = pid; m_finalPassengerName = targetName; accept();
        } else {
            QMessageBox::warning(this, "失败", "未找到该账号，请核对！"); ui->leOtherUser->setFocus();
        }
    } else {
        m_finalPassengerId = m_currentUser.id; m_finalPassengerName = m_currentUser.realName; accept();
    }
}
