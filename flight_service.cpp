#include "flight_service.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QSet>

// 订单号生成
QString FlightService::generateOrderNumber(const QString& flightCode, const QDateTime& depTime) {
    QString dateStr = QDate::currentDate().toString("yyyyMMdd");
    QString timeStr = depTime.toString("HHmm");
    int randomNum = QRandomGenerator::global()->bounded(1000, 9999);
    return QString("%1-%2-%3-%4").arg(dateStr, flightCode, timeStr, QString::number(randomNum));
}

// 航班查询
QList<QVariantMap> FlightService::searchFlights(const QString& from, const QString& to, const QDate& date) {
    QList<QVariantMap> list;
    QSqlDatabase db = DbManager::getConn();
    if (!db.open()) return list;

    QSqlQuery q(db);
    // 增加 status 字段查询
    QString sql = "SELECT id, flight_code, airline, dep_time, arr_time, model, price, origin, destination, status FROM flights WHERE DATE(dep_time) = :dt";
    if (!from.isEmpty()) sql += " AND origin LIKE :f";
    if (!to.isEmpty()) sql += " AND destination LIKE :t";
    sql += " ORDER BY dep_time ASC";

    q.prepare(sql);
    q.bindValue(":dt", date.toString("yyyy-MM-dd"));
    if (!from.isEmpty()) q.bindValue(":f", "%" + from + "%");
    if (!to.isEmpty()) q.bindValue(":t", "%" + to + "%");
    q.exec();

    while (q.next()) {
        QVariantMap map;
        map["id"] = q.value("id");
        map["flight_code"] = q.value("flight_code");
        map["airline"] = q.value("airline");
        map["dep_time"] = q.value("dep_time");
        map["arr_time"] = q.value("arr_time");
        map["model"] = q.value("model");
        map["price"] = q.value("price");
        map["origin"] = q.value("origin");
        map["destination"] = q.value("destination");
        map["status"] = q.value("status"); // 读取数据库状态
        list.append(map);
    }
    db.close();
    return list;
}

// 获取最低价
double FlightService::getMinPrice(const QString& from, const QString& to, const QDate& date) {
    double minPrice = -1.0;
    QSqlDatabase db = DbManager::getConn();
    if (db.open()) {
        QSqlQuery q(db);
        q.prepare("SELECT MIN(price) FROM flights WHERE origin LIKE :from AND destination LIKE :to AND DATE(dep_time) = :date");
        q.bindValue(":from", "%" + from + "%");
        q.bindValue(":to", "%" + to + "%");
        q.bindValue(":date", date.toString("yyyy-MM-dd"));
        if (q.exec() && q.next()) {
            double p = q.value(0).toDouble();
            if (p > 0.01) minPrice = p;
        }
        db.close();
    }
    QList<QVariantMap> transits = searchTransitFlights(from, to, date);
    for (const auto& flight : transits) {
        double transitPrice = flight["price"].toDouble();
        if (minPrice < 0 || transitPrice < minPrice) minPrice = transitPrice;
    }
    return (minPrice < 0) ? 0.0 : minPrice;
}

// 热门推荐
QList<QVariantMap> FlightService::getHotRecommendations() {
    QList<QVariantMap> list;
    QSqlDatabase db = DbManager::getConn();
    if (!db.open()) return list;
    QSqlQuery q(db);
    q.exec("SELECT id, flight_code, origin, destination, dep_time, price FROM flights ORDER BY price ASC LIMIT 3");
    while(q.next()) {
        QVariantMap map;
        map["id"] = q.value("id");
        map["flight_code"] = q.value("flight_code");
        map["origin"] = q.value("origin");
        map["destination"] = q.value("destination");
        map["dep_time"] = q.value("dep_time");
        map["price"] = q.value("price");
        list.append(map);
    }
    db.close();
    return list;
}

// 获取用户订单
QList<OrderDTO> FlightService::getUserOrders(int userId) {
    QList<OrderDTO> list;
    QSqlDatabase db = DbManager::getConn();
    if (!db.open()) return list;

    QString sql = "SELECT b.id, b.order_number, b.flight_id, f.flight_code, f.airline, f.origin, f.destination, f.dep_time, "
                  "b.seat_row, b.seat_col, b.price, b.booking_time, b.status, b.passenger_name "
                  "FROM bookings b JOIN flights f ON b.flight_id = f.id "
                  "WHERE b.user_id = :uid ORDER BY b.id DESC";

    QSqlQuery q(db);
    q.prepare(sql);
    q.bindValue(":uid", userId);
    q.exec();

    while (q.next()) {
        OrderDTO item;
        item.id = q.value("id").toInt();
        item.orderNumber = q.value("order_number").toString();
        item.flightId = q.value("flight_id").toInt();
        item.flightCode = q.value("flight_code").toString();
        item.airline = q.value("airline").toString();
        item.route = q.value("origin").toString() + " ➝ " + q.value("destination").toString();
        item.depTime = q.value("dep_time").toDateTime();
        item.price = q.value("price").toDouble();
        item.passengerName = q.value("passenger_name").toString();
        item.seatRow = q.value("seat_row").toInt();
        item.seatCol = q.value("seat_col").toInt();
        item.status = q.value("status").toString();
        list.append(item);
    }
    db.close();
    return list;
}

// 中转搜索
QList<QVariantMap> FlightService::searchTransitFlights(const QString& from, const QString& to, const QDate& date) {
    QList<QVariantMap> results;
    QSqlDatabase db = DbManager::getConn();
    if (!db.open()) return results;

    QList<QVariantMap> listA;
    QSqlQuery qA(db);
    qA.prepare("SELECT * FROM flights WHERE origin LIKE :o AND DATE(dep_time) = :dt");
    qA.bindValue(":o", from + "%");
    qA.bindValue(":dt", date.toString("yyyy-MM-dd"));
    if (qA.exec()) {
        while (qA.next()) {
            QVariantMap f;
            f["id"] = qA.value("id");
            f["code"] = qA.value("flight_code");
            f["airline"] = qA.value("airline");
            f["origin"] = qA.value("origin");
            f["dest"] = qA.value("destination");
            f["dep"] = qA.value("dep_time").toDateTime();
            f["arr"] = qA.value("arr_time").toDateTime();
            f["price"] = qA.value("price").toDouble();
            f["model"] = qA.value("model");
            listA.append(f);
        }
    }

    QList<QVariantMap> listB;
    QSqlQuery qB(db);
    qB.prepare("SELECT * FROM flights WHERE destination LIKE :d AND (DATE(dep_time) = :dt1 OR DATE(dep_time) = :dt2)");
    qB.bindValue(":d", to + "%");
    qB.bindValue(":dt1", date.toString("yyyy-MM-dd"));
    qB.bindValue(":dt2", date.addDays(1).toString("yyyy-MM-dd"));

    if (qB.exec()) {
        while (qB.next()) {
            QVariantMap f;
            f["id"] = qB.value("id");
            f["code"] = qB.value("flight_code");
            f["airline"] = qB.value("airline");
            f["origin"] = qB.value("origin");
            f["dest"] = qB.value("destination");
            f["dep"] = qB.value("dep_time").toDateTime();
            f["arr"] = qB.value("arr_time").toDateTime();
            f["price"] = qB.value("price").toDouble();
            f["model"] = qB.value("model");
            listB.append(f);
        }
    }

    for (const auto& leg1 : listA) {
        for (const auto& leg2 : listB) {
            QString transitCity1 = leg1["dest"].toString().trimmed();
            QString transitCity2 = leg2["origin"].toString().trimmed();

            if (transitCity1 == transitCity2) {
                QDateTime arr1 = leg1["arr"].toDateTime();
                QDateTime dep2 = leg2["dep"].toDateTime();
                qint64 secs = arr1.secsTo(dep2);

                if (secs >= 7200 && secs <= 86400) {
                    QVariantMap trans;
                    trans["is_transit"] = true;
                    trans["id"] = leg1["id"].toString() + "_" + leg2["id"].toString();
                    trans["flight_code"] = leg1["code"].toString() + " + " + leg2["code"].toString();
                    trans["airline"] = leg1["airline"].toString() + " / " + leg2["airline"].toString();
                    trans["origin"] = leg1["origin"];
                    trans["destination"] = leg2["dest"];
                    trans["via_city"] = leg1["dest"];
                    trans["dep_time"] = leg1["dep"];
                    trans["arr_time"] = leg2["arr"];
                    trans["price"] = leg1["price"].toDouble() + leg2["price"].toDouble();
                    trans["model"] = leg1["model"].toString() + " / " + leg2["model"].toString();
                    results.append(trans);
                }
            }
        }
    }
    db.close();
    return results;
}

// 密码验证
bool FlightService::verifyPassword(int userId, const QString& password) {
    QSqlDatabase db = DbManager::getConn();
    if (!db.open()) return false;
    QSqlQuery q(db);
    q.prepare("SELECT id FROM users WHERE id = :uid AND password = :pwd");
    q.bindValue(":uid", userId); q.bindValue(":pwd", password);
    bool ok = (q.exec() && q.next());
    db.close(); return ok;
}

// 查找用户
int FlightService::findUserByName(const QString& username) {
    int id = -1; QSqlDatabase db = DbManager::getConn();
    if (db.open()) {
        QSqlQuery q(db); q.prepare("SELECT id FROM users WHERE username = :u"); q.bindValue(":u", username);
        if (q.exec() && q.next()) id = q.value(0).toInt();
        db.close();
    } return id;
}

// 预订直飞
bool FlightService::bookFlight(int userId, int passengerId, QString passengerName, int flightId, double price, QString orderNumber, QString& errorMsg) {
    QSqlDatabase db = DbManager::getConn(true);
    if (!db.open()) { errorMsg = "数据库连接失败"; return false; }
    if (!db.transaction()) { errorMsg = "事务启动失败"; return false; }

    bool ok = true;
    QSqlQuery q(db);

    if(ok) {
        q.prepare("SELECT balance FROM users WHERE id = :uid");
        q.bindValue(":uid", userId);
        if(q.exec() && q.next()) {
            if(q.value(0).toDouble() < price) { ok = false; errorMsg = "余额不足"; }
        } else { ok = false; errorMsg = "用户查询失败"; }
    }

    if(ok) {
        q.prepare("INSERT INTO bookings (order_number, user_id, passenger_id, passenger_name, flight_id, seat_row, seat_col, status, price, booking_time) "
                  "VALUES (:ord, :uid, :pid, :pname, :fid, -1, -1, 'Paid', :price, NOW())");
        q.bindValue(":ord", orderNumber);
        q.bindValue(":uid", userId);
        q.bindValue(":pid", passengerId);
        q.bindValue(":pname", passengerName);
        q.bindValue(":fid", flightId);
        q.bindValue(":price", price);
        if (!q.exec()) { ok = false; errorMsg = "订单创建失败: " + q.lastError().text(); }
    }

    if(ok) {
        q.prepare("UPDATE users SET balance = balance - :price WHERE id = :uid");
        q.bindValue(":price", price);
        q.bindValue(":uid", userId);
        if (!q.exec()) { ok = false; errorMsg = "扣款失败"; }
    }

    if (ok) db.commit(); else db.rollback();
    DbManager::removeConn(db);
    return ok;
}

// 预订中转
bool FlightService::bookTransitFlight(int userId, int passengerId, QString passengerName, int flightId1, int flightId2, double totalPrice, QString orderNum1, QString orderNum2, QString& errorMsg) {
    QSqlDatabase db = DbManager::getConn(true);
    if (!db.open()) { errorMsg = "数据库连接失败"; return false; }
    if (!db.transaction()) { errorMsg = "事务启动失败"; return false; }

    bool ok = true;
    QSqlQuery q(db);

    double price1 = 0, price2 = 0;
    if (ok) {
        QSqlQuery pQ(db); pQ.prepare("SELECT id, price FROM flights WHERE id IN (:id1, :id2)");
        pQ.bindValue(":id1", flightId1); pQ.bindValue(":id2", flightId2);
        if(pQ.exec()) {
            while(pQ.next()) {
                if(pQ.value("id").toInt() == flightId1) price1 = pQ.value("price").toDouble();
                if(pQ.value("id").toInt() == flightId2) price2 = pQ.value("price").toDouble();
            }
        } else { ok = false; errorMsg = "获取航班价格失败"; }
    }

    if(ok) {
        q.prepare("SELECT balance FROM users WHERE id = :uid");
        q.bindValue(":uid", userId);
        if(q.exec() && q.next()) {
            if(q.value(0).toDouble() < totalPrice) { ok = false; errorMsg = "余额不足"; }
        } else { ok = false; errorMsg = "用户查询失败"; }
    }

    if(ok) {
        q.prepare("INSERT INTO bookings (order_number, user_id, passenger_id, passenger_name, flight_id, seat_row, seat_col, status, price, booking_time) VALUES (:ord, :uid, :pid, :pname, :fid, -1, -1, 'Paid', :price, NOW())");
        q.bindValue(":ord", orderNum1); q.bindValue(":uid", userId); q.bindValue(":pid", passengerId); q.bindValue(":pname", passengerName);
        q.bindValue(":fid", flightId1); q.bindValue(":price", price1);
        if(!q.exec()) ok = false;

        q.bindValue(":ord", orderNum2);
        q.bindValue(":fid", flightId2); q.bindValue(":price", price2);
        if(!q.exec()) ok = false;
    }

    if(ok) {
        q.prepare("UPDATE users SET balance = balance - :total WHERE id = :uid");
        q.bindValue(":total", price1 + price2);
        q.bindValue(":uid", userId);
        if(!q.exec()) ok = false;
    }

    if (ok) db.commit(); else { db.rollback(); if(errorMsg.isEmpty()) errorMsg = "数据库写入失败"; }
    DbManager::removeConn(db);
    return ok;
}

// === FlightsStatus 自动计算 ===
QList<QVariantMap> FlightService::getFlightStatus(const QString& keyword) {
    QList<QVariantMap> list;
    QSqlDatabase db = DbManager::getConn();
    if (!db.open()) return list;

    QSqlQuery q(db);
    // 查询 status 字段
    QString sql = "SELECT flight_code, origin, destination, dep_time, arr_time, status FROM flights WHERE 1=1";

    QDateTime now = QDateTime::currentDateTime();
    if (keyword.isEmpty()) {
        sql += " AND dep_time >= :minTime AND dep_time <= :maxTime";
    } else {
        sql += " AND flight_code LIKE :k";
    }
    sql += " ORDER BY dep_time ASC LIMIT 50";

    q.prepare(sql);

    if (keyword.isEmpty()) {
        q.bindValue(":minTime", now.addDays(-1));
        q.bindValue(":maxTime", now.addDays(2));
    } else {
        q.bindValue(":k", "%" + keyword + "%");
    }

    if (q.exec()) {
        while (q.next()) {
            QVariantMap map;
            map["flight_code"] = q.value(0);
            QString route = q.value(1).toString() + " ➝ " + q.value(2).toString();
            map["route"] = route;

            QDateTime dep = q.value(3).toDateTime();
            QDateTime arr = q.value(4).toDateTime();
            map["dep_time"] = dep;
            map["arr_time"] = arr;

            // 读取数据库中的原始状态
            QString dbStatus = q.value(5).toString().toUpper();
            // 如果为空，默认为 ON TIME
            if (dbStatus.isEmpty()) dbStatus = "ON TIME";

            QString finalStatus = dbStatus;

            // 如果管理员没标记延误/取消，则根据时间自动计算
            if (dbStatus == "ON TIME") {
                if (now >= arr) {
                    finalStatus = "ARRIVED";
                } else if (now >= dep) {
                    finalStatus = "FLYING";
                } else if (now >= dep.addSecs(-40 * 60)) {
                    // 起飞前 40 分钟 -> Boarding
                    finalStatus = "BOARDING";
                } else {
                    finalStatus = "ON TIME";
                }
            } else {
                // 如果是 DELAYED 或 CANCELLED，则以数据库为准
                finalStatus = dbStatus;
            }

            map["status"] = finalStatus;
            list.append(map);
        }
    }
    db.close();
    return list;
}
