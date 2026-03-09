#include "order_detail_dialog.h"
#include "ui_order_detail_dialog.h"

#include "reschedule_dialog.h"
#include <QGraphicsDropShadowEffect>
#include <QSqlQuery>
#include <QMessageBox>
#include <QTimer>
#include <QLineEdit>

OrderDetailDialog::OrderDetailDialog(int orderId, const UserInfo& user, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OrderDetailDialog)
    , m_orderId(orderId)
    , m_user(user)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(25);
    shadow->setColor(QColor(0,0,0,50));
    shadow->setOffset(0, 8);
    ui->cardFrame->setGraphicsEffect(shadow);

    connect(ui->btnClose, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->btnReschedule, &QPushButton::clicked, this, &OrderDetailDialog::onRescheduleClicked);
    connect(ui->btnRefund, &QPushButton::clicked, this, &OrderDetailDialog::onRefundClicked);

    loadData();
    updateUI();
}

OrderDetailDialog::~OrderDetailDialog() {
    delete ui;
}

void OrderDetailDialog::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // Qt6: globalPos() -> globalPosition().toPoint()
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}
void OrderDetailDialog::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        // Qt6: globalPos() -> globalPosition().toPoint()
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void OrderDetailDialog::loadData() {
    QSqlDatabase db = DbManager::getConn();
    if(db.open()){
        QSqlQuery q(db);
        q.prepare("SELECT b.flight_id, f.flight_code, f.origin, f.destination, f.dep_time, "
                  "b.price, b.status, b.is_rescheduled, b.seat_row, b.seat_col "
                  "FROM bookings b JOIN flights f ON b.flight_id = f.id WHERE b.id = :id");
        q.bindValue(":id", m_orderId);
        if(q.exec() && q.next()){
            m_flightId = q.value(0).toInt();
            m_flightCode = q.value(1).toString();
            m_origin = q.value(2).toString();
            m_dest = q.value(3).toString();
            m_depTime = q.value(4).toDateTime();
            m_paidPrice = q.value(5).toDouble();
            m_status = q.value(6).toString();
            m_isRescheduled = q.value(7).toInt();
            m_seatRow = q.value(8).toInt();
            m_seatCol = q.value(9).toInt();
        }
    }
    db.close();
}

void OrderDetailDialog::updateUI() {
    QString headerStyle;
    QString statusText;

    if (m_status == "Checked-in") {
        statusText = "已值机";
        headerStyle = "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #11998e, stop:1 #38ef7d);";
    } else if (m_status == "Refunded" || m_status == "Cancelled") {
        statusText = "已退票";
        headerStyle = "background: #7F8C8D;";
    } else {
        statusText = "已出票";
        headerStyle = "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #0052D4, stop:1 #4364F7);";
    }
    headerStyle += "border-top-left-radius: 16px; border-top-right-radius: 16px;";

    ui->headerFrame->setStyleSheet(headerStyle);
    ui->lblStatusBig->setText(statusText);

    ui->lblOrigin->setText(m_origin);
    ui->lblDest->setText(m_dest);
    ui->lblDepTime->setText(m_depTime.toString("MM月dd日 HH:mm"));

    ui->lblPassengerName->setText(m_user.realName);
    ui->lblFlightCode->setText(m_flightCode);
    ui->lblDate->setText(m_depTime.toString("yyyy-MM-dd"));
    ui->lblPrice->setText(QString("¥%1").arg(m_paidPrice));

    QString seatStr;
    if (m_status == "Cancelled" || m_status == "Refunded") {
        seatStr = "--";
        ui->lblSeat->setStyleSheet("color: #999; font-size:14px; font-weight:bold;");
    } else if (m_seatRow == -1) {
        seatStr = "未选座";
        ui->lblSeat->setStyleSheet("color: #FF9900; font-size:14px; font-weight:bold;");
    } else {
        seatStr = QString("%1排%2座").arg(m_seatRow).arg(QChar('A' + m_seatCol));
        ui->lblSeat->setStyleSheet("color: #0086F6; font-size:14px; font-weight:bold;");
    }
    ui->lblSeat->setText(seatStr);

    bool canOperate = (m_status == "Paid" || m_status == "Checked-in");
    ui->btnContainer->setVisible(canOperate);

    if (canOperate) {
        if (m_isRescheduled) {
            ui->btnReschedule->setText("不可改签");
            ui->btnReschedule->setEnabled(false);
            ui->btnReschedule->setStyleSheet("background: #F5F5F5; color: #CCC; border: none; border-radius: 8px;");
        } else {
            ui->btnReschedule->setText("改签");
            ui->btnReschedule->setEnabled(true);
            ui->btnReschedule->setStyleSheet("QPushButton { background: white; border: 1px solid #DDD; color: #666; border-radius: 8px; font-weight:bold; } QPushButton:hover { border-color: #999; color: #333; }");
        }
    }
}

void OrderDetailDialog::onRescheduleClicked() {
    RescheduleDialog dlg(m_origin, m_dest, m_orderId, m_paidPrice, m_user, this);
    if(dlg.exec() == QDialog::Accepted) {
        loadData();
        updateUI();
        emit orderUpdated();
    }
}

void OrderDetailDialog::onRefundClicked() {
    QDialog dlg(this);
    dlg.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    dlg.setAttribute(Qt::WA_TranslucentBackground);
    dlg.resize(380, 280);
    dlg.setStyleSheet(R"(
        QWidget#Bg { background: white; border-radius: 12px; }
        QLabel { color: #333; font-size: 14px; }
        QLineEdit { border: 1px solid #DDD; border-radius: 6px; padding: 8px; }
        QPushButton#BtnOk { background: #FF4D4F; color: white; border-radius: 18px; font-weight: bold; border:none; }
        QPushButton#BtnCancel { background: #F5F5F5; color: #666; border-radius: 18px; border:none; }
    )");

    QVBoxLayout* root = new QVBoxLayout(&dlg);
    root->setContentsMargins(10,10,10,10);
    QFrame* bg = new QFrame; bg->setObjectName("Bg");
    QGraphicsDropShadowEffect* eff = new QGraphicsDropShadowEffect;
    eff->setBlurRadius(20); eff->setColor(QColor(0,0,0,50)); eff->setOffset(0,4);
    bg->setGraphicsEffect(eff);
    root->addWidget(bg);

    QVBoxLayout* lay = new QVBoxLayout(bg);
    lay->setContentsMargins(25, 25, 25, 25); lay->setSpacing(15);

    QLabel* icon = new QLabel("⚠️"); icon->setStyleSheet("font-size: 30px;"); icon->setAlignment(Qt::AlignCenter);
    QLabel* title = new QLabel("确定要退票吗？"); title->setStyleSheet("font-size: 18px; font-weight: bold;"); title->setAlignment(Qt::AlignCenter);
    QLabel* sub = new QLabel("将收取 10% 手续费，款项将退回余额。"); sub->setStyleSheet("color:#888; font-size: 12px;"); sub->setAlignment(Qt::AlignCenter);

    QLineEdit* pwd = new QLineEdit; pwd->setEchoMode(QLineEdit::Password); pwd->setPlaceholderText("请输入登录密码确认");

    QHBoxLayout* btnLay = new QHBoxLayout;
    QPushButton* bCancel = new QPushButton("取消"); bCancel->setObjectName("BtnCancel"); bCancel->setFixedSize(90, 36);
    QPushButton* bOk = new QPushButton("确认退票"); bOk->setObjectName("BtnOk"); bOk->setFixedSize(90, 36);
    btnLay->addStretch(); btnLay->addWidget(bCancel); btnLay->addWidget(bOk); btnLay->addStretch();

    lay->addWidget(icon); lay->addWidget(title); lay->addWidget(sub); lay->addWidget(pwd); lay->addLayout(btnLay);

    connect(bCancel, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(bOk, &QPushButton::clicked, [&](){
        if(pwd->text().isEmpty()) return;
        QSqlDatabase db = DbManager::getConn();
        if(db.open()){
            QSqlQuery q(db);
            q.prepare("SELECT id FROM users WHERE id=:uid AND password=:p");
            q.bindValue(":uid", m_user.id); q.bindValue(":p", pwd->text());
            if(q.exec() && q.next()) dlg.accept();
            else QMessageBox::critical(&dlg, "错误", "密码错误");
            db.close();
        }
    });

    if(dlg.exec() != QDialog::Accepted) return;

    QSqlDatabase db = DbManager::getConn(true);
    if(db.open()){
        double refundAmount = m_paidPrice * 0.9;
        QSqlQuery q(db);
        db.transaction();
        bool ok = true;

        q.prepare("UPDATE bookings SET status='Refunded', seat_row=-1, seat_col=-1 WHERE id=:id"); q.bindValue(":id", m_orderId);
        if(!q.exec()) ok = false;

        q.prepare("UPDATE users SET balance = balance + :amount WHERE id=:uid");
        q.bindValue(":amount", refundAmount); q.bindValue(":uid", m_user.id);
        if(!q.exec()) ok = false;

        if(ok && db.commit()){
            QDialog succDlg(this); succDlg.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
            succDlg.resize(200, 100); succDlg.setStyleSheet("background:white; border:1px solid #CCC;");
            QVBoxLayout* l = new QVBoxLayout(&succDlg);
            QLabel* lbl = new QLabel("✅ 退票成功", &succDlg); lbl->setAlignment(Qt::AlignCenter);
            l->addWidget(lbl);
            QTimer::singleShot(1000, &succDlg, &QDialog::accept);
            succDlg.exec();

            loadData();
            updateUI();
            emit orderUpdated();
        } else { db.rollback(); }
    }
    DbManager::removeConn(db);
}
