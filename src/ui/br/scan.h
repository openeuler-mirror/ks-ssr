/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     chendingjian <chendingjian@kylinos.com.cn> 
 */

#include <QWidget>
#include "br-i.h"
#include "src/ui/br/progress.h"
#include "src/ui/br/reinforcement-items/category.h"

class BRDbusProxy;

namespace Ui
{
class Scan;
}
namespace KS
{
namespace BR
{
class ReinforcementArgsDialog;

struct ArgTransfer
{
    QString categoryName;
    QString argName;
    QString value;
    KS::Protocol::WidgetType::Value widgetType;

    ArgTransfer(const QString &category = "",
                const QString &argLabel = "",
                const QString &argValue = "",
                KS::Protocol::WidgetType::Value type = KS::Protocol::WidgetType::Value::DEFAULT)
    {
        categoryName = category;
        argName = argLabel;
        value = argValue;
        widgetType = type;
    }
};

class Scan : public QWidget
{
    Q_OBJECT

public:
    explicit Scan(QWidget *parent = nullptr);
    virtual ~Scan();

    void emitScanSignal();
    void usingSystemStrategy();
    void usingCustomStrategy();
    // 回到初始页面状态
    void reset();
    // 导出策略
    bool exportStrategy();
    // 重置所有加固项
    void resetAllReinforcementItem();

signals:
    void returnHomeClicked();
    // 加固完成信号，用于修改home页面上一次加固时间
    void reinforcementFinished();

private:
    void init();
    void initUI();
    void initConnection();

    // 清空非法数据
    void clearInvalidData();
    // 清空加固/扫描状态
    void clearState();
    // 重置进程info
    void flushProgressInfo();
    // 加固参数重置
    void argReset(const QString &categoryName, const QString &argName);
    // 设置加固项
    void setReinforcement();
    bool checkAndSetCheckbox();

private slots:
    // progress
    void startScan();
    void startReinforcement();
    void generateReport();
    void cancelProgress();
    // table
    void showErrorMessage(const QModelIndex &model);
    void popReinforcecmentDialog(const QModelIndex &model);
    // dbus
    void runProgress(const QString &jobResult);

private:
    Ui::Scan *m_ui;

    BRDbusProxy *m_dbusProxy;
    InvalidData m_invalidData = {};
    QList<Category *> m_categories = {};
    QList<Category *> m_afterReinForcementCategories = {};
    ProgressInfo m_progressInfo = {};
    BRStrategyType m_strategyType;
    ReinforcementArgsDialog *m_customArgsDialog;
    QList<ArgTransfer *> m_argTransfers = {};
};

}  // namespace BR
}  // namespace KS
