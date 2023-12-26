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

#include "result.h"
#include <kylin-license/license-i.h>
#include <QDataStream>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QNetworkInterface>
#include <QPainter>
#include <QSysInfo>
#include <QVBoxLayout>
#include <QtMath>
#include "include/ssr-marcos.h"
#include "src/ui/br/reinforcement-items/category.h"
#include "src/ui/br/reports/pdf.h"
#include "src/ui/br/reports/table.h"

#define TABLE_MAX_LINE 28
#define TABLE_SHOW_TAIL_MAX_LINE 20
#define SSR_REPORTS_STYLE_PATH ":/styles/br-reports"

namespace KS
{
namespace BR
{
Result::Result(QWidget *parent)
    : QWidget(parent)
{
    init();
}

QSharedPointer<Result> Result::m_instance = nullptr;
QSharedPointer<Result> Result::getDefault()
{
    if (!m_instance)
    {
        m_instance = QSharedPointer<Result>::create();
    }
    return m_instance;
}

void Result::init()
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

QString Result::state2Str(int state)
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
        retStr = QString(tr("Scannig"));
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
        retStr = QString(tr("Reinforcing"));
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

QColor Result::state2Color(int state)
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

bool Result::generateReports(const QList<Category *> &beforeReinforcementList, const QList<Category *> &afterReinforcementList,
                             int status,
                             const InvalidData &invalidData)
{
    m_categories = beforeReinforcementList;
    return exportReport(afterReinforcementList, status, invalidData);
}

bool Result::scanFilesAnalysis(QStringList &filelist, const InvalidData &invalidData)
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

bool Result::scanVulnerability(QStringList &rpmlist, const InvalidData &invalidData)
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

void Result::addCategoryResults(QPrinter &printer, const QList<Result::CategoryContent> &categoryContents, bool &showTailFlag)
{
    auto count = 0;
    // 先遍历添加不符合的项后添加符合项
    for (auto& categoryContent : categoryContents)
    {
        count++;
        if ((categoryContent.scanStatus & BR_REINFORCEMENT_STATE_SAFE) == 1)
        {
            count--;
            continue;
        }

        if (count >= TABLE_MAX_LINE)
        {
            count = 1;
            addNewPainterPage(printer);
        }
        showTailFlag = (count >= TABLE_SHOW_TAIL_MAX_LINE) ? true : false;

        m_table->addLine(categoryContent.itemName,
                         state2Str(categoryContent.scanStatus),
                         state2Str(categoryContent.afterReinforceScanStatus),
                         categoryContent.remarks,
                         state2Color(categoryContent.scanStatus),
                         state2Color(categoryContent.afterReinforceScanStatus),
                         count % 2 == 1 ? "#f2f2f2" : "#ffffff");
    }
    // 符合项
    for (auto& categoryContent : categoryContents)
    {
        count++;
        if ((categoryContent.scanStatus & BR_REINFORCEMENT_STATE_UNSAFE) == 2)
        {
            count--;
            continue;
        }

        if (count >= TABLE_MAX_LINE)
        {
            count = 1;
            addNewPainterPage(printer);
        }
        showTailFlag = (count >= TABLE_SHOW_TAIL_MAX_LINE) ? true : false;
        m_table->addLine(categoryContent.itemName,
                         state2Str(categoryContent.scanStatus),
                         state2Str(categoryContent.afterReinforceScanStatus),
                         categoryContent.remarks,
                         state2Color(categoryContent.scanStatus),
                         state2Color(categoryContent.afterReinforceScanStatus),
                         count % 2 == 1 ? "#f2f2f2" : "#ffffff");
    }
}

void Result::addNewPainterPage(QPrinter &printer)
{
    m_table->addSpacer();
    auto page = m_table->grab(m_table->rect());
    m_painter->drawPixmap(0, 0, page);
    printer.newPage();

    delete m_table;
    m_table = new Table(this);
}

QString Result::getIPPath()
{
    QString strIpAddress;
    auto ipAddressesList = QNetworkInterface::allAddresses();

    // 获取第一个本主机的IPv4地址
    auto nListSize = ipAddressesList.size();
    for (int i = 0; i < nListSize; ++i)
    {
        CONTINUE_IF_TRUE(ipAddressesList.at(i) == QHostAddress::LocalHost || !ipAddressesList.at(i).toIPv4Address())
        strIpAddress = ipAddressesList.at(i).toString();
        break;
    }

    // 如果没有找到，则以本地IP地址为IP
    if (strIpAddress.isEmpty())
    {
        strIpAddress = QHostAddress(QHostAddress::LocalHost).toString();
    }
    return strIpAddress;
}

QString Result::getMacPath()
{
    auto nets = QNetworkInterface::allInterfaces();  // 获取所有网络接口列表
    auto nCnt = nets.count();
    QString strMacAddr = "";
    for (int i = 0; i < nCnt; i++)
    {
        // 如果此网络接口被激活并且正在运行并且不是回环地址，则就是我们需要找的Mac地址
        if (nets[i].flags().testFlag(QNetworkInterface::IsUp) && nets[i].flags().testFlag(QNetworkInterface::IsRunning) && !nets[i].flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            strMacAddr = nets[i].hardwareAddress();
            break;
        }
    }

    return strMacAddr;
}

void Result::createPainter(QPrinter &printer)
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

void Result::createReportHomePage(int status, const QRect &rect)
{
    auto activeStatus = status == LicenseActivationStatus::LAS_UNACTIVATED ? QString(tr("Unactivated")) : QString(tr("Activated"));

    m_pdf = new PDF(QSysInfo::prettyProductName(), getIPPath(), getMacPath(), QSysInfo::kernelType() + QSysInfo::kernelVersion(), activeStatus, this);
    m_pdf->setPieChartText(m_categoryName, m_total, m_conform, m_inconform);
    auto pixmap = m_pdf->grab(m_pdf->rect());
    // 计算painter视口区域与抓取图片区域的尺寸比例因子
    float factor = (float)rect.width() / pixmap.width();
    // 绘制时按照比例因子放大
    m_painter->scale(factor, factor);

    // 按照坐标画图
    m_painter->drawPixmap(0, 0, pixmap);
}

void Result::createReportContent(QPrinter &printer, const QList<Category *> &afterReinforcementList, const InvalidData &invalidData)
{
    bool flag = false;
    int i = 0;
    // 用于排序，不符合项需放在最前面
    QList<CategoryContent> categoryContents;
    m_table = new Table(this);
    // 扫描结果
    for (auto category : m_categories)
    {
        i++;
        for (auto reinforcementItem : category->getReinforcementItem())
        {
            CONTINUE_IF_TRUE(!reinforcementItem->getCheckStatus());
            auto afterReinforcementScanState = afterReinforcementList.isEmpty() ? BR_REINFORCEMENT_STATE_UNREINFORCE : afterReinforcementList.value(i - 1)->find(reinforcementItem->getName())->getScanState();
            categoryContents << CategoryContent{
                .itemName = reinforcementItem->getLabel(),
                .scanStatus = reinforcementItem->getScanState(),
                .afterReinforceScanStatus = afterReinforcementScanState,
                .remarks = "-"};
        }
    }
    addCategoryResults(printer, categoryContents, flag);

    // 扫描文件结果
    auto isScan = createFilesScanResults(printer, invalidData, flag);

    // 漏洞扫描结果
    auto isVulnerability = createVulnerabilityResults(printer, invalidData, flag);

    if (flag)
    {
        m_table->addSpacer();
        QPixmap page = m_table->grab(m_table->rect());
        m_painter->drawPixmap(0, 0, page);
        printer.newPage();
        delete m_table;
        m_table = new Table(this, isScan, isVulnerability);
    }

    m_table->addSpacer();
    m_table->showTailBar();

    auto pagePixmap = m_table->grab(m_table->rect());
    m_painter->drawPixmap(0, 0, pagePixmap);

    m_painter->end();
}

bool Result::createFilesScanResults(QPrinter &printer, const InvalidData &invalidData, bool &showTailFlag)
{
    QStringList scanList;
    bool is_scan = scanFilesAnalysis(scanList, invalidData);
    RETURN_VAL_IF_FALSE(is_scan, false);

    m_table->addSpacer();
    QPixmap page = m_table->grab(m_table->rect());
    m_painter->drawPixmap(0, 0, page);
    printer.newPage();
    delete m_table;
    m_table = new Table(this, true);
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
            m_table = new Table(this, is_scan);
        }
        showTailFlag = (i >= TABLE_SHOW_TAIL_MAX_LINE) ? true : false;
        if (i % 2 == 1)
            m_table->addScanLine(scanFilesList.at(count), scanTypeList.at(count), "-", "#f2f2f2");
        else
            m_table->addScanLine(scanFilesList.at(count), scanTypeList.at(count), "-", "#ffffff");
    }
    return true;
}

bool Result::createVulnerabilityResults(QPrinter &printer, const InvalidData &invalidData, bool &showTailFlag)
{
    QStringList vulnerabilityList;
    bool is_vulnerability = scanVulnerability(vulnerabilityList, invalidData);
    RETURN_VAL_IF_FALSE(is_vulnerability, false);

    m_table->addSpacer();
    QPixmap page = m_table->grab(m_table->rect());
    m_painter->drawPixmap(0, 0, page);
    printer.newPage();
    delete m_table;
    m_table = new Table(this, false, is_vulnerability);
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
            m_table = new Table(this, false, is_vulnerability);
        }
        showTailFlag = (i >= TABLE_SHOW_TAIL_MAX_LINE) ? true : false;
        m_table->addScanLine(rpmNameList.at(count), rpmResultList.at(count), "-", i % 2 == 1 ? "#f2f2f2" : "#ffffff");
    }
    return true;
}

void Result::calculateRatio()
{
    int i = 0, j = 0;
    memset(m_total, 0, sizeof(m_total));
    memset(m_conform, 0, sizeof(m_total));
    memset(m_inconform, 0, sizeof(m_total));
    for (auto categories : m_categories)
    {
        m_categoryName[i++] = categories->getLabel();
        for (auto reinforcementItem : categories->getReinforcementItem())
        {
            CONTINUE_IF_TRUE(!reinforcementItem->getCheckStatus())
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
bool Result::exportReport(const QList<Category *> &afterReinforcementList, int status, const InvalidData &invalidData)
{
    calculateRatio();

    QFileDialog fileDialog;
    auto file = QString(tr("KylinSecHostReinforcementReport_%1_%2.pdf")).arg(QSysInfo::machineHostName()).arg(getIPPath());
    auto fileName = fileDialog.getSaveFileName(this, tr("Open File"), file, tr("PDF(*.pdf)"));
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
    createReportContent(printerPixmap, afterReinforcementList, invalidData);

    return true;
}
}  // namespace BR
}  // namespace KS
