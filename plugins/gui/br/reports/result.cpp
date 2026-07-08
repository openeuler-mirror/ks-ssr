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

#include "report.h"
#include <kylin-license/license-i.h>
#include <QDataStream>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QPainter>
#include <QSysInfo>
#include <QVBoxLayout>
#include <QtMath>
#include "include/ssr-marcos.h"
#include "lib/base/sys-info.h"
#include "reinforcement-items/category.h"
#include "reports/pdf-details.h"
#include "reports/pdf-summary.h"

#define TABLE_MAX_LINE 28
#define TABLE_SHOW_TAIL_MAX_LINE 20
#define SSR_REPORTS_STYLE_PATH ":/styles/br-reports"
// 初始扫描状态（未扫描）
#define INI_SCAN_STATUS 4

namespace KS
{
namespace BR
{
Report::Report(QWidget *parent)
    : QWidget(parent)
{
    init();
}

QSharedPointer<Report> Report::m_instance = nullptr;
QSharedPointer<Report> Report::getDefault()
{
    if (!m_instance)
    {
        m_instance = QSharedPointer<Report>::create();
    }
    return m_instance;
}

void Report::init()
{
    setWindowModality(Qt::ApplicationModal);
    // 初始化样式表
    QFile file(SSR_REPORTS_STYLE_PATH);
    if (file.open(QIODevice::ReadOnly))
    {
        QString windowStyle = file.readAll();
        setStyleSheet(styleSheet() + windowStyle);
    }
    else
    {
        KLOG_WARNING() << "Failed to open file " << SSR_REPORTS_STYLE_PATH;
    }
}

QString Report::state2Str(int state)
{
    QString retStr;
    switch (state)
    {
    case BR_REINFORCEMENT_STATE_UNKNOWN:
        retStr = QString(tr("Unknown"));
        break;
    case BR_REINFORCEMENT_STATE_SAFE:
        retStr = QString(tr("Conformity"));
        break;
    case BR_REINFORCEMENT_STATE_UNSAFE:
        retStr = QString(tr("Inconformity"));
        break;
    case BR_REINFORCEMENT_STATE_UNSCAN:
        retStr = QString(tr("Not Scanned"));
        break;
    case BR_REINFORCEMENT_STATE_SCANNING:
        retStr = QString(tr("Scannig..."));
        break;
    case BR_REINFORCEMENT_STATE_SCAN_ERROR:
        retStr = QString(tr("Scan Failed"));
        break;
    case BR_REINFORCEMENT_STATE_SCAN_DONE:
        retStr = QString(tr("Scan Complete"));
        break;
    case BR_REINFORCEMENT_STATE_UNREINFORCE:
        retStr = QString(tr("Unreinforcement"));
        break;
    case BR_REINFORCEMENT_STATE_REINFORCING:
        retStr = QString(tr("Reinforcing..."));
        break;
    case BR_REINFORCEMENT_STATE_REINFORCE_ERROR:
        retStr = QString(tr("Reinforcement Failure"));
        break;
    case BR_REINFORCEMENT_STATE_REINFORCE_DONE:
        retStr = QString(tr("Reinforced"));
        break;
    default:
        if ((state & BR_REINFORCEMENT_STATE_SAFE) == 1)
            retStr = QString(tr("Conformity"));
        else if ((state & BR_REINFORCEMENT_STATE_UNSAFE) == 2)
            retStr = QString(tr("Inconformity"));
        else
            retStr = QString(tr("Unknown"));
        break;
    }
    return retStr;
}

QColor Report::state2Color(int state)
{
    QColor retColor;
    switch (state)
    {
    case BR_REINFORCEMENT_STATE_UNKNOWN:
        retColor = QColor("#ed6262");
        break;
    case BR_REINFORCEMENT_STATE_SAFE:
        retColor = QColor("#1200ff");
        break;
    case BR_REINFORCEMENT_STATE_UNSAFE:
        retColor = QColor("#ed6262");
        break;
    case BR_REINFORCEMENT_STATE_UNSCAN:
        retColor = QColor("#000000");
        break;
    case BR_REINFORCEMENT_STATE_SCANNING:
        retColor = QColor("#000000");
        break;
    case BR_REINFORCEMENT_STATE_SCAN_ERROR:
        retColor = QColor("#ed6262");
        break;
    case BR_REINFORCEMENT_STATE_SCAN_DONE:
        retColor = QColor("#1200ff");
        break;
    case BR_REINFORCEMENT_STATE_UNREINFORCE:
        retColor = QColor("#000000");
        break;
    case BR_REINFORCEMENT_STATE_REINFORCING:
        retColor = QColor("#000000");
        break;
    case BR_REINFORCEMENT_STATE_REINFORCE_ERROR:
        retColor = QColor("#ed6262");
        break;
    case BR_REINFORCEMENT_STATE_REINFORCE_DONE:
        retColor = QColor("#1200ff");
        break;
    default:
        if ((state & BR_REINFORCEMENT_STATE_SAFE) == 1)
            retColor = QColor("#1200ff");
        else if ((state & BR_REINFORCEMENT_STATE_UNSAFE) == 2)
            retColor = QColor("#ed6262");
        else
            retColor = QColor("#000000");
        break;
    }
    return retColor;
}

bool Report::scanFilesAnalysis(QStringList &filelist, const InvalidData &invalidData)
{
    RETURN_VAL_IF_TRUE((invalidData.NouserFilesList.count() < 1 && invalidData.SuidSgidFilesList.count() < 1 && invalidData.AuthorityFilesList.count() < 1), false)

    for (int i = 0; i < invalidData.NouserFilesList.count(); ++i)
    {
        CONTINUE_IF_TRUE(invalidData.NouserFilesList.at(i) == "" || filelist.contains(invalidData.NouserFilesList.at(i)))
        filelist << invalidData.NouserFilesList.at(i) << tr("No master file");
    }

    for (int i = 0; i < invalidData.AuthorityFilesList.count(); ++i)
    {
        CONTINUE_IF_TRUE(invalidData.AuthorityFilesList.at(i) == "" || filelist.contains(invalidData.AuthorityFilesList.at(i)))
        filelist << invalidData.AuthorityFilesList.at(i) << tr("Files with 777 permissions");
    }

    bool is_sgid = false;
    for (int i = 0; i < invalidData.SuidSgidFilesList.count(); ++i)
    {
        if (invalidData.SuidSgidFilesList.at(i) == "[  GUID ]")
            is_sgid = true;
        CONTINUE_IF_TRUE((invalidData.SuidSgidFilesList.at(i) == "[  SUID ]" || invalidData.SuidSgidFilesList.at(i) == "[  GUID ]" || invalidData.SuidSgidFilesList.at(i) == ""))
        CONTINUE_IF_TRUE(filelist.contains(invalidData.SuidSgidFilesList.at(i)))
        if (is_sgid)
            filelist << invalidData.SuidSgidFilesList.at(i) << tr("Files with sgid permission");
        else
            filelist << invalidData.SuidSgidFilesList.at(i) << tr("Files with suid permission");
    }
    return true;
}

bool Report::scanVulnerability(QStringList &rpmlist, const InvalidData &invalidData)
{
    RETURN_VAL_IF_TRUE(invalidData.vulnerabilityScanInvalidList.count() < 1, false)
    for (int i = 0; i < invalidData.vulnerabilityScanInvalidList.count(); ++i)
    {
        auto invalidRpm = invalidData.vulnerabilityScanInvalidList.at(i);
        auto rpms = invalidRpm.split(" ");
        for (auto rpm : rpms)
        {
            CONTINUE_IF_TRUE(rpm == "" || rpmlist.contains(rpm))
            rpmlist << rpm;
        }
    }

    return true;
}

void Report::addCategoryResults(QPrinter &printer, const QList<Report::CategoryContent> &categoryContents, bool &showTailFlag)
{
    auto count = 0;
    // 由于会有三种状态，符合/不符合/未扫描，需要遍历三次进行添加
    // 先遍历添加不符合的项后添加符合项
    for (auto &categoryContent : categoryContents)
    {
        CONTINUE_IF_TRUE((categoryContent.scanStatus & BR_REINFORCEMENT_STATE_SAFE) == 1);
        addLineToTable(printer, categoryContent, showTailFlag, count);
    }
    // 符合项
    for (auto &categoryContent : categoryContents)
    {
        CONTINUE_IF_TRUE((categoryContent.scanStatus & BR_REINFORCEMENT_STATE_UNSAFE) == 2);
        addLineToTable(printer, categoryContent, showTailFlag, count);
    }
    // TODO ： 取消扫描，但是加固项是勾选的，确认取消扫描后未扫描项是否需要在报表中展示 #25701
    // for (auto &categoryContent : categoryContents)
    // {
    //    CONTINUE_IF_TRUE(categoryContent.scanStatus != INI_SCAN_STATUS);
    //    addLineToTable(printer, categoryContent, showTailFlag, count);
    // }
}

void Report::addLineToTable(QPrinter &printer, const Report::CategoryContent &categoryContent, bool &showTailFlag, int &count)
{
    count++;
    if (count >= TABLE_MAX_LINE)
    {
        count = 1;
        addNewPainterPage(printer);
    }
    showTailFlag = (count >= TABLE_SHOW_TAIL_MAX_LINE) ? true : false;
    m_table->addLine(categoryContent.itemName,
                     state2Str(categoryContent.scanStatus),
                     state2Color(categoryContent.scanStatus),
                     count % 2 == 1 ? "#f2f2f2" : "#ffffff");
}

void Report::addNewPainterPage(QPrinter &printer)
{
    m_table->addSpacer();
    auto page = m_table->grab(m_table->rect());
    m_painter->drawPixmap(0, 0, page);
    printer.newPage();

    delete m_table;
    m_table = new PDFDetails(this);
}

void Report::createPainter(QPrinter &printer)
{
    m_painter = QSharedPointer<QPainter>::create();
    m_painter->begin(&printer);

    QFont font;
    font.setPointSize(40);
    font.setFamily(QString("NotoSansCJKsc-Regular"));
    font.setLetterSpacing(QFont::AbsoluteSpacing, 0);
    m_painter->setFont(font);

    QPen oriPen;
    oriPen.setWidth(2);
    oriPen.setColor(Qt::black);
    m_painter->setPen(oriPen);
    m_painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
}

void Report::createReportHomePage(int status, const QRect &rect)
{
    auto activeStatus = status == LicenseActivationStatus::LAS_UNACTIVATED ? QString(tr("Unactivated")) : QString(tr("Activated"));

    m_pdf = new PDFSummary(QSysInfo::prettyProductName(), getIPPath(), getMacPath(), getKernelInfo(), activeStatus, this);
    m_pdf->setPieChartText(m_categoryName, m_total, m_conform, m_inconform);
    auto pixmap = m_pdf->grab(m_pdf->rect());
    // 计算painter视口区域与抓取图片区域的尺寸比例因子
    float factor = (float)rect.width() / pixmap.width();
    // 绘制时按照比例因子放大
    m_painter->scale(factor, factor);

    // 按照坐标画图
    m_painter->drawPixmap(0, 0, pixmap);
}

void Report::createReportContent(QPrinter &printer, const QList<Category *> &scanList, const InvalidData &invalidData)
{
    bool flag = false;
    m_table = new PDFDetails(this);

    QList<CategoryContent> categoryContents;
    for (auto category : scanList)
    {
        for (auto item : category->getReinforcementItem())
        {
            categoryContents << CategoryContent{
                .itemName = item->getLabel(),
                .scanStatus = item->getScanState()};
        }
    }

    addCategoryResults(printer, categoryContents, flag);

    // 扫描文件结果
    createFilesScanResults(printer, invalidData, flag);

    // 漏洞扫描结果
    //    auto isVulnerability = createVulnerabilityResults(printer, invalidData, flag);

    m_table->addSpacer();
    m_table->showTailBar();

    auto pagePixmap = m_table->grab(m_table->rect());
    m_painter->drawPixmap(0, 0, pagePixmap);
}

bool Report::createFilesScanResults(QPrinter &printer, const InvalidData &invalidData, bool &showTailFlag)
{
    QStringList scanList;
    bool is_scan = scanFilesAnalysis(scanList, invalidData);
    RETURN_VAL_IF_FALSE(is_scan, false);

    m_table->addSpacer();
    QPixmap page = m_table->grab(m_table->rect());
    m_painter->drawPixmap(0, 0, page);
    printer.newPage();
    delete m_table;
    m_table = new PDFDetails(this, true);
    // 解析文件名与扫描类型
    QStringList scanFilesList;
    QStringList scanTypeList;
    for (int count = 0; count < scanList.count(); ++count)
    {
        auto test = scanList.at(count);
        count % 2 == 1 ? scanTypeList << scanList.at(count) : scanFilesList << scanList.at(count);
    }

    // 插入报表
    int i = 0;
    for (int count = 0; count < scanFilesList.count(); ++count)
    {
        ++i;
        if (i >= TABLE_MAX_LINE)
        {
            m_table->addSpacer();
            i = 1;
            QPixmap page = m_table->grab(m_table->rect());
            m_painter->drawPixmap(0, 0, page);
            printer.newPage();

            delete m_table;
            m_table = new PDFDetails(this, is_scan);
        }
        showTailFlag = (i >= TABLE_SHOW_TAIL_MAX_LINE) ? true : false;
        if (i % 2 == 1)
            m_table->addScanLine(scanFilesList.at(count), scanTypeList.at(count), "-", "#f2f2f2");
        else
            m_table->addScanLine(scanFilesList.at(count), scanTypeList.at(count), "-", "#ffffff");
    }

    return true;
}

bool Report::createVulnerabilityResults(QPrinter &printer, const InvalidData &invalidData, bool &showTailFlag)
{
    QStringList vulnerabilityList;
    bool is_vulnerability = scanVulnerability(vulnerabilityList, invalidData);
    RETURN_VAL_IF_FALSE(is_vulnerability, false);

    m_table->addSpacer();
    QPixmap page = m_table->grab(m_table->rect());
    m_painter->drawPixmap(0, 0, page);
    printer.newPage();
    delete m_table;
    m_table = new PDFDetails(this, false, is_vulnerability);
    // 解析文件名与扫描类型
    QStringList rpmNameList;
    QStringList rpmResultList;
    for (int count = 0; count < vulnerabilityList.count(); ++count)
    {
        if (count % 2 == 1)
        {
            auto result = QString(tr("Vulnerability exists in version %1")).arg(vulnerabilityList.at(count));
            rpmResultList << result;
        }
        else
        {
            rpmNameList << vulnerabilityList.at(count);
        }
    }

    // 插入报表
    int i = 0;
    for (int count = 0; count < rpmNameList.count(); ++count)
    {
        ++i;
        if (i >= TABLE_MAX_LINE)
        {
            m_table->addSpacer();
            i = 1;
            QPixmap page = m_table->grab(m_table->rect());
            m_painter->drawPixmap(0, 0, page);
            printer.newPage();

            delete m_table;
            m_table = new PDFDetails(this, false, is_vulnerability);
        }
        showTailFlag = (i >= TABLE_SHOW_TAIL_MAX_LINE) ? true : false;
        m_table->addScanLine(rpmNameList.at(count), rpmResultList.at(count), "-", i % 2 == 1 ? "#f2f2f2" : "#ffffff");
    }
    return true;
}

void Result::calculateRatio(const QList<Category *> &categories)
{
    int i = 0, j = 0;
    memset(m_total, 0, sizeof(m_total));
    memset(m_conform, 0, sizeof(m_total));
    memset(m_inconform, 0, sizeof(m_total));
    for (auto category : categories)
    {
        m_categoryName[i++] = category->getLabel();
        for (auto reinforcementItem : category->getReinforcementItem())
        {
            if (((reinforcementItem->getScanState() & BR_REINFORCEMENT_STATE_SAFE) == 1))
            {
                m_conform[j]++;
                m_total[j]++;
            }
            else if ((reinforcementItem->getScanState() & BR_REINFORCEMENT_STATE_UNSAFE) == 2)
            {
                m_inconform[j]++;
                m_total[j]++;
            }
        }
        m_allcategories += m_total[j];
        j++;
    }
}

// picture
bool Result::generateReports(const QList<Category *> &scanList, int status, const InvalidData &invalidData)
{
    calculateRatio(scanList);

    auto file = QString(tr("KylinSecHostReinforcementReport_%1_%2.pdf")).arg(QSysInfo::machineHostName()).arg(getIPPath());
    auto fileName = QFileDialog::getSaveFileName(nullptr, tr("export report"), file, tr("PDF(*.pdf)"));
    RETURN_VAL_IF_TRUE(fileName == "", false)
    // 定义打印机 631端口被禁用可能会导致阻塞
    QPrinter printerPixmap(QPrinter::ScreenResolution);
#if QT_DEPRECATED_SINCE(5, 15)
    printerPixmap.setPageSize(QPageSize(QPageSize::PageSizeId::A4));
#else
    printerPixmap.setPageSize(QPainter::A4);
#endif
    printerPixmap.setResolution(300);
    printerPixmap.setOutputFormat(QPrinter::PdfFormat);
    printerPixmap.setOutputFileName(fileName);
    printerPixmap.setFullPage(true);

    createPainter(printerPixmap);
    // 报表首页
    createReportHomePage(status, printerPixmap.pageLayout().fullRectPixels(printerPixmap.resolution()));
    printerPixmap.newPage();
    createReportContent(printerPixmap, scanList, invalidData);

    m_painter->end();

    return true;
}
}  // namespace BR
}  // namespace KS
