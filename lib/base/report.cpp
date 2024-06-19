#include "report.h"
#include <hpdf.h>
#include <math.h>
#include <qt5-log-i.h>
#include <QDateTime>
#include <QNetworkInterface>
#include <QObject>
#include "config.h"
#include "include/ssr-marcos.h"

#define TTF_PATH SSR_INSTALL_DATADIR "/AlibabaPuHuiTi-3-55-Regular.ttf"

#define TITLE_FONT_SIZE 24          // 一级标题字体大小
#define TITLE_POS_Y 760             // 页首标题Y坐标
#define TITLE_SECOND_FONT_SIZE 20   // 二级标题字体大小
#define CONTENT_FONT_SIZE 16        // 内容字体大小
#define CONTENT_MARGIN 40           // 报告主页边距
#define HOME_LINE_SPACING 30        // 主页行间距
#define TABLE_LINE_PER_PAGE 20      // 表格一页行数
#define TABLE_LINE_HEIGHT 32        // 表格行高
#define TABLE_CONTENT_FONT_SIZE 10  // 表格内容字体大小
#define TAIL_POS_Y 40               // 页尾Y坐标
#define TAIL_FONT_SIZE 8            // 页尾字体大小

static void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data)
{
    KLOG_ERROR() << "libharu error: " << error_no << ", detail: " << detail_no;
}

static QString getIPPath()
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

static QString getMacPath()
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

void addTail(HPDF_Page page, HPDF_Font font)
{
    QString text = QObject::tr("KylinSec Co., Ltd. Contact:400-012-6606");  // 湖南麒麟信安科技股份有限公司，联系电话：400-012-6606
    // 获取页面宽度和高度
    float pageWidth = HPDF_Page_GetWidth(page);

    HPDF_Page_BeginText(page);
    HPDF_Page_SetFontAndSize(page, font, TAIL_FONT_SIZE);
    HPDF_REAL tw = HPDF_Page_TextWidth(page, text.toLocal8Bit());
    HPDF_Page_TextOut(page, pageWidth / 2 - tw / 2, TAIL_POS_Y, text.toLocal8Bit());
    HPDF_Page_EndText(page);
}

void addWatermark(HPDF_Page page, HPDF_Font font)
{
    QString text = QObject::tr("KylinSec Co., Ltd.");  // 湖南麒麟信安科技股份有限公司
    float angle = 45.0 * M_PI / 180.0;                 // 45 degree rotation, 30% opacity

    // Save the current graphic state
    HPDF_Page_GSave(page);

    // Set transparency
    HPDF_Page_SetRGBFill(page, 0.9, 0.9, 0.9);  // Light gray color

    // Apply transparency
    // float opacity = 0.3;
    // HPDF_Page_SetAlphaFill(page, opacity);

    // Get page dimensions
    float page_width = HPDF_Page_GetWidth(page);
    float page_height = HPDF_Page_GetHeight(page);

    // Rotate text
    HPDF_Page_Concat(page, cos(angle), sin(angle), -sin(angle), cos(angle), page_width / 2, page_height / 2);

    // Set font and size
    HPDF_Page_SetFontAndSize(page, font, 50);

    // Calculate text width and height for centering
    float text_width = HPDF_Page_TextWidth(page, text.toLocal8Bit());
    float text_height = 50;  // Same as font size

    // Draw the text
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, -text_width / 2, -text_height / 2, text.toLocal8Bit());
    HPDF_Page_EndText(page);

    // Restore the previous graphic state
    HPDF_Page_GRestore(page);
}

static void makeHomePage(HPDF_Doc pdf, HPDF_Font font, const QList<QPair<QString, QString>> &homeExtraData)
{
    HPDF_Page page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);  // A4,横向纵向
    addWatermark(page, font);                                        // 45 degree rotation, 30% opacity
    addTail(page, font);

    // 获取页面宽度和高度
    float pageWidth = HPDF_Page_GetWidth(page);

    HPDF_Page_BeginText(page);
    {
        HPDF_Page_SetFontAndSize(page, font, TITLE_FONT_SIZE);
        QString text = QObject::tr("KylinSec Host Security Reinforcement V1 report");  // 麒麟信安主机安全加固软件V1报告
        HPDF_REAL tw = HPDF_Page_TextWidth(page, text.toLocal8Bit());
        HPDF_Page_TextOut(page, pageWidth / 2 - tw / 2, TITLE_POS_Y, text.toLocal8Bit());
    }
    {
        HPDF_Page_SetFontAndSize(page, font, TITLE_SECOND_FONT_SIZE);
        QString text = QObject::tr("Information overview");  // 信息总览
        HPDF_REAL tw = HPDF_Page_TextWidth(page, text.toLocal8Bit());
        HPDF_Page_TextOut(page, pageWidth / 2 - tw / 2, TITLE_POS_Y - CONTENT_MARGIN, text.toLocal8Bit());
    }

    QList<QPair<QString, QString>> homeData;
    homeData.append(qMakePair(QObject::tr("OS:"), QSysInfo::prettyProductName()));  // 操作系统:
    homeData.append(qMakePair(QString("IP:"), getIPPath()));
    homeData.append(qMakePair(QString("MAC:"), getMacPath()));
    homeData.append(qMakePair(QObject::tr("kernel version:"), QSysInfo::kernelType() + QSysInfo::kernelVersion()));         // 系统内核版本
    homeData.append(qMakePair(QObject::tr("Software active state:"), QObject::tr("activated")));                            // 软件激活状态 //已激活
    homeData.append(qMakePair(QObject::tr("Export time:"), QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));  // 导出时间

    homeData.append(homeExtraData);

    HPDF_Page_SetFontAndSize(page, font, CONTENT_FONT_SIZE);
    int startYPos = TITLE_POS_Y - CONTENT_MARGIN * 2;
    for (auto data : homeData)
    {
        HPDF_Page_TextOut(page, CONTENT_MARGIN, startYPos, data.first.toLocal8Bit());

        HPDF_REAL tw = HPDF_Page_TextWidth(page, data.second.toLocal8Bit());
        HPDF_Page_TextOut(page, pageWidth - CONTENT_MARGIN - tw, startYPos, data.second.toLocal8Bit());

        startYPos -= HOME_LINE_SPACING;
    }

    HPDF_Page_EndText(page);
}

static void draw_table(HPDF_Page page, float x, float y, float width, float height, int rows, int cols)
{
    float cell_width = width / cols;
    float cell_height = height / rows;

    // Draw horizontal lines
    for (int i = 0; i <= rows; ++i)
    {
        float y_offset = y - (i * cell_height);
        HPDF_Page_MoveTo(page, x, y_offset);
        HPDF_Page_LineTo(page, x + width, y_offset);
        HPDF_Page_Stroke(page);
    }

    // Draw vertical lines
    for (int j = 0; j <= cols; ++j)
    {
        float x_offset = x + (j * cell_width);
        HPDF_Page_MoveTo(page, x_offset, y);
        HPDF_Page_LineTo(page, x_offset, y - height);
        HPDF_Page_Stroke(page);
    }
}

static void add_table_data(HPDF_Page page, HPDF_Font font, float x, float y, float width, float height, int rows, int cols, const QList<QStringList> &tabelData, bool hasFirstRow)
{
    float cell_width = width / cols;
    float cell_height = height / rows;

    for (int i = 0; i < tabelData.size(); ++i)
    {
        for (int j = 0; j < tabelData.at(i).size(); ++j)
        {
            float x_pos = x + (j * cell_width) + 2;                 // Add some padding
            float y_pos = y - (i * cell_height) - cell_height + 2;  // Add some padding

            if (hasFirstRow && 0 == i)
            {
                HPDF_Page_SetFontAndSize(page, font, TABLE_CONTENT_FONT_SIZE + 3);
                HPDF_Page_BeginText(page);
                HPDF_Page_TextOut(page, x_pos, y_pos, tabelData[i][j].toLocal8Bit());
                HPDF_Page_EndText(page);
                HPDF_Page_SetFontAndSize(page, font, TABLE_CONTENT_FONT_SIZE);
            }
            else
            {
                HPDF_Page_BeginText(page);
                HPDF_Page_TextOut(page, x_pos, y_pos, tabelData[i][j].toLocal8Bit());
                HPDF_Page_EndText(page);
            }
        }
    }
}

static void makeTableTitle(HPDF_Page page, HPDF_Font font, const QString &tableTitle, bool emptyTable = false)
{
    // 获取页面宽度和高度
    float pageWidth = HPDF_Page_GetWidth(page);

    {
        HPDF_Page_BeginText(page);
        HPDF_Page_SetFontAndSize(page, font, TITLE_SECOND_FONT_SIZE);

        HPDF_REAL tw = HPDF_Page_TextWidth(page, tableTitle.toLocal8Bit());
        HPDF_Page_TextOut(page, pageWidth / 2 - tw / 2, TITLE_POS_Y, tableTitle.toLocal8Bit());
        HPDF_Page_EndText(page);
    }

    if (emptyTable)
    {
        HPDF_Page_BeginText(page);
        HPDF_Page_SetFontAndSize(page, font, TABLE_CONTENT_FONT_SIZE);
        QString text = QObject::tr("No problems were found in this test");  // 本次检测没有发现问题项
        HPDF_REAL tw = HPDF_Page_TextWidth(page, text.toLocal8Bit());
        HPDF_Page_TextOut(page, pageWidth / 2 - tw / 2, TITLE_POS_Y - CONTENT_MARGIN, text.toLocal8Bit());
        HPDF_Page_EndText(page);
    }
}

static void makeTablePage(HPDF_Doc pdf, HPDF_Font font, const QString &tableTitle, const QList<QStringList> &tabelData)
{
    int totalRows = tabelData.size();
    int curRowIndex = 0;
    float table_x = CONTENT_MARGIN;
    float table_y = TITLE_POS_Y - CONTENT_MARGIN / 2;

    bool hasFirstRow = true;
    do
    {
        HPDF_Page page = HPDF_AddPage(pdf);
        HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);  // A4,横向纵向
        addWatermark(page, font);
        addTail(page, font);

        // 获取页面宽度和高度
        float pageWidth = HPDF_Page_GetWidth(page);

        if (tabelData.isEmpty())
        {
            makeTableTitle(page, font, tableTitle, true);
            return;
        }

        makeTableTitle(page, font, tableTitle);
        HPDF_Page_SetFontAndSize(page, font, TABLE_CONTENT_FONT_SIZE);

        int rows = TABLE_LINE_PER_PAGE;
        int cols = tabelData.at(0).size();
        if (rows >= totalRows)
        {
            rows = totalRows;
        }

        totalRows -= rows;

        float table_width = pageWidth - CONTENT_MARGIN * 2;
        float table_height = TABLE_LINE_HEIGHT * rows;

        draw_table(page, table_x, table_y, table_width, table_height, rows, cols);
        add_table_data(page, font, table_x, table_y, table_width, table_height, rows, cols, tabelData.mid(curRowIndex, rows), hasFirstRow);
        hasFirstRow = false;
        curRowIndex += rows;

    } while (totalRows > 0);
}

QString Report::genReport(const QString &savePath, const QList<QPair<QString, QString>> &homeExtraData, const QString &tableTitle, const QList<QStringList> &tabelData)
{
    QString failedReason;

    HPDF_Doc pdf = HPDF_New(error_handler, nullptr);
    if (!pdf)
    {
        failedReason = "Failed to create PDF object";
        KLOG_ERROR() << failedReason;
        return failedReason;
    }

    HPDF_UseUTFEncodings(pdf);
    HPDF_SetCurrentEncoder(pdf, "UTF-8");
    // 字体
    const char *font_name = HPDF_LoadTTFontFromFile(pdf, TTF_PATH, HPDF_TRUE);
    if (!font_name)
    {
        failedReason = QString("Failed to Load TTF file:") + TTF_PATH;
        KLOG_ERROR() << failedReason;
        return failedReason;
    }

    HPDF_Font font = HPDF_GetFont(pdf, font_name, "UTF-8");

    makeHomePage(pdf, font, homeExtraData);
    makeTablePage(pdf, font, tableTitle, tabelData);

    HPDF_SaveToFile(pdf, savePath.toLocal8Bit());
    HPDF_Free(pdf);

    return failedReason;
}
