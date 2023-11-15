#pragma once

#include <QWidget>
#include "src/ui/br/progress.h"

namespace Ui {
class BaselineReinforcement;
}

class BRDbusProxy;


namespace KS
{
namespace BR {
namespace Plugins {
class Categories;
}
}

namespace Settings
{
class BaselineReinforcement : public QWidget
{
    Q_OBJECT

public:
    explicit BaselineReinforcement(QWidget *parent = nullptr);
    ~BaselineReinforcement();

private:
    void initConnection();
    void initUI();

    void updateProgressInfo(KS::BR::ProgressInfo &progressInfo);

private slots:
    void importStrategy();
    void timedScanSettings(int hours);
    void scan();
    void setMonitorStatus(bool isOpen);
    void fallback(int status);

signals:
    // 需要通过勾选项进行导出，设置界面无法获取到表格勾选信息，需要在page中获取
    void exportStrategyClicked();
    // 重置所有加固项需要与表格界面交互
    void resetAllArgsClicked();

private:
    Ui::BaselineReinforcement *m_ui;

    QTimer *m_timedScan;
    QList<KS::BR::Plugins::Categories *> m_categoriesList = {};
    KS::BR::ProgressInfo m_progressInfo = {};

    BRDbusProxy *m_dbusProxy;
};
}  // namespace Settings
}  // namespace KS
