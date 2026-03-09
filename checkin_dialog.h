#ifndef CHECKIN_DIALOG_H
#define CHECKIN_DIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QMap>
#include <QMouseEvent>

// 引入命名空间
namespace Ui {
class CheckInDialog;
}

class CheckInDialog : public QDialog
{
    Q_OBJECT

public:
    CheckInDialog(int flightId, int bookingId, QWidget *parent = nullptr);
    ~CheckInDialog(); // 必须析构 ui

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void onSeatClicked();
    void onConfirm();

private:
    void generateSeatGrid(); // 动态生成座位的函数
    void loadOccupiedSeats();

private:
    Ui::CheckInDialog *ui; // UI 指针

    int m_flightId;
    int m_bookingId;

    int m_selectedRow = -1;
    int m_selectedCol = -1;
    QString m_selectedSeatKey;

    // 依然需要 Map 来管理动态生成的按钮
    QMap<QString, QPushButton*> m_seatBtns;

    QPoint m_dragPosition;
};

#endif // CHECKIN_DIALOG_H
