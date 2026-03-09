#ifndef RESCHEDULE_DIALOG_H
#define RESCHEDULE_DIALOG_H

#include <QDialog>
#include <QMouseEvent>
#include <QPoint>
#include "db_conn.h"

// 前向声明 UI 类
namespace Ui {
class RescheduleDialog;
}

class RescheduleDialog : public QDialog {
    Q_OBJECT

public:
    RescheduleDialog(QString origin, QString dest, int oldOrderId, double oldPrice, UserInfo user, QWidget *parent = nullptr);
    ~RescheduleDialog();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void onSearch(); // 搜索按钮或日期变更槽函数
    // 确认改签的内部逻辑，不需要作为 slots 暴露给外部，但在 lambda 中调用
    void onConfirmChange(QString newFlightId, double newPrice, QString flightCode);

private:
    Ui::RescheduleDialog *ui; // UI 指针

    void initTableConfig(); // 初始化表格列宽
    bool verifyPassword();

    QString m_origin;
    QString m_dest;
    int m_oldOrderId;
    double m_oldPrice;
    UserInfo m_user;

    QPoint m_dragPosition;
};

#endif // RESCHEDULE_DIALOG_H
