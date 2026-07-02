#include "src/ui/trusted/tp-kernel.h"
#include <qt5-log-i.h>
#include <QDir>
#include <QFileDialog>
#include "ksc-i.h"
#include "src/ui/trusted_proxy.h"
#include "src/ui/ui_tp-kernel.h"

namespace KS
{
TPKernel::TPKernel(QWidget *parent) : QWidget(parent),
                   m_ui(new Ui::TPKernel)
{
    this->m_ui->setupUi(this);

    this->m_trustedProtectedProxy = new TrustedProxy(KSC_DBUS_NAME,
                                                        KSC_FILE_PROTECTED_DBUS_OBJECT_PATH,
                                                        QDBusConnection::systemBus(),
                                                        this);
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records")).arg(this->m_ui->m_kernelTable->getKernelRecords().size());
    this->m_ui->m_tips->setText(text);

    // TODO:需要绘制颜色
    this->m_ui->m_search->addAction(QIcon(":/images/search"), QLineEdit::ActionPosition::LeadingPosition);

    connect(this->m_ui->m_search, SIGNAL(textChanged(const QString &)), this, SLOT(searchTextChanged(const QString &)));
    connect(this->m_ui->m_add, SIGNAL(clicked(bool)), this, SLOT(addClicked(bool)));
    connect(this->m_ui->m_update, SIGNAL(clicked(bool)), this, SLOT(updateClicked(bool)));
    connect(this->m_ui->m_unprotect, SIGNAL(clicked(bool)), this, SLOT(unprotectClicked(bool)));
}

TPKernel::~TPKernel()
{
    delete m_ui;
}

void TPKernel::updateInfo()
{
    this->m_ui->m_kernelTable->updateRecord();
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records")).arg(this->m_ui->m_kernelTable->getKernelRecords().size());
    this->m_ui->m_tips->setText(text);
}

void TPKernel::searchTextChanged(const QString &text)
{
    this->m_ui->m_kernelTable->searchTextChanged(text);
}

void TPKernel::addClicked(bool checked)
{
    auto fileName = QFileDialog::getOpenFileName(this, tr("Open file"), QDir::homePath());
    if (!fileName.isEmpty())
    {
        this->m_trustedProtectedProxy->AddFile(fileName);
        updateInfo();
    }
}

void TPKernel::updateClicked(bool checked)
{
    updateInfo();
}

void TPKernel::unprotectClicked(bool checked)
{
    auto trustedInfos = this->m_ui->m_kernelTable->getKernelRecords();
    for (auto trustedInfo : trustedInfos)
    {
        if (trustedInfo.selected)
        {
            this->m_trustedProtectedProxy->RemoveFile(trustedInfo.filePath);
        }
    }
    updateInfo();
}

}  // namespace KS
