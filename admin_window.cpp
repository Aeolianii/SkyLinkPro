#include "admin_window.h"
#include "ui_admin_window.h"
#include "db_conn.h"
#include "city_selector.h" // 引用城市选择器

#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QMenu>
#include <QAction>
#include <QDateTime>
#include <QRandomGenerator>

AdminWindow::AdminWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AdminWindow)
{
    ui->setupUi(this);

    // 窗口样式设置
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 阴影效果
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 50));
    shadow->setOffset(0, 5);
    ui->centralwidget->setGraphicsEffect(shadow);

    initLogic();
    loadFlights();
}

AdminWindow::~AdminWindow() {
    delete ui;
}

// === 鼠标拖拽窗口逻辑 ===
void AdminWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void AdminWindow::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void AdminWindow::initLogic() {
    // 侧边栏导航按钮组
    m_navGroup = new QButtonGroup(this);
    m_navGroup->setExclusive(true);
    m_navGroup->addButton(ui->btnNavFlight, 0);
    m_navGroup->addButton(ui->btnNavOrder, 1);
    m_navGroup->addButton(ui->btnNavUser, 2);

    connect(m_navGroup, &QButtonGroup::idClicked, this, &AdminWindow::onNavClicked);

    // 表格右键菜单策略
    ui->m_flightTable->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->m_allOrdersTable->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->m_userTable->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->m_flightTable, &QTableWidget::customContextMenuRequested, this, &AdminWindow::onTableContextMenu);
    connect(ui->m_allOrdersTable, &QTableWidget::customContextMenuRequested, this, &AdminWindow::onTableContextMenu);
    connect(ui->m_userTable, &QTableWidget::customContextMenuRequested, this, &AdminWindow::onTableContextMenu);

    // 按钮事件连接
    connect(ui->m_btnCommitFlight, &QPushButton::clicked, this, &AdminWindow::onCommitFlight);
    connect(ui->m_btnCancelFlight, &QPushButton::clicked, this, &AdminWindow::onCancelFlightEdit);
    connect(ui->m_btnCommitOrder, &QPushButton::clicked, this, &AdminWindow::onCommitOrder);
    connect(ui->m_btnCancelOrder, &QPushButton::clicked, this, &AdminWindow::onCancelOrderEdit);
    connect(ui->m_btnCommitUser, &QPushButton::clicked, this, &AdminWindow::onCommitUser);
    connect(ui->m_btnCancelUser, &QPushButton::clicked, this, &AdminWindow::onCancelUserEdit);

    // 表格样式调整
    ui->m_flightTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->m_allOrdersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->m_userTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 隐藏 ID 列
    ui->m_flightTable->setColumnHidden(0, true);
    ui->m_allOrdersTable->setColumnHidden(0, true);
    ui->m_userTable->setColumnHidden(0, true);
}

void AdminWindow::onNavClicked(int index) {
    ui->m_mainStack->setCurrentIndex(index);
    if (index == 0) loadFlights();
    else if (index == 1) loadAllOrders();
    else if (index == 2) loadUsers();
}

void AdminWindow::onTableContextMenu(const QPoint &pos) {
    QTableWidget* table = qobject_cast<QTableWidget*>(sender());
    if (!table) return;

    QTableWidgetItem* item = table->itemAt(pos);
    if (!item) return;

    int row = item->row();
    table->selectRow(row);
    QString id = table->item(row, 0)->text();

    QMenu menu(table);
    menu.setStyleSheet("QMenu { background: white; border: 1px solid #ccc; } "
                       "QMenu::item { padding: 5px 20px; color: #333; } "
                       "QMenu::item:selected { background: #e6f7ff; }");

    QAction* actEdit = menu.addAction("✏️ 编辑");
    QAction* actDel = menu.addAction("🗑️ 删除");

    connect(actEdit, &QAction::triggered, this, [=]() {
        if (table == ui->m_flightTable) performEditFlight(row);
        else if (table == ui->m_allOrdersTable) performEditOrder(row);
        else if (table == ui->m_userTable) performEditUser(row);
    });

    connect(actDel, &QAction::triggered, this, [=]() {
        if (table == ui->m_flightTable) performDeleteFlight(id);
        else if (table == ui->m_allOrdersTable) performDeleteOrder(id);
        else if (table == ui->m_userTable) performDeleteUser(id);
    });

    menu.exec(QCursor::pos());
}

// ==========================================
// 航班管理模块
// ==========================================

void AdminWindow::loadFlights() {
    QSqlDatabase db = DbManager::getConn();
    if (!db.open()) return;

    QSqlQuery q(db);
    q.exec("SELECT id, flight_code, airline, origin, destination, dep_time, arr_time, model, price, status "
           "FROM flights ORDER BY dep_time DESC");

    ui->m_flightTable->setRowCount(0);
    int row = 0;
    while (q.next()) {
        ui->m_flightTable->insertRow(row);
        ui->m_flightTable->setRowHeight(row, 50);

        auto newItem = [](QString text) {
            QTableWidgetItem* i = new QTableWidgetItem(text);
            i->setTextAlignment(Qt::AlignCenter);
            return i;
        };

        ui->m_flightTable->setItem(row, 0, newItem(q.value(0).toString()));

        // 格式化航班号和状态
        QString info = q.value(1).toString() + "\n" + q.value(2).toString();
        QString status = q.value(9).toString();
        if (status != "ON TIME") info += "\n[" + status + "]";

        QTableWidgetItem* code = newItem(info);
        code->setFont(QFont("Arial", 9, QFont::Bold));

        if (status == "CANCELLED") code->setForeground(Qt::gray);
        else if (status == "DELAYED") code->setForeground(Qt::red);
        else code->setForeground(QBrush(QColor("#3498db")));

        ui->m_flightTable->setItem(row, 1, code);
        ui->m_flightTable->setItem(row, 2, newItem(q.value(3).toString()));
        ui->m_flightTable->setItem(row, 3, newItem(q.value(4).toString()));
        ui->m_flightTable->setItem(row, 4, newItem(q.value(5).toDateTime().toString("MM-dd HH:mm")));
        ui->m_flightTable->setItem(row, 5, newItem(q.value(6).toDateTime().toString("MM-dd HH:mm")));
        ui->m_flightTable->setItem(row, 6, newItem(q.value(7).toString()));
        ui->m_flightTable->setItem(row, 7, newItem("¥" + q.value(8).toString()));
        row++;
    }
    db.close();
}

void AdminWindow::onCommitFlight() {
    QString code = ui->m_inputCode->text().trimmed();
    QString airline = ui->m_inputAirline->text().trimmed();
    QString status = ui->m_inputStatus->currentText();
    QString inputOri = ui->m_inputOrigin->text().trimmed();
    QString inputDest = ui->m_inputDest->text().trimmed();
    QString priceStr = ui->m_inputPrice->text().trimmed();

    // 基础非空校验
    if (code.isEmpty() || airline.isEmpty() || priceStr.isEmpty()) {
        QMessageBox::warning(this, "提示", "必填项不能为空");
        return;
    }

    // 价格校验
    double priceVal = priceStr.toDouble();
    if (priceVal < 0) {
        QMessageBox::warning(this, "错误", "票价不能为负数！");
        return;
    }

    // === 智能城市修正逻辑 ===
    QString fixedOri = CitySelector::tryFixCityName(inputOri);
    QString fixedDest = CitySelector::tryFixCityName(inputDest);

    // 1. 检查出发地
    if (fixedOri.isEmpty()) {
        QMessageBox::warning(this, "错误", QString("无法识别出发地: \"%1\"\n请检查是否输入正确，或尝试输入完整城市名称。").arg(inputOri));
        return;
    } else if (fixedOri != inputOri) {
        ui->m_inputOrigin->setText(fixedOri);
    }

    // 2. 检查目的地
    if (fixedDest.isEmpty()) {
        QMessageBox::warning(this, "错误", QString("无法识别目的地: \"%1\"\n请检查是否输入正确，或尝试输入完整城市名称。").arg(inputDest));
        return;
    } else if (fixedDest != inputDest) {
        ui->m_inputDest->setText(fixedDest);
    }

    // 3. 检查是否相同
    if (fixedOri == fixedDest) {
        QMessageBox::warning(this, "错误", "出发地和目的地不能相同！");
        return;
    }

    // 日期时间处理
    QDate flightDate = ui->m_inputDate->date();
    QTime depTime = ui->m_inputDepTime->time();
    QTime arrTime = ui->m_inputArrTime->time();
    QDateTime depDateTime(flightDate, depTime);
    QDateTime arrDateTime(flightDate, arrTime);

    // 跨天逻辑：如果落地时间 <= 起飞时间，默认为次日
    if (arrTime <= depTime) {
        arrDateTime = arrDateTime.addDays(1);
    }

    // 过去时间校验
    if (depDateTime < QDateTime::currentDateTime()) {
        QMessageBox::warning(this, "错误", "不能添加过去时间的航班！");
        return;
    }

    QSqlDatabase db = DbManager::getConn();
    if (db.open()) {
        QSqlQuery q(db);

        // 航班号查重
        QSqlQuery checkQ(db);
        QString checkSql = "SELECT id FROM flights WHERE flight_code = :code";
        if (m_editingFlightId != -1) checkSql += " AND id != :id";

        checkQ.prepare(checkSql);
        checkQ.bindValue(":code", code);
        if (m_editingFlightId != -1) checkQ.bindValue(":id", m_editingFlightId);

        if (checkQ.exec() && checkQ.next()) {
            QMessageBox::warning(this, "错误", "该航班号已存在！");
            db.close();
            return;
        }

        // 准备 SQL 语句
        if (m_editingFlightId == -1) {
            q.prepare("INSERT INTO flights (flight_code, airline, origin, destination, dep_time, arr_time, model, price, status) "
                      "VALUES (:c, :a, :o, :d, :dt, :at, :m, :p, :s)");
        } else {
            q.prepare("UPDATE flights SET flight_code=:c, airline=:a, origin=:o, destination=:d, dep_time=:dt, arr_time=:at, model=:m, price=:p, status=:s WHERE id=:id");
            q.bindValue(":id", m_editingFlightId);
        }

        q.bindValue(":c", code);
        q.bindValue(":a", airline);
        q.bindValue(":o", fixedOri);
        q.bindValue(":d", fixedDest);
        q.bindValue(":dt", depDateTime);
        q.bindValue(":at", arrDateTime);
        q.bindValue(":m", ui->m_inputModel->text());
        q.bindValue(":p", priceVal);
        q.bindValue(":s", status);

        if (q.exec()) {
            resetFlightForm();
            loadFlights();
            QMessageBox::information(this, "成功", "航班保存成功！");
        } else {
            QMessageBox::warning(this, "错误", q.lastError().text());
        }
        db.close();
    }
}

void AdminWindow::performEditFlight(int row) {
    m_editingFlightId = ui->m_flightTable->item(row, 0)->text().toInt();

    // 解析航班号和航司
    QString fullCode = ui->m_flightTable->item(row, 1)->text();
    QStringList parts = fullCode.split("\n");
    ui->m_inputCode->setText(parts.value(0));
    if (parts.size() > 1) ui->m_inputAirline->setText(parts.value(1));

    ui->m_inputOrigin->setText(ui->m_flightTable->item(row, 2)->text());
    ui->m_inputDest->setText(ui->m_flightTable->item(row, 3)->text());
    ui->m_inputModel->setText(ui->m_flightTable->item(row, 6)->text());
    ui->m_inputPrice->setText(ui->m_flightTable->item(row, 7)->text().remove("¥"));

    QSqlDatabase db = DbManager::getConn();
    if (db.open()) {
        QSqlQuery q(db);
        q.prepare("SELECT status FROM flights WHERE id=:id");
        q.bindValue(":id", m_editingFlightId);
        if (q.exec() && q.next()) {
            ui->m_inputStatus->setCurrentText(q.value(0).toString());
        }
        db.close();
    }

    ui->m_btnCommitFlight->setText("保存修改");
    ui->m_btnCancelFlight->setVisible(true);
}

void AdminWindow::performDeleteFlight(QString id) {
    if (QMessageBox::question(this, "确认", "确定删除？") == QMessageBox::Yes) {
        QSqlDatabase db = DbManager::getConn();
        if (db.open()) {
            QSqlQuery q(db);
            q.prepare("DELETE FROM flights WHERE id=:id");
            q.bindValue(":id", id);
            q.exec();
            loadFlights();
            db.close();
        }
    }
}

void AdminWindow::resetFlightForm() {
    m_editingFlightId = -1;
    ui->m_inputCode->clear();
    ui->m_inputAirline->clear();
    ui->m_inputOrigin->clear();
    ui->m_inputDest->clear();
    ui->m_inputModel->clear();
    ui->m_inputPrice->clear();
    ui->m_inputStatus->setCurrentIndex(0);

    ui->m_btnCommitFlight->setText("发布航班");
    ui->m_btnCancelFlight->setVisible(false);
}

void AdminWindow::onCancelFlightEdit() {
    resetFlightForm();
}

// ==========================================
// 订单管理模块
// ==========================================

void AdminWindow::loadAllOrders() {
    QSqlDatabase db = DbManager::getConn();
    if (!db.open()) return;

    QSqlQuery q(db);
    q.exec("SELECT b.id, u.username, b.order_number, f.flight_code, f.airline, f.origin, f.destination, "
           "b.seat_row, b.seat_col, b.price, b.status, b.booking_time "
           "FROM bookings b "
           "LEFT JOIN users u ON b.user_id = u.id "
           "LEFT JOIN flights f ON b.flight_id = f.id "
           "ORDER BY b.id DESC");

    ui->m_allOrdersTable->setRowCount(0);
    int row = 0;
    while (q.next()) {
        ui->m_allOrdersTable->insertRow(row);
        ui->m_allOrdersTable->setRowHeight(row, 50);

        auto newItem = [](QString t) {
            QTableWidgetItem* i = new QTableWidgetItem(t);
            i->setTextAlignment(Qt::AlignCenter);
            return i;
        };

        ui->m_allOrdersTable->setItem(row, 0, newItem(q.value(0).toString()));
        ui->m_allOrdersTable->setItem(row, 1, newItem(q.value(1).toString()));

        QString flightInfo = QString("单号:%1\n%2 %3")
                                 .arg(q.value(2).toString())
                                 .arg(q.value(3).toString())
                                 .arg(q.value(4).toString());
        QTableWidgetItem* fItem = newItem(flightInfo);
        fItem->setFont(QFont("Arial", 8));
        ui->m_allOrdersTable->setItem(row, 2, fItem);

        ui->m_allOrdersTable->setItem(row, 3, newItem(q.value(5).toString() + " -> " + q.value(6).toString()));

        int seatR = q.value(7).toInt();
        int seatC = q.value(8).toInt();
        QString seatDisplay = (seatR == -1 || seatC == -1) ? "未选座" : QString::number(seatR) + "排" + QChar('A' + seatC);
        ui->m_allOrdersTable->setItem(row, 4, newItem(seatDisplay));
        ui->m_allOrdersTable->setItem(row, 5, newItem("¥" + q.value(9).toString()));

        // 状态颜色逻辑
        QString statusVal = q.value(10).toString();
        QString statusText = statusVal;
        QColor statusColor = Qt::black;

        if (statusVal == "Paid") {
            statusText = "已支付";
            statusColor = QColor("#27ae60");
        } else if (statusVal == "Checked-in") {
            statusText = "已值机";
            statusColor = QColor("#2980b9");
        } else if (statusVal == "Cancelled" || statusVal == "Refunded") {
            statusText = "已退票";
            statusColor = Qt::gray;
        } else if (statusVal == "Booked") {
            statusText = "待支付";
            statusColor = QColor("#e67e22");
        }

        QTableWidgetItem* st = newItem(statusText);
        st->setForeground(QBrush(statusColor));
        st->setFont(QFont("Microsoft YaHei", 9, QFont::Bold));
        ui->m_allOrdersTable->setItem(row, 6, st);
        ui->m_allOrdersTable->setItem(row, 7, newItem(q.value(11).toDateTime().toString("MM-dd HH:mm")));
        row++;
    }
    db.close();
}

void AdminWindow::onCommitOrder() {
    QString username = ui->m_ordInputUser->text().trimmed();
    QString flightCode = ui->m_ordInputFlight->text().trimmed();
    QString priceStr = ui->m_ordInputPrice->text().trimmed();

    if (username.isEmpty() || flightCode.isEmpty()) {
        QMessageBox::warning(this, "提示", "用户名和航班号不能为空");
        return;
    }

    // 订单金额负数校验
    if (!priceStr.isEmpty()) {
        if (priceStr.toDouble() < 0) {
            QMessageBox::warning(this, "错误", "订单金额不能为负数！");
            return;
        }
    }

    // 解析座位
    int seatRow = -1;
    int seatCol = -1;
    QString rowText = ui->m_ordInputRow->text().trimmed();
    if (!rowText.isEmpty() && rowText.toInt() > 0) {
        seatRow = rowText.toInt();
        seatCol = ui->m_ordInputCol->currentIndex();
    }

    // 解析状态
    QString statusStr = "Paid";
    int statusIdx = ui->m_ordInputStatus->currentIndex();
    if (statusIdx == 0) statusStr = "Booked";
    else if (statusIdx == 1) statusStr = "Paid";
    else if (statusIdx == 2) statusStr = "Cancelled";

    // 如果选了座且没退票，状态自动变为值机
    if (seatRow != -1 && statusStr != "Cancelled") {
        statusStr = "Checked-in";
    }

    QSqlDatabase db = DbManager::getConn();
    if (!db.open()) return;
    QSqlQuery q(db);

    // 1. 获取用户 ID
    int userId = -1;
    q.prepare("SELECT id FROM users WHERE username = :u");
    q.bindValue(":u", username);
    if (q.exec() && q.next()) {
        userId = q.value(0).toInt();
    } else {
        QMessageBox::warning(this, "错误", "未找到该用户");
        db.close();
        return;
    }

    // 2. 获取航班信息
    int flightId = -1;
    double finalPrice = 0.0;
    QString fullFlightCode = "";
    QDateTime depTime;
    q.prepare("SELECT id, price, flight_code, dep_time FROM flights WHERE flight_code = :f");
    q.bindValue(":f", flightCode);
    if (q.exec() && q.next()) {
        flightId = q.value(0).toInt();
        finalPrice = priceStr.isEmpty() ? q.value(1).toDouble() : priceStr.toDouble();
        fullFlightCode = q.value(2).toString();
        depTime = q.value(3).toDateTime();
    } else {
        QMessageBox::warning(this, "错误", "未找到该航班");
        db.close();
        return;
    }

    // 3. 检查座位冲突
    if (seatRow != -1 && seatCol != -1) {
        QSqlQuery checkQ(db);
        QString checkSql = "SELECT id FROM bookings WHERE flight_id = :fid AND seat_row = :row AND seat_col = :col AND status != 'Cancelled' AND status != 'Refunded'";
        if (m_editingOrderId != -1) checkSql += " AND id != :oid";

        checkQ.prepare(checkSql);
        checkQ.bindValue(":fid", flightId);
        checkQ.bindValue(":row", seatRow);
        checkQ.bindValue(":col", seatCol);
        if (m_editingOrderId != -1) checkQ.bindValue(":oid", m_editingOrderId);

        if (checkQ.exec() && checkQ.next()) {
            QMessageBox::warning(this, "冲突", "该座位已被占用");
            db.close();
            return;
        }
    }

    bool success = false;
    if (m_editingOrderId == -1) {
        // 生成订单号
        QString dateStr = QDate::currentDate().toString("yyyyMMdd");
        QString timeStr = depTime.toString("HHmm");
        int randomNum = QRandomGenerator::global()->bounded(1000, 9999);
        QString orderNum = QString("%1-%2-%3-%4").arg(dateStr, fullFlightCode, timeStr, QString::number(randomNum));

        q.prepare("INSERT INTO bookings (order_number, user_id, flight_id, passenger_name, passenger_id, seat_row, seat_col, status, price, booking_time, is_rescheduled) "
                  "VALUES (:ord, :uid, :fid, :pname, 0, :row, :col, :st, :price, NOW(), 0)");
        q.bindValue(":ord", orderNum);
        q.bindValue(":uid", userId);
        q.bindValue(":fid", flightId);
        q.bindValue(":pname", username);
        q.bindValue(":row", seatRow);
        q.bindValue(":col", seatCol);
        q.bindValue(":st", statusStr);
        q.bindValue(":price", finalPrice);
        success = q.exec();
    } else {
        q.prepare("UPDATE bookings SET user_id = :uid, flight_id = :fid, price = :price, seat_row = :row, seat_col = :col, status = :st WHERE id = :oid");
        q.bindValue(":uid", userId);
        q.bindValue(":fid", flightId);
        q.bindValue(":price", finalPrice);
        q.bindValue(":row", seatRow);
        q.bindValue(":col", seatCol);
        q.bindValue(":st", statusStr);
        q.bindValue(":oid", m_editingOrderId);
        success = q.exec();
    }

    db.close();
    if (success) {
        QMessageBox::information(this, "成功", "操作成功");
        resetOrderForm();
        loadAllOrders();
    } else {
        QMessageBox::warning(this, "失败", "数据库错误");
    }
}

void AdminWindow::performEditOrder(int row) {
    m_editingOrderId = ui->m_allOrdersTable->item(row, 0)->text().toInt();
    ui->m_ordInputUser->setText(ui->m_allOrdersTable->item(row, 1)->text());

    QSqlDatabase db = DbManager::getConn();
    if (db.open()) {
        QSqlQuery q(db);
        q.prepare("SELECT f.flight_code FROM bookings b JOIN flights f ON b.flight_id=f.id WHERE b.id=:id");
        q.bindValue(":id", m_editingOrderId);
        if (q.exec() && q.next()) {
            ui->m_ordInputFlight->setText(q.value(0).toString());
        }
        db.close();
    }

    QString seatStr = ui->m_allOrdersTable->item(row, 4)->text();
    if (seatStr == "未选座" || seatStr.contains("-1")) {
        ui->m_ordInputRow->clear();
        ui->m_ordInputCol->setCurrentIndex(0);
    } else {
        QString r = seatStr.split("排").first();
        ui->m_ordInputRow->setText(r);
        if (!seatStr.isEmpty()) {
            QChar c = seatStr.back();
            int colIdx = c.unicode() - 'A';
            if (colIdx >= 0 && colIdx < 6) ui->m_ordInputCol->setCurrentIndex(colIdx);
        }
    }

    ui->m_ordInputPrice->setText(ui->m_allOrdersTable->item(row, 5)->text().remove("¥"));

    QString stText = ui->m_allOrdersTable->item(row, 6)->text();
    if (stText == "待支付") ui->m_ordInputStatus->setCurrentIndex(0);
    else if (stText == "已支付" || stText == "已值机") ui->m_ordInputStatus->setCurrentIndex(1);
    else ui->m_ordInputStatus->setCurrentIndex(2);

    ui->m_btnCommitOrder->setText("保存修改");
    ui->m_btnCancelOrder->setVisible(true);
}

void AdminWindow::performDeleteOrder(QString id) {
    if (QMessageBox::question(this, "确认", "确定删除此订单吗？") == QMessageBox::Yes) {
        QSqlDatabase db = DbManager::getConn();
        if (db.open()) {
            QSqlQuery q(db);
            q.prepare("DELETE FROM bookings WHERE id=:id");
            q.bindValue(":id", id);
            q.exec();
            db.close();
        }
        loadAllOrders();
    }
}

void AdminWindow::resetOrderForm() {
    m_editingOrderId = -1;
    ui->m_ordInputUser->clear();
    ui->m_ordInputFlight->clear();
    ui->m_ordInputRow->clear();
    ui->m_ordInputPrice->clear();
    ui->m_btnCommitOrder->setText("创建订单");
    ui->m_btnCancelOrder->setVisible(false);
}

void AdminWindow::onCancelOrderEdit() {
    resetOrderForm();
}

// ==========================================
// 用户管理模块
// ==========================================

void AdminWindow::loadUsers() {
    QSqlDatabase db = DbManager::getConn();
    if (!db.open()) return;

    QSqlQuery q(db);
    q.exec("SELECT id, username, real_name, balance, is_admin FROM users ORDER BY id DESC");

    ui->m_userTable->setRowCount(0);
    int row = 0;
    while (q.next()) {
        ui->m_userTable->insertRow(row);
        ui->m_userTable->setRowHeight(row, 45);

        auto newItem = [](QString t) {
            QTableWidgetItem* i = new QTableWidgetItem(t);
            i->setTextAlignment(Qt::AlignCenter);
            return i;
        };

        ui->m_userTable->setItem(row, 0, newItem(q.value(0).toString()));
        ui->m_userTable->setItem(row, 1, newItem(q.value(1).toString()));
        ui->m_userTable->setItem(row, 2, newItem(q.value(2).toString()));
        ui->m_userTable->setItem(row, 3, newItem("¥" + q.value(3).toString()));
        ui->m_userTable->setItem(row, 4, newItem(q.value(4).toBool() ? "管理员" : "用户"));
        row++;
    }
    db.close();
}

void AdminWindow::onCommitUser() {
    QString name = ui->m_userInputName->text();
    QString pass = ui->m_userInputPass->text();

    if (name.isEmpty()) return;

    QSqlDatabase db = DbManager::getConn();
    if (db.open()) {
        QSqlQuery q(db);
        if (m_editingUserId == -1) {
            q.prepare("INSERT INTO users (username, password, real_name, balance, is_admin) VALUES (:u, :p, :r, :b, :a)");
            q.bindValue(":p", pass);
        } else {
            q.prepare("UPDATE users SET username=:u, real_name=:r, balance=:b, is_admin=:a WHERE id=:id");
            q.bindValue(":id", m_editingUserId);
        }

        q.bindValue(":u", name);
        q.bindValue(":r", ui->m_userInputRealName->text());
        q.bindValue(":b", ui->m_userInputBalance->text().toDouble());
        q.bindValue(":a", ui->m_userInputRole->currentIndex());
        q.exec();

        loadUsers();
        resetUserForm();
        db.close();
    }
}

void AdminWindow::performEditUser(int row) {
    m_editingUserId = ui->m_userTable->item(row, 0)->text().toInt();
    ui->m_userInputName->setText(ui->m_userTable->item(row, 1)->text());
    ui->m_userInputRealName->setText(ui->m_userTable->item(row, 2)->text());
    ui->m_userInputBalance->setText(ui->m_userTable->item(row, 3)->text().remove("¥"));
    ui->m_btnCommitUser->setText("保存修改");
    ui->m_btnCancelUser->setVisible(true);
}

void AdminWindow::performDeleteUser(QString id) {
    if (QMessageBox::question(this, "确认", "确定删除该用户？") == QMessageBox::Yes) {
        QSqlDatabase db = DbManager::getConn();
        if (db.open()) {
            QSqlQuery q(db);
            q.prepare("DELETE FROM users WHERE id=:id");
            q.bindValue(":id", id);
            q.exec();
            db.close();
        }
        loadUsers();
    }
}

void AdminWindow::resetUserForm() {
    m_editingUserId = -1;
    ui->m_userInputName->clear();
    ui->m_userInputPass->clear();
    ui->m_userInputRealName->clear();
    ui->m_userInputBalance->clear();
    ui->m_btnCommitUser->setText("添加用户");
    ui->m_btnCancelUser->setVisible(false);
}

void AdminWindow::onCancelUserEdit() {
    resetUserForm();
}
