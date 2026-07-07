#include "pdf.h"
#include "round-progressbar.h"
#include "ui_pdf.h"

namespace KS
{
PDF::PDF(
    const QString &IP,
    const QString &MAC,
    const QString &kernel,
    const QString &activeStatus,
    const QList<QPair<QString, QString>> &homeExtra,
    QWidget *parent)
    : QWidget(parent),
      m_ui(new Ui::PDF)
{
    m_ui->setupUi(this);

    m_ui->m_systemName->setText(QSysInfo::prettyProductName());
    m_ui->m_IP->setText(IP);
    m_ui->m_MAC->setText(MAC);
    m_ui->m_kernel->setText(kernel);
    m_ui->m_activeStatus->setText(activeStatus);

    for (auto extraData : homeExtra)
    {
        QLabel *labelName = new QLabel(extraData.first, this);
        m_ui->verticalLayoutName->addWidget(labelName);

        QLabel *labelValue = new QLabel(extraData.second, this);
        labelValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_ui->verticalLayoutName->addWidget(labelValue);
    }
}

PDF::~PDF()
{
    delete m_ui;
}

void PDF::setPieChartText(const QString name[],
                          int total[],
                          int conform[],
                          int inconform[])
{
    update();
}
}  // namespace KS
