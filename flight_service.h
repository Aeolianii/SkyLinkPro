#ifndef FLIGHT_SERVICE_H
#define FLIGHT_SERVICE_H

#include <QString>
#include <QList>
#include <QDateTime>
#include <QVariantMap>
#include <QRandomGenerator>
#include "db_conn.h"

struct OrderDTO {
    int id;
    QString orderNumber;
    int flightId;
    QString flightCode;
    QString airline;
    QString route;
    QDateTime depTime;
    double price;
    QString passengerName;
    int seatRow;
    int seatCol;
    QString status;
};

class FlightService {
public:
    static QList<QVariantMap> searchFlights(const QString& from, const QString& to, const QDate& date);
    static QList<QVariantMap> searchTransitFlights(const QString& from, const QString& to, const QDate& date);
    static double getMinPrice(const QString& from, const QString& to, const QDate& date);
    static QList<OrderDTO> getUserOrders(int userId);
    static QList<QVariantMap> getHotRecommendations();
    static bool verifyPassword(int userId, const QString& password);
    static int findUserByName(const QString& username);
    static bool bookFlight(int userId, int passengerId, QString passengerName, int flightId, double price, QString orderNumber, QString& errorMsg);
    static bool bookTransitFlight(int userId, int passengerId, QString passengerName, int flightId1, int flightId2, double totalPrice, QString orderNum1, QString orderNum2, QString& errorMsg);
    static QList<QVariantMap> getFlightStatus(const QString& keyword);
    static QString generateOrderNumber(const QString& flightCode, const QDateTime& depTime);
};

#endif // FLIGHT_SERVICE_H
