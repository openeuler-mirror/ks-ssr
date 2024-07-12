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

#include "report.h"
#include <hpdf.h>
#include <math.h>
#include <qt5-log-i.h>
#include <QDateTime>
#include <QNetworkInterface>
#include <QObject>
#include "config.h"
#include "include/ssr-marcos.h"
#include "sys-info.h"

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

static void errorHandler(HPDF_STATUS errorNo, HPDF_STATUS detailNo, void *userData)
{
    KLOG_ERROR() << "libharu error: " << errorNo << ", detail: " << detailNo;
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
    float pageWidth = HPDF_Page_GetWidth(page);
    float pageHeight = HPDF_Page_GetHeight(page);

    // Rotate text
    HPDF_Page_Concat(page, cos(angle), sin(angle), -sin(angle), cos(angle), pageWidth / 2, pageHeight / 2);

    // Set font and size
    HPDF_Page_SetFontAndSize(page, font, 50);

    // Calculate text width and height for centering
    float textWidth = HPDF_Page_TextWidth(page, text.toLocal8Bit());
    float textHeight = 50;  // Same as font size

    // Draw the text
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, -textWidth / 2, -textHeight / 2, text.toLocal8Bit());
    HPDF_Page_EndText(page);

    // Restore the previous graphic state
    HPDF_Page_GRestore(page);
}

static void makeHomePage(HPDF_Doc pdf,
                         HPDF_Font font,
                         const QList<QPair<QString, QString>> &homeExtraData)
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
    homeData.append(qMakePair(QObject::tr("Host:"), QSysInfo::machineHostName()));
    homeData.append(qMakePair(QString("IP:"), getIPPath()));
    homeData.append(qMakePair(QString("MAC:"), getMacPath()));
    homeData.append(qMakePair(QObject::tr("kernel version:"), getKernelInfo()));                                            // 系统内核版本
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

static void makeTableTitle(HPDF_Page page,
                           HPDF_Font font,
                           const QString &tableTitle,
                           bool emptyTable = false)
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

// 将给定宽度的文本进行换行处理，并返回换行后的文本
typedef struct
{
    QStringList lines;
    int total_height;
} WrappedTextResult;

WrappedTextResult wrap_text(HPDF_Page page, QString text, HPDF_Font font, HPDF_REAL font_size, HPDF_REAL max_width)
{
    HPDF_REAL line_height = HPDF_Font_GetCapHeight(font) * font_size / 1000 + 4;  // 行高
    HPDF_REAL total_height = 0;

    // 保存每行文本
    QStringList lines;

    QByteArray ba = text.toLocal8Bit();
    HPDF_UINT text_length = ba.size();
    uint curIndex = 0;

    while (curIndex < text_length)
    {
        HPDF_UINT break_index = HPDF_Page_MeasureText(page, text.mid(curIndex).toLocal8Bit().data(), max_width, HPDF_FALSE, NULL);
        if (break_index == 0)
        {
            break;
        }

        // 保存当前行
        lines.append(QString::fromLocal8Bit(ba.mid(curIndex, break_index)));
        curIndex += break_index;

        total_height += line_height;
    }

    total_height += line_height;

    WrappedTextResult result;
    result.lines = lines;
    result.total_height = total_height + 1;  // 计算的是浮点,此处加1
    return result;
}

// 绘制单元格内的文本
void draw_text_in_cell(HPDF_Page page, const char *text, HPDF_Font font, HPDF_REAL font_size, HPDF_REAL x, HPDF_REAL y, HPDF_REAL cell_width, HPDF_REAL cell_height)
{
    WrappedTextResult wrapped_text = wrap_text(page, text, font, font_size, cell_width - 4);  // 留出一些内边距
    HPDF_REAL line_height = HPDF_Font_GetCapHeight(font) * font_size / 1000 + 4;              // 行高
    //    HPDF_REAL text_y = y - 1 - font_size;                                                     // 从单元格顶部留出一些内边距
    HPDF_REAL text_y = y - line_height;

    for (int i = 0; i < wrapped_text.lines.size(); i++)
    {
        HPDF_Page_BeginText(page);
        HPDF_Page_MoveTextPos(page, x + 2, text_y);  // 从单元格左边留出一些内边距
        HPDF_Page_ShowText(page, wrapped_text.lines[i].toLocal8Bit().data());
        HPDF_Page_EndText(page);
        text_y -= line_height;
    }
}

// 绘制表格
void draw_table(HPDF_Page page, HPDF_Font font, HPDF_REAL font_size, HPDF_REAL start_x, HPDF_REAL start_y, QStringList data, const QList<uint> &colWidth)
{
    HPDF_Page_SetFontAndSize(page, font, font_size);
    HPDF_Page_SetLineWidth(page, 0.5);

    // 计算每行的高度
    HPDF_REAL row_height = 0;

    HPDF_REAL max_height = 0;
    for (int col = 0; col < data.size(); col++)
    {
        WrappedTextResult wrapped_text = wrap_text(page, data[col], font, TABLE_CONTENT_FONT_SIZE, colWidth[col] - 4);
        if (wrapped_text.total_height > max_height)
        {
            max_height = wrapped_text.total_height;
        }
    }
    row_height = max_height;

    // 绘制表格
    HPDF_REAL x = start_x;
    for (int col = 0; col < data.size(); col++)
    {
        HPDF_REAL y = start_y - row_height;

        // 绘制单元格边框
        HPDF_Page_Rectangle(page, x, y, colWidth[col], row_height);
        HPDF_Page_Stroke(page);

        // 绘制单元格内的文本
        draw_text_in_cell(page, data[col].toLocal8Bit().data(), font, TABLE_CONTENT_FONT_SIZE, x, start_y, colWidth[col], row_height);

        x += colWidth[col];
    }
}

static void makeTablePage(HPDF_Doc pdf, HPDF_Font font, const QString &tableTitle, const QList<QStringList> &tabelData, const QList<uint> &colWidth)
{
    int totalRows = tabelData.size();
    int curRowIndex = 0;
    float tableX = CONTENT_MARGIN;
    float tableY = TITLE_POS_Y - CONTENT_MARGIN / 2;

    //    bool hasFirstRow = true;
    do
    {
        // 新建一页
        HPDF_Page page = HPDF_AddPage(pdf);
        HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);  // A4,横向纵向
        // 水印
        addWatermark(page, font);

        // 页脚
        addTail(page, font);

        // 获取页面宽度和高度
        //        float pageWidth = HPDF_Page_GetWidth(page);
        //        float page_height = HPDF_Page_GetHeight(page);
        //        float table_width = pageWidth - CONTENT_MARGIN * 2;

        if (tabelData.isEmpty())
        {
            makeTableTitle(page, font, tableTitle, true);
            return;
        }

        // 表格标题
        makeTableTitle(page, font, tableTitle);

        HPDF_Page_SetFontAndSize(page, font, TABLE_CONTENT_FONT_SIZE);

        // 一行一行画
        // 计算换行后的高度和文本
        // 画一行表格
        // 画一行数据

        int minY = TAIL_POS_Y + 10;
        int curY = tableY;
        while (minY < curY && totalRows > 0)
        {
            // 简单点,直接获取第2位的数据高度
            QStringList curRowData = tabelData.at(curRowIndex);

            HPDF_REAL rowHeight = 0;

            for (int col = 0; col < curRowData.size(); col++)
            {
                WrappedTextResult wrapped_text = wrap_text(page, curRowData[col], font, TABLE_CONTENT_FONT_SIZE, colWidth[col] - 4);
                if (wrapped_text.total_height > rowHeight)
                {
                    rowHeight = wrapped_text.total_height;
                }
            }

            // 换页
            if (minY > curY - rowHeight)
            {
                break;
            }

            draw_table(page, font, TABLE_CONTENT_FONT_SIZE, tableX, curY, curRowData, colWidth);

            curY -= rowHeight;
            curRowIndex++;
            totalRows--;
        }

#if 0
        int rows = TABLE_LINE_PER_PAGE;
        int cols = tabelData.at(0).size();
        if (rows >= totalRows)
        {
            rows = totalRows;
        }

        totalRows -= rows;

        float table_width = pageWidth - CONTENT_MARGIN * 2;
        float table_height = TABLE_LINE_HEIGHT * rows;

        // 画表格
        draw_table(page, table_x, table_y, table_width, table_height, rows, cols);
        // 填充表格数据
        add_table_data(page, font, table_x, table_y, table_width, table_height, rows, cols, tabelData.mid(curRowIndex, rows), hasFirstRow, colWidth);

        hasFirstRow = false;
        curRowIndex += rows;
#endif
    } while (totalRows > 0);
}

QString Report::genReport(const QString &savePath, const QList<QPair<QString, QString>> &homeExtraData, const QString &tableTitle, const QList<QStringList> &tabelData, const QList<uint> &colWidth)
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
    const char *fontName = HPDF_LoadTTFontFromFile(pdf, TTF_PATH, HPDF_TRUE);
    if (!fontName)
    {
        failedReason = QString("Failed to Load TTF file:") + TTF_PATH;
        KLOG_ERROR() << failedReason;
        return failedReason;
    }

    HPDF_Font font = HPDF_GetFont(pdf, fontName, "UTF-8");

    makeHomePage(pdf, font, homeExtraData);
    makeTablePage(pdf, font, tableTitle, tabelData, colWidth);

    HPDF_SaveToFile(pdf, savePath.toLocal8Bit());

    HPDF_Free(pdf);

    return failedReason;
}
