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

#include "br-i.h"
#include "br-protocol.hxx"
#include "progress.h"
#include "src/ui/br/reinforcement-items/category.h"
#include "src/ui/br/reinforcement-items/reinforcement-item.h"
namespace KS
{
namespace BR
{
// 解析
class Utils
{
public:
    Utils();
    virtual ~Utils();

    static QSharedPointer<Utils> getDefault();

    QString state2Str(int state);
    QColor state2Color(int status);
    // Category json字符串插入到QList<Category *>
    void jsonParsing(const QByteArray &categoriesJson, QList<Category *> &categoriesList);
    // 设置加固项，返回设置完成的xml格式加固项参数
    QStringList ssrSetReinforcement(const QString &xmlString, QList<Category *> &categoriesList);
    // 从xml字符串解析加固项，存入Categories
    bool ssrReinforcements(const QString &xmlString, QList<Category *> &categoriesList);
    // 从xml字符串解析加固项，获取重置加固项后的值并返回
    QString ssrResetReinforcement(const QString &xmlString,
                                  const QString &categoryName,
                                  const QString &argName);
    // 从xml字符串解析加固项，获取重置所有加固项后的值并返回，且修改Categories中储存的值
    QList<QJsonValue> ssrResetReinforcements(const QString &xmlString, QList<Category *> &categoriesList);
    // 从xml字符串解析扫描/加固结果，并储存至progressInfo，categoriesList，invalidData
    bool ssrJobResult(const QString &xmlString,
                      ProgressInfo &progressInfo,
                      QList<Category *> &categoriesList,
                      InvalidData &invalidData);
    // ra文件（自定义参数文件）解析，返回自定义修改参数后的加固项信息
    KS::Protocol::RA::ReinforcementSequence raAnalysis(const QString &filePath);

private:
    // 非法数据解析
    void invalidDataParsing(const QString &json, const QString &checkKey, InvalidData &invalidData);
    // #define QT_TRANSLATE_NOOP(scope, x) QCoreApplication::translate(scope,x)
    QString noop2Translate(const QString &souceTxt);
    QString categoriesLabel2Translate(const QString &souceTxt);
    QString python2Translate(const QString &souceTxt);

private:
    static QSharedPointer<Utils> m_instance;
};

}  // namespace BR
}  // namespace KS
