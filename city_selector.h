#ifndef CITY_SELECTOR_H
#define CITY_SELECTOR_H

#include <QDialog>
#include <QTreeWidget>
#include <QMap>
#include <QMouseEvent>

// 前向声明 UI 类
namespace Ui {
class CitySelector;
}

class CitySelector : public QDialog {
    Q_OBJECT

public:
    explicit CitySelector(QWidget *parent = nullptr);
    ~CitySelector();

    QString getSelectedCity() const { return m_selectedCity; }

    // === 核心功能：静态验证与修正 ===
    static bool isValidCity(const QString& city);

    // 新增：智能修正城市名称
    // 输入 "北京" -> 返回 "北京市"
    // 输入 "大理" -> 返回 "大理白族自治州"
    // 输入 "火星" -> 返回 "" (空字符串表示失败)
    static QString tryFixCityName(const QString& input);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void onItemClicked(QTreeWidgetItem *item, int column);
    void onSearchTextChanged(const QString &text);

private:
    Ui::CitySelector *ui;

    void initData();
    void loadTree();

    QString m_selectedCity;
    QMap<QString, QStringList> m_cityData;

    QPoint m_dragPosition;

    static const QMap<QString, QStringList>& getStaticData();
};

#endif // CITY_SELECTOR_H
