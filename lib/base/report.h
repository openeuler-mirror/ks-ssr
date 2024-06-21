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
 * Author:     yangfeng <yangfeng@kylinsec.com.cn>
 */

#ifndef REPORT_H
#define REPORT_H
#include <QList>
#include <QObject>
#include <QPair>
#include <QString>

class Report : public QObject
{
public:
    static QString genReport(const QString &savePath,
                             const QList<QPair<QString, QString>> &homeExtraData,
                             const QString &tableTitle,
                             const QList<QStringList> &tabelData);

private:
    Report(){};
};

#endif  // REPORT_H
