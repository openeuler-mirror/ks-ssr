#include "baseline-reinforcement.h"
#include "ui_baseline-reinforcement.h"
#include "br_dbus_proxy.h"
#include <QFileDialog>
#include <qt5-log-i.h>
#include "src/ui/common/ssr-marcos-ui.h"
#include "lib/base/notification-wrapper.h"
#include "src/ui/br/plugins/categories.h"
#include "src/ui/br/xmlutils.h"
#include "include/ssr-i.h"
#include "src/ui/br/br-i.h"

namespace KS
{
namespace Settings
{
BaselineReinforcement::BaselineReinforcement(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::BaselineReinforcement)
{
    m_ui->setupUi(this);

    m_dbusProxy = new BRDbusProxy(SSR_DBUS_NAME,
                                  BR_DBUS_OBJECT_PATH,
                                  QDBusConnection::systemBus(),
                                  this);
    Notify::NotificationWrapper::globalInit(tr("Safety reinforcement").toStdString());
    initUI();
    initConnection();
}

BaselineReinforcement::~BaselineReinforcement()
{
    delete m_ui;
}

void BaselineReinforcement::initConnection()
{
    connect(m_ui->m_importStrategy, &QPushButton::clicked, this, &BaselineReinforcement::importStrategy);
    connect(m_ui->m_exportStrategy, &QPushButton::clicked, this, &BaselineReinforcement::exportStrategyClicked);
    connect(m_ui->m_resetAllArgs, &QPushButton::clicked, this, &BaselineReinforcement::resetAllArgsClicked);
    connect(m_ui->m_timeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &BaselineReinforcement::timedScanSettings);
    m_timedScan = new QTimer(this);
    connect(m_timedScan, &QTimer::timeout, this, &BaselineReinforcement::scan);
    connect(m_ui->m_openMonitor, &QRadioButton::clicked, [this]{
        m_dbusProxy->SetResourceMonitorSwitch(BR_RESOURCE_MONITOR_OPEN);
        if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
        {
            Notify::NOTIFY_INFO(tr("Resource monitor open!").toUtf8());
        }
        KLOG_INFO() << "Resource monitor open!";
    });
    connect(m_ui->m_closeMonitor, &QRadioButton::clicked, [this]{
        m_dbusProxy->SetResourceMonitorSwitch(BR_RESOURCE_MONITOR_CLOSE);
        if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
        {
            Notify::NOTIFY_INFO(tr("Resource monitor close!").toUtf8());
        }
        KLOG_INFO() << "Resource monitor close!";
    });

    connect(m_ui->m_openNotify, &QRadioButton::clicked, [this]{
        m_dbusProxy->SetNotificationStatus(BR_NOTIFICATION_STATUS_OPEN);
        Notify::NOTIFY_INFO(tr("Notify open!").toUtf8());
        KLOG_INFO() << "Notify open!";
    });
    connect(m_ui->m_closeNotify, &QRadioButton::clicked, [this]{
        m_dbusProxy->SetNotificationStatus(BR_NOTIFICATION_STATUS_CLOSE);
        Notify::NOTIFY_INFO(tr("Notify close!").toUtf8());
        KLOG_INFO() << "Notify close!";
    });

    connect(m_ui->m_fallbackInit, &QPushButton::clicked, this, [this]{
        fallback(BRSnapshotStatus::BR_SNAPSHOT_STATUS_INITIAL);
    });
    connect(m_ui->m_fallbackInit, &QPushButton::clicked, this, [this]{
        fallback(BRSnapshotStatus::BR_SNAPSHOT_STATUS_LAST);
    });

    connect(m_dbusProxy, &BRDbusProxy::HomeFreeSpaceRatioLower, this, [this](const QString &spaceRatio){
        // 家目录可用空间小于10%告警
        KLOG_WARNING() << "home free space less than 10% , ratio : " << spaceRatio;
        if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
        {
            Notify::NOTIFY_WARN(tr("home free space less than 10%").toUtf8());
        }
    });
    connect(m_dbusProxy, &BRDbusProxy::RootFreeSpaceRatioLower, this, [this](const QString &spaceRatio){
        // 根目录可用空间小于10%告警
        KLOG_WARNING() << "root free space less than 10% , ratio : " << spaceRatio;
        if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
        {
            Notify::NOTIFY_WARN(tr("root free space less than 10%").toUtf8());
        }
    });
    connect(m_dbusProxy, &BRDbusProxy::CpuAverageLoadRatioHigher, this, [this](const QString &loadRatio){
        // cpu单核五分钟平均负载大于1告警
        KLOG_WARNING() << "The average load of a single core CPU exceeds 1 , ratio : " << loadRatio;
        if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
        {
            Notify::NOTIFY_WARN(tr("The average load of a single core CPU exceeds 1").toUtf8())
        }
    });

    connect(m_dbusProxy, &BRDbusProxy::MemoryAbnormal, this, [this](const QString &ratio){
        // 内存不足10%告警
        KLOG_WARNING() << "Memory space remaining " << ratio << ", below 10%";
        if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
        {
            Notify::NOTIFY_WARN(tr("Memory space remaining").toUtf8())
        }
    });
}

void BaselineReinforcement::initUI()
{
    if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
    {
        m_ui->m_openNotify->setChecked(true);
        m_ui->m_closeNotify->setChecked(false);
    }
    else
    {
        m_ui->m_openNotify->setChecked(false);
        m_ui->m_closeNotify->setChecked(true);
    }
    if (m_dbusProxy->resource_monitor() == BRResourceMonitor::BR_RESOURCE_MONITOR_OPEN)
    {
        m_ui->m_openMonitor->setChecked(true);
        m_ui->m_closeMonitor->setChecked(false);
    }
    else
    {
        m_ui->m_openMonitor->setChecked(false);
        m_ui->m_closeMonitor->setChecked(true);
    }
    m_ui->m_timeSpinBox->setValue(m_dbusProxy->time_scan());
}

void BaselineReinforcement::updateProgressInfo(KS::BR::ProgressInfo &progressInfo)
{
    for (auto categories : m_categoriesList)
    {
        for (auto category : categories->getCategory())
        {
            auto state = category->getState();
            if (state == BR_REINFORCEMENT_STATE_SCAN_DONE || state == BR_REINFORCEMENT_STATE_REINFORCE_DONE || (state & BR_REINFORCEMENT_STATE_SAFE) == BR_REINFORCEMENT_STATE_SAFE)
            {
                progressInfo.successCount += 1;
            }
            else if (state == BR_REINFORCEMENT_STATE_UNKNOWN || state == BR_REINFORCEMENT_STATE_SCAN_ERROR || state == BR_REINFORCEMENT_STATE_REINFORCE_ERROR || (state & BR_REINFORCEMENT_STATE_UNSAFE) == BR_REINFORCEMENT_STATE_UNSAFE)
            {
                progressInfo.failureCount += 1;
            }
        }
    }
}

void BaselineReinforcement::importStrategy()
{
    auto fileName = QFileDialog::getOpenFileName(this, tr("Files"), "/", tr("strategy(*.xml)"));
    RETURN_IF_TRUE(fileName.isEmpty())

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        KLOG_WARNING() << "Open files failed!";
        POPUP_MESSAGE_DIALOG(tr("Open files failed!"))
    }
    auto reply = m_dbusProxy->ImportCustomRA(QString::fromUtf8(file.readAll()));
    reply.waitForFinished();
    POPUP_MESSAGE_DIALOG(reply.isError() ? tr("Failed to import strategy file. Please whether the file is valid!") : tr("Import succeeded!"));
    file.close();
}

void BaselineReinforcement::timedScanSettings(int hours)
{
    m_dbusProxy->SetTimeScan(hours);
    if (hours == 0)
    {
        m_timedScan->stop();
        if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
        {
            Notify::NOTIFY_INFO(tr("Scheduled scanning task has been closed!").toUtf8());
        }
        return;
    }

    // 每hours小时扫描一次
    if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
    {
        Notify::NOTIFY_INFO(QString(tr("Scheduled scanning task has been started, every interval %1 scan once every hour.")).arg(hours).toUtf8());
    }
    m_timedScan->start(hours * 1000 * 3600);
    QTimer::singleShot(hours * 1000 * 3600, this, &BaselineReinforcement::scan);
}

void BaselineReinforcement::scan()
{
    // 获取加固项信息
    auto reply = m_dbusProxy->GetCategories();
    reply.waitForFinished();
    KS::BR::XMLUtils::getDefault()->jsonParsing(reply.value().toUtf8(), m_categoriesList);
    KS::BR::XMLUtils::getDefault()->ssrReinforcements(m_dbusProxy->GetReinforcements().value(), m_categoriesList);

    // 断开scan进程连接
    disconnect(m_dbusProxy, &BRDbusProxy::ScanProgress, 0, 0);
    // 进行一次扫描 仅获取扫描结果
    connect(m_dbusProxy, &BRDbusProxy::ScanProgress, this, [this](const QString &jobResult){
        InvalidData invalidData = {};
        KS::BR::XMLUtils::getDefault()->ssrJobResult(jobResult, m_progressInfo, m_categoriesList, invalidData);
    });
    // 监听进程完成后
    connect(m_dbusProxy, &BRDbusProxy::ProgressFinished, this, [this]{
        disconnect(m_dbusProxy, &BRDbusProxy::ScanProgress, 0, 0);
        disconnect(m_dbusProxy, &BRDbusProxy::ProgressFinished, 0, 0);
        updateProgressInfo(m_progressInfo);
        if (m_dbusProxy->notification_status() == BRNotificationStatus::BR_NOTIFICATION_STATUS_OPEN)
        {
            Notify::NOTIFY_INFO(QString(tr("Timed scan finied, Scaned %1, %2 conform, %3 inconform!")).arg(m_progressInfo.failureCount + m_progressInfo.successCount)
                                .arg(m_progressInfo.successCount).
                                arg(m_progressInfo.failureCount).toUtf8());
        }
        KLOG_INFO() << "Timed scan finied, Scaned "
                    << m_progressInfo.failureCount + m_progressInfo.successCount
                    << ", "
                    << m_progressInfo.successCount
                    << " conform, "
                    << m_progressInfo.failureCount
                    << "inconform!";
    });

    QStringList scanStr;
    for (auto categories : m_categoriesList)
    {
        for (auto category : categories->getCategory())
        {
            scanStr << category->getName();
        }
    }
    m_dbusProxy->Scan(scanStr);
}

void BaselineReinforcement::setMonitorStatus(bool isOpen)
{
    m_dbusProxy->SetResourceMonitorSwitch(isOpen ? BR_RESOURCE_MONITOR_OPEN : BR_RESOURCE_MONITOR_CLOSE);

    RETURN_IF_TRUE(m_dbusProxy->notification_status() == BR_NOTIFICATION_STATUS_CLOSE);
    Notify::NOTIFY_INFO(isOpen ? tr("Open resource monitoring").toUtf8() : tr("Close resource monitoring").toUtf8());
    KLOG_DEBUG()<< QString(isOpen ? "Open resource monitoring" : "Close resource monitoring");
}

void BaselineReinforcement::fallback(int status)
{
    auto reply = m_dbusProxy->SetFallback(BRSnapshotStatus(status));
    CHECK_ERROR_FOR_DBUS_REPLY(reply);
    disconnect(m_dbusProxy, &BRDbusProxy::ProgressFinished, 0, 0);
    connect(m_dbusProxy, &BRDbusProxy::ProgressFinished, this, [this]{
        POPUP_MESSAGE_DIALOG(tr("Fallback finished!"));
    });
}
}  // namespace Settings
}  // namespace KS
