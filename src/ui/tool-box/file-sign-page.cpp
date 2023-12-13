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
 * Author:     wangyucheng <wangyucheng@kylinos.com.cn>
 */

#include "src/ui/tool-box/file-sign-page.h"
#include <QDir>
#include <QFileDialog>
#include <QMetaEnum>
#include <QWidgetAction>
#include "QInputDialog"
#include "include/ssr-i.h"
#include "include/ssr-marcos.h"
#include "src/ui/tool-box/file-sign-table.h"
#include "src/ui/toolbox_dbus_proxy.h"
#include "src/ui/ui_file-sign-page.h"

#define FILE_SIGN_ICON_NAME "/images/file-sign"

namespace KS
{
namespace ToolBox
{

FileSign::FileSign(QWidget* parent)
    : Page(parent),
      m_ui(new Ui::FileSignPage)

{
    m_dbusProxy = new ToolBoxDbusProxy(SSR_DBUS_NAME,
                                       SSR_TOOL_BOX_DBUS_OBJECT_PATH,
                                       QDBusConnection::systemBus(),
                                       this);
    m_ui->setupUi(this);
    // TODO:需要绘制颜色
    auto searchButton = new QPushButton(m_ui->m_search);
    searchButton->setObjectName("searchButton");
    searchButton->setIcon(QIcon(":/images/search"));
    searchButton->setIconSize(QSize(16, 16));
    auto action = new QWidgetAction(m_ui->m_search);
    action->setDefaultWidget(searchButton);
    m_ui->m_search->addAction(action, QLineEdit::ActionPosition::LeadingPosition);

    // 刷新
    m_ui->m_refresh->setIcon(QIcon(":/images/refresh"));
    m_ui->m_refresh->setIconSize(QSize(16, 16));
    m_ui->m_refresh->installEventFilter(this);

    m_ui->m_select->setCursor(Qt::PointingHandCursor);
    m_ui->m_clean->setCursor(Qt::PointingHandCursor);
    QObject::connect(m_ui->m_search, SIGNAL(textChanged(const QString&)), m_ui->m_fileSignTable, SLOT(searchTextChanged(const QString&)));
    QObject::connect(m_ui->m_refresh, &QPushButton::clicked, this, &FileSign::refreshTable);
    QObject::connect(m_ui->m_select, &QPushButton::clicked, this, &FileSign::openFileDialog);
    QObject::connect(m_ui->m_clean, &QPushButton::clicked, m_ui->m_fileSignTable, &FileSignTable::cleanSelectedData);
    m_ui->m_tips->setText(tr("A total of %1 records").arg((m_ui->m_fileSignTable->getData().size())));
    QObject::connect(m_ui->m_fileSignTable, &FileSignTable::dataSizeChanged, [this] {
        m_ui->m_tips->setText(tr("A total of %1 records").arg(m_ui->m_fileSignTable->getData().size()));
    });
    QObject::connect(m_ui->m_fileSignTable, &FileSignTable::doubleClicked, m_ui->m_fileSignTable, [this](const QModelIndex& index) {
        // 目前功能只有双击点击编辑安全上下文
        RETURN_IF_TRUE(index.column() != FileSignField::FILE_SIGN_FIELD_FILE_SE_CONTEXT);
        auto data = this->m_ui->m_fileSignTable->getData();
        auto oldIterator = data.begin() + index.row();
        auto oldSecurityContext = oldIterator->fileSeContext;
        auto filePath = oldIterator->filePath;
        QString newSecurityContext = QInputDialog::getText(this,
                                                           tr("modify security context"),
                                                           tr("Please enter a new security context"),
                                                           QLineEdit::Normal,
                                                           oldSecurityContext);
        KLOG_DEBUG() << "New Security context: " << newSecurityContext;
        RETURN_IF_TRUE(newSecurityContext.isEmpty() || newSecurityContext == oldSecurityContext);
        this->m_dbusProxy->SetSecurityContext(filePath, newSecurityContext);
    });
}

FileSign::~FileSign()
{
    delete m_ui;
}

QString FileSign::getNavigationUID()
{
    return tr("Tool Box");
}

QString FileSign::getSidebarUID()
{
    return tr("File Sign");
}

QString FileSign::getSidebarIcon()
{
    return ":" FILE_SIGN_ICON_NAME;
}

void FileSign::openFileDialog(bool)
{
    auto files = QFileDialog::getOpenFileNames(nullptr, tr("Open files"), QDir::homePath());
    RETURN_IF_TRUE(files.isEmpty());
    updateTableData(files);
}

int FileSign::getSelinuxType()
{
    return 0;
}
void FileSign::updateTableData(const QStringList& fileList)
{
    FileSignRecordMap newData;
    for (const auto& file : fileList)
    {
// auto reply =  m_dbusProxy->GetSecurityContext(file);
#pragma message("完整性标签需要内核支持，内核支持后使用 rbapol 工具即可获取标签")
        newData[file] = {false, file, m_dbusProxy->GetSecurityContext(file).value(), "完整性标签需要内核支持，内核支持后使用 rbapol 工具即可获取标签"};
    }
    m_ui->m_fileSignTable->updateData(newData);
}

void FileSign::refreshTable(bool)
{
    updateTableData(m_ui->m_fileSignTable->getData().keys());
}
}  // namespace ToolBox
}  // namespace KS