#ifndef PASSENGER_CONFIRM_DIALOG_H
#define PASSENGER_CONFIRM_DIALOG_H

#include <QDialog>
#include "db_conn.h"

namespace Ui {
class PassengerConfirmDialog;
}

class PassengerConfirmDialog : public QDialog {
    Q_OBJECT
public:
    explicit PassengerConfirmDialog(const UserInfo& user, const QString& orderNumber, QWidget* parent = nullptr);
    ~PassengerConfirmDialog();

    int getPassengerId() const { return m_finalPassengerId; }
    QString getPassengerName() const { return m_finalPassengerName; }

private slots:
    void onConfirm();

private:
    Ui::PassengerConfirmDialog *ui;
    UserInfo m_currentUser;
    int m_finalPassengerId = -1;
    QString m_finalPassengerName;
    // 保存订单号
    QString m_orderNumber;
};

#endif // PASSENGER_CONFIRM_DIALOG_H
