#ifndef ORDER_DETAIL_DIALOG_H
#define ORDER_DETAIL_DIALOG_H

#include <QDialog>
#include <QMouseEvent>
#include <QPoint>
#include <QDateTime>
#include "db_conn.h"

// 前向声明 UI 类
namespace Ui {
class OrderDetailDialog;
}

class OrderDetailDialog : public QDialog {
    Q_OBJECT

public:
    explicit OrderDetailDialog(int orderId, const UserInfo& user, QWidget *parent = nullptr);
    ~OrderDetailDialog();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

signals:
    void orderUpdated();

private slots:
    void onRefundClicked();
    void onRescheduleClicked();

private:
    Ui::OrderDetailDialog *ui; // UI 指针

    void updateUI(); // 用于刷新界面数据的函数
    void loadData();

    int m_orderId;
    UserInfo m_user;

    // 数据缓存
    int m_flightId = 0;
    QString m_flightCode;
    QString m_origin;
    QString m_dest;
    QDateTime m_depTime;
    double m_paidPrice = 0.0;
    QString m_status;
    int m_isRescheduled = 0;
    int m_seatRow = -1;
    int m_seatCol = -1;

    QPoint m_dragPosition;
};

#endif // ORDER_DETAIL_DIALOG_H
