#include "report.h"
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
// #include "reinforcement-items/category.h"
#include "pdf.h"
#include "ssr-i.h"
#include "table.h"

#define TABLE_MAX_LINE 28
#define TABLE_SHOW_TAIL_MAX_LINE 20
#define SSR_REPORTS_STYLE_PATH ":/styles/br-reports"

namespace KS
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
                     state2Str(categoryContent.afterReinforceScanStatus),
                     categoryContent.remarks,
                     state2Color(categoryContent.scanStatus),
                     state2Color(categoryContent.afterReinforceScanStatus),
                     count % 2 == 1 ? "#f2f2f2" : "#ffffff");
}

void Report::addNewPainterPage(QPrinter &printer)
{
    m_table->addSpacer();
    auto page = m_table->grab(m_table->rect());
    m_painter->drawPixmap(0, 0, page);
    printer.newPage();

    delete m_table;
    m_table = new Table(this);
}

QString Report::getIPPath()
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

QString Report::getMacPath()
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

bool Report::generateReports(QString savePath, const ReportData &reportData)
{
    if (savePath.isEmpty())
    {
        savePath = QString(tr("KylinSecReport_%1_%2.pdf")).arg(QSysInfo::machineHostName()).arg(getIPPath());
    }

    // 定义打印机 631端口被禁用可能会导致阻塞
    QPrinter printerPixmap(QPrinter::ScreenResolution);
#if QT_DEPRECATED_SINCE(5, 15)
    printerPixmap.setPageSize(QPageSize(QPageSize::PageSizeId::A4));
#else
    printerPixmap.setPageSize(QPainter::A4);
#endif
    printerPixmap.setResolution(300);
    printerPixmap.setOutputFormat(QPrinter::PdfFormat);
    printerPixmap.setOutputFileName(savePath);
    printerPixmap.setFullPage(true);

    createPainter(printerPixmap);

    // 报表首页
    createReportHomePage(printerPixmap.pageLayout().fullRectPixels(printerPixmap.resolution()), reportData);
    printerPixmap.newPage();
    createReportContent(printerPixmap, reportData);
}

bool Report::generateReports(CveDataList bugList)
{
    auto file = QString(tr("KylinSecVulnerabilityReport_%1_%2.pdf")).arg(QSysInfo::machineHostName()).arg(getIPPath());
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
    createReportHomePage(printerPixmap.pageLayout().fullRectPixels(printerPixmap.resolution()), bugList);
    printerPixmap.newPage();
    createReportContent(printerPixmap, bugList);

    return true;
}

void Report::createReportHomePage(const QRect &rect, const ReportData &reportData)
{
    auto activeStatus = QString(tr("Activated"));
    m_pdf = new PDF(
        getIPPath(),
        getMacPath(),
        QSysInfo::kernelType() + QSysInfo::kernelVersion(),
        activeStatus,
        reportData.homeExtra,
        this);
    m_pdf->setPieChartText(m_categoryName, m_total, m_conform, m_inconform);
    auto pixmap = m_pdf->grab(m_pdf->rect());
    // 计算painter视口区域与抓取图片区域的尺寸比例因子
    float factor = (float)rect.width() / pixmap.width();
    // 绘制时按照比例因子放大
    m_painter->scale(factor, factor);

    // 按照坐标画图
    m_painter->drawPixmap(0, 0, pixmap);
}

void Report::createReportContent(QPrinter &printer, CveDataList &bugList)
{
    m_table = new Table(this);
    createVulnerabilityReports(printer, bugList);

    m_table->addSpacer();
    m_table->showTailBar();

    auto pagePixmap = m_table->grab(m_table->rect());
    m_painter->drawPixmap(0, 0, pagePixmap);

    m_painter->end();
}

bool Report::createVulnerabilityReports(QPrinter &printer, CveDataList &bugList)
{
    m_table->addSpacer();
    QPixmap page = m_table->grab(m_table->rect());
    m_painter->drawPixmap(0, 0, page);
    // 插入报表
    int i = 0;
    for (int row = 0; row < bugList.count(); ++row)
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
            m_table = new Table(this);
        }

        QColor color;
        switch (bugList.at(row).level)
        {
        case 0:
            color = QColor("#fa4949");
            break;
        case 1:
            color = QColor("#fa7b49");
            break;
        case 2:
            color = QColor("#fab649");
            break;
        case 3:
            color = QColor("#ffe431");
            break;
        default:
            color = Qt::black;
        }
        QString level;
        switch (bugList.at(row).level)
        {
        case 0:
            level = tr("fatal");
            break;
        case 1:
            level = tr("hight");
            break;
        case 2:
            level = tr("middle");
            break;
        default:
            level = tr("low");
            break;
        }

        m_table->addLine(bugList.at(row).bugId, level, bugList.at(row).score, bugList.at(row).date, color, color, row % 2 == 1 ? "#f2f2f2" : "#ffffff");
    }
    return true;
}

}  // namespace KS
