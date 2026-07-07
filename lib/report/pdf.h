#pragma once
#include <QWidget>

namespace Ui
{
class PDF;
}
namespace KS
{
class RoundProgressBar;

class PDF : public QWidget
{
    Q_OBJECT

public:
    explicit PDF(
        const QString &IP,
        const QString &MAC,
        const QString &kernel,
        const QString &activeStatus,
        const QList<QPair<QString, QString>> &homeExtra,
        QWidget *parent = 0);
    virtual ~PDF();

    void setPieChartText(const QString name[],
                         int total[],
                         int conform[],
                         int inconform[]);

private:
    Ui::PDF *m_ui;

    RoundProgressBar *m_pieChart1;
    RoundProgressBar *m_pieChart2;
    RoundProgressBar *m_pieChart3;
    RoundProgressBar *m_pieChart4;
};

}  // namespace KS
