#include "reschedule_dialog.h"
#include "ui_reschedule_dialog.h"

#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QSqlQuery>
#include <QInputDialog>
#include <QSqlError>
#include <QHeaderView>
#include <QDateTime>

RescheduleDialog::RescheduleDialog(QString origin, QString dest, int oldOrderId, double oldPrice, UserInfo user, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RescheduleDialog)
    , m_origin(origin)
    , m_dest(dest)
    , m_oldOrderId(oldOrderId)
    , m_oldPrice(oldPrice)
    , m_user(user)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0,0,0,40));
    shadow->setOffset(0, 5);
    ui->mainFrame->setGraphicsEffect(shadow);

    ui->lblRouteInfo->setText(QString("%1 ➝ %2").arg(m_origin, m_dest));
    ui->dateEdit->setDate(QDate::currentDate());

    initTableConfig();

    connect(ui->btnClose, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->dateEdit, &QDateEdit::dateChanged, this, &RescheduleDialog::onSearch);

    onSearch();
}

RescheduleDialog::~RescheduleDialog() {
    delete ui;
}

void RescheduleDialog::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // Qt6: globalPos() -> globalPosition().toPoint()
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}
void RescheduleDialog::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        // Qt6: globalPos() -> globalPosition().toPoint()
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void RescheduleDialog::initTableConfig() {
    ui->tableReschedule->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableReschedule->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableReschedule->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->tableReschedule->setColumnWidth(2, 120);
    ui->tableReschedule->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    ui->tableReschedule->setColumnWidth(3, 110);
}

void RescheduleDialog::onSearch() {
    QSqlDatabase db = DbManager::getConn();
    if (!db.open()) return;

    QSqlQuery q(db);
    QString searchDate = ui->dateEdit->date().toString("yyyy-MM-dd");

    q.prepare("SELECT id, flight_code, dep_time, model, price FROM flights WHERE origin=:o AND destination=:d AND dep_time >= :dt AND dep_time > NOW() ORDER BY dep_time ASC LIMIT 20");
    q.bindValue(":o", m_origin); q.bindValue(":d", m_dest); q.bindValue(":dt", searchDate);
    q.exec();

    ui->tableReschedule->setRowCount(0);
    int r = 0;
    while(q.next()) {
        ui->tableReschedule->insertRow(r);
        QString fid = q.value(0).toString();
        QString code = q.value(1).toString();
        QDateTime dt = q.value(2).toDateTime();
        QString model = q.value(3).toString();
        double price = q.value(4).toDouble();

        QWidget* w1 = new QWidget; QVBoxLayout* v1 = new QVBoxLayout(w1);
        v1->setContentsMargins(20,5,0,5); v1->setSpacing(2);
        QLabel* lCode = new QLabel(code); lCode->setStyleSheet("font-size: 16px; font-weight: bold; color: #333;");
        QLabel* lModel = new QLabel(model); lModel->setStyleSheet("font-size: 12px; color: #999;");
        v1->addWidget(lCode); v1->addWidget(lModel);
        ui->tableReschedule->setCellWidget(r, 0, w1);

        QWidget* w2 = new QWidget; QVBoxLayout* v2 = new QVBoxLayout(w2);
        v2->setContentsMargins(0,5,0,5); v2->setSpacing(2);
        QLabel* lTime = new QLabel(dt.toString("HH:mm")); lTime->setStyleSheet("font-size: 20px; font-weight: bold; color: #333;");
        QLabel* lDate = new QLabel(dt.toString("MM-dd")); lDate->setStyleSheet("font-size: 12px; color: #666;");
        v2->addWidget(lTime); v2->addWidget(lDate); v2->setAlignment(Qt::AlignCenter);
        ui->tableReschedule->setCellWidget(r, 1, w2);

        double diff = price - m_oldPrice;
        QString diffText; QString color;
        if (diff > 0) { diffText = QString("+¥%1").arg(diff); color = "#FF5500"; }
        else if (diff < 0) { diffText = QString("-¥%1").arg(qAbs(diff)); color = "#00AA00"; }
        else { diffText = "免费"; color = "#333"; }

        QWidget* w3 = new QWidget; QVBoxLayout* v3 = new QVBoxLayout(w3);
        v3->setContentsMargins(0,5,0,5); v3->setSpacing(2);
        QLabel* lPrice = new QLabel(QString("¥%1").arg(price)); lPrice->setStyleSheet("font-size: 16px; font-weight: bold; color: #333;");
        QLabel* lDiff = new QLabel(diffText); lDiff->setStyleSheet(QString("font-size: 12px; color: %1;").arg(color));
        v3->addWidget(lPrice); v3->addWidget(lDiff); v3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->tableReschedule->setCellWidget(r, 2, w3);

        QWidget* w4 = new QWidget; QHBoxLayout* h4 = new QHBoxLayout(w4); h4->setContentsMargins(5,0,15,0);
        QPushButton* btn = new QPushButton("改签");
        btn->setFixedSize(85, 34);
        btn->setCursor(Qt::PointingHandCursor);

        if (diff > 0 && m_user.balance < diff) {
            btn->setText("余额不足"); btn->setFixedSize(85, 34); btn->setEnabled(false);
            btn->setStyleSheet("background: #F0F0F0; color: #CCC; border-radius: 17px; border:none; font-size: 12px;");
        } else {
            btn->setStyleSheet("QPushButton { background: #0086F6; color: white; border-radius: 17px; font-weight: bold; } QPushButton:hover { background: #0076D6; }");
            connect(btn, &QPushButton::clicked, [=](){ onConfirmChange(fid, price, code); });
        }
        h4->addWidget(btn);
        ui->tableReschedule->setCellWidget(r, 3, w4);
        ui->tableReschedule->setRowHeight(r, 75);
        r++;
    }
    db.close();
}

void RescheduleDialog::onConfirmChange(QString newFlightId, double newPrice, QString flightCode) {
    double diff = newPrice - m_oldPrice;
    QString msg;
    if (diff > 0) msg = QString("原价: ¥%1\n新价: ¥%2\n需补: ¥%3").arg(m_oldPrice).arg(newPrice).arg(diff);
    else if (diff < 0) msg = QString("原价: ¥%1\n新价: ¥%2\n退还: ¥%3").arg(m_oldPrice).arg(newPrice).arg(qAbs(diff));
    else msg = "无需补差价";

    QDialog dlg(this);
    dlg.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    dlg.setAttribute(Qt::WA_TranslucentBackground);
    dlg.resize(320, 240);
    dlg.setStyleSheet(R"(
        QWidget#Bg { background: white; border-radius: 12px; border: 1px solid #EEE; }
        QLabel { color: #333; font-size: 14px; }
        QPushButton#BtnOk { background: #0086F6; color: white; border-radius: 18px; font-weight: bold; border:none; }
        QPushButton#BtnCancel { background: #F5F5F5; color: #666; border-radius: 18px; border:none; }
    )");

    QVBoxLayout* root = new QVBoxLayout(&dlg);
    root->setContentsMargins(10,10,10,10);
    QFrame* bg = new QFrame; bg->setObjectName("Bg");
    QGraphicsDropShadowEffect* eff = new QGraphicsDropShadowEffect;
    eff->setBlurRadius(20); eff->setColor(QColor(0,0,0,50)); eff->setOffset(0,5);
    bg->setGraphicsEffect(eff);
    root->addWidget(bg);

    QVBoxLayout* lay = new QVBoxLayout(bg);
    lay->setContentsMargins(20, 20, 20, 20); lay->setSpacing(15);

    QLabel* title = new QLabel("确认改签"); title->setStyleSheet("font-size: 18px; font-weight: bold;"); title->setAlignment(Qt::AlignCenter);
    QLabel* info = new QLabel(msg); info->setStyleSheet("color: #666; line-height: 150%;"); info->setAlignment(Qt::AlignCenter);

    QHBoxLayout* btnLay = new QHBoxLayout;
    QPushButton* bCancel = new QPushButton("取消"); bCancel->setObjectName("BtnCancel"); bCancel->setFixedSize(90, 36);
    QPushButton* bOk = new QPushButton("确定"); bOk->setObjectName("BtnOk"); bOk->setFixedSize(90, 36);
    btnLay->addStretch(); btnLay->addWidget(bCancel); btnLay->addWidget(bOk); btnLay->addStretch();

    lay->addWidget(title); lay->addWidget(info); lay->addLayout(btnLay);

    connect(bCancel, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(bOk, &QPushButton::clicked, &dlg, &QDialog::accept);

    if(dlg.exec() != QDialog::Accepted) return;

    if(!verifyPassword()) return;

    QSqlDatabase db = DbManager::getConn(true);
    if(!db.open()) return;
    if (!db.transaction()) return;

    bool ok = true;
    QSqlQuery q(db);
    q.prepare("UPDATE bookings SET flight_id = :nid, price = :np, is_rescheduled = 1, seat_row = -1, seat_col = -1, status = 'Paid' WHERE id = :oid");
    q.bindValue(":nid", newFlightId); q.bindValue(":np", newPrice); q.bindValue(":oid", m_oldOrderId);
    if (!q.exec()) ok = false;

    if (ok && diff != 0) {
        q.prepare("UPDATE users SET balance = balance - :diff WHERE id = :uid");
        q.bindValue(":diff", diff); q.bindValue(":uid", m_user.id); if (!q.exec()) ok = false;
    }

    if (ok && db.commit()) {
        QMessageBox::information(this, "成功", "改签成功！请重新选择座位。");
        accept();
    } else {
        db.rollback(); QMessageBox::critical(this, "失败", "数据库错误");
    }
    DbManager::removeConn(db);
}

bool RescheduleDialog::verifyPassword() {
    bool ok;
    QString text = QInputDialog::getText(this, "安全验证", "请输入登录密码确认操作:", QLineEdit::Password, "", &ok);
    if (!ok || text.isEmpty()) return false;
    QSqlDatabase db = DbManager::getConn();
    if(db.open()){
        QSqlQuery q(db); q.prepare("SELECT id FROM users WHERE id=:uid AND password=:p");
        q.bindValue(":uid", m_user.id); q.bindValue(":p", text);
        if(q.exec() && q.next()) return true;
    }
    return false;
}
