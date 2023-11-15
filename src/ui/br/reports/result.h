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

#include <QList>
#include <QWidget>
#include <QtPrintSupport/QPrinter>
#include "src/ui/br/br-i.h"

namespace KS
{
namespace BR
{
namespace Plugins
{
class Categories;
}

namespace Reports
{
class Table;
class PDF;

class Result : public QWidget
{
    Q_OBJECT

public:
    explicit Result(QWidget *parent = 0);
    ~Result();

    static QSharedPointer<Result> getDefault();

    bool generateReports(const QList<Plugins::Categories *> &beforeReinforcementList,
                         const QList<Plugins::Categories *> &afterReinforcementList,
                         int status,
                         const InvalidData &invalidData);
    int getHeight();

private slots:
    bool exportReport(const QList<Plugins::Categories *> &afterReinforcementList, int status, const InvalidData &invalidData);

private:
    void init();
    QString state2Str(int state);
    QColor state2Color(int state);
    QString getIPPath();
    QString getMacPath();
    void createPainter(QPrinter &printer);
    void createReportHomePage(int status, const QRect &rect);
    void createReportcontent(QPrinter &printer, const QList<Plugins::Categories *> &afterReinforcementList, const InvalidData &invalidData);
    bool createFilesScanResults(QPrinter &printer, const InvalidData &invalidData, bool &showTailFlag);
    bool createVulnerabilityResults(QPrinter &printer, const InvalidData &invalidData, bool &showTailFlag);
    void calculateRatio();
    bool scanFilesAnalysis(QStringList &filelist, const InvalidData &invalidData);
    bool scanVulnerability(QStringList &rpmlist, const InvalidData &invalidData);

private:
    QList<Plugins::Categories *> m_categories;

    PDF *m_pdf;
    Table *m_table;
    QSharedPointer<QPainter> m_painter;

    QString m_categoryName[128];
    int m_total[4] = {0};
    int m_conform[4] = {0};
    int m_inconform[4] = {0};
    int m_allcategories = 0;
    static QSharedPointer<Result> m_instance;
};
}  // namespace Reports
}  // namespace BR
}  // namespace KS
