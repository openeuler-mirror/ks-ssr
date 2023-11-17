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

#pragma once
#include <QLocale>

#include "br-i.h"
#include "br-protocol.hxx"
#include "progress.h"
#include "src/ui/br/plugins/categories.h"
#include "src/ui/br/plugins/category.h"
namespace KS
{
namespace BR
{
class XMLUtils
{
public:
    XMLUtils();
    virtual ~XMLUtils();

    static QSharedPointer<XMLUtils> getDefault();

    QString state2Str(int state);
    QColor state2Color(int status);
    void jsonParsing(const QByteArray &json, QList<Plugins::Categories *> &categoriesList);

    QStringList ssrSetReinforcement(const QString &xmlString, QList<Plugins::Categories *> &categoriesList);
    bool ssrReinforcements(const QString &xmlString, QList<Plugins::Categories *> &categoriesList);
    QString ssrResetReinforcement(const QString &xmlString,
                                  const QString &categoryName,
                                  const QString &argName);
    QList<QJsonValue> ssrResetReinforcements(const QString &xmlString, QList<Plugins::Categories *> &categoriesList);
    bool ssrJobResult(const QString &xmlString,
                      ProgressInfo &progressInfo,
                      QList<Plugins::Categories *> &categoriesList,
                      InvalidData &invalidData);
    KS::Protocol::RA::ReinforcementSequence raAnalysis(const QString &filePath);

private:
    static QSharedPointer<XMLUtils> m_instance;
    // #define QT_TRANSLATE_NOOP(scope, x) QCoreApplication::translate(scope,x)
    QString noop2Translate(const QString &souceTxt);
    QString categoriesLabel2Translate(const QString &souceTxt);
    QString python2Translate(const QString &souceTxt);
};

}  // namespace BR
}  // namespace KS
