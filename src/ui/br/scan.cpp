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
#include "scan.h"
#include <kylin-license/license-i.h>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QToolTip>
#include "br-i.h"
#include "include/ssr-i.h"
#include "include/ssr-marcos.h"
#include "lib/base/str-utils.h"
#include "src/ui/br/reports/result.h"
#include "src/ui/br/xmlutils.h"
#include "src/ui/br_dbus_proxy.h"
#include "src/ui/common/ssr-marcos-ui.h"
#include "ui_scan.h"

namespace KS
{
namespace BR
{
Scan::Scan(QWidget *parent) : QWidget(parent),
                              m_ui(new Ui::Scan)
{
    m_ui->setupUi(this);

    init();
    initConnection();
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
    KLOG_DEBUG() << "use system strategy.";
    resetAllReinforcementItem();
    m_ui->m_itemTable->setAllCheckBoxEditStatus(false);
    m_ui->m_itemTable->hideCheckBox(true);
    disconnect(m_ui->m_itemTable, SIGNAL(modifyItemArgsClicked(QModelIndex)), this, SLOT(popReinforcecmentDialog(QModelIndex)));
}

void Scan::usingCustomStrategy()
{
    KLOG_DEBUG() << "use custom strategy.";
    disconnect(m_ui->m_itemTable, SIGNAL(modifyItemArgsClicked(QModelIndex)), this, SLOT(popReinforcecmentDialog(QModelIndex)));
    connect(m_ui->m_itemTable, SIGNAL(modifyItemArgsClicked(QModelIndex)), this, SLOT(popReinforcecmentDialog(QModelIndex)));
    // 修改UI界面参数以及复选框状态
    m_ui->m_itemTable->setAllCheckBoxEditStatus(true);
    m_ui->m_itemTable->setAllChecked(Qt::Unchecked);
    m_ui->m_itemTable->hideCheckBox(false);
    // 所有状态重置后再修改
    resetAllReinforcementItem();

    auto raReinforcements = XMLUtils::getDefault()->raAnalysis(SSR_BR_CUSTOM_RA_STRATEGY_FILEPATH);
    if (raReinforcements.empty())
    {
        m_ui->m_itemTable->setAllChecked(Qt::Checked);
    }
    else
    {
        for (auto iter : m_categoriesList)
        {
            for (auto raReinforcement = raReinforcements.begin(); raReinforcement != raReinforcements.end(); ++raReinforcement)
            {
                auto category = iter->find(raReinforcement->name().c_str());
                CONTINUE_IF_TRUE(category == NULL)
                bool raCheckbox = false;
                // TODO 尝试不用try 这里的checkbox（）可能不存在
                try
                {
                    raCheckbox = raReinforcement->checkbox().get();
                }
                catch (const std::exception &e)
                {
                    KLOG_WARNING("%s", e.what());
                }

                m_ui->m_itemTable->setArgChecked(category->getLabel(), raCheckbox);

                category->changeFlag = true;
                // 未勾选不修改值 #14216
                CONTINUE_IF_TRUE(!raCheckbox)
                for (auto raArg = raReinforcement->arg().begin(); raArg != raReinforcement->arg().end(); ++raArg)
                {
                    auto arg = category->find(raArg->name().c_str());
                    CONTINUE_IF_TRUE(arg == NULL)
                    arg->jsonValue = StrUtils::str2jsonValue(raArg->value());
                }
            }
        }

        auto reinforcementXML = XMLUtils::getDefault()->ssrSetReinforcement(m_dbusProxy->GetReinforcements(), m_categoriesList);
        for (auto xml : reinforcementXML)
        {
            CONTINUE_IF_TRUE(xml == NULL)
            auto reply = m_dbusProxy->SetReinforcement(xml);
            CHECK_ERROR_FOR_DBUS_REPLY(reply)
        }
    }
}

void Scan::reset()
{
    m_ui->m_progress->resetProgress();
    clearState();
}

bool Scan::exportStrategy()
{
    RETURN_VAL_IF_TRUE(!checkAndSetCheckbox(), false);

    // 导出自定义策略，xml格式
    auto fileName = QFileDialog::getSaveFileName(this, tr("Files"), "./br-strategy.xml", tr("strategy(*.xml)"));
    RETURN_VAL_IF_TRUE(fileName.isEmpty(), false)

    // 打开要写入的文件
    QFile fileSave(fileName);
    if (!fileSave.open(QIODevice::WriteOnly))
    {
        // 无法打开
        KLOG_WARNING() << "Please check the file name and whether you have write permission!";
        POPUP_MESSAGE_DIALOG(tr("Please check the file name and whether you have write permission!"))
        return false;
    }

    // 打开ra文件
    QFile file(SSR_BR_CUSTOM_RA_FILEPATH);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        KLOG_WARNING() << "Open RA file failed!";
        POPUP_MESSAGE_DIALOG(tr("Open RA file failed!"))
        return false;
    }

    // 写入文件
    POPUP_MESSAGE_DIALOG(fileSave.write(file.readAll()) ? tr("Export successed!") : tr("Export failed!"))
    file.close();
    fileSave.close();
    return true;
}

void Scan::init()
{
    m_dbusProxy = new BRDbusProxy(SSR_DBUS_NAME,
                                  BR_DBUS_OBJECT_PATH,
                                  QDBusConnection::systemBus(),
                                  this);
    // TODO ： 加固标准功能，暂未使用
    // if (m_dbusProxy->standard_type() == STANDARD_TYPE_SYSTEM)
    // else if (m_dbusProxy->standard_type() == STANDARD_TYPE_CUSTOM)
    auto reply = m_dbusProxy->GetCategories();
    reply.waitForFinished();
    RETURN_IF_TRUE(reply.isError())
    XMLUtils::getDefault()->jsonParsing(reply.value().toUtf8(), m_categoriesList);
    XMLUtils::getDefault()->ssrReinforcements(m_dbusProxy->GetReinforcements().value(), m_categoriesList);
}

void Scan::initUI()
{
    m_ui->m_itemTable->setItem(m_categoriesList);
    if (BRStrategyType(m_dbusProxy->strategy_type()) == BR_STRATEGY_TYPE_SYSTEM)
    {
        m_strategyType = BR_STRATEGY_TYPE_SYSTEM;
        resetAllReinforcementItem();
        m_ui->m_itemTable->hideCheckBox(true);
    }
    else
    {
        m_strategyType = BR_STRATEGY_TYPE_CUSTOM;
        m_ui->m_itemTable->hideCheckBox(false);

        connect(m_ui->m_itemTable, SIGNAL(modifyItemArgsClicked(QModelIndex)), this, SLOT(popReinforcecmentDialog(QModelIndex)));
    }

    m_customArgsDialog = new Plugins::CustomArgs(this);
    m_customArgsDialog->hide();

    connect(m_customArgsDialog, &Plugins::CustomArgs::okClicked, this, &Scan::setReinforcement);
    connect(m_customArgsDialog, &Plugins::CustomArgs::argError, this, [this](const QString &error)
            { POPUP_MESSAGE_DIALOG(error) });
    connect(m_customArgsDialog, &Plugins::CustomArgs::valueChanged, this, [this](const QString &category, const QString &argLabel, const QString &argValue, KS::Protocol::WidgetType::Value type)
            { m_argTransfers.append(new ArgTransfer(category, argLabel, argValue, type)); });
    connect(m_customArgsDialog, &Plugins::CustomArgs::closed, this, [this]
            { m_argTransfers.clear(); });
    connect(m_customArgsDialog, &Plugins::CustomArgs::reseted, this, &Scan::argReset);
}

void Scan::initConnection()
{
    connect(m_ui->m_progress, &Progress::scanClicked, this, &Scan::startScan);
    connect(m_ui->m_progress, &Progress::reinforcementClicked, this, &Scan::startReinforcement);
    connect(m_ui->m_progress, &Progress::returnHomeClicked, this, &Scan::returnHomeClicked);
    connect(m_ui->m_progress, &Progress::generateReportClicked, this, &Scan::generateReport);
    connect(m_ui->m_progress, &Progress::cancelClicked, this, &Scan::cancelProgress);

    connect(m_ui->m_itemTable, SIGNAL(modelEntered(QModelIndex)), this, SLOT(showErrorMessage(QModelIndex)));

    //    connect(m_dbusProxy, SIGNAL(standardChanged(uint)), this, SLOT(standardTypeChanged(uint)));
    connect(m_dbusProxy, SIGNAL(ReinforceProgress(QString)), this, SLOT(runProgress(QString)));
}

void Scan::resetAllReinforcementItem()
{
    auto reply = m_dbusProxy->ResetReinforcements();
    reply.waitForFinished();
    CHECK_ERROR_FOR_DBUS_REPLY(reply)
    auto allResetStr = m_dbusProxy->GetReinforcements();
    XMLUtils::getDefault()->ssrResetReinforcements(allResetStr, m_categoriesList);
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
    for (int i = 0; i < m_categoriesList.length(); ++i)
    {
        CONTINUE_IF_TRUE(m_categoriesList.at(i)->getCategory().length() == 0)

        switch (m_progressInfo.method)
        {
        case PROCESS_METHOD_FASTEN:
            m_categoriesList.at(i)->clearState(BR_REINFORCEMENT_STATE_UNREINFORCE);
            break;
        case PROCESS_METHOD_SCAN:
            m_categoriesList.at(i)->clearState(BR_REINFORCEMENT_STATE_UNSCAN);
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
    m_dbusProxy->ResetReinforcement(categoryName);
    auto resetStr = m_dbusProxy->GetReinforcements();
    auto value = XMLUtils::getDefault()->ssrResetReinforcement(resetStr, categoryName, argName);
    m_customArgsDialog->setValue(StrUtils::str2jsonValue(value));
}

void Scan::setReinforcement()
{
    for (auto argTransfer : m_argTransfers)
    {
        for (auto iter : m_categoriesList)
        {
            auto label = iter->getLabel();
            auto category = iter->find(argTransfer->categoryName);
            CONTINUE_IF_TRUE(category == NULL)
            auto arg = category->find(argTransfer->argName);
            CONTINUE_IF_TRUE(arg == NULL)

            category->changeFlag = true;
            arg->jsonValue = StrUtils::str2jsonValue(argTransfer->value);
            arg->widgetType = argTransfer->widgetType;
        }
    }

    auto reinforcementXML = XMLUtils::getDefault()->ssrSetReinforcement(m_dbusProxy->GetReinforcements(), m_categoriesList);
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

    for (auto iter : m_categoriesList)
    {
        // 勾选item等同于修改值，向后台发送修改请求，实际上不修改加固项的值，将勾选的项添加到RA文件
        for (auto checkedItem : checkedList)
        {
            for (auto category : iter->getCategory())
            {
                CONTINUE_IF_TRUE(category->getLabel() != checkedItem)
                for (auto rsIter : rsReinforcement)
                {
                    CONTINUE_IF_TRUE(category->getName() != rsIter.name().c_str())
                    category->changeFlag = true;
                }

                auto reinforcementXML = XMLUtils::getDefault()->ssrSetReinforcement(m_dbusProxy->GetReinforcements(), m_categoriesList);
                for (auto xml : reinforcementXML)
                {
                    CONTINUE_IF_TRUE(xml.isEmpty())
                    m_dbusProxy->SetReinforcement(xml);
                }
            }
        }
    }

    // 读ra文件，设置复选框
    auto raReinforcements = XMLUtils::getDefault()->raAnalysis(SSR_BR_CUSTOM_RA_FILEPATH);
    for (auto iter : m_categoriesList)
    {
        for (auto raReinforcement = raReinforcements.begin(); raReinforcement != raReinforcements.end(); ++raReinforcement)
        {
            auto category = iter->find(raReinforcement->name().c_str());
            CONTINUE_IF_TRUE(category == NULL)
            m_dbusProxy->SetCheckBox(raReinforcement->name().c_str(), m_ui->m_itemTable->checkedArgStatus(category->getLabel()));
        }
    }
    return true;
}

void Scan::startScan()
{
    // 设置页面定时扫描时会操作这个信号，为保证不起冲突，每次扫描时断开后重新连接
    disconnect(m_dbusProxy, SIGNAL(ScanProgress(QString)), 0, 0);
    connect(m_dbusProxy, SIGNAL(ScanProgress(QString)), this, SLOT(runProgress(QString)));
    m_progressInfo.method = PROCESS_METHOD_SCAN;
    clearState();
    // 清空扫描文件数据
    clearInvalidData();
    // TODO 托盘功能是否有必要
    //    if (is_minTray)
    //        showNormal();
    auto scanItems = BRStrategyType(m_dbusProxy->strategy_type()) == BR_STRATEGY_TYPE_CUSTOM ? m_ui->m_itemTable->getString(m_categoriesList) : m_ui->m_itemTable->getAllString(m_categoriesList);
    if (scanItems.empty())
    {
        POPUP_MESSAGE_DIALOG(tr("Please check the reinforcement items to be scanned or reinforcement classification for scanning."))
        m_ui->m_progress->resetProgress();
        return;
    }
    m_ui->m_progress->updateProgressUI(m_progressInfo.method);
    disconnect(m_ui->m_itemTable, SIGNAL(modifyItemArgsClicked(QModelIndex)), this, SLOT(popReinforcecmentDialog(QModelIndex)));

    m_dbusProxy->Scan(scanItems);
    update();
}

void Scan::startReinforcement()
{
    m_progressInfo.method = PROCESS_METHOD_FASTEN;
    clearState();
    m_ui->m_itemTable->clearCheckedStatus(m_categoriesList, BR_REINFORCEMENT_STATE_UNREINFORCE);

    auto reinforcementItem = BRStrategyType(m_dbusProxy->strategy_type()) == BR_STRATEGY_TYPE_CUSTOM ? m_ui->m_itemTable->getString(m_categoriesList) : m_ui->m_itemTable->getAllString(m_categoriesList);
    if (reinforcementItem.empty())
    {
        POPUP_MESSAGE_DIALOG(tr("Please check the content to be reinforced."))
        return;
    }

    m_ui->m_progress->updateProgressUI(m_progressInfo.method);

    disconnect(m_ui->m_itemTable, SIGNAL(modifyItemArgsClicked(QModelIndex)), this, SLOT(popReinforcecmentDialog(QModelIndex)));
    auto reply = m_dbusProxy->Reinforce(reinforcementItem);
    reply.waitForFinished();
    CHECK_ERROR_FOR_DBUS_REPLY(reply)
    update();
}

void Scan::generateReport()
{
    KLOG_DEBUG() << "generate reports !";
    if (m_progressInfo.method == PROCESS_METHOD_FASTEN)
    {
        auto reply = m_dbusProxy->GetCategories();
        reply.waitForFinished();
        XMLUtils::getDefault()->jsonParsing(reply.value().toUtf8(), m_afterReinForcementCategoriesList);
        XMLUtils::getDefault()->ssrReinforcements(m_dbusProxy->GetReinforcements().value(), m_afterReinForcementCategoriesList);
    }
    // 断开scan进程连接
    disconnect(m_dbusProxy, &BRDbusProxy::ScanProgress, 0, 0);
    // 进行一次扫描 仅获取扫描结果，不对UI进行调整
    connect(m_dbusProxy, &BRDbusProxy::ScanProgress, this, [this](const QString &jobResult)
            {
                ProgressInfo progressInfo;
                // 获取加固后扫描结果
                XMLUtils::getDefault()->ssrJobResult(jobResult, progressInfo, m_afterReinForcementCategoriesList, m_invalidData);
            });
    // 监听进程完成后 导出报表
    disconnect(m_dbusProxy, &BRDbusProxy::ProgressFinished, 0, 0);
    connect(m_dbusProxy, &BRDbusProxy::ProgressFinished, this, [this]
            {
                disconnect(m_dbusProxy, &BRDbusProxy::ScanProgress, 0, 0);
                connect(m_dbusProxy, SIGNAL(ScanProgress(QString)), this, SLOT(runProgress(QString)));
                disconnect(m_dbusProxy, &BRDbusProxy::ProgressFinished, 0, 0);
                RETURN_IF_TRUE(!Reports::Result::getDefault()->generateReports(m_categoriesList, m_afterReinForcementCategoriesList, LicenseActivationStatus::LAS_ACTIVATED, m_invalidData))
                POPUP_MESSAGE_DIALOG(tr("Export succeeded!"))
                m_afterReinForcementCategoriesList.clear();
            });
    // 生成报表前扫描
    m_progressInfo.method = PROCESS_METHOD_SCAN;
    auto scanItems = BRStrategyType(m_dbusProxy->strategy_type()) == BR_STRATEGY_TYPE_CUSTOM ? m_ui->m_itemTable->getString(m_categoriesList) : m_ui->m_itemTable->getAllString(m_categoriesList);
    m_dbusProxy->Scan(scanItems);
}

void Scan::cancelProgress()
{
    auto reply = m_dbusProxy->Cancel(m_progressInfo.jobID);
    reply.waitForFinished();
    CHECK_ERROR_FOR_DBUS_REPLY(reply)
    flushProgressInfo();
}

void Scan::showErrorMessage(const QModelIndex &model)
{
    RETURN_IF_TRUE(model.parent().row() < 0)
    RETURN_IF_TRUE(model.column() != 2)

    auto indexCategories = model.parent().row();
    auto indexCategory = model.row();
    auto category = m_categoriesList.at(indexCategories)->getCategory().at(indexCategory);
    if (category->getState() == BR_REINFORCEMENT_STATE_REINFORCE_ERROR || category->getState() == BR_REINFORCEMENT_STATE_SCAN_ERROR)
    {
        QToolTip::showText(QCursor::pos(), category->getErrorMessage(), this, rect(), 2000);
    }
}

void Scan::popReinforcecmentDialog(const QModelIndex &model)
{
    m_customArgsDialog->clear();
    RETURN_IF_TRUE(model.column() != 1)
    if (model.parent().row() < 0)
    {
        auto indexCategories = model.row();
        auto categories = m_categoriesList.at(indexCategories)->getCategory();
        for (auto category : categories)
        {
            auto args = category->getArgs();
            for (auto arg : args)
            {
                m_customArgsDialog->addOneLine(category->getName(), arg->name, arg->label, arg->valueLimits, arg->inputExample, arg->jsonValue, arg->widgetType, arg->note);
            }
        }
    }
    else
    {
        auto indexCategories = model.parent().row();
        auto indexCategory = model.row();
        auto args = m_categoriesList.at(indexCategories)->getCategory().at(indexCategory)->getArgs();
        for (auto arg : args)
        {
            m_customArgsDialog->addOneLine(m_categoriesList.at(indexCategories)->getCategory().at(indexCategory)->getName(), arg->name, arg->label, arg->valueLimits, arg->inputExample, arg->jsonValue, arg->widgetType, arg->note);
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
    XMLUtils::getDefault()->ssrJobResult(jobResult, m_progressInfo, m_categoriesList, m_invalidData);
    m_ui->m_itemTable->setAllCheckBoxEditStatus(false);

    if (double(100) == m_progressInfo.progress)
    {
        m_ui->m_itemTable->getProgressCount(m_categoriesList, m_progressInfo);
        m_ui->m_progress->updateProgress(m_progressInfo);
        flushProgressInfo();
        if (m_progressInfo.method == PROCESS_METHOD_FASTEN)
        {
            emit reinforcementFinished();
        }
    }
    else
    {
        m_ui->m_progress->updateProgress(m_progressInfo);
    }
    m_ui->m_itemTable->updateStatus(m_categoriesList);
}
}  // namespace BR
}  // namespace KS
