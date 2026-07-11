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
#pragma once

#include <QWidget>
#include <QtPrintSupport/QPrinter>
#include "br-i.h"

namespace KS
{
namespace BR
{
class Category;
class PDFDetails;
class PDFSummary;

class Report : public QWidget
{
    Q_OBJECT

public:
    explicit Report(QWidget *parent = nullptr);
    virtual ~Report(){};

    static QSharedPointer<Report> getDefault();

    bool generateReports(const QList<Category *> &scanList, int status, const InvalidData &invalidData);

    int getHeight();

private:
    struct CategoryContent
    {
        QString itemName;
        int scanStatus;
    };

private:
    void init();
    QString state2Str(int state);
    QColor state2Color(int state);
    void createPainter(QPrinter &printer);
    void createReportHomePage(int status, const QRect &rect);
    void createReportContent(QPrinter &printer, const QList<Category *> &scanList, const InvalidData &invalidData);
    bool createFilesScanResults(QPrinter &printer, const InvalidData &invalidData, bool &showTailFlag);
    bool createVulnerabilityResults(QPrinter &printer, const InvalidData &invalidData, bool &showTailFlag);
    void calculateRatio(const QList<Category *> &categories);
    bool scanFilesAnalysis(QStringList &filelist, const InvalidData &invalidData);
    bool scanVulnerability(QStringList &rpmlist, const InvalidData &invalidData);
    void addCategoryResults(QPrinter &printer, const QList<CategoryContent> &categoryContents, bool &showTailFlag);
    // 插入一行到扫描结果表格
    void addLineToTable(QPrinter &printer, const CategoryContent &categoryContent, bool &showTailFlag, int &count);
    void addNewPainterPage(QPrinter &printer);

private:
    PDFSummary *m_pdfSummary;
    PDFDetails *m_pdfDetails;
    QSharedPointer<QPainter> m_painter;

    QString m_categoryName[128];
    int m_total[4] = {0};
    int m_conform[4] = {0};
    int m_inconform[4] = {0};
    int m_allcategories = 0;
    static QSharedPointer<Report> m_instance;
};
}  // namespace BR
}  // namespace KS
