#include "city_selector.h"
#include "ui_city_selector.h"

#include <QDebug>
#include <QGraphicsDropShadowEffect>

CitySelector::CitySelector(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CitySelector)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 40));
    ui->mainFrame->setGraphicsEffect(shadow);

    initData();
    loadTree();

    connect(ui->btnClose, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->btnConfirm, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui->leSearch, &QLineEdit::textChanged, this, &CitySelector::onSearchTextChanged);
    connect(ui->treeCities, &QTreeWidget::itemClicked, this, &CitySelector::onItemClicked);
}

CitySelector::~CitySelector() {
    delete ui;
}

void CitySelector::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}
void CitySelector::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

// === 核心数据源 ===
const QMap<QString, QStringList>& CitySelector::getStaticData() {
    static QMap<QString, QStringList> data;
    if (data.isEmpty()) {
        data["热门城市"] = {"北京市", "上海市", "广州市", "深圳市", "成都市", "杭州市"};
        data["北京市"] = {"北京市"};
        data["天津市"] = {"天津市"};
        data["上海市"] = {"上海市"};
        data["重庆市"] = {"重庆市"};
        data["河北省"] = {"石家庄市", "唐山市", "秦皇岛市", "邯郸市", "邢台市", "保定市", "张家口市", "承德市", "沧州市", "廊坊市", "衡水市"};
        data["山西省"] = {"太原市", "大同市", "阳泉市", "长治市", "晋城市", "朔州市", "晋中市", "运城市", "忻州市", "临汾市", "吕梁市"};
        data["内蒙古自治区"] = {"呼和浩特市", "包头市", "乌海市", "赤峰市", "通辽市", "鄂尔多斯市", "呼伦贝尔市", "巴彦淖尔市", "乌兰察布市", "兴安盟", "锡林郭勒盟", "阿拉善盟"};
        data["辽宁省"] = {"沈阳市", "大连市", "鞍山市", "抚顺市", "本溪市", "丹东市", "锦州市", "营口市", "阜新市", "辽阳市", "盘锦市", "铁岭市", "朝阳市", "葫芦岛市"};
        data["吉林省"] = {"长春市", "吉林市", "四平市", "辽源市", "通化市", "白山市", "松原市", "白城市", "延边朝鲜族自治州"};
        data["黑龙江省"] = {"哈尔滨市", "齐齐哈尔市", "鸡西市", "鹤岗市", "双鸭山市", "大庆市", "伊春市", "佳木斯市", "七台河市", "牡丹江市", "黑河市", "绥化市", "大兴安岭地区"};
        data["江苏省"] = {"南京市", "无锡市", "徐州市", "常州市", "苏州市", "南通市", "连云港市", "淮安市", "盐城市", "扬州市", "镇江市", "泰州市", "宿迁市"};
        data["浙江省"] = {"杭州市", "宁波市", "温州市", "嘉兴市", "湖州市", "绍兴市", "金华市", "衢州市", "舟山市", "台州市", "丽水市"};
        data["安徽省"] = {"合肥市", "芜湖市", "蚌埠市", "淮南市", "马鞍山市", "淮北市", "铜陵市", "安庆市", "黄山市", "滁州市", "阜阳市", "宿州市", "六安市", "亳州市", "池州市", "宣城市"};
        data["福建省"] = {"福州市", "厦门市", "莆田市", "三明市", "泉州市", "漳州市", "南平市", "龙岩市", "宁德市"};
        data["江西省"] = {"南昌市", "景德镇市", "萍乡市", "九江市", "新余市", "鹰潭市", "赣州市", "吉安市", "宜春市", "抚州市", "上饶市"};
        data["山东省"] = {"济南市", "青岛市", "淄博市", "枣庄市", "东营市", "烟台市", "潍坊市", "济宁市", "泰安市", "威海市", "日照市", "临沂市", "德州市", "聊城市", "滨州市", "菏泽市"};
        data["河南省"] = {"郑州市", "开封市", "洛阳市", "平顶山市", "安阳市", "鹤壁市", "新乡市", "焦作市", "濮阳市", "许昌市", "漯河市", "三门峡市", "南阳市", "商丘市", "信阳市", "周口市", "驻马店市", "济源市"};
        data["湖北省"] = {"武汉市", "黄石市", "十堰市", "宜昌市", "襄阳市", "鄂州市", "荆门市", "孝感市", "荆州市", "黄冈市", "咸宁市", "随州市", "恩施土家族苗族自治州", "仙桃市", "潜江市", "天门市", "神农架林区"};
        data["湖南省"] = {"长沙市", "株洲市", "湘潭市", "衡阳市", "邵阳市", "岳阳市", "常德市", "张家界市", "益阳市", "郴州市", "永州市", "怀化市", "娄底市", "湘西土家族苗族自治州"};
        data["广东省"] = {"广州市", "韶关市", "深圳市", "珠海市", "汕头市", "佛山市", "江门市", "湛江市", "茂名市", "肇庆市", "惠州市", "梅州市", "汕尾市", "河源市", "阳江市", "清远市", "东莞市", "中山市", "潮州市", "揭阳市", "云浮市"};
        data["广西壮族自治区"] = {"南宁市", "柳州市", "桂林市", "梧州市", "北海市", "防城港市", "钦州市", "贵港市", "玉林市", "百色市", "贺州市", "河池市", "来宾市", "崇左市"};
        data["海南省"] = {"海口市", "三亚市", "三沙市", "儋州市", "五指山市", "琼海市", "文昌市", "万宁市", "东方市", "定安县", "屯昌县", "澄迈县", "临高县", "白沙黎族自治县", "昌江黎族自治县", "乐东黎族自治县", "陵水黎族自治县", "保亭黎族苗族自治县", "琼中黎族苗族自治县"};
        data["四川省"] = {"成都市", "自贡市", "攀枝花市", "泸州市", "德阳市", "绵阳市", "广元市", "遂宁市", "内江市", "乐山市", "南充市", "眉山市", "宜宾市", "广安市", "达州市", "雅安市", "巴中市", "资阳市", "阿坝藏族羌族自治州", "甘孜藏族自治州", "凉山彝族自治州"};
        data["贵州省"] = {"贵阳市", "六盘水市", "遵义市", "安顺市", "毕节市", "铜仁市", "黔西南布依族苗族自治州", "黔东南苗族侗族自治州", "黔南布依族苗族自治州"};
        data["云南省"] = {"昆明市", "曲靖市", "玉溪市", "保山市", "昭通市", "丽江市", "普洱市", "临沧市", "楚雄彝族自治州", "红河哈尼族彝族自治州", "文山壮族苗族自治州", "西双版纳傣族自治州", "大理白族自治州", "德宏傣族景颇族自治州", "怒江傈僳族自治州", "迪庆藏族自治州"};
        data["西藏自治区"] = {"拉萨市", "日喀则市", "昌都市", "林芝市", "山南市", "那曲市", "阿里地区"};
        data["陕西省"] = {"西安市", "铜川市", "宝鸡市", "咸阳市", "渭南市", "延安市", "汉中市", "榆林市", "安康市", "商洛市"};
        data["甘肃省"] = {"兰州市", "嘉峪关市", "金昌市", "白银市", "天水市", "武威市", "张掖市", "平凉市", "酒泉市", "庆阳市", "定西市", "陇南市", "临夏回族自治州", "甘南藏族自治州"};
        data["青海省"] = {"西宁市", "海东市", "海北藏族自治州", "黄南藏族自治州", "海南藏族自治州", "果洛藏族自治州", "玉树藏族自治州", "海西蒙古族藏族自治州"};
        data["宁夏回族自治区"] = {"银川市", "石嘴山市", "吴忠市", "固原市", "中卫市"};
        data["新疆维吾尔自治区"] = {"乌鲁木齐市", "克拉玛依市", "吐鲁番市", "哈密市", "昌吉回族自治州", "博尔塔拉蒙古自治州", "巴音郭楞蒙古自治州", "阿克苏地区", "克孜勒苏柯尔克孜自治州", "喀什地区", "和田地区", "伊犁哈萨克自治州", "塔城地区", "阿勒泰地区", "石河子市", "阿拉尔市", "图木舒克市", "五家渠市", "北屯市", "铁门关市", "双河市", "可克达拉市", "昆玉市", "胡杨河市", "新星市"};
        data["特别行政区"] = {"香港特别行政区", "澳门特别行政区"};
        data["台湾省"] = {"台北市", "高雄市", "新北市", "台中市", "台南市", "桃园市", "基隆市", "新竹市", "嘉义市", "新竹县", "苗栗县", "彰化县", "南投县", "云林县", "嘉义县", "屏东县", "宜兰县", "花莲县", "台东县", "澎湖县"};
    }
    return data;
}

void CitySelector::initData() {
    m_cityData = getStaticData();
}

bool CitySelector::isValidCity(const QString &city) {
    if (city.isEmpty()) return false;
    const auto& data = getStaticData();
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        if (it.key() == "热门城市") continue;
        if (it.value().contains(city)) return true;
    }
    // 双重检查热门
    if(data["热门城市"].contains(city)) return true;
    return false;
}

// === 智能修正逻辑 ===
QString CitySelector::tryFixCityName(const QString& input) {
    QString trimmed = input.trimmed();
    if (trimmed.isEmpty()) return "";

    // 1. 如果已经是完全正确的名字，直接返回
    if (isValidCity(trimmed)) return trimmed;

    // 2. 尝试寻找以此开头的城市 (例如 "北京" -> "北京市", "大理" -> "大理白族...")
    const auto& data = getStaticData();
    QStringList candidates;

    // 收集所有候选者
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        if (it.key() == "热门城市") continue;

        foreach (const QString& city, it.value()) {
            if (city.startsWith(trimmed)) {
                if (!candidates.contains(city)) {
                    candidates.append(city);
                }
            }
        }
    }

    // 3. 分析候选者
    if (candidates.isEmpty()) {
        return ""; // 没找到
    } else if (candidates.size() == 1) {
        return candidates.first(); // 只有唯一匹配，就是它了！
    } else {
        // 有多个匹配 (例如输入"南"，匹配"南京","南昌"等)
        // 尝试优先匹配 "输入 + 市" 的情况
        QString tryAppendShi = trimmed + "市";
        if (candidates.contains(tryAppendShi)) {
            return tryAppendShi;
        }
        // 如果依然无法确定（比如输入"南"且没有"南市"），则认为输入不明确，返回空
        return "";
    }
}

void CitySelector::loadTree() {
    ui->treeCities->clear();
    if(m_cityData.contains("热门城市")) {
        QTreeWidgetItem* hotItem = new QTreeWidgetItem(ui->treeCities);
        hotItem->setText(0, "热门城市");
        hotItem->setExpanded(true);
        foreach(const QString& city, m_cityData["热门城市"]) {
            new QTreeWidgetItem(hotItem, QStringList(city));
        }
    }
    QMapIterator<QString, QStringList> i(m_cityData);
    while (i.hasNext()) {
        i.next();
        if(i.key() == "热门城市" || i.key() == "特别行政区") continue;
        QTreeWidgetItem* prov = new QTreeWidgetItem(ui->treeCities);
        prov->setText(0, i.key());
        foreach(const QString& city, i.value()) {
            new QTreeWidgetItem(prov, QStringList(city));
        }
    }
    if(m_cityData.contains("特别行政区")) {
        QTreeWidgetItem* special = new QTreeWidgetItem(ui->treeCities);
        special->setText(0, "特别行政区");
        foreach(const QString& city, m_cityData["特别行政区"]) {
            new QTreeWidgetItem(special, QStringList(city));
        }
    }
}

void CitySelector::onSearchTextChanged(const QString &text) {
    QString keyword = text.trimmed();
    bool isEmpty = keyword.isEmpty();
    for(int i = 0; i < ui->treeCities->topLevelItemCount(); ++i) {
        QTreeWidgetItem* provinceItem = ui->treeCities->topLevelItem(i);
        QString provName = provinceItem->text(0);
        if (provName == "热门城市") {
            provinceItem->setHidden(!isEmpty);
            if(isEmpty) provinceItem->setExpanded(true);
            continue;
        }
        bool hasVisibleChild = false;
        for(int j = 0; j < provinceItem->childCount(); ++j) {
            QTreeWidgetItem* cityItem = provinceItem->child(j);
            QString cityName = cityItem->text(0);
            bool isMatch = isEmpty || cityName.contains(keyword);
            cityItem->setHidden(!isMatch);
            if (isMatch) hasVisibleChild = true;
        }
        provinceItem->setHidden(!hasVisibleChild);
        if (!isEmpty && hasVisibleChild) provinceItem->setExpanded(true);
        else if (isEmpty) provinceItem->setExpanded(false);
    }
}

void CitySelector::onItemClicked(QTreeWidgetItem *item, int column) {
    if(item->childCount() == 0 && item->parent() != nullptr) {
        m_selectedCity = item->text(0);
    } else {
        m_selectedCity = "";
    }
}
