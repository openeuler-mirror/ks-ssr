#pragma once

#include <QWidget>
#include <QtPrintSupport/QPrinter>

namespace KS
{
class Category;
class Table;
class PDF;

struct ReportData
{
    QList<QPair<QString, QString>> homeExtra;

    QList<QStringList> table;
};

class Report : public QWidget
{
    Q_OBJECT

public:
    explicit Report(QWidget *parent = nullptr);
    virtual ~Report(){};

    static QSharedPointer<Report> getDefault();

    bool generateReports(QString savePath, const ReportData &reportData);
    int getHeight();

private:
    struct CategoryContent
    {
        QString itemName;
        int scanStatus;
        int afterReinforceScanStatus;
        QString remarks;
    };

private:
    void init();
    QString state2Str(int state);
    QColor state2Color(int state);
    QString getIPPath();
    QString getMacPath();
    void createPainter(QPrinter &printer);

    void createReportHomePage(const QRect &rect, const ReportData &reportData);
    void createReportContent(QPrinter &printer, CveDataList &bugList);
    bool createVulnerabilityReports(QPrinter &printer, CveDataList &bugList);

    // 插入一行到扫描结果表格
    void addLineToTable(QPrinter &printer, const CategoryContent &categoryContent, bool &showTailFlag, int &count);
    void addNewPainterPage(QPrinter &printer);

private:
    QList<Category *> m_categories;

    PDF *m_pdf;
    Table *m_table;
    QSharedPointer<QPainter> m_painter;

    QString m_categoryName[128];
    int m_total[4] = {0};
    int m_conform[4] = {0};
    int m_inconform[4] = {0};
    int m_allcategories = 0;
    static QSharedPointer<Report> m_instance;
};
}  // namespace KS
