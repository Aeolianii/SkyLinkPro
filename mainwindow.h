#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QButtonGroup>
#include <QPoint>
#include <QHBoxLayout>

#include "login_dialog.h"
#include "db_conn.h"

// 命名空间声明，对应 .ui 文件生成的类
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const UserInfo& user, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // 鼠标拖拽事件
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void onNavClicked(int index);
    void onSelectOrigin();
    void onSelectDest();
    void onSwapCity();
    void onSearchFlights();

    // 点击“选座购票”或“特惠抢票”时触发
    void onPreBookClicked(const QString& flightId, const QString& flightCode, double price, const QDateTime& depTime);

    void onSearchStatus();
    void onBackToSearch(); // 返回搜索表单页

private:
    Ui::MainWindow *ui; // 指向界面控件的指针

    UserInfo m_user;
    QPoint m_dragPosition;

    // 逻辑组件
    QButtonGroup* m_navGroup;
    QHBoxLayout* m_recLayout;    // 热门推荐的布局管理器
    QHBoxLayout* m_dateBarLayout;// 日期条的布局管理器

    // 初始化方法
    void initUI(); // 做额外的 UI 配置（如阴影、动态布局初始化）
    void setupConnections(); // 信号槽连接
    void initHotRecommendations(); // 加载热门推荐

    // --- 业务逻辑 ---
    void loadFlightList(QString from, QString to, const QDate& date, bool allowTransit = false);
    void updateDateBar(QString from, QString to, const QDate& centerDate);
    void loadOrderList();
    void updateBalanceUI();
};

#endif // MAINWINDOW_H
