#include "checkin_dialog.h"
#include "ui_checkin_dialog.h" // 必须引入
#include "db_conn.h"

#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>

CheckInDialog::CheckInDialog(int flightId, int bookingId, QWidget *parent)
    : QDialog(parent), ui(new Ui::CheckInDialog), m_flightId(flightId), m_bookingId(bookingId) {

    // 1. 设置 UI
    ui->setupUi(this);

    // 2. 窗口基础属性
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    // 3. 阴影效果 (加在 MainFrame 上)
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(30);
    shadow->setColor(QColor(0, 0, 0, 40));
    shadow->setOffset(0, 8);
    ui->MainFrame->setGraphicsEffect(shadow);

    // 4. 生成动态座位
    generateSeatGrid();

    // 5. 绑定确认按钮
    connect(ui->btn_Confirm, &QPushButton::clicked, this, &CheckInDialog::onConfirm);

    // 6. 加载数据
    loadOccupiedSeats();
}

CheckInDialog::~CheckInDialog() {
    delete ui;
}

// 鼠标拖拽
void CheckInDialog::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}
void CheckInDialog::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

// 动态生成座位网格
void CheckInDialog::generateSeatGrid() {
    // 获取 UI 中的容器
    QVBoxLayout *cardLay = new QVBoxLayout(ui->cabinCard);
    cardLay->setContentsMargins(10, 30, 10, 30);

    QGridLayout *gl = new QGridLayout;
    gl->setSpacing(10);
    gl->setAlignment(Qt::AlignHCenter);

    // 1. 列头 (A B  C D)
    QStringList cols = {"A", "B", "C", "D"};
    for(int c=0; c<4; ++c) {
        QLabel *l = new QLabel(cols[c]);
        l->setFixedSize(44, 20);
        l->setAlignment(Qt::AlignCenter);
        l->setStyleSheet("color: #9AA5B1; font-weight: bold; font-size: 13px; background: transparent; border: none;");

        int gridCol = (c >= 2) ? c + 2 : c + 1; // 留出过道
        gl->addWidget(l, 0, gridCol);
    }

    // 过道宽度
    gl->setColumnMinimumWidth(3, 30);

    // 2. 生成座位按钮
    for (int r = 1; r <= 8; ++r) {
        // 行号
        QLabel *rowLbl = new QLabel(QString::number(r));
        rowLbl->setFixedSize(20, 48);
        rowLbl->setAlignment(Qt::AlignCenter);
        rowLbl->setStyleSheet("color: #CBD5E0; font-weight: bold; font-size: 14px; background: transparent; border: none;");
        gl->addWidget(rowLbl, r, 0);

        for (int c = 0; c < 4; ++c) {
            QPushButton *btn = new QPushButton();
            btn->setFixedSize(44, 50);
            btn->setCursor(Qt::PointingHandCursor);
            btn->setCheckable(true);

            // 样式定义
            btn->setStyleSheet(R"(
                QPushButton {
                    background-color: #EEF2F7;
                    border: none;
                    border-radius: 8px;
                    border-top-left-radius: 18px;
                    border-top-right-radius: 18px;
                }
                QPushButton:hover { background-color: #E0E7F0; }
                QPushButton:checked { background-color: #0086F6; }
                QPushButton:disabled {
                    background-color: #F5F5F5;
                    color: #CCC;
                }
            )");

            QString key = QString("%1_%2").arg(r).arg(c);
            btn->setProperty("seatKey", key);
            connect(btn, &QPushButton::clicked, this, &CheckInDialog::onSeatClicked);

            int gridCol = (c >= 2) ? c + 2 : c + 1;
            gl->addWidget(btn, r, gridCol);

            m_seatBtns[key] = btn;
        }
    }

    cardLay->addLayout(gl);
}

void CheckInDialog::onSeatClicked() {
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if(!btn) return;

    QString key = btn->property("seatKey").toString();

    // 1. 取消选中
    if (m_selectedSeatKey == key && !btn->isChecked()) {
        m_selectedSeatKey = "";
        ui->lbl_SeatInfo->setText("未选择");
        ui->btn_Confirm->setEnabled(false);
        return;
    }

    // 2. 互斥
    if (!m_selectedSeatKey.isEmpty() && m_seatBtns.contains(m_selectedSeatKey)) {
        m_seatBtns[m_selectedSeatKey]->setChecked(false);
    }

    // 3. 选中新座
    m_selectedSeatKey = key;
    btn->setChecked(true);

    QStringList parts = key.split("_");
    m_selectedRow = parts[0].toInt();
    m_selectedCol = parts[1].toInt();
    QString colChar = QString(QChar('A' + m_selectedCol));

    ui->lbl_SeatInfo->setText(QString("<font color='#0086F6'>%1排%2座</font>").arg(m_selectedRow).arg(colChar));
    ui->btn_Confirm->setEnabled(true);
}

void CheckInDialog::onConfirm() {
    if(m_selectedRow == -1) return;

    // 二次确认弹窗
    QDialog dlg(this);
    dlg.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    dlg.setAttribute(Qt::WA_TranslucentBackground);
    dlg.resize(300, 180);
    dlg.setStyleSheet("QDialog { background: transparent; } QFrame { background: white; border-radius: 12px; } QLabel { color: #333; }");

    QVBoxLayout *l = new QVBoxLayout(&dlg);
    QFrame *bg = new QFrame;
    QGraphicsDropShadowEffect *eff = new QGraphicsDropShadowEffect;
    eff->setBlurRadius(15); eff->setColor(QColor(0,0,0,30));
    bg->setGraphicsEffect(eff);
    l->addWidget(bg);

    QVBoxLayout *bl = new QVBoxLayout(bg);
    QLabel *t = new QLabel("确认选座"); t->setStyleSheet("font-size: 16px; font-weight: bold;"); t->setAlignment(Qt::AlignCenter);
    QLabel *c = new QLabel("您确定要选择此座位吗？\n选定后可打印登机牌。"); c->setStyleSheet("color: #666; margin: 10px 0;"); c->setAlignment(Qt::AlignCenter);

    QHBoxLayout *hl = new QHBoxLayout;
    QPushButton *b1 = new QPushButton("取消"); b1->setStyleSheet("border:1px solid #DDD; background:white; color:#666; border-radius:15px; height:30px;");
    QPushButton *b2 = new QPushButton("确定"); b2->setStyleSheet("border:none; background:#0086F6; color:white; border-radius:15px; height:30px;");
    hl->addWidget(b1); hl->addWidget(b2);

    bl->addWidget(t); bl->addWidget(c); bl->addLayout(hl);

    connect(b1, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(b2, &QPushButton::clicked, &dlg, &QDialog::accept);

    if(dlg.exec() != QDialog::Accepted) return;

    // 数据库更新
    QSqlDatabase db = DbManager::getConn();
    if(db.open()) {
        QSqlQuery q(db);
        q.prepare("UPDATE bookings SET seat_row = :r, seat_col = :c, status = 'Checked-in' WHERE id = :bid");
        q.bindValue(":r", m_selectedRow);
        q.bindValue(":c", m_selectedCol);
        q.bindValue(":bid", m_bookingId);

        if(q.exec()) {
            accept();
        } else {
            QMessageBox::warning(this, "失败", "选座失败，请重试");
        }
    }
}

void CheckInDialog::loadOccupiedSeats() {
    QSqlDatabase db = DbManager::getConn();
    if (!db.open()) return;

    QSqlQuery q(db);
    q.prepare("SELECT seat_row, seat_col FROM bookings WHERE flight_id = :fid AND status != 'Cancelled' AND id != :bid");
    q.bindValue(":fid", m_flightId);
    q.bindValue(":bid", m_bookingId);
    q.exec();

    while(q.next()) {
        int r = q.value(0).toInt();
        int c = q.value(1).toInt();
        QString key = QString("%1_%2").arg(r).arg(c);

        if(m_seatBtns.contains(key)) {
            QPushButton *btn = m_seatBtns[key];
            btn->setEnabled(false);
            btn->setStyleSheet(R"(
                background-color: #F5F5F5;
                border: none;
                border-radius: 8px;
                border-top-left-radius: 18px;
                border-top-right-radius: 18px;
                color: #CCC;
                font-weight: bold;
            )");
            btn->setText("×");
        }
    }
}
