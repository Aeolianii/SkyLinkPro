#ifndef ADMIN_WINDOW_H
#define ADMIN_WINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QTableWidget>
#include <QMouseEvent>

// 前向声明 UI 类
namespace Ui {
class AdminWindow;
}

class AdminWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit AdminWindow(QWidget *parent = nullptr);
    ~AdminWindow();

protected:
    // 窗口拖拽保持不变
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void onNavClicked(int index);

    // 队友的：右键菜单
    void onTableContextMenu(const QPoint &pos);

    // 提交与取消
    void onCommitFlight();
    void onCancelFlightEdit();

    void onCommitOrder();
    void onCancelOrderEdit();

    void onCommitUser();
    void onCancelUserEdit();

private:
    // 初始化函数（现在只需要初始化逻辑，不需要初始化UI）
    void initLogic();
    void loadFlights();
    void loadAllOrders();
    void loadUsers();

    // 编辑逻辑辅助函数
    void performEditFlight(int row);
    void performDeleteFlight(QString id);
    void performEditOrder(int row);
    void performDeleteOrder(QString id);
    void performEditUser(int row);
    void performDeleteUser(QString id);

    // 重置表单
    void resetFlightForm();
    void resetOrderForm();
    void resetUserForm();

private:
    Ui::AdminWindow *ui; // 指向 UI 文件

    QPoint m_dragPosition;
    QButtonGroup* m_navGroup; // 保留按钮组用于逻辑控制

    // 状态变量（保持不变）
    int m_editingFlightId = -1;
    int m_editingOrderId = -1;
    int m_editingUserId = -1;
};

#endif // ADMIN_WINDOW_H
