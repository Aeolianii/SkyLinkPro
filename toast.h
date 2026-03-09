#ifndef TOAST_H
#define TOAST_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>

class Toast : public QWidget {
    Q_OBJECT
public:
    // 静态调用方法，方便直接使用
    static void showTip(const QString& message, QWidget* parent = nullptr);
    static void showSuccess(const QString& message, QWidget* parent = nullptr);
    static void showError(const QString& message, QWidget* parent = nullptr);

    // ✅ 【修改这里】将构造函数从 protected 移动到 public，或者直接改为 public
    explicit Toast(QWidget* parent, const QString& message, const QString& color);

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void closeAnimation();

private:
    QLabel* m_label;
    QTimer* m_timer;
};

#endif // TOAST_H
