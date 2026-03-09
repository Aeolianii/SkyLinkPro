#include "toast.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QGraphicsDropShadowEffect>

// 构造函数：定义样式
Toast::Toast(QWidget* parent, const QString& message, const QString& themeColor)
    : QWidget(parent)
{
    // 1. 窗口属性：无边框、工具窗口、置顶、透明背景
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动释放内存

    // 2. 布局与控件
    QHBoxLayout* lay = new QHBoxLayout(this);
    lay->setContentsMargins(25, 12, 25, 12); // 内边距：上下窄，左右宽

    m_label = new QLabel(message);
    m_label->setStyleSheet("color: white; font-size: 14px; font-weight: bold; font-family: 'Microsoft YaHei UI';");
    m_label->setAlignment(Qt::AlignCenter);

    lay->addWidget(m_label);

    // 3. 动态背景颜色 (使用 property 保存，以便在 paintEvent 使用)
    this->setProperty("themeColor", themeColor);

    // 4. 阴影
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(15);
    shadow->setColor(QColor(0, 0, 0, 60));
    shadow->setOffset(0, 4);
    setGraphicsEffect(shadow);

    // 5. 定时关闭
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &Toast::closeAnimation);
    m_timer->start(2000); // 2秒后消失
}

// 绘制圆角背景
void Toast::paintEvent(QPaintEvent* event) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QColor color(property("themeColor").toString());
    p.setBrush(color);
    p.setPen(Qt::NoPen);

    // 绘制圆角矩形 (胶囊形状)
    p.drawRoundedRect(rect(), height()/2, height()/2);
}

// 消失动画
void Toast::closeAnimation() {
    QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity");
    anim->setDuration(500);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    connect(anim, &QPropertyAnimation::finished, this, &Toast::close);
    anim->start();
}

// === 静态帮助函数 ===

// 计算位置并显示
void showToastInstance(const QString& msg, const QString& color, QWidget* parent) {
    Toast* t = new Toast(parent, msg, color);

    // 自适应大小
    t->adjustSize();

    // 计算中心位置
    QWidget* host = parent ? parent->window() : QApplication::activeWindow();
    if (host) {
        QPoint center = host->geometry().center();
        t->move(center.x() - t->width() / 2, center.y() - t->height() / 2 + 50); // 稍微偏下一点
    } else {
        // 如果没有父窗口，显示在屏幕中间
        QRect screen = QApplication::primaryScreen()->geometry();
        t->move(screen.center() - QPoint(t->width()/2, t->height()/2));
    }
    t->show();
}

void Toast::showTip(const QString& message, QWidget* parent) {
    // 黑色半透明
    showToastInstance(message, "#333333", parent);
}

void Toast::showSuccess(const QString& message, QWidget* parent) {
    // 绿色
    showToastInstance(message, "#07C160", parent);
}

void Toast::showError(const QString& message, QWidget* parent) {
    // 红色
    showToastInstance(message, "#FF4D4F", parent);
}
