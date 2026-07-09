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

#include "job-dispatcher.h"
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
#define RH_BR_OPERATE_DATA_FIRST SSR_BR_INSTALL_DATADIR "/br-rh-first.xml"
#define RH_BR_OPERATE_DATA_LAST SSR_BR_INSTALL_DATADIR "/br-rh-last.xml"

JobDispatcher::JobDispatcher(Configuration* configuration, Plugins* plugins, QObject* parent)
    : QObject(parent),
      m_configuration(configuration),
      m_plugins(plugins),
      m_jobResult(0, 0, 0),
      m_state(BRDispatchState::BR_DISPATCH_STATE_IDLE)
{
    m_job = Job::create();
}

void JobDispatcher::init()
{
}

bool JobDispatcher::scanAll()
{
    return scan(m_plugins->getReinforcementNames());
}

bool JobDispatcher::scan(const QStringList& names)
{
    if (m_state != BRDispatchState::BR_DISPATCH_STATE_IDLE)
    {
        // 打印日志，说明已经有任务正在运行
        KLOG_WARNING() << "A" << stateEnum2Str(m_state) << "job is running";
        return false;
    }

    m_job->reset();
    initJobResult(names);

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

        this->m_job->addOperation(reinforcement->getPluginName(),
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

    if (!this->m_job->runAsync())
    {
        return false;
    }

    m_state = BRDispatchState::BR_DISPATCH_STATE_SCAN;
    connect(this->m_job.data(), &Job::processChanged, this, &JobDispatcher::processScanProgress);
    connect(this->m_job.data(), &Job::processFinished, this, &JobDispatcher::processScanFinished);

    return true;
}

Protocol::JobResult JobDispatcher::getScanResult()
{
    if (m_state != BRDispatchState::BR_DISPATCH_STATE_SCAN)
    {
        return Protocol::JobResult(0, 0, 0);
    }
    return m_jobResult;
}

bool JobDispatcher::reinforceAll()
{
    return reinforce(m_plugins->getReinforcementNames());
}

bool JobDispatcher::reinforce(const QStringList& names)
{
    if (m_state != BRDispatchState::BR_DISPATCH_STATE_IDLE)
    {
        // 打印日志，说明已经有任务正在运行
        KLOG_WARNING() << "A" << stateEnum2Str(m_state) << "job is running";
        return false;
    }

    // 加固前先进行备份
    if (!backup(m_plugins->getReinforcementNames()))
    {
        return false;
    }
    m_state = BRDispatchState::BR_DISPATCH_STATE_REINFORCE;
    connect(this->m_job.data(), &Job::processChanged, this, &JobDispatcher::processBackupProgress);
    connect(this->m_job.data(), &Job::processFinished, std::bind(&JobDispatcher::processBackupFinished, this, names));
    return true;
}

Protocol::JobResult JobDispatcher::getReinforceResult()
{
    if (m_state != BRDispatchState::BR_DISPATCH_STATE_REINFORCE)
    {
        return Protocol::JobResult(0, 0, 0);
    }
    return m_jobResult;
}

bool JobDispatcher::rollback(int method)
{
    if (m_state != BRDispatchState::BR_DISPATCH_STATE_IDLE)
    {
        // 打印日志，说明已经有任务正在运行
        KLOG_WARNING() << "A" << stateEnum2Str(m_state) << "job is running";
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

    m_job->reset();
    QStringList reinforcementNames;
    auto rh = m_configuration->readRhFromFile(rhFile);
    for (const auto& rhReinforcement : rh->reinforcement())
    {
        auto name = QString(rhReinforcement.name().c_str());
        if (!m_configuration->setCustomRA(rhReinforcement))
        {
            KLOG_ERROR() << "Fallback reinforcement to custom reinforcement argument failed for" << name;
        }

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

        auto paramStr = reinforcementArgXml2Str(rhReinforcement.arg());
        m_job->addOperation(reinforcement->getPluginName(),
                            reinforcement->getName(),
                            [reinforcementInterface, paramStr]() -> QString
                            {
                                QString error;
                                QJsonObject retval;
                                if (!reinforcementInterface->rollback(paramStr, error))
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

        reinforcementNames.push_back(name);
    }

    initJobResult(reinforcementNames);

    if (!this->m_job->runAsync())
    {
        return false;
    }
    m_state = BRDispatchState::BR_DISPATCH_STATE_ROLLBACK;

    connect(this->m_job.data(), &Job::processChanged, this, &JobDispatcher::processRollbackProgress);
    connect(this->m_job.data(), &Job::processFinished, this, &JobDispatcher::processRollbackFinished);
    return true;
}

BRDispatchState JobDispatcher::getState()
{
    return m_state;
}

bool JobDispatcher::cancel(const qlonglong&)
{
    // 该版本忽略函数参数，不需要任务ID
    return m_job->cancel();
}

bool JobDispatcher::backup(const QStringList& names)
{
    m_job->reset();
    initJobResult(names);

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

        this->m_job->addOperation(reinforcement->getPluginName(),
                                  reinforcement->getName(),
                                  [reinforcementInterface]() -> QString
                                  {
                                      QJsonObject retval;
                                      QString args;
                                      QString error;
                                      if (reinforcementInterface->backup(args, error))
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

    if (!this->m_job->runAsync())
    {
        return false;
    }
    return true;
}

void JobDispatcher::initJobResult(const QStringList& names)
{
    m_jobResult = Protocol::JobResult(0, 0, 0);
    m_jobResult.reinforcement().clear();

    for (auto& name : names)
    {
        m_jobResult.reinforcement().push_back(Protocol::ReinforcementResult(name.toStdString(), 0));
    }
}

void JobDispatcher::cacheResult(const Protocol::ReinforcementResult& reinforcementResult)
{
    for (auto& reinforcement : m_jobResult.reinforcement())
    {
        CONTINUE_IF_TRUE(reinforcement.name() != reinforcementResult.name());
        reinforcement = reinforcementResult;
    }
}

void JobDispatcher::saveHistoryBeforeReinforce(const QStringList& reinforcementNames, const QString& savePath)
{
    auto rh = QSharedPointer<Protocol::ReinforcementHistory>::create();

    for (const auto& reinforcementName : reinforcementNames)
    {
        for (const auto& reinforcementResult : m_jobResult.reinforcement())
        {
            CONTINUE_IF_TRUE(QString(reinforcementResult.name().c_str()) != reinforcementName);
            CONTINUE_IF_FALSE(reinforcementResult.args().present())

            Protocol::Reinforcement reinforcement(reinforcementResult.name());

            auto resultValues = StrUtils::str2jsonObject(reinforcementResult.args().get());
            auto args = resultValues[JOB_RETURN_VALUE].toObject();
            for (const auto& key : args.keys())
            {
                auto value = args.value(key).toVariant().toString();
                // TODO: 需要验证值为空的情况
                reinforcement.arg().push_back(Protocol::ReinforcementArg(key.toStdString(), value.toStdString()));
            }

            rh->reinforcement().push_back(reinforcement);
        }
    }
    this->m_configuration->writeRhToFile(rh, savePath);
}

bool JobDispatcher::startReinforce(const QStringList& names)
{
    // 这里复用备份任务的任务ID
    m_job->clear();
    initJobResult(names);

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
        this->m_job->addOperation(reinforcement->getPluginName(),
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
    if (!this->m_job->runAsync())
    {
        return false;
    }

    connect(this->m_job.data(), &Job::processChanged, this, &JobDispatcher::processReinforceProgress);
    connect(this->m_job.data(), &Job::processFinished, this, &JobDispatcher::processReinforceFinished);
    return true;
}

void JobDispatcher::processScanProgress(const JobResult& jobResult)
{
    // 这里记录上一次信号到这一次信号的结果
    Protocol::JobResult scanResult(0, 0, 0);
    scanResult.process(jobResult.finished_operation_num * 100.0 / jobResult.sum_operation_num);
    scanResult.job_id(jobResult.job_id);
    scanResult.job_state(this->m_job->getState());

    m_jobResult.process(jobResult.finished_operation_num * 100.0 / jobResult.sum_operation_num);
    m_jobResult.job_id(jobResult.job_id);
    m_jobResult.job_state(this->m_job->getState());

    for (auto iter = jobResult.running_operations.begin(); iter != jobResult.running_operations.end(); ++iter)
    {
        auto operation = this->m_job->getOperation((*iter));

        Protocol::ReinforcementResult reinforcementResult(std::string(), 0);
        reinforcementResult.name(operation->reinforcement_name.toStdString());
        reinforcementResult.state(BRReinforcementState::BR_REINFORCEMENT_STATE_SCANNING);
        reinforcementResult.args("");
        scanResult.reinforcement().push_back(reinforcementResult);
        cacheResult(reinforcementResult);
    }

    for (auto iter = jobResult.current_finished_operations.begin(); iter != jobResult.current_finished_operations.end(); ++iter)
    {
        auto& operationResult = (*iter);
        auto operation = this->m_job->getOperation(operationResult.operation_id);
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
        cacheResult(reinforcementResult);
    }

    std::ostringstream ostringStream;
    Protocol::br_job_result(ostringStream, scanResult);
    emit scanProgress(QString(ostringStream.str().c_str()));
}

void JobDispatcher::processReinforceProgress(const JobResult& jobResult)
{
    Protocol::JobResult reinforceResult(0, 0, 0);

    reinforceResult.process(jobResult.finished_operation_num * 100.0 / jobResult.sum_operation_num);
    reinforceResult.job_id(jobResult.job_id);
    reinforceResult.job_state(this->m_job->getState());

    m_jobResult.process(jobResult.finished_operation_num * 100.0 / jobResult.sum_operation_num);
    m_jobResult.job_id(jobResult.job_id);
    m_jobResult.job_state(this->m_job->getState());

    for (auto iter = jobResult.running_operations.begin(); iter != jobResult.running_operations.end(); ++iter)
    {
        auto operation = this->m_job->getOperation(*iter);
        Protocol::ReinforcementResult reinforcementResult(std::string(), 0);

        reinforcementResult.name(operation->reinforcement_name.toStdString());
        reinforcementResult.state(BRReinforcementState::BR_REINFORCEMENT_STATE_REINFORCING);
        reinforceResult.reinforcement().push_back(reinforcementResult);
        cacheResult(reinforcementResult);
    }

    for (auto iter = jobResult.current_finished_operations.begin(); iter != jobResult.current_finished_operations.end(); ++iter)
    {
        auto& operationResult = (*iter);
        auto operation = this->m_job->getOperation(operationResult.operation_id);
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
        cacheResult(reinforcementResult);
    }
    std::ostringstream ostringStream;
    Protocol::br_job_result(ostringStream, reinforceResult);
    emit reinforceProgress(QString(ostringStream.str().c_str()));
}

void JobDispatcher::processBackupProgress(const JobResult& jobResult)
{
    m_jobResult.process(jobResult.finished_operation_num * 100.0 / jobResult.sum_operation_num);
    m_jobResult.job_id(jobResult.job_id);
    m_jobResult.job_state(this->m_job->getState());

    for (auto iter = jobResult.current_finished_operations.begin(); iter != jobResult.current_finished_operations.end(); ++iter)
    {
        auto& operationResult = (*iter);
        auto operation = this->m_job->getOperation(operationResult.operation_id);
        Protocol::ReinforcementResult reinforcementResult(std::string(), 0);

        reinforcementResult.name(operation->reinforcement_name.toStdString());

        BRReinforcementState state = BRReinforcementState::BR_REINFORCEMENT_STATE_UNKNOWN;
        const auto resultValues = StrUtils::str2jsonObject(operationResult.result);
        // 如果结果为空应该时任务被取消了，如果在收到客户端的任务取消命令时操作已经在执行，结果也可能不为空，所以这里不能通过任务是否被取消的状态来判断
        if (resultValues.isEmpty())
        {
            state = BRReinforcementState::BR_REINFORCEMENT_STATE_UNBACKUP;
            reinforcementResult.args("");
        }
        else if (resultValues[JOB_ERROR_STR].isString())
        {
            state = BRReinforcementState::BR_REINFORCEMENT_STATE_BACKUP_ERROR;
            reinforcementResult.args("");
            reinforcementResult.error(resultValues[JOB_ERROR_STR].toString().toStdString());
        }
        else
        {
            state = BRReinforcementState::BR_REINFORCEMENT_STATE_BACKUP_DONE;
            reinforcementResult.args(StrUtils::json2str(resultValues).toStdString());
        }
        reinforcementResult.state(int32_t(state));

        cacheResult(reinforcementResult);
    }
}

void JobDispatcher::processRollbackProgress(const JobResult& jobResult)
{
    Protocol::JobResult rollbackResult(0, 0, 0);

    rollbackResult.process(jobResult.finished_operation_num * 100.0 / jobResult.sum_operation_num);
    rollbackResult.job_id(jobResult.job_id);
    rollbackResult.job_state(this->m_job->getState());

    std::ostringstream ostringStream;
    Protocol::br_job_result(ostringStream, rollbackResult);
    emit rollbackProgress(QString(ostringStream.str().c_str()));
}

void JobDispatcher::processScanFinished()
{
    disconnect(this->m_job.data(), &Job::processFinished, this, &JobDispatcher::processScanFinished);
    disconnect(this->m_job.data(), &Job::processChanged, this, &JobDispatcher::processScanProgress);
    emit scanFinished();
    m_state = BRDispatchState::BR_DISPATCH_STATE_IDLE;
}

void JobDispatcher::processReinforceFinished()
{
    disconnect(this->m_job.data(), &Job::processChanged, this, &JobDispatcher::processReinforceProgress);
    disconnect(this->m_job.data(), &Job::processFinished, this, &JobDispatcher::processReinforceFinished);
    emit reinforceFinished();
    m_state = BRDispatchState::BR_DISPATCH_STATE_IDLE;
}

void JobDispatcher::processBackupFinished(const QStringList& reinforcementNames)
{
    disconnect(this->m_job.data(), &Job::processChanged, this, &JobDispatcher::processBackupProgress);
    disconnect(this->m_job.data(), &Job::processFinished, 0, 0);

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

void JobDispatcher::processRollbackFinished()
{
    disconnect(this->m_job.data(), &Job::processChanged, this, &JobDispatcher::processRollbackProgress);
    disconnect(this->m_job.data(), &Job::processFinished, this, &JobDispatcher::processRollbackFinished);
    emit rollbackFinished();
    m_state = BRDispatchState::BR_DISPATCH_STATE_IDLE;
}

QString JobDispatcher::reinforcementArgXml2Str(const KS::Protocol::Reinforcement::ArgSequence& args)
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

QString JobDispatcher::stateEnum2Str(BRDispatchState state)
{
    switch (state)
    {
    case BRDispatchState::BR_DISPATCH_STATE_IDLE:
        return "idle";
    case BRDispatchState::BR_DISPATCH_STATE_SCAN:
        return "scan";
    case BRDispatchState::BR_DISPATCH_STATE_REINFORCE:
        return "reinforce";
    case BRDispatchState::BR_DISPATCH_STATE_BACKUP:
        return "backup";
    case BRDispatchState::BR_DISPATCH_STATE_ROLLBACK:
        return "rollback";
    default:
        return "unknown";
    }
    return "unknown";
}

}  // namespace BR
}  // namespace KS