#ifndef THEME_H
#define THEME_H

#include <QString>

const QString APP_STYLE = R"(
    /* 全局深色背景 */
    QMainWindow, QWidget#CentralWidget { background-color: #1e1e2d; }

    /* 标题文字 */
    QLabel { color: #e0e0e0; font-family: 'Segoe UI', 'Microsoft YaHei'; }
    QLabel#Title { font-size: 26px; font-weight: bold; color: #ffffff; }
    QLabel#SubTitle { font-size: 16px; color: #a0a0b0; }

    /* 输入框：扁平化设计 */
    QLineEdit {
        background-color: #2b2b3c;
        border: 1px solid #3f3f50;
        border-radius: 8px;
        padding: 12px;
        color: white;
        font-size: 14px;
    }
    QLineEdit:focus { border: 1px solid #3a86ff; background-color: #323246; }

    /* 日期选择器 */
    QDateEdit {
        background-color: #2b2b3c; border: 1px solid #3f3f50; border-radius: 8px; padding: 12px; color: white;
    }

    /* 按钮：渐变色，悬停光效 */
    QPushButton {
        background-color: #3a86ff;
        color: white;
        border: none;
        border-radius: 8px;
        padding: 12px 24px;
        font-weight: bold;
        font-size: 14px;
    }
    QPushButton:hover { background-color: #5ba1ff; }
    QPushButton:pressed { background-color: #2a66cc; }

    QPushButton#SecondaryBtn {
        background-color: transparent; border: 1px solid #555; color: #aaa;
    }
    QPushButton#SecondaryBtn:hover { border-color: #888; color: white; }

    /* 表格/列表：卡片化风格 */
    QTableWidget {
        background-color: transparent;
        border: none;
        gridline-color: transparent;
        color: white;
    }
    QTableWidget::item {
        background-color: #2b2b3c;
        margin-bottom: 10px; /* 卡片间距 */
        border-radius: 8px;
        padding: 10px;
    }
    QTableWidget::item:selected {
        background-color: #3a86ff;
        border: 1px solid #5ba1ff;
    }
)";

#endif // THEME_H
