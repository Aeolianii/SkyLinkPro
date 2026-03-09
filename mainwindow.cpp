#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "city_selector.h"
#include "order_detail_dialog.h"
#include "checkin_dialog.h"
#include "flight_service.h"
#include "passenger_confirm_dialog.h"
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QDate>

MainWindow::MainWindow(const UserInfo& user, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_user(user)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_TranslucentBackground);
    initUI();
    setupConnections();
    m_navGroup->button(0)->click();
    ui->dateEdit->setDate(QDate::currentDate());
    initHotRecommendations();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::mousePressEvent(QMouseEvent *event) { if (event->button() == Qt::LeftButton) { m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft(); event->accept(); } }
void MainWindow::mouseMoveEvent(QMouseEvent *event) { if (event->buttons() & Qt::LeftButton) { move(event->globalPosition().toPoint() - m_dragPosition); event->accept(); } }

void MainWindow::initUI() {
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(25); shadow->setColor(QColor(0, 0, 0, 60)); shadow->setOffset(0, 8);
    ui->containerMain->setGraphicsEffect(shadow);
    ui->lblWelcome->setText(QString("欢迎回来，%1").arg(m_user.realName));
    updateBalanceUI();
    m_navGroup = new QButtonGroup(this); m_navGroup->setExclusive(true);
    m_navGroup->addButton(ui->btnNavSearch, 0); m_navGroup->addButton(ui->btnNavOrder, 1); m_navGroup->addButton(ui->btnNavStatus, 2);
    m_recLayout = new QHBoxLayout(ui->recommendContainer); m_recLayout->setSpacing(20); m_recLayout->setContentsMargins(0, 0, 0, 0);
    m_dateBarLayout = new QHBoxLayout(ui->dateBarContainer); m_dateBarLayout->setContentsMargins(0, 0, 0, 0);

    // 设置列宽自动拉伸
    ui->tableFlightList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableFlightList->setColumnHidden(0, true);

    ui->tableOrder->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableOrder->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableOrder->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);

    // 确保所有表格表头文字强制居中
    ui->tableStatus->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableStatus->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->tableFlightList->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->tableOrder->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
}

void MainWindow::setupConnections() {
    connect(ui->btnMin, &QPushButton::clicked, this, &QMainWindow::showMinimized);
    connect(ui->btnClose, &QPushButton::clicked, this, &QMainWindow::close);
    connect(m_navGroup, &QButtonGroup::idClicked, this, &MainWindow::onNavClicked);
    connect(ui->leOrigin, &QLineEdit::selectionChanged, this, &MainWindow::onSelectOrigin);
    connect(ui->btnSelectOrigin, &QPushButton::clicked, this, &MainWindow::onSelectOrigin);
    connect(ui->leDest, &QLineEdit::selectionChanged, this, &MainWindow::onSelectDest);
    connect(ui->btnSelectDest, &QPushButton::clicked, this, &MainWindow::onSelectDest);
    connect(ui->btnSwapCity, &QPushButton::clicked, this, &MainWindow::onSwapCity);
    connect(ui->btnDoSearch, &QPushButton::clicked, this, &MainWindow::onSearchFlights);
    connect(ui->btnBackToSearch, &QPushButton::clicked, this, &MainWindow::onBackToSearch);
    connect(ui->btnRefreshOrder, &QPushButton::clicked, this, &MainWindow::loadOrderList);
    connect(ui->btnStatusSearch, &QPushButton::clicked, this, &MainWindow::onSearchStatus);
    connect(ui->leStatusSearch, &QLineEdit::returnPressed, this, &MainWindow::onSearchStatus);
}
void MainWindow::onNavClicked(int index) { ui->mainStack->setCurrentIndex(index); if (index == 1) loadOrderList(); if (index == 2) onSearchStatus(); }
void MainWindow::onBackToSearch() { ui->searchStack->setCurrentIndex(0); }
void MainWindow::updateBalanceUI() { ui->lblUserBalance->setText(QString("👤 %1\n余额: ¥%2").arg(m_user.realName).arg(QString::number(m_user.balance, 'f', 2))); }
void MainWindow::onSelectOrigin() { CitySelector dlg(this); if(dlg.exec()) ui->leOrigin->setText(dlg.getSelectedCity()); }
void MainWindow::onSelectDest() { CitySelector dlg(this); if(dlg.exec()) ui->leDest->setText(dlg.getSelectedCity()); }
void MainWindow::onSwapCity() { QString t = ui->leOrigin->text(); ui->leOrigin->setText(ui->leDest->text()); ui->leDest->setText(t); }
void MainWindow::initHotRecommendations() {
    QList<QVariantMap> hots = FlightService::getHotRecommendations();
    for(const auto& h : hots) {
        QFrame* miniCard = new QFrame;
        miniCard->setStyleSheet("QFrame { background: white; border-radius: 12px; border: 1px solid #EDF2F7; } QFrame:hover { border: 1px solid #0086F6; }");
        miniCard->setFixedSize(260, 150);
        QVBoxLayout* vLay = new QVBoxLayout(miniCard);
        QString routeStr = h["origin"].toString() + " ➝ " + h["destination"].toString();
        QLabel* lbRoute = new QLabel(routeStr); lbRoute->setStyleSheet("font-size: 16px; font-weight: bold; color: #2D3748; border:none;"); lbRoute->setAlignment(Qt::AlignCenter);
        QLabel* lbPrice = new QLabel("¥" + h["price"].toString()); lbPrice->setStyleSheet("color: #E53E3E; font-size: 24px; font-weight: bold; border:none;"); lbPrice->setAlignment(Qt::AlignCenter);
        QPushButton* btnGrab = new QPushButton("特惠抢票 >"); btnGrab->setCursor(Qt::PointingHandCursor); btnGrab->setStyleSheet("background-color: #FFF5F5; color: #E53E3E; border: none; border-radius: 15px; height: 30px; font-weight: bold;");
        QString fId = h["id"].toString(); QString fCode = h["flight_code"].toString(); double fPrice = h["price"].toDouble(); QDateTime fDep = h["dep_time"].toDateTime();
        connect(btnGrab, &QPushButton::clicked, [=](){ onPreBookClicked(fId, fCode, fPrice, fDep); });
        vLay->addWidget(lbRoute); vLay->addWidget(lbPrice); vLay->addWidget(btnGrab); m_recLayout->addWidget(miniCard);
    }
    m_recLayout->addStretch();
}

void MainWindow::onSearchFlights() {
    QString from = ui->leOrigin->text().trimmed();
    QString to = ui->leDest->text().trimmed();

    // 1. 定义一个清洗函数，用于去除地名后缀，使之与数据库中的简写匹配
    auto cleanCityName = [](QString name) -> QString {
        // 先判断较长的后缀 "特别行政区" (例如: 香港特别行政区 -> 香港)
        if (name.endsWith("特别行政区")) {
            return name.left(name.length() - 5); // 5是"特别行政区"的长度
        }
        // 再判断 "市" (例如: 北京市 -> 北京)
        if (name.endsWith("市")) {
            return name.left(name.length() - 1);
        }
        // 如果需要，这里还可以加 "省"、"自治区" 等判断
        return name;
    };

    // 2. 清洗输入
    from = cleanCityName(from);
    to = cleanCityName(to);

    // 3. 校验
    if (from.isEmpty() || to.isEmpty()) {
        QMessageBox::warning(this, "提示", "请选择城市");
        return;
    }

    // 4. 执行搜索
    ui->searchStack->setCurrentIndex(1);
    bool allowTransit = ui->chkTransit->isChecked();

    // 传入清洗后的 from 和 to
    loadFlightList(from, to, ui->dateEdit->date(), allowTransit);
}

void MainWindow::loadFlightList(QString from, QString to, const QDate& date, bool allowTransit) {
    updateDateBar(from, to, date);
    QList<QVariantMap> flights = FlightService::searchFlights(from, to, date);
    if (allowTransit) { QList<QVariantMap> transits = FlightService::searchTransitFlights(from, to, date); flights.append(transits); }
    ui->tableFlightList->setRowCount(0);
    for (int i = 0; i < flights.size(); ++i) {
        ui->tableFlightList->insertRow(i); ui->tableFlightList->setRowHeight(i, 80);
        QVariantMap f = flights[i];
        QString fId = f["id"].toString(); QString fCode = f["flight_code"].toString(); QString fAirline = f["airline"].toString(); double price = f["price"].toDouble(); QDateTime dep = f["dep_time"].toDateTime(); QDateTime arr = f["arr_time"].toDateTime(); bool isTransit = f.contains("is_transit") && f["is_transit"].toBool();
        auto newItem = [](QString t) { QTableWidgetItem* i = new QTableWidgetItem(t); i->setTextAlignment(Qt::AlignCenter); return i; };
        ui->tableFlightList->setItem(i, 0, newItem(fId));
        QString codeDisplay = fCode + "\n" + fAirline; if(isTransit) { codeDisplay = fCode + "\n(中转)"; if (!fAirline.isEmpty()) codeDisplay += "\n" + fAirline; }
        QTableWidgetItem* iCode = newItem(codeDisplay); iCode->setFont(QFont("Arial", 10, QFont::Bold)); if(isTransit) iCode->setForeground(QColor("#FF9900")); else iCode->setForeground(QColor("#333"));
        ui->tableFlightList->setItem(i, 1, iCode);
        QString timeStr = dep.toString("HH:mm") + " ➝ " + arr.toString("HH:mm"); if(isTransit) { qint64 mins = dep.secsTo(arr) / 60; timeStr += QString("\n全程 %1h%2m").arg(mins/60).arg(mins%60); }
        QTableWidgetItem* iTime = newItem(timeStr); iTime->setFont(QFont("Arial", 10)); ui->tableFlightList->setItem(i, 2, iTime);
        QString routeStr; if(isTransit) routeStr = f["origin"].toString() + " ➝ " + f["via_city"].toString() + " ➝ " + f["destination"].toString(); else routeStr = f["origin"].toString() + " ➝ " + f["destination"].toString();
        ui->tableFlightList->setItem(i, 3, newItem(routeStr)); ui->tableFlightList->setItem(i, 4, newItem(f["model"].toString()));
        QTableWidgetItem* pItem = newItem(QString("¥%1").arg(price)); pItem->setForeground(QColor("#E53E3E")); pItem->setFont(QFont("Arial", 14, QFont::Bold)); ui->tableFlightList->setItem(i, 5, pItem);
        QWidget* w = new QWidget; QHBoxLayout* hl = new QHBoxLayout(w); hl->setContentsMargins(0,0,0,0); hl->setAlignment(Qt::AlignCenter);
        QPushButton* btn = new QPushButton("预 订"); btn->setFixedSize(120, 36);
        if (QDateTime::currentDateTime() >= dep) { btn->setText("停止"); btn->setEnabled(false); btn->setStyleSheet("background-color: #EDF2F7; color: #A0AEC0; border: none; border-radius: 6px;"); } else { btn->setCursor(Qt::PointingHandCursor); btn->setStyleSheet("background-color: #0086F6; color: white; border-radius: 6px; font-weight: bold;"); connect(btn, &QPushButton::clicked, [=](){ onPreBookClicked(fId, fCode, price, dep); }); }
        hl->addWidget(btn); ui->tableFlightList->setCellWidget(i, 6, w);
    }
}
void MainWindow::updateDateBar(QString from, QString to, const QDate& centerDate) {
    QLayoutItem *item; while ((item = m_dateBarLayout->takeAt(0)) != nullptr) { delete item->widget(); delete item; }
    for (int i = -3; i <= 3; ++i) { QDate d = centerDate.addDays(i); double minPrice = FlightService::getMinPrice(from, to, d); QToolButton* btn = new QToolButton; btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); if (i == 0) btn->setStyleSheet("background: #0086F6; color: white; border: none; border-radius: 6px; font-weight: bold;"); else btn->setStyleSheet("background: white; color: #4A5568; border: 1px solid #E2E8F0; border-radius: 6px;"); QString priceStr = (minPrice > 0) ? QString("¥%1").arg(minPrice) : "--"; btn->setText(QString("%1\n%2").arg(d.toString("MM-dd")).arg(priceStr)); connect(btn, &QToolButton::clicked, [=](){ ui->dateEdit->setDate(d); loadFlightList(from, to, d, ui->chkTransit->isChecked()); }); m_dateBarLayout->addWidget(btn); }
}
void MainWindow::onPreBookClicked(const QString& flightId, const QString& flightCode, double price, const QDateTime& depTime) {
    if (m_user.balance < price) { QMessageBox::warning(this, "余额不足", "您的余额不足，请先充值。"); return; }
    QString displayOrderNum; QString orderNum1, orderNum2; bool isTransit = flightId.contains("_");
    if (isTransit) { QStringList codes = flightCode.split("+"); QString c1 = codes.value(0).trimmed(); QString c2 = (codes.size() > 1) ? codes.value(1).trimmed() : "FLT2"; orderNum1 = FlightService::generateOrderNumber(c1, depTime); orderNum2 = FlightService::generateOrderNumber(c2, depTime.addSecs(3600*3)); displayOrderNum = orderNum1 + "\n" + orderNum2; } else { displayOrderNum = FlightService::generateOrderNumber(flightCode, depTime); }
    PassengerConfirmDialog dlg(m_user, displayOrderNum, this);
    if (dlg.exec() == QDialog::Accepted) { int pid = dlg.getPassengerId(); QString pname = dlg.getPassengerName(); QString errorMsg; bool ok = false; if (isTransit) { QStringList parts = flightId.split("_"); int id1 = parts[0].toInt(); int id2 = parts[1].toInt(); ok = FlightService::bookTransitFlight(m_user.id, pid, pname, id1, id2, price, orderNum1, orderNum2, errorMsg); } else { ok = FlightService::bookFlight(m_user.id, pid, pname, flightId.toInt(), price, displayOrderNum, errorMsg); } if (ok) { m_user.balance -= price; updateBalanceUI(); QMessageBox::information(this, "成功", "预订成功！\n请前往订单页进行选座。"); ui->btnNavOrder->click(); } else { QMessageBox::critical(this, "预订失败", errorMsg); } }
}

void MainWindow::loadOrderList() {
    QList<OrderDTO> orders = FlightService::getUserOrders(m_user.id);
    ui->tableOrder->setRowCount(0);
    for (int i = 0; i < orders.size(); ++i) {
        ui->tableOrder->insertRow(i);
        ui->tableOrder->setRowHeight(i, 65);

        OrderDTO o = orders[i];
        bool isCancelled = (o.status == "Cancelled" || o.status == "Refunded");
        bool isDeparted = (QDateTime::currentDateTime() >= o.depTime);

        auto newItem = [](QString t) { QTableWidgetItem* x = new QTableWidgetItem(t); x->setTextAlignment(Qt::AlignCenter); return x; };
        ui->tableOrder->setItem(i, 0, newItem(o.orderNumber));
        ui->tableOrder->setItem(i, 1, newItem(o.passengerName));
        ui->tableOrder->setItem(i, 2, newItem(o.flightCode + "\n" + o.airline));
        ui->tableOrder->setItem(i, 3, newItem(o.route));
        ui->tableOrder->setItem(i, 4, newItem(o.depTime.toString("MM-dd HH:mm")));

        QString seatTxt;
        if (isCancelled) seatTxt = "已退票"; else if (o.seatRow == -1) seatTxt = "待选座"; else seatTxt = QString("%1排%2座").arg(o.seatRow).arg(QChar('A' + o.seatCol));
        QTableWidgetItem* seatItem = newItem(seatTxt);
        if (isCancelled) seatItem->setForeground(Qt::gray); else if (o.seatRow == -1) seatItem->setForeground(QColor("#ff9900"));
        ui->tableOrder->setItem(i, 5, seatItem);
        ui->tableOrder->setItem(i, 6, newItem("¥" + QString::number(o.price, 'f', 2)));

        QWidget* wCheck = new QWidget; QHBoxLayout* hlC = new QHBoxLayout(wCheck); hlC->setContentsMargins(0,0,0,0); hlC->setAlignment(Qt::AlignCenter);
        QPushButton* btnCheck = new QPushButton; btnCheck->setFixedSize(70, 28);
        if (isCancelled) {
            btnCheck->setText("失效"); btnCheck->setEnabled(false);
            btnCheck->setStyleSheet("background-color: #F5F5F5; color: #BDBDBD; border: none; border-radius: 4px; font-size: 12px;");
        } else if (isDeparted) {
            btnCheck->setText("已起飞"); btnCheck->setEnabled(false);
            btnCheck->setStyleSheet("background-color: #F5F5F5; color: #BDBDBD; border: none; border-radius: 4px; font-size: 12px;");
        } else if (o.seatRow != -1) {
            btnCheck->setText("已值机"); btnCheck->setEnabled(false);
            btnCheck->setStyleSheet("background-color: #E6F7FF; color: #0086F6; border: 1px solid #1890FF; border-radius: 4px; font-size: 12px; font-weight: bold;");
        } else {
            btnCheck->setText("选座"); btnCheck->setCursor(Qt::PointingHandCursor);
            btnCheck->setStyleSheet("background-color: #0086F6; color: white; border-radius: 4px; font-weight: bold; border: none; font-size: 12px;");
            connect(btnCheck, &QPushButton::clicked, [=](){ CheckInDialog dlg(o.flightId, o.id, this); if(dlg.exec() == QDialog::Accepted) loadOrderList(); });
        }
        hlC->addWidget(btnCheck); ui->tableOrder->setCellWidget(i, 7, wCheck);

        QWidget* wDetail = new QWidget; QHBoxLayout* hlD = new QHBoxLayout(wDetail); hlD->setContentsMargins(0,0,0,0); hlD->setAlignment(Qt::AlignCenter);
        QPushButton* btnDetail = new QPushButton("详情"); btnDetail->setFixedSize(60, 28);
        if (isCancelled) {
            btnDetail->setText("已退"); btnDetail->setEnabled(true);
            btnDetail->setStyleSheet("background-color: #F7FAFC; color: #A0AEC0; border: 1px solid #E2E8F0; border-radius: 4px; font-size: 12px;");
        } else if (isDeparted) {
            btnDetail->setText("结束"); btnDetail->setEnabled(false);
            btnDetail->setStyleSheet("background-color: #F7FAFC; color: #CBD5E0; border: 1px solid #E2E8F0; border-radius: 4px; font-size: 12px;");
        } else {
            btnDetail->setStyleSheet("QPushButton { background-color: white; border: 1px solid #E2E8F0; border-radius: 4px; color: #4A5568; font-size: 12px; } QPushButton:hover { border-color: #0086F6; color: #0086F6; }");
            btnDetail->setCursor(Qt::PointingHandCursor);
        }
        if (!isDeparted) { connect(btnDetail, &QPushButton::clicked, [=](){ OrderDetailDialog dlg(o.id, m_user, this); connect(&dlg, &OrderDetailDialog::orderUpdated, this, &MainWindow::loadOrderList); dlg.exec(); }); }
        hlD->addWidget(btnDetail); ui->tableOrder->setCellWidget(i, 8, wDetail);
    }
}

void MainWindow::onSearchStatus() {
    QString keyword = ui->leStatusSearch->text().trimmed();
    QList<QVariantMap> list = FlightService::getFlightStatus(keyword);
    ui->tableStatus->setRowCount(0);
    for (int i = 0; i < list.size(); ++i) {
        ui->tableStatus->insertRow(i); ui->tableStatus->setRowHeight(i, 60);
        QVariantMap m = list[i];
        QString status = m["status"].toString();
        QDateTime dep = m["dep_time"].toDateTime();
        QDateTime arr = m["arr_time"].toDateTime();

        auto newItem = [](QString t) {
            QTableWidgetItem* item = new QTableWidgetItem(t);
            item->setTextAlignment(Qt::AlignCenter);
            return item;
        };

        ui->tableStatus->setItem(i, 0, newItem(m["flight_code"].toString()));
        ui->tableStatus->setItem(i, 1, newItem(m["route"].toString()));
        ui->tableStatus->setItem(i, 2, newItem(dep.toString("MM-dd HH:mm")));
        ui->tableStatus->setItem(i, 3, newItem(arr.toString("MM-dd HH:mm")));

        QTableWidgetItem* sItem = newItem(status);
        sItem->setFont(QFont("Arial", 10, QFont::Bold));

        if (status == "ON TIME") {
            sItem->setForeground(QColor("#0086F6")); sItem->setText("准点");
        } else if (status == "BOARDING") {
            sItem->setForeground(QColor("#27AE60")); sItem->setText("正在登机");
        } else if (status == "FLYING") {
            sItem->setForeground(QColor("#FF9900")); sItem->setText("飞行中");
        } else if (status == "ARRIVED") {
            sItem->setForeground(QColor("#718096")); sItem->setText("已到达");
        } else if (status == "DELAYED") {
            sItem->setForeground(QColor("#E53E3E")); sItem->setText("延误");
        } else if (status == "CANCELLED") {
            sItem->setForeground(QColor("#A0AEC0")); sItem->setText("已取消");
        } else {
            sItem->setForeground(Qt::black);
        }
        ui->tableStatus->setItem(i, 4, sItem);
    }
}
