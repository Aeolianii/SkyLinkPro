#ifndef APP_STYLE_H
#define APP_STYLE_H

#include <QString>

class AppStyle {
public:
    static QString getQSS() {
        return R"(
            /* ================= 全局基础设置 ================= */
            QWidget {
                font-family: 'Microsoft YaHei UI', 'Segoe UI', sans-serif;
                font-size: 14px;
                color: #2D3748; /* 深灰字体 */
            }

            /* 主窗口背景透明 (为了圆角和阴影) */
            QMainWindow {
                background-color: transparent;
            }

            /* ================= 布局容器 ================= */
            /* 侧边栏 (深色) */
            QWidget#SideBar, QWidget#sideBar {
                background-color: #1A202C;
                border-top-left-radius: 16px;
                border-bottom-left-radius: 16px;
            }

            /* 内容区域 */
            QWidget#MainArea, QWidget#mainArea {
                background-color: #F7FAFC;
                border-top-right-radius: 16px;
                border-bottom-right-radius: 16px;
            }

            /* 白色卡片 */
            QWidget[class="Card"], QFrame#cardSearch, QFrame#cardResult, QFrame#cardOrder, QWidget#cardStatus {
                background-color: #FFFFFF;
                border-radius: 12px;
                border: 1px solid #EDF2F7;
            }

            /* ================= 文本排版 ================= */
            QLabel[class="PageTitle"] {
                font-size: 26px; font-weight: 800; color: #1A202C; margin-bottom: 10px;
            }
            QLabel[class="Highlight"] {
                color: #0086F6; font-weight: bold; font-size: 18px;
            }

            /* ================= 交互控件 ================= */
            QLineEdit, QDateEdit, QTimeEdit, QComboBox {
                background-color: #FFFFFF;
                border: 1px solid #E2E8F0;
                border-radius: 8px;
                padding: 8px 12px;
                font-size: 14px;
                color: #333;
                selection-background-color: #0086F6;
            }
            QLineEdit:focus, QDateEdit:focus, QTimeEdit:focus, QComboBox:focus {
                border: 1px solid #0086F6;
                background-color: #FFFFFF;
            }

            /* 侧边栏导航按钮 */
            QToolButton {
                background-color: transparent;
                color: #A0AEC0;
                border: none;
                border-radius: 8px;
                padding: 12px 20px;
                text-align: left;
                font-size: 15px;
                font-weight: 600;
                margin: 2px 10px;
            }
            QToolButton:hover {
                background-color: #2D3748;
                color: #FFFFFF;
            }
            QToolButton:checked {
                background-color: #2D3748;
                color: #FFFFFF;
                border-left: 4px solid #0086F6;
            }

            /* 按钮 */
            QPushButton {
                background-color: #0086F6; color: white; border-radius: 8px;
                padding: 8px 16px; font-size: 14px; font-weight: bold; border: none;
            }
            QPushButton:hover { background-color: #0070CD; }
            QPushButton:disabled { background-color: #CBD5E0; color: #F7FAFC; }

            /* 表格 */
            QTableWidget {
                background-color: white; border: none; gridline-color: transparent; outline: none;
            }
            QTableWidget::item {
                border-bottom: 1px solid #EDF2F7; padding-left: 10px; color: #4A5568;
            }
            QTableWidget::item:selected {
                background-color: #EBF8FF; color: #2B6CB0;
            }
            QHeaderView::section {
                background-color: #F7FAFC; border: none; border-bottom: 2px solid #E2E8F0;
                color: #718096; font-weight: bold; height: 40px; padding-left: 10px;
            }

            /* 滚动条 */
            QScrollBar:vertical { border: none; background: #F7FAFC; width: 8px; margin: 0px; }
            QScrollBar::handle:vertical { background: #CBD5E0; border-radius: 4px; min-height: 20px; }
            QScrollBar::handle:vertical:hover { background: #A0AEC0; }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }

            /* =======================================================
               修复点：针对系统弹窗 (QMessageBox, QInputDialog) 的样式强制覆盖
               ======================================================= */
            QMessageBox, QInputDialog {
                background-color: #FFFFFF;
                border: 1px solid #CCCCCC;
                border-radius: 8px;
            }
            QMessageBox QLabel, QInputDialog QLabel {
                color: #333333; /* 强制黑色字体，防止背景白字也白 */
                font-size: 14px;
                background-color: transparent;
            }
            QMessageBox QPushButton, QInputDialog QPushButton {
                background-color: #0086F6;
                color: white;
                border-radius: 6px;
                padding: 6px 20px;
                min-width: 60px;
            }
            QMessageBox QPushButton:hover, QInputDialog QPushButton:hover {
                background-color: #0070CD;
            }
            /* 输入对话框的输入框样式 */
            QInputDialog QLineEdit {
                background-color: #F9FAFB;
                border: 1px solid #E2E8F0;
                color: #333;
            }
        )";
    }
};

#endif // APP_STYLE_H
