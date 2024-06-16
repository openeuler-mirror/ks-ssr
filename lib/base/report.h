#ifndef REPORT_H
#define REPORT_H
#include <QList>
#include <QObject>
#include <QPair>
#include <QString>

class Report : public QObject
{
public:
    static void genReport(const QString &savePath,
                          const QList<QPair<QString, QString>> &homeExtraData,
                          const QString &tableTitle,
                          const QList<QStringList> &tabelData);

private:
    Report(){};
};

#endif  // REPORT_H
