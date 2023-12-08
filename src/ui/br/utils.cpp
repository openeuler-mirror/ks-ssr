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
#include "utils.h"
#include <qt5-log-i.h>
#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QList>
#include <QObject>
#include "include/ssr-marcos.h"
#include "lib/base/str-utils.h"
#include "src/ui/br/reinforcement-items/plugins-translation.h"

namespace KS
{
namespace BR
{
Utils::Utils()
{
    PluginsTranslation::globalInit();
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("ini", "configuration class"));
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("ini", "network class"));
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("ini", "audit class"));
    Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("ini", "external class"));
}

Utils::~Utils()
{
    PluginsTranslation::globalDeinit();
}

QSharedPointer<Utils> Utils::m_instance = nullptr;
QSharedPointer<Utils> Utils::getDefault()
{
    if (!m_instance)
    {
        m_instance = QSharedPointer<Utils>::create();
    }
    return m_instance;
}

QString Utils::state2Str(int state)
{
    QString retStr;
    switch (state)
    {
    case BR_REINFORCEMENT_STATE_UNKNOWN:
        retStr = QString(QObject::tr("Unknown"));
        break;
    case BR_REINFORCEMENT_STATE_SAFE:
        retStr = QString(QObject::tr("Conformity"));
        break;
    case BR_REINFORCEMENT_STATE_UNSAFE:
        retStr = QString(QObject::tr("Inconformity"));
        break;
    case BR_REINFORCEMENT_STATE_UNSCAN:
        retStr = QString(QObject::tr("Not Scanned"));
        break;
    case BR_REINFORCEMENT_STATE_SCANNING:
        retStr = QString(QObject::tr("Scannig"));
        break;
    case BR_REINFORCEMENT_STATE_SCAN_ERROR:
        retStr = QString(QObject::tr("Scan Failed"));
        break;
    case BR_REINFORCEMENT_STATE_SCAN_DONE:
        retStr = QString(QObject::tr("Scan Complete"));
        break;
    case BR_REINFORCEMENT_STATE_UNREINFORCE:
        retStr = QString(QObject::tr("Unreinforcement"));
        break;
    case BR_REINFORCEMENT_STATE_REINFORCING:
        retStr = QString(QObject::tr("Reinforcing"));
        break;
    case BR_REINFORCEMENT_STATE_REINFORCE_ERROR:
        retStr = QString(QObject::tr("Reinforcement Failure"));
        break;
    case BR_REINFORCEMENT_STATE_REINFORCE_DONE:
        retStr = QString(QObject::tr("Reinforced"));
        break;
    default:
        if ((state & BR_REINFORCEMENT_STATE_SAFE) == 1)
            retStr = QString(QObject::tr("Conformity"));
        else if ((state & BR_REINFORCEMENT_STATE_UNSAFE) == 2)
            retStr = QString(QObject::tr("Inconformity"));
        else if ((state & BR_REINFORCEMENT_STATE_UNSCAN) == 4)
            retStr = QString(QObject::tr("Unscan"));
        else
            retStr = QString(QObject::tr("Unknown"));
        break;
    }
    return retStr;
}

QColor Utils::state2Color(int state)
{
    QColor retColor;
    switch (state)
    {
    case BR_REINFORCEMENT_STATE_UNKNOWN:
        retColor = QColor("#919191");
        break;
    case BR_REINFORCEMENT_STATE_SAFE:
        retColor = QColor("#00a2ff");
        break;
    case BR_REINFORCEMENT_STATE_UNSAFE:
        retColor = QColor("#FA4949");
        break;
    case BR_REINFORCEMENT_STATE_UNSCAN:
        retColor = QColor("#919191");
        break;
    case BR_REINFORCEMENT_STATE_SCANNING:
        retColor = QColor("#ffffff");
        break;
    case BR_REINFORCEMENT_STATE_SCAN_ERROR:
        retColor = QColor("#FA4949");
        break;
    case BR_REINFORCEMENT_STATE_SCAN_DONE:
        retColor = QColor("#00a2ff");
        break;
    case BR_REINFORCEMENT_STATE_UNREINFORCE:
        retColor = QColor("#919191");
        break;
    case BR_REINFORCEMENT_STATE_REINFORCING:
        retColor = QColor("#ffffff");
        break;
    case BR_REINFORCEMENT_STATE_REINFORCE_ERROR:
        retColor = QColor("#FA4949");
        break;
    case BR_REINFORCEMENT_STATE_REINFORCE_DONE:
        retColor = QColor("#00a2ff");
        break;
    default:
        if ((state & BR_REINFORCEMENT_STATE_SAFE) == 1)
            retColor = QColor("#00a2ff");
        else if ((state & BR_REINFORCEMENT_STATE_UNSAFE) == 2)
            retColor = QColor("#FA4949");
        else
            retColor = QColor("#919191");
        break;
    }
    return retColor;
}

void Utils::jsonParsing(const QByteArray &categoriesJson, QList<Category *> &categoriesList)
{
    QJsonParseError jsonError;
    auto jsonDoc = QJsonDocument::fromJson(categoriesJson, &jsonError);
    auto arrayObj = jsonDoc.object().value("items").toArray();
    for (int i = 0; i < arrayObj.size(); ++i)
    {
        auto iconName = arrayObj.at(i).toObject().value("icon_name");
        auto label = arrayObj.at(i).toObject().value("label");
        auto name = arrayObj.at(i).toObject().value("name");
        auto category = new Category;
        category->setIconName(iconName.toString());
        category->setLabel(categoriesLabel2Translate(label.toString()));
        category->setName(name.toString());
        categoriesList.append(category);
    }
}

bool Utils::ssrReinforcements(const QString &xmlString, QList<Category *> &categoriesList)
{
    RETURN_VAL_IF_TRUE(xmlString == "", false)

    QLocale local;
    std::istringstream istringStream(xmlString.toStdString());
    auto rsReinforcements = KS::Protocol::br_reinforcements(istringStream, xml_schema::Flags::dont_validate);
    auto rsReinforcement = rsReinforcements.get()->reinforcement();

    for (auto iter : rsReinforcement)
    {
        QString str = iter.name().c_str();
        iter.checkbox().set(false);
        auto reinforcementItem = new ReinforcementItem;
        reinforcementItem->setName(iter.name().c_str());

        for (auto arg : iter.arg())
        {
            auto value = StrUtils::str2jsonValue(arg.value());
            QString defaultLabel = "", defaultNote = "";
            for (auto label : arg.layout().get().label())
            {
                if (label.lang() == nullptr)
                {
                    defaultLabel = QString(label.c_str());
                    continue;
                }
                if (local.name().toStdString() == label.lang().get())
                {
                    defaultLabel = QString(label.c_str());
                }
            }

            for (auto note : arg.note())
            {
                if (note.lang() == nullptr)
                {
                    defaultNote = QString(note.c_str());
                    continue;
                }
                if (local.name().toStdString() == note.lang().get())
                {
                    defaultNote = QString(note.c_str());
                }
            }
            reinforcementItem->setArg(arg.name().c_str(),
                                      value,
                                      arg.layout().get().widget_type(),
                                      arg.value_limits().get().c_str(),
                                      arg.input_example() != nullptr ? arg.input_example().get().c_str() : "",
                                      noop2Translate(defaultLabel),
                                      noop2Translate(defaultNote));
        }

        QString defaultLabel;
        for (auto label : iter.label())
        {
            if (label.lang() == nullptr)
            {
                ////KLOG_DEBUG("label = %s",label.c_str());
                defaultLabel = QString(label.c_str());
                continue;
            }

            if (local.name().toStdString() == label.lang().get())
            {
                defaultLabel = QString(label.c_str());
            }
        }
        auto test = qApp->translate("xml", "Turn on ICMP redirection");
        reinforcementItem->setLabel(noop2Translate(defaultLabel));

        QString defaultDescription;
        for (auto description : iter.description())
        {
            if (description.lang() == nullptr)
            {
                defaultDescription = QString(description.c_str());
                continue;
            }
            //获取系统语言来进行匹配，符合获取符合系统语言的label
            if (local.name().toStdString() == description.lang().get())
            {
                defaultDescription = QString(description.c_str());
            }
        }
        reinforcementItem->setDescription(noop2Translate(defaultDescription));
        reinforcementItem->setCategoryName(iter.category().get().c_str());

        for (auto categories : categoriesList)
        {
            CONTINUE_IF_TRUE(categories->getName() != iter.category().get().c_str())
            BREAK_IF_TRUE(reinforcementItem->getName() == "external-hosts-login-limit" && !QFile::exists("/etc/hosts.allow"))
            categories->setReinforcementItem(reinforcementItem);
            break;
        }
    }
    return true;
}

QString Utils::ssrResetReinforcement(const QString &xmlString,
                                     const QString &categoryName,
                                     const QString &argName)
{
    RETURN_VAL_IF_TRUE(xmlString == "", QString(""))

    std::istringstream istringStream(xmlString.toStdString());
    auto rsReinforcements = KS::Protocol::br_reinforcements(istringStream, xml_schema::Flags::dont_validate);
    auto rsReinforcement = rsReinforcements.get()->reinforcement();

    for (auto iter : rsReinforcement)
    {
        CONTINUE_IF_TRUE(iter.name().c_str() != categoryName)

        for (auto arg : iter.arg())
        {
            CONTINUE_IF_TRUE(arg.name().c_str() != argName)
            return QString(arg.value().c_str());
        }
    }
    return QString("");
}

QList<QJsonValue> Utils::ssrResetReinforcements(const QString &xmlString, QList<Category *> &categoriesList)
{
    QList<QJsonValue> valueList;
    RETURN_VAL_IF_TRUE(xmlString == "", valueList)

    std::istringstream istringStream(xmlString.toStdString());
    auto rsReinforcements = KS::Protocol::br_reinforcements(istringStream, xml_schema::Flags::dont_validate);
    auto rsReinforcement = rsReinforcements.get()->reinforcement();

    for (auto iter : rsReinforcement)
    {
        for (auto arg : iter.arg())
        {
            for (auto categories : categoriesList)
            {
                auto reinforcementItem = categories->find(iter.name().c_str());
                CONTINUE_IF_TRUE(reinforcementItem == nullptr)
                auto categoryArg = reinforcementItem->find(arg.name().c_str());
                CONTINUE_IF_TRUE(categoryArg == nullptr)

                categoryArg->jsonValue = StrUtils::str2jsonValue(arg.value());
                valueList << categoryArg->jsonValue;
            }
        }
    }
    return valueList;
}

void Utils::invalidDataParsing(const QString &json, const QString &checkKey, InvalidData &invalidData)
{
    QJsonParseError jsonError;

    auto jsonDoc = QJsonDocument::fromJson(json.toUtf8(), &jsonError);
    if (jsonDoc.isNull())
    {
        KLOG_WARNING() << "Parser files information failed: " << jsonError.errorString();
        return;
    }
    auto invalids = jsonDoc.object().value("return_value").toObject().value(checkKey).toString();
    ;
    if (checkKey == CHECK_INVALID_CVE_VULNERABLITY_KEY)
    {
        invalidData.vulnerabilityScanInvalidList.clear();
        auto invalidsList = invalids.split(";");
        for (auto invalid : invalidsList)
        {
            CONTINUE_IF_TRUE(invalid.isEmpty())
            invalidData.vulnerabilityScanInvalidList.push_back(invalid);
        }
    }
    else if (checkKey == CHECK_INVALID_NOUSER_FILES_KEY)
    {
        invalidData.NouserFilesList.clear();
        auto invalidsList = invalids.split("\n");
        for (auto invalid : invalidsList)
        {
            CONTINUE_IF_TRUE(invalid.isEmpty())
            invalidData.NouserFilesList.push_back(invalid);
        }
    }
    else if (checkKey == CHECK_INVALID_AUTHORITY_FILES_KEY)
    {
        invalidData.AuthorityFilesList.clear();
        auto invalidsList = invalids.split("\n");
        for (auto invalid : invalidsList)
        {
            CONTINUE_IF_TRUE(invalid.isEmpty())
            invalidData.AuthorityFilesList.push_back(invalid);
        }
    }
    else if (checkKey == CHECK_INVALID_SUID_SGID_FILES_KEY)
    {
        invalidData.SuidSgidFilesList.clear();
        auto invalidsList = invalids.split("\n");
        for (auto invalid : invalidsList)
        {
            CONTINUE_IF_TRUE(invalid.isEmpty())
            invalidData.SuidSgidFilesList.push_back(invalid);
        }
    }
}

// 加固/扫描 结果，进度
bool Utils::ssrJobResult(const QString &xmlString,
                         ProgressInfo &progressInfo,
                         QList<Category *> &categoriesList,
                         InvalidData &invalidData)
{
    RETURN_VAL_IF_TRUE(xmlString == "", false)

    std::istringstream istringStream(xmlString.toStdString());
    auto jobResult = KS::Protocol::br_job_result(istringStream, xml_schema::Flags::dont_validate);
    progressInfo.jobID = jobResult->job_id();
    progressInfo.jobState = jobResult->job_state();
    progressInfo.progress = jobResult->process();

    for (auto reinforcement : jobResult->reinforcement())
    {
        for (auto categories : categoriesList)
        {
            auto reinforcementItem = categories->find(reinforcement.name().c_str());
            CONTINUE_IF_TRUE(reinforcementItem == nullptr)
            if (reinforcement.error() != nullptr)
            {
                reinforcementItem->setErrorMessage(python2Translate(reinforcement.error().get().c_str()));
            }

            reinforcementItem->setState(reinforcement.state());
            if (progressInfo.method == PROCESS_METHOD_SCAN)
            {
                reinforcementItem->setScanState(reinforcement.state());

                auto categoryName = QString(reinforcement.name().c_str());
                if (categoryName == CHECK_INVALID_CVE_VULNERABLITY)
                {
                    auto json = reinforcement.args().get();
                    invalidDataParsing(json.c_str(), CHECK_INVALID_CVE_VULNERABLITY_KEY, invalidData);
                }
                else if (categoryName == CHECK_INVALID_NOUSER_FILES)
                {
                    auto json = reinforcement.args().get();
                    invalidDataParsing(json.c_str(), CHECK_INVALID_NOUSER_FILES_KEY, invalidData);
                }
                else if (categoryName == CHECK_INVALID_AUTHORITY_FILES)
                {
                    auto json = reinforcement.args().get();
                    invalidDataParsing(json.c_str(), CHECK_INVALID_AUTHORITY_FILES_KEY, invalidData);
                }
                else if (categoryName == CHECK_INVALID_SUID_SGID_FILES)
                {
                    auto json = reinforcement.args().get();
                    invalidDataParsing(json.c_str(), CHECK_INVALID_SUID_SGID_FILES_KEY, invalidData);
                }
            }
            else if (progressInfo.method == PROCESS_METHOD_FASTEN)
            {
                reinforcementItem->setFastenState(reinforcement.state());
            }
        }
    }
    return true;
}

//设置加固项
QStringList Utils::ssrSetReinforcement(const QString &xmlString, QList<Category *> &categoriesList)
{
    QStringList retStringList = {};
    RETURN_VAL_IF_TRUE(xmlString == "", retStringList)

    std::istringstream istringStream(xmlString.toStdString());
    auto rsReinforcements = KS::Protocol::br_reinforcements(istringStream, xml_schema::Flags::dont_validate);
    auto &rsReinforcement = rsReinforcements.get()->reinforcement();

    int count = 0, index = 0;
    for (auto &iter : rsReinforcement)
    {
        count++;
        if (count > categoriesList.at(index)->getReinforcementItem().length())
        {
            count = 1;
            ++index;
        }
        auto str = QString(iter.name().c_str());
        if (str == "external-hosts-login-limit" && !QFile::exists("/etc/hosts.allow"))
        {
            count = count - 1;
            continue;
        }

        auto reinforcementItem = categoriesList.at(index)->find(str);
        CONTINUE_IF_TRUE(reinforcementItem == nullptr || reinforcementItem->changeFlag == false)
        reinforcementItem->changeFlag = false;

        for (auto &arg : iter.arg())
        {
            auto categoryArgs = reinforcementItem->find(arg.name().c_str());
            CONTINUE_IF_TRUE(categoryArgs == nullptr)
            arg.value(categoryArgs->jsonValue.toVariant().toString().toStdString());
        }
        std::ostringstream ostring_stream;
        KS::Protocol::br_reinforcement(ostring_stream, iter);

        retStringList.append(QString::fromStdString(ostring_stream.str()));
    }
    return retStringList;
}

KS::Protocol::RA::ReinforcementSequence Utils::raAnalysis(const QString &filePath)
{
    RETURN_VAL_IF_TRUE(!QFile::exists(filePath) || filePath == "", KS::Protocol::RA::ReinforcementSequence();)

    auto ra = KS::Protocol::br_ra(filePath.toStdString(), xml_schema::Flags::dont_validate);
    return ra->reinforcement();
}

QString Utils::categoriesLabel2Translate(const QString &souceTxt)
{
    return qApp->translate("ini", souceTxt.toUtf8());
}

QString Utils::python2Translate(const QString &souceTxt)
{
    return qApp->translate("python", souceTxt.toUtf8());
}

QString Utils::noop2Translate(const QString &souceTxt)
{
    auto tmpSouce = souceTxt;
    auto tmpList = tmpSouce.split("\"");
    QStringList translateList;
    for (auto key : tmpList)
    {
        CONTINUE_IF_TRUE(key.isEmpty() || key == "," || key == ", " || key == "QT_TRANSLATE_NOOP(" || key == "QT_TRANSLATE_NOOP_UTF8(" || key == ")")
        key.remove(QRegExp("^ +\\s*"));
        translateList << key;
    }

    RETURN_VAL_IF_TRUE(translateList.size() != 2, souceTxt)
    return qApp->translate(translateList[0].toUtf8(), translateList[1].toUtf8());
}
}  // namespace BR
}  // namespace KS
