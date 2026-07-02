#include "kss.h"
#include <qt5-log-i.h>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

namespace KS
{
#define DIGEST_SCAN_ADD_FILE_CMD "kss digest scan -u"
#define DIGEST_SCAN_REMOVE_FILE_CMD "kss digest scan -r"
#define DIGEST_SCAN_GET_EXECUTE_CMD "kss digest info -e"
#define DIGEST_SCAN_GET_KERNEL_CMD "kss digest info -m"

#define ADD_FILE_CMD "kss file add"
#define REMOVE_FILE_CMD "kss file del"
#define GET_FILES_CMD "kss file info"

#define KSS_JSON_KEY_RES "res"
#define KSS_JSON_KEY_DATA "data"
#define KSS_JSON_KEY_COUNT "count"
#define KSS_JSON_KEY_DATA_PATH "path"

Kss::Kss(QObject *parent) : QObject(parent)
{
    m_process = new QProcess(parent);
}

void Kss::addTrustedFile(const QString &filePath)
{
    auto cmd = QString("%1 %2").arg(DIGEST_SCAN_ADD_FILE_CMD, filePath);
    this->execute(cmd);
}

void Kss::removeTrustedFile(const QString &filePath)
{
    auto cmd = QString("%1 %2").arg(DIGEST_SCAN_REMOVE_FILE_CMD, filePath);
    this->execute(cmd);
}

QString Kss::getModuleFiles()
{
    this->execute(DIGEST_SCAN_GET_KERNEL_CMD);
    return m_processOutput;
}

void Kss::prohibitUnloading(bool prohibited, const QString &filePath)
{

}

QString Kss::getExecuteFiles()
{
    this->execute(DIGEST_SCAN_GET_EXECUTE_CMD);
    return m_processOutput;
}

void Kss::addFile(const QString &fileName, const QString &filePath, const QString &insertTime)
{
    auto cmd = QString("%1 -n %2 -p %3 -t '%4'").arg(ADD_FILE_CMD, fileName, filePath, insertTime);
    this->execute(cmd);
}

void Kss::removeFile(const QString &filePath)
{
    auto cmd = QString("%1 -p %2").arg(REMOVE_FILE_CMD, filePath);
    this->execute(cmd);
}

QString Kss::getFiles()
{
    this->execute(GET_FILES_CMD);
    return m_processOutput;
}

QString Kss::search(const QString &pathKey, const QString &fileList)
{
    QJsonDocument resultJsonDoc;
    QJsonArray jsonArr;

    QJsonParseError jsonError;
    auto jsonDoc = QJsonDocument::fromJson(fileList.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser kernel protected information failed: " << jsonError.errorString();
        return QString();
    }

    auto jsonModules = jsonDoc.object().value(KSS_JSON_KEY_DATA).toArray();

    for (auto module : jsonModules)
    {
        auto jsonMod = module.toObject();

        // 通过输入的pathKey，判断list中path字段是否包含pathKey
        if (jsonMod.value(KSS_JSON_KEY_DATA_PATH).toString().contains(pathKey))
        {
            jsonArr.push_back(jsonMod);
        }
    }
    resultJsonDoc.setArray(jsonArr);
    return QString(resultJsonDoc.toJson());
}

void Kss::onProcessExit(int exitCode, QProcess::ExitStatus exitStatus)
{
    KLOG_DEBUG() << "Command execution completed. exitcode = " << exitCode << "exitStatus = " << exitStatus;
    this->m_process->disconnect();

    auto standardOutput = this->m_process->readAllStandardOutput();

    KLOG_DEBUG() << "Execute the command to successfully output: " << standardOutput;
    m_processOutput = standardOutput;

    auto errordOutput = this->m_process->readAllStandardError();
    if (!errordOutput.isEmpty())
    {
        KLOG_ERROR() << "Execution command error output: " << errordOutput;
    }
    m_errorOutput = errordOutput;
}

void Kss::execute(const QString &cmd)
{
    KLOG_DEBUG() << "Start executing the command. cmd = " << cmd;
    m_process->start("bash", QStringList() << "-c" << cmd);
    connect(this->m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onProcessExit(int, QProcess::ExitStatus)));
    m_process->waitForFinished();
}
}
