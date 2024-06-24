/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinsec.com.cn>
 */

#include "job-manager.h"
#include <qt5-log-i.h>
#include <ssr-i.h>
#include <QJsonObject>
#include "configuration.h"
#include "plugins.h"

namespace KS
{
namespace BR
{
#define JOB_ERROR_STR "error"
#define JOB_RETURN_VALUE "return_value"
#define RH_BR_OPERATE_DATA_FIRST SSR_INSTALL_DATADIR "/br-rh-first.xml"
#define RH_BR_OPERATE_DATA_LAST SSR_INSTALL_DATADIR "/br-rh-last.xml"

JobManager::JobManager(Configuration* configuration, Plugins* plugins, QObject* parent)
    : QObject(parent),
      m_configuration(configuration),
      m_plugins(plugins),
      m_scanJobResult(0, 0, 0),
      m_reinforceJobResult(0, 0, 0),
      isFallback(false)
{
}

void JobManager::init()
{
}

bool JobManager::scanAll()
{
    return scan(m_plugins->getReinforcementNames());
}

bool JobManager::scan(const QStringList& names)
{
    // 已经在扫描则返回错误
    if (this->m_scanJob && this->m_scanJob->getState() == BRJobState::BR_JOB_STATE_RUNNING)
    {
        KLOG_WARNING() << "Already scanning.";
        return false;
    }

    initScanResult(names);
    this->m_scanJob = Job::create();

    for (auto iter = names.begin(); iter != names.end(); ++iter)
    {
        auto& name = (*iter);
        auto reinforcement = m_plugins->getReinforcement(name);

        if (!reinforcement)
        {
            KLOG_WARNING() << "Reinforcement" << name << "not found.";
            return false;
        }

        auto reinforcementInterface = this->m_plugins->getReinforcementInterface(reinforcement->getPluginName(),
                                                                                 reinforcement->getName());
        if (!reinforcementInterface)
        {
            KLOG_WARNING() << "Reinforcement interface not found for" << name << ".";
            return false;
        }

        this->m_scanJob->addOperation(reinforcement->getPluginName(),
                                      reinforcement->getName(),
                                      [reinforcementInterface]() -> QString
                                      {
                                          QJsonObject retval;
                                          QString args;
                                          QString error;
                                          if (reinforcementInterface->get(args, error))
                                          {
                                              retval[JOB_RETURN_VALUE] = StrUtils::str2jsonObject(args);
                                          }
                                          else
                                          {
                                              retval[JOB_ERROR_STR] = error;
                                          }
                                          return StrUtils::json2str(retval);
                                      });
    }

    if (!this->m_scanJob->runAsync())
    {
        return false;
    }

    connect(this->m_scanJob.data(), &Job::processChanged, this, &JobManager::processScanProgress);
    connect(this->m_scanJob.data(), &Job::processFinished, this, &JobManager::processScanFinished);

    return true;
}

uint JobManager::getScanStatus()
{
    if (!m_scanJob)
    {
        return BRJobState::BR_JOB_STATE_IDLE;
    }
    else
    {
        return m_scanJob->getState();
    }
}

Protocol::JobResult JobManager::getScanResult()
{
    return m_scanJobResult;
}

bool JobManager::reinforceAll()
{
    return reinforce(m_plugins->getReinforcementNames());
}

bool JobManager::reinforce(const QStringList& names)
{
    // 加固前需要先进行扫描，如果已经处于扫描状态了则不允许进行加固
    if (this->m_scanJob && this->m_scanJob->getState() == BRJobState::BR_JOB_STATE_RUNNING)
    {
        KLOG_WARNING() << "Already scanning.";
        return false;
    }

    // 已经在加固则返回错误
    if (this->m_reinforceJob && this->m_reinforceJob->getState() == BRJobState::BR_JOB_STATE_RUNNING)
    {
        KLOG_WARNING() << "Already reinforcing.";
        return false;
    }

    // 需要先扫描记录系统状态，这里需要进行全量扫描来记录所有加固项在系统中的状态，如果加固有问题可以进行回退
    if (!scanAll())
    {
        return false;
    }
    // 必须放在scanAll之后执行，因为如果scanAll返回false，isReinforce状态不会被置为false
    isReinforce = true;
    m_scanBeforeReinforceConnection = connect(this, &JobManager::scanFinished,
                                              std::bind(&JobManager::processScanResultBeforeReinforce, this, names));

    return true;
}

uint JobManager::getReinforceStatus()
{
    if (!m_reinforceJob)
    {
        return BRJobState::BR_JOB_STATE_IDLE;
    }
    // 加固之前需要先扫描，此时m_reinforceJob还未启动，扫描期间应该也要算作任务正在运行
    if (isReinforce && m_reinforceJob->getState() == BRJobState::BR_JOB_STATE_IDLE)
    {
        return BRJobState::BR_JOB_STATE_RUNNING;
    }
    return m_reinforceJob->getState();
}

Protocol::JobResult JobManager::getReinforceResult()
{
    return m_reinforceJobResult;
}

bool JobManager::fallback(int method)
{
    // 已经在加固则返回错误
    if (this->m_reinforceJob && this->m_reinforceJob->getState() == BRJobState::BR_JOB_STATE_RUNNING)
    {
        KLOG_WARNING() << "Already reinforcing.";
        return false;
    }

    QString rhFile;
    switch (method)
    {
    case BRFallbackMethod::BR_FALLBACK_METHOD_INITIAL:
        rhFile = RH_BR_OPERATE_DATA_FIRST;
        break;
    case BRFallbackMethod::BR_FALLBACK_METHOD_LAST:
        rhFile = RH_BR_OPERATE_DATA_LAST;
        break;
    default:
        KLOG_WARNING() << "Unsupport fallback method.";
        return false;
    }

    QStringList reinforcementNames;
    auto rh = m_configuration->readRhFromFile(rhFile);
    for (const auto& reinforcement : rh->reinforcement())
    {
        if (!m_configuration->setCustomRA(reinforcement))
        {
            KLOG_ERROR() << "Fallback reinforcement to custom reinforcement argument failed for" << reinforcement.name().c_str();
            continue;
        }
        reinforcementNames.push_back(reinforcement.name().c_str());
    }

    if (!startReinforce(reinforcementNames))
    {
        return false;
    }
    isFallback = true;

    connect(this, &JobManager::reinforceProgress, this, &JobManager::processFallbackProgress);
    connect(this, &JobManager::reinforceFinished, this, &JobManager::processFallbackFinished);
    return true;
}

uint JobManager::getFallbackStatus()
{
    if (isFallback)
    {
        return getReinforceStatus();
    }
    return BRJobState::BR_JOB_STATE_IDLE;
}

bool JobManager::cancel(const qlonglong& jobID)
{
    if (this->m_scanJob &&
        jobID == this->m_scanJob->getId() &&
        this->m_scanJob->getState() == BRJobState::BR_JOB_STATE_RUNNING)
    {
        return m_scanJob->cancel();
    }

    if (this->m_reinforceJob &&
        jobID == this->m_reinforceJob->getId() &&
        this->m_reinforceJob->getState() == BRJobState::BR_JOB_STATE_RUNNING)
    {
        return this->m_reinforceJob->cancel();
    }

    return false;
}

void JobManager::initScanResult(const QStringList& names)
{
    m_scanJobResult = Protocol::JobResult(0, 0, 0);
    m_scanJobResult.reinforcement().clear();

    for (auto& name : names)
    {
        m_scanJobResult.reinforcement().push_back(Protocol::ReinforcementResult(name.toStdString(), 0));
    }
}

void JobManager::initReinforceResult(const QStringList& names)
{
    m_reinforceJobResult = Protocol::JobResult(0, 0, 0);
    m_reinforceJobResult.reinforcement().clear();

    for (auto& name : names)
    {
        m_reinforceJobResult.reinforcement().push_back(Protocol::ReinforcementResult(name.toStdString(), 0));
    }
}

void JobManager::cacheScanResult(const Protocol::ReinforcementResult& reinforcementResult)
{
    for (auto& reinforcement : m_scanJobResult.reinforcement())
    {
        CONTINUE_IF_TRUE(reinforcement.name() != reinforcementResult.name());
        reinforcement = reinforcementResult;
    }
}

void JobManager::cacheReinforceResult(const Protocol::ReinforcementResult& reinforcementResult)
{
    for (auto& reinforcement : m_reinforceJobResult.reinforcement())
    {
        CONTINUE_IF_TRUE(reinforcement.name() != reinforcementResult.name());
        reinforcement = reinforcementResult;
    }
}

void JobManager::processScanResultBeforeReinforce(const QStringList& reinforcementNames)
{
    disconnect(m_scanBeforeReinforceConnection);

    // 初始状态需要保存全部加固项状态
    if (!QFile::exists(RH_BR_OPERATE_DATA_FIRST))
    {
        saveHistoryBeforeReinforce(m_plugins->getReinforcementNames(), RH_BR_OPERATE_DATA_FIRST);
    }
    // 保存加固前状态用于回退到上一次
    saveHistoryBeforeReinforce(reinforcementNames, RH_BR_OPERATE_DATA_LAST);

    if (!startReinforce(reinforcementNames))
    {
        // 该函数是回调函数，所以必须发送信号通知客户端任务结束。
        processReinforceFinished();
    }
}

void JobManager::saveHistoryBeforeReinforce(const QStringList& reinforcementNames, const QString& savePath)
{
    auto rh = QSharedPointer<Protocol::ReinforcementHistory>::create();

    for (const auto& reinforcementName : reinforcementNames)
    {
        for (const auto& reinforcementResult : m_scanJobResult.reinforcement())
        {
            CONTINUE_IF_TRUE(QString(reinforcementResult.name().c_str()) != reinforcementName);
            CONTINUE_IF_FALSE(reinforcementResult.args().present())

            Protocol::Reinforcement reinforcement(reinforcementResult.name());

            auto resultValues = StrUtils::str2jsonObject(reinforcementResult.args().get());
            auto args = resultValues[JOB_RETURN_VALUE].toObject();
            for (const auto& key : args.keys())
            {
                auto value = args.value(key);
                // TODO: 需要验证值为空的情况
                reinforcement.arg().push_back(Protocol::ReinforcementArg(key.toStdString(), value.toString().toStdString()));
            }

            rh->reinforcement().push_back(reinforcement);
        }
    }
    this->m_configuration->writeRhToFile(rh, savePath);
}

bool JobManager::startReinforce(const QStringList& names)
{
    this->m_reinforceJob = Job::create();
    initReinforceResult(names);

    for (auto iter = names.begin(); iter != names.end(); ++iter)
    {
        auto& name = (*iter);
        auto reinforcement = this->m_plugins->getReinforcement(name);
        if (!reinforcement)
        {
            KLOG_WARNING() << "Reinforcement" << name << "not found.";
            return false;
        }

        auto reinforcementInterface = this->m_plugins->getReinforcementInterface(reinforcement->getPluginName(),
                                                                                 reinforcement->getName());
        if (!reinforcementInterface)
        {
            KLOG_WARNING() << "Reinforcement interface not found for" << name << ".";
            return false;
        }

        auto paramStr = reinforcementArgXml2Str(reinforcement->getRs().arg());
        this->m_reinforceJob->addOperation(reinforcement->getPluginName(),
                                           reinforcement->getName(),
                                           [reinforcementInterface, paramStr]() -> QString
                                           {
                                               QString error;
                                               QJsonObject retval;
                                               if (!reinforcementInterface->set(paramStr, error))
                                               {
                                                   retval[JOB_ERROR_STR] = error;
                                               }
                                               else
                                               {
                                                   // 设置为空字符串，这里主要是为了区分加固成功和取消加固两种状态，后续可能会调整改逻辑
                                                   retval[JOB_RETURN_VALUE] = QString();
                                               }
                                               return StrUtils::json2str(retval);
                                           });
    }
    if (!this->m_reinforceJob->runAsync())
    {
        return false;
    }

    connect(this->m_reinforceJob.data(), &Job::processChanged, this, &JobManager::processReinforceProgress);
    connect(this->m_reinforceJob.data(), &Job::processFinished, this, &JobManager::processReinforceFinished);
    return true;
}

void JobManager::processScanProgress(const JobResult& jobResult)
{
    // 这里记录上一次信号到这一次信号的结果
    Protocol::JobResult scanResult(0, 0, 0);
    scanResult.process(jobResult.finished_operation_num * 100.0 / jobResult.sum_operation_num);
    scanResult.job_id(jobResult.job_id);
    scanResult.job_state(this->m_scanJob->getState());

    m_scanJobResult.process(jobResult.finished_operation_num * 100.0 / jobResult.sum_operation_num);
    m_scanJobResult.job_id(jobResult.job_id);
    m_scanJobResult.job_state(this->m_scanJob->getState());

    for (auto iter = jobResult.running_operations.begin(); iter != jobResult.running_operations.end(); ++iter)
    {
        auto operation = this->m_scanJob->getOperation((*iter));

        Protocol::ReinforcementResult reinforcementResult(std::string(), 0);
        reinforcementResult.name(operation->reinforcement_name.toStdString());
        reinforcementResult.state(BRReinforcementState::BR_REINFORCEMENT_STATE_SCANNING);
        reinforcementResult.args("");
        scanResult.reinforcement().push_back(reinforcementResult);
        cacheScanResult(reinforcementResult);
    }

    for (auto iter = jobResult.current_finished_operations.begin(); iter != jobResult.current_finished_operations.end(); ++iter)
    {
        auto& operationResult = (*iter);
        auto operation = this->m_scanJob->getOperation(operationResult.operation_id);
        Protocol::ReinforcementResult reinforcementResult(std::string(), 0);

        reinforcementResult.name(operation->reinforcement_name.toStdString());

        BRReinforcementState state = BRReinforcementState::BR_REINFORCEMENT_STATE_UNKNOWN;
        const auto resultValues = StrUtils::str2jsonObject(operationResult.result);
        // 如果结果为空应该时任务被取消了，如果在收到客户端的任务取消命令时操作已经在执行，结果也可能不为空，所以这里不能通过任务是否被取消的状态来判断
        if (resultValues.isEmpty())
        {
            state = BRReinforcementState::BR_REINFORCEMENT_STATE_UNSCAN;
            reinforcementResult.args("");
        }
        else if (resultValues[JOB_ERROR_STR].isString())
        {
            state = BRReinforcementState::BR_REINFORCEMENT_STATE_SCAN_ERROR;
            reinforcementResult.args("");
            reinforcementResult.error(resultValues[JOB_ERROR_STR].toString().toStdString());
        }
        else
        {
            state = BRReinforcementState::BR_REINFORCEMENT_STATE_SCAN_DONE;
            reinforcementResult.args(StrUtils::json2str(resultValues).toStdString());
        }
        auto reinforcement = this->m_plugins->getReinforcement(operation->reinforcement_name);
        if ((state & BRReinforcementState::BR_REINFORCEMENT_STATE_SCAN_DONE) != 0 &&
            reinforcement &&
            reinforcement->matchRules(resultValues[JOB_RETURN_VALUE].toObject()))
        {
            state = BRReinforcementState(state | BRReinforcementState::BR_REINFORCEMENT_STATE_SAFE);
        }
        else
        {
            // 保留未扫描状态，否则扫描结果仅为符合和不符合了
            if (state != BRReinforcementState::BR_REINFORCEMENT_STATE_UNSCAN)
            {
                state = BRReinforcementState(state | BRReinforcementState::BR_REINFORCEMENT_STATE_UNSAFE);
            }
        }
        reinforcementResult.state(int32_t(state));

        scanResult.reinforcement().push_back(reinforcementResult);
        cacheScanResult(reinforcementResult);
    }

    std::ostringstream ostringStream;
    Protocol::br_job_result(ostringStream, scanResult);
    emit scanProgress(QString(ostringStream.str().c_str()));
}

void JobManager::processReinforceProgress(const JobResult& jobResult)
{
    Protocol::JobResult reinforceResult(0, 0, 0);

    reinforceResult.process(jobResult.finished_operation_num * 100.0 / jobResult.sum_operation_num);
    reinforceResult.job_id(jobResult.job_id);
    reinforceResult.job_state(this->m_reinforceJob->getState());

    m_reinforceJobResult.process(jobResult.finished_operation_num * 100.0 / jobResult.sum_operation_num);
    m_reinforceJobResult.job_id(jobResult.job_id);
    m_reinforceJobResult.job_state(this->m_reinforceJob->getState());

    for (auto iter = jobResult.running_operations.begin(); iter != jobResult.running_operations.end(); ++iter)
    {
        auto operation = this->m_reinforceJob->getOperation(*iter);
        Protocol::ReinforcementResult reinforcementResult(std::string(), 0);

        reinforcementResult.name(operation->reinforcement_name.toStdString());
        reinforcementResult.state(BRReinforcementState::BR_REINFORCEMENT_STATE_REINFORCING);
        reinforceResult.reinforcement().push_back(reinforcementResult);
        cacheReinforceResult(reinforcementResult);
    }

    for (auto iter = jobResult.current_finished_operations.begin(); iter != jobResult.current_finished_operations.end(); ++iter)
    {
        auto& operationResult = (*iter);
        auto operation = this->m_reinforceJob->getOperation(operationResult.operation_id);
        Protocol::ReinforcementResult reinforcementResult(std::string(), 0);

        reinforcementResult.name(operation->reinforcement_name.toStdString());

        BRReinforcementState state = BRReinforcementState::BR_REINFORCEMENT_STATE_UNKNOWN;
        auto resultValues = StrUtils::str2jsonObject(operationResult.result);
        if (resultValues.isEmpty())
        {
            state = BRReinforcementState::BR_REINFORCEMENT_STATE_UNREINFORCE;
        }
        else if (resultValues[JOB_ERROR_STR].isString())
        {
            state = BRReinforcementState::BR_REINFORCEMENT_STATE_REINFORCE_ERROR;
            reinforcementResult.error(resultValues[JOB_ERROR_STR].toString().toStdString());
        }
        else
        {
            state = BRReinforcementState::BR_REINFORCEMENT_STATE_REINFORCE_DONE;
        }
        reinforcementResult.state(int32_t(state));
        reinforceResult.reinforcement().push_back(reinforcementResult);
        cacheReinforceResult(reinforcementResult);
    }
    std::ostringstream ostringStream;
    Protocol::br_job_result(ostringStream, reinforceResult);
    emit reinforceProgress(QString(ostringStream.str().c_str()));
}

void JobManager::processFallbackProgress(const QString& progress)
{
    Q_EMIT fallbackProgress(progress);
}

void JobManager::processScanFinished()
{
    disconnect(this->m_scanJob.data(), &Job::processFinished, this, &JobManager::processScanFinished);
    disconnect(this->m_scanJob.data(), &Job::processChanged, this, &JobManager::processScanProgress);
    emit scanFinished();
}

void JobManager::processReinforceFinished()
{
    disconnect(this->m_reinforceJob.data(), &Job::processChanged, this, &JobManager::processReinforceProgress);
    disconnect(this->m_reinforceJob.data(), &Job::processFinished, this, &JobManager::processReinforceFinished);
    emit reinforceFinished();
    isReinforce = false;
}

void JobManager::processFallbackFinished()
{
    disconnect(this, &JobManager::reinforceProgress, this, &JobManager::processFallbackProgress);
    disconnect(this, &JobManager::reinforceFinished, this, &JobManager::processFallbackFinished);
    emit fallbackFinished();
    isFallback = false;
}

QString JobManager::reinforcementArgXml2Str(const KS::Protocol::Reinforcement::ArgSequence& args)
{
    QJsonObject jsonArgs;
    for (auto iter = args.begin(); iter != args.end(); ++iter)
    {
        QString inputExample = iter->input_example() != nullptr ? iter->input_example().get().c_str() : "";

        // FIXME:这是什么特殊逻辑？无法理解
        // str2jsonValue中的类型转换没法区分line输入纯数字和数字输入框spin输入的纯数字，都会被转为double类型，这里需要进行判断,
        // 如果存在inputExample则肯定为line输入的纯数字，参数应该为str类型
        jsonArgs.insert(iter->name().c_str(), inputExample.isEmpty() ? StrUtils::str2jsonValue(iter->value())
                                                                     : QJsonValue::fromVariant(iter->value().c_str()));
    }
    return StrUtils::json2str(jsonArgs);
}

}  // namespace BR
}  // namespace KS