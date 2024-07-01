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
 * Author:     chendingjian <chendingjian@kylinsec.com.cn>
 */
#include "scan.h"
#include <kylin-license/license-i.h>
#include <QDBusServiceWatcher>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QToolTip>
#include "br-i.h"
#include "br_dbus_proxy.h"
#include "include/ssr-i.h"
#include "include/ssr-marcos.h"
#include "lib/base/str-utils.h"
#include "lib/widgets/ssr-marcos-ui.h"
#include "reinforcement-items/reinforcement-args-dialog.h"
#include "reports/report.h"
#include "ui_scan.h"
#include "utils.h"

namespace KS
{
namespace BR
{
Scan::Scan(QWidget *parent)
    : QWidget(parent),
      m_ui(new Ui::Scan)
{
    m_ui->setupUi(this);
    initConnection();
    parsingCategories();
    initUI();
}

Scan::~Scan()
{
    delete m_ui;
}

void Scan::emitScanSignal()
{
    emit m_ui->m_progress->scanClicked();
}

void Scan::usingSystemStrategy()
{
    m_strategyType = BR_STRATEGY_TYPE_SYSTEM;
    KLOG_DEBUG() << "use system strategy.";
    resetAllReinforcementItem();
    m_ui->m_itemTable->setStrategy(BR_STRATEGY_TYPE_SYSTEM);
    disconnect(m_ui->m_itemTable, &ItemTable::modifyItemArgsClicked, this, &Scan::popReinforcecmentDialog);
    m_ui->m_progress->hideStrategy();
}

void Scan::usingCustomStrategy()
{
    KLOG_DEBUG() << "use custom strategy.";
    m_strategyType = BR_STRATEGY_TYPE_CUSTOM;
    disconnect(m_ui->m_itemTable, &ItemTable::modifyItemArgsClicked, this, &Scan::popReinforcecmentDialog);
    connect(m_ui->m_itemTable, &ItemTable::modifyItemArgsClicked, this, &Scan::popReinforcecmentDialog);
    // 修改UI界面参数以及复选框状态
    m_ui->m_itemTable->setStrategy(BR_STRATEGY_TYPE_CUSTOM);
    // 更新加固项和相关参数
    Utils::getDefault()->ssrResetReinforcements(m_dbusProxy->GetReinforcements().value(), m_categories);

    m_ui->m_progress->showStrategy();
}

void Scan::reset()
{
    disconnect(m_dbusProxy, &BRDbusProxy::ScanProgress, 0, 0);
    disconnect(m_dbusProxy, &BRDbusProxy::ProgressFinished, 0, 0);

    m_progressInfo.method = PROCESS_METHOD_STANDBY;

    m_ui->m_progress->resetProgress();
}

bool Scan::exportStrategy()
{
    RETURN_VAL_IF_TRUE(!checkAndSetCheckbox(), false);

    // 导出自定义策略，xml格式

    auto fileName = QFileDialog::getSaveFileName(nullptr, tr("export strategy"), "./br-strategy.xml", tr("strategy(*.xml)"));
    RETURN_VAL_IF_TRUE(fileName.isEmpty(), false)

    // 打开要写入的文件
    QFile fileSave(fileName);
    if (!fileSave.open(QIODevice::WriteOnly))
    {
        // 无法打开
        KLOG_WARNING() << "Please check the file name and whether you have write permission!";
        POPUP_MESSAGE_DIALOG(tr("Please check the file name and whether you have write permission!"));
        m_dbusProxy->ExportStrategy(false);
        return false;
    }

    // 打开ra文件
    QFile file(SSR_BR_CUSTOM_RA_FILEPATH);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        KLOG_WARNING() << "Open RA file failed!";
        POPUP_MESSAGE_DIALOG(tr("Open RA file failed!"));
        m_dbusProxy->ExportStrategy(false);
        return false;
    }

    // 写入文件
    auto isSuccess = fileSave.write(file.readAll());
    POPUP_MESSAGE_DIALOG(isSuccess ? tr("Export successed!") : tr("Export failed!"));
    file.close();
    fileSave.close();
    m_dbusProxy->ExportStrategy(isSuccess);
    return true;
}

bool Scan::importStrategy()
{
    auto fileName = QFileDialog::getOpenFileName(nullptr, tr("import strategy"), "/", tr("strategy(*.xml)"));
    if (fileName.isEmpty())
    {
        return false;
    }
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

    return true;
}

void Scan::parsingCategories()
{
    m_serviceWatcher = new QDBusServiceWatcher(SSR_DBUS_NAME, QDBusConnection::systemBus(), QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &Scan::serviceOwnerChanged);

    m_dbusProxy = new BRDbusProxy(SSR_DBUS_NAME,
                                  BR_DBUS_OBJECT_PATH,
                                  QDBusConnection::systemBus(),
                                  this);
    // TODO ： 加固标准功能，暂未使用
    // if (m_dbusProxy->standard_type() == STANDARD_TYPE_SYSTEM)
    // else if (m_dbusProxy->standard_type() == STANDARD_TYPE_CUSTOM)
    auto reply = m_dbusProxy->GetCategories();
    reply.waitForFinished();
    CHECK_ERROR_FOR_DBUS_REPLY(reply)
    RETURN_IF_TRUE(reply.isError())
    Utils::getDefault()->jsonParsing(reply.value().toUtf8(), m_categories);
    Utils::getDefault()->ssrReinforcements(m_dbusProxy->GetReinforcements().value(), m_categories);

    // 如果分类中没有加固项，则因此此分类
    auto iter = std::remove_if(m_categories.begin(), m_categories.end(), [](Category *category)
                               {
                                   return category->getReinforcementItem().size() == 0;
                               });
    if (iter != m_categories.end())
    {
        m_categories.erase(iter);
    }
}

void Scan::initUI()
{
    m_ui->m_itemTable->setItem(m_categories);
    if (BRStrategyType(m_dbusProxy->strategy_type()) == BR_STRATEGY_TYPE_SYSTEM)
    {
        m_strategyType = BR_STRATEGY_TYPE_SYSTEM;
    }
    else
    {
        m_strategyType = BR_STRATEGY_TYPE_CUSTOM;
        connect(m_ui->m_itemTable, &ItemTable::modifyItemArgsClicked, this, &Scan::popReinforcecmentDialog);
    }

    m_customArgsDialog = new ReinforcementArgsDialog(this);
    m_customArgsDialog->hide();

    connect(m_customArgsDialog, &ReinforcementArgsDialog::okClicked, this, &Scan::setReinforcement);
    connect(m_customArgsDialog, &ReinforcementArgsDialog::argError, this, [this](const QString &error)
            {
                POPUP_MESSAGE_DIALOG(error)
            });
    connect(m_customArgsDialog,
            &ReinforcementArgsDialog::valueChanged,
            this,
            [this](const QString &reinforcementItem,
                   const QString &argLabel,
                   const QString &argValue,
                   KS::Protocol::WidgetType::Value type)
            {
                m_argTransfers.append(new ArgTransfer(reinforcementItem, argLabel, argValue, type));
            });
    connect(m_customArgsDialog, &ReinforcementArgsDialog::closed, this, [this]
            {
                m_argTransfers.clear();
            });
    connect(m_customArgsDialog, &ReinforcementArgsDialog::reseted, this, &Scan::argReset);
}

void Scan::initConnection()
{
    connect(m_ui->m_progress, &Progress::scanClicked, this, &Scan::startScan);
    connect(m_ui->m_progress, &Progress::reinforcementClicked, this, &Scan::startReinforcement);
    connect(m_ui->m_progress, &Progress::returnHomeClicked, this, &Scan::returnHomeClicked);
    connect(m_ui->m_progress, &Progress::generateReportClicked, this, &Scan::generateReport);
    connect(m_ui->m_progress, &Progress::exportStrategyClicked, this, &Scan::exportStrategy);
    connect(m_ui->m_progress, &Progress::importStrategyClicked, this, &Scan::importStrategy);
    connect(m_ui->m_progress, &Progress::resetStrategyClicked, this, &Scan::resetAllReinforcementItem);
    connect(m_ui->m_progress, &Progress::cancelClicked, this, &Scan::cancelProgress);

    connect(m_ui->m_itemTable, &ItemTable::modelEntered, this, &Scan::showErrorMessage);
}

void Scan::resetAllReinforcementItem()
{
    auto reply = m_dbusProxy->ResetReinforcements();
    reply.waitForFinished();
    CHECK_ERROR_FOR_DBUS_REPLY(reply)
    RETURN_IF_TRUE(reply.isError())
    auto allResetStr = m_dbusProxy->GetReinforcements();
    Utils::getDefault()->ssrResetReinforcements(allResetStr, m_categories);
}

bool Scan::checkCanExit()
{
    return PROCESS_METHOD_STANDBY == m_progressInfo.method;
}

void Scan::clearInvalidData()
{
    if (m_invalidData.NouserFilesList.count() != 0)
    {
        m_invalidData.NouserFilesList.clear();
    }
    if (m_invalidData.SuidSgidFilesList.count() != 0)
    {
        m_invalidData.SuidSgidFilesList.clear();
    }
    if (m_invalidData.AuthorityFilesList.count() != 0)
    {
        m_invalidData.AuthorityFilesList.clear();
    }
    if (m_invalidData.vulnerabilityScanInvalidList.count() != 0)
    {
        m_invalidData.vulnerabilityScanInvalidList.clear();
    }
}

void Scan::clearState()
{
    for (int i = 0; i < m_categories.length(); ++i)
    {
        CONTINUE_IF_TRUE(m_categories.at(i)->getReinforcementItem().length() == 0)

        switch (m_progressInfo.method)
        {
        case PROCESS_METHOD_FASTEN:
            m_categories.at(i)->clearState(BR_REINFORCEMENT_STATE_UNREINFORCE);
            break;
        case PROCESS_METHOD_SCAN:
            m_categories.at(i)->clearState(BR_REINFORCEMENT_STATE_UNSCAN);
            break;
        default:
            break;
        }
    }
}

void Scan::flushProgressInfo()
{
    m_progressInfo.jobID = -1;
    m_progressInfo.jobState = -1;
    m_progressInfo.successCount = 0;
    m_progressInfo.failureCount = 0;
    m_progressInfo.progress = 0;
}

void Scan::argReset(const QString &categoryName, const QString &argName)
{
    auto reply = m_dbusProxy->ResetReinforcement(categoryName);
    reply.waitForFinished();
    if (reply.isError())
    {
        POPUP_MESSAGE_DIALOG(tr("Failed to reset arg! Error message:%1").arg(reply.error().message()));
        return;
    }

    auto resetStr = m_dbusProxy->GetReinforcements();
    auto value = Utils::getDefault()->ssrResetReinforcement(resetStr, categoryName, argName);
    m_customArgsDialog->setValue(StrUtils::str2jsonValue(value));
}

void Scan::setReinforcement()
{
    for (auto argTransfer : m_argTransfers)
    {
        for (auto iter : m_categories)
        {
            auto label = iter->getLabel();
            auto reinforcementItem = iter->find(argTransfer->categoryName);
            CONTINUE_IF_TRUE(reinforcementItem == NULL)
            auto arg = reinforcementItem->find(argTransfer->argName);
            CONTINUE_IF_TRUE(arg == NULL)

            reinforcementItem->changeFlag = true;
            arg->jsonValue = StrUtils::str2jsonValue(argTransfer->value);
            arg->widgetType = argTransfer->widgetType;
            // str2jsonValue中的类型转换没法区分line输入纯数字和数字输入框spin输入的纯数字，都会被转为double类型，这里需要进行判断
            if (arg->widgetType == KS::Protocol::WidgetType::TEXT)
            {
                arg->jsonValue = QJsonValue::fromVariant(argTransfer->value);
            }
        }
    }

    auto reinforcementXML = Utils::getDefault()->ssrSetReinforcement(m_dbusProxy->GetReinforcements(), m_categories);
    KLOG_DEBUG() << "reinforcement item xml is :" << reinforcementXML;

    for (auto xml : reinforcementXML)
    {
        CONTINUE_IF_TRUE(xml.isEmpty())
        m_dbusProxy->SetReinforcement(xml);
    }
    m_argTransfers.clear();
}

bool Scan::checkAndSetCheckbox()
{
    auto checkedList = m_ui->m_itemTable->checkedAllStatus();
    if (checkedList.count() == 0)
    {
        POPUP_MESSAGE_DIALOG(tr("Please select the item to export!"))
        return false;
    }

    std::istringstream istringStream(m_dbusProxy->GetReinforcements().value().toStdString());
    auto rsReinforcements = KS::Protocol::br_reinforcements(istringStream, xml_schema::Flags::dont_validate);
    auto rsReinforcement = rsReinforcements.get()->reinforcement();

    for (auto iter : m_categories)
    {
        // 勾选item等同于修改值，向后台发送修改请求，实际上不修改加固项的值，将勾选的项添加到RA文件
        for (auto checkedItem : checkedList)
        {
            for (auto reinforcementItem : iter->getReinforcementItem())
            {
                CONTINUE_IF_TRUE(reinforcementItem->getLabel() != checkedItem)
                for (auto rsIter : rsReinforcement)
                {
                    CONTINUE_IF_TRUE(reinforcementItem->getName() != rsIter.name().c_str())
                    reinforcementItem->changeFlag = true;
                }

                auto reinforcementXML = Utils::getDefault()->ssrSetReinforcement(m_dbusProxy->GetReinforcements(), m_categories);
                for (auto xml : reinforcementXML)
                {
                    CONTINUE_IF_TRUE(xml.isEmpty())
                    m_dbusProxy->SetReinforcement(xml);
                }
            }
        }
    }

    // 读ra文件，设置复选框
    auto raReinforcements = Utils::getDefault()->raAnalysis(SSR_BR_CUSTOM_RA_FILEPATH);
    for (auto iter : m_categories)
    {
        for (auto raReinforcement = raReinforcements.begin(); raReinforcement != raReinforcements.end(); ++raReinforcement)
        {
            auto reinforcementItem = iter->find(raReinforcement->name().c_str());
            CONTINUE_IF_TRUE(reinforcementItem == NULL)
            m_dbusProxy->SetCheckBox(raReinforcement->name().c_str(), m_ui->m_itemTable->checkedArgStatus(reinforcementItem->getLabel()));
        }
    }
    return true;
}

void Scan::serviceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner)
{
    // Note that this signal is also emitted whenever the serviceName service was registered or unregistered.
    // If it was registered, oldOwner will contain an empty string,
    // whereas if it was unregistered, newOwner will contain an empty string
    if (newOwner.isEmpty())
    {
        KLOG_ERROR() << "QDBusServiceWatcher got" << service << " was exit";

        if (isVisible())
        {
            auto messageDialog = new KS::MessageDialog(this);
            messageDialog->setMessage(tr("Listening service stop, return to initial page"));

            adjustWidgetPosition(messageDialog);
            messageDialog->show();
        }

        emit returnHomeClicked();
    }
}

void Scan::adjustWidgetPosition(QWidget *widget)
{
    QObject *p = this;
    while (p->parent())
    {
        p = p->parent();
    }
    QWidget *topParentWidget = (QWidget *)p;
    QRect rect = topParentWidget->geometry();
    widget->move(rect.x() + (rect.width() - widget->width()) / 2,
                 rect.y() + ((rect.height() - widget->height()) / 2));
}

void Scan::startScan()
{
    // 设置页面定时扫描时会操作这个信号，为保证不起冲突，每次扫描时断开后重新连接
    connect(m_dbusProxy, &BRDbusProxy::ScanProgress, this, &Scan::runProgress);
    m_progressInfo.method = PROCESS_METHOD_SCAN;
    clearState();
    // 清空扫描文件数据
    clearInvalidData();
    // TODO 托盘功能是否有必要
    //    if (is_minTray)
    //        showNormal();
    auto scanItems = m_strategyType == BR_STRATEGY_TYPE_CUSTOM
                         ? m_ui->m_itemTable->getString(m_categories)
                         : m_ui->m_itemTable->getAllString(m_categories);
    if (scanItems.empty())
    {
        POPUP_MESSAGE_DIALOG(tr("Please check the reinforcement items to be scanned or reinforcement classification for scanning."))
        m_ui->m_progress->resetProgress();
        return;
    }
    m_ui->m_progress->updateProgressUI(m_progressInfo.method);

    m_dbusProxy->Scan(scanItems);
    update();
}

void Scan::startReinforcement()
{
    auto reinforcementItem = m_strategyType == BR_STRATEGY_TYPE_CUSTOM ? m_ui->m_itemTable->getString(m_categories) : m_ui->m_itemTable->getAllString(m_categories);
    if (reinforcementItem.empty())
    {
        POPUP_MESSAGE_DIALOG(tr("Please check the content to be reinforced."))
        return;
    }

    auto reply = m_dbusProxy->Reinforce(reinforcementItem);
    CHECK_ERROR_FOR_DBUS_REPLY(reply);
    RETURN_IF_TRUE(reply.isError());
    // 设置页面回退会进行加固，为保证不起冲突，每次加固时断开后重新连接
    connect(m_dbusProxy, &BRDbusProxy::ReinforceProgress, this, &Scan::runProgress);
    m_progressInfo.method = PROCESS_METHOD_FASTEN;
    clearState();
    m_ui->m_itemTable->clearCheckedStatus(m_categories, BR_REINFORCEMENT_STATE_UNREINFORCE);
    m_ui->m_progress->updateProgressUI(m_progressInfo.method);
    m_ui->m_progress->updateProgress(m_progressInfo);
    disconnect(m_ui->m_itemTable, &ItemTable::modifyItemArgsClicked, this, &Scan::popReinforcecmentDialog);
    update();
}

void Scan::generateReport()
{
    // test dbus export
    //    m_dbusProxy->ExportReport("/root/Desktop/br.pdf");
    //    return;

    KLOG_DEBUG() << "generate reports !";

    static QList<Category *> categories;
    while (!categories.isEmpty())
    {
        Category *p = categories.takeFirst();
        delete p;
    }

    auto reply = m_dbusProxy->GetCategories();
    reply.waitForFinished();
    Utils::getDefault()->jsonParsing(reply.value().toUtf8(), categories);
    Utils::getDefault()->ssrReinforcements(m_dbusProxy->GetReinforcements().value(), categories);

    connect(m_dbusProxy, &BRDbusProxy::ScanProgress, this, [this](const QString &jobResult)
            {
                ProgressInfo progressInfo;
                progressInfo.method = PROCESS_METHOD_SCAN;
                Utils::getDefault()->ssrJobResult(jobResult, progressInfo, categories, m_invalidData);
                if (double(100) == progressInfo.progress)
                {
                    // 扫描完成,断开信号
                    disconnect(m_dbusProxy, &BRDbusProxy::ScanProgress, 0, 0);
                }
            });
    // 监听进程完成后 导出报表
    connect(m_dbusProxy, &BRDbusProxy::ProgressFinished, this, [this]
            {
                m_progressInfo.method = PROCESS_METHOD_STANDBY;
                disconnect(m_dbusProxy, &BRDbusProxy::ProgressFinished, 0, 0);
                RETURN_IF_TRUE(!Report::getDefault()->generateReports(categories, LicenseActivationStatus::LAS_ACTIVATED, m_invalidData))
                POPUP_MESSAGE_DIALOG(tr("Export succeeded!"))
                m_dbusProxy->GenerateReport(true);
            });
    // 生成报表前扫描,全量
    auto scanItems = m_ui->m_itemTable->getAllString(m_categories);
    m_progressInfo.method = PROCESS_METHOD_SCAN;
    m_dbusProxy->Scan(scanItems);
}

bool Scan::cancelProgress()
{
    // 进程未开始，不允许忽略，无提示
    RETURN_VAL_IF_TRUE(double(0) == m_progressInfo.progress, true);
    auto reply = m_dbusProxy->Cancel(m_progressInfo.jobID);
    reply.waitForFinished();
    if (reply.isError())
    {
        POPUP_MESSAGE_DIALOG((reply).error().message());
        return false;
    }

    m_ui->m_progress->stopWorkingProcess();

    m_progressInfo.method = PROCESS_METHOD_STANDBY;

    return true;
}

void Scan::showErrorMessage(const QModelIndex &model)
{
    RETURN_IF_TRUE(model.parent().row() < 0);

    // 判断内容是否显示完整
    auto itemRect = m_ui->m_itemTable->visualRect(model);
    // 计算文本宽度
    QFontMetrics metrics(this->font());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    auto textWidth = metrics.horizontalAdvance(m_ui->m_itemTable->model()->data(model).toString());
#else
    auto textWidth = metrics.width(m_ui->m_itemTable->model()->data(model).toString());
#endif
    if (textWidth > itemRect.width())
    {
        auto mod = m_ui->m_itemTable->selectionModel()->model()->data(model);
        QToolTip::showText(QCursor::pos(), mod.toString(), this, rect(), 5000);
    }

    // 错误消息显示
    RETURN_IF_TRUE(model.column() != 2);
    auto indexCategories = model.parent().row();
    auto indexCategory = model.row();
    auto reinforcementItem = m_categories.at(indexCategories)->getReinforcementItem().at(indexCategory);
    if (reinforcementItem->getState() == BR_REINFORCEMENT_STATE_REINFORCE_ERROR || reinforcementItem->getState() == BR_REINFORCEMENT_STATE_SCAN_ERROR)
    {
        QToolTip::showText(QCursor::pos(), reinforcementItem->getErrorMessage(), this, rect(), 5000);
    }
}

void Scan::popReinforcecmentDialog(const QModelIndex &model)
{
    m_customArgsDialog->clear();
    RETURN_IF_TRUE(model.column() != 1)
    if (model.parent().row() < 0)
    {
        auto indexCategories = model.row();
        auto categories = m_categories.at(indexCategories)->getReinforcementItem();
        for (auto reinforcementItem : categories)
        {
            auto args = reinforcementItem->getArgs();
            for (auto arg : args)
            {
                m_customArgsDialog->addLine(reinforcementItem->getName(),
                                            arg->name,
                                            arg->label,
                                            arg->valueLimits,
                                            arg->inputExample,
                                            arg->jsonValue,
                                            arg->widgetType,
                                            arg->note);
            }
        }
    }
    else
    {
        auto indexCategories = model.parent().row();
        auto indexCategory = model.row();
        auto args = m_categories.at(indexCategories)->getReinforcementItem().at(indexCategory)->getArgs();
        for (auto arg : args)
        {
            m_customArgsDialog->addLine(m_categories.at(indexCategories)->getReinforcementItem().at(indexCategory)->getName(),
                                        arg->name,
                                        arg->label,
                                        arg->valueLimits,
                                        arg->inputExample,
                                        arg->jsonValue,
                                        arg->widgetType,
                                        arg->note);
        }
    }

    int x = window()->x() + window()->width() / 2 - m_customArgsDialog->width() / 2;
    int y = window()->y() + window()->height() / 2 - m_customArgsDialog->getHeight() / 2;
    m_customArgsDialog->move(x, y);
    m_customArgsDialog->show();
}

void Scan::runProgress(const QString &jobResult)
{
    m_progressInfo.total = m_ui->m_itemTable->getCount();
    Utils::getDefault()->ssrJobResult(jobResult, m_progressInfo, m_categories, m_invalidData);
    // TODO 确认在扫描完成之后是否允许修改勾选的加固项 m_progressInfo.method == PROCESS_METHOD_FASTEN
    m_ui->m_itemTable->setAllCheckBoxEditStatus(false);
    if (double(100) == m_progressInfo.progress)
    {
        m_ui->m_itemTable->getProgressCount(m_categories, m_progressInfo);
        m_ui->m_progress->updateProgress(m_progressInfo);
        flushProgressInfo();
        if (m_progressInfo.method == PROCESS_METHOD_FASTEN)
        {
            emit reinforcementFinished();
        }

        disconnect(m_dbusProxy, &BRDbusProxy::ScanProgress, 0, 0);
        disconnect(m_dbusProxy, &BRDbusProxy::ProgressFinished, 0, 0);

        m_progressInfo.method = PROCESS_METHOD_STANDBY;
    }
    else
    {
        m_ui->m_progress->updateProgress(m_progressInfo);
    }
    m_ui->m_itemTable->updateStatus(m_categories);
}
}  // namespace BR
}  // namespace KS
