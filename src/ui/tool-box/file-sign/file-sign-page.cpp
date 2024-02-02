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

#include "src/ui/tool-box/file-sign/file-sign-page.h"
#include <QDir>
#include <QFileDialog>
#include <QMetaEnum>
#include <QWidgetAction>
#include "QInputDialog"
#include "include/ssr-i.h"
#include "src/ui/common/ssr-marcos-ui.h"
#include "src/ui/tool-box/file-sign/add-user-dialog.h"
#include "src/ui/tool-box/file-sign/file-sign-table.h"
#include "src/ui/tool-box/file-sign/modify-security-context.h"
#include "src/ui/toolbox_dbus_proxy.h"
#include "src/ui/ui_file-sign-page.h"

#define FILE_SIGN_ICON_NAME "/images/file-sign"

namespace KS
{
namespace ToolBox
{
FileSign::FileSign(QWidget* parent)
    : Page(parent),
      m_ui(new Ui::FileSignPage),
      m_modifySecurityContext(nullptr),
      m_inputUsers(nullptr)
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

    m_ui->m_selectFile->setCursor(Qt::PointingHandCursor);
    m_ui->m_clean->setCursor(Qt::PointingHandCursor);
    initConnection();
    // 获取保存在数据库中的标记的文件列表,
    updateTableData(m_dbusProxy->GetObjListFromSecuritySign().value());
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
    return tr("Security Sign");
}

QString FileSign::getSidebarIcon()
{
    return ":" FILE_SIGN_ICON_NAME;
}

void FileSign::openFileDialog(bool)
{
    auto files = QFileDialog::getOpenFileNames(nullptr, tr("Open files"), QDir::homePath());
    RETURN_IF_TRUE(files.isEmpty());
    // 不在这里更新前台中的数据， 通过监听后台的 FileSignListChanged 信号实现前台数据更新。
    // updateTableData(files);
    m_dbusProxy->AddObjToSecuritySign(files);
}

QString FileSign::getAccountRoleName()
{
    return SSR_ACCOUNT_NAME_SECADM;
}

// 前台不应该主动调用此函数
// 应该由 调用后台 dbus 接口 插入/删除数据 -> 后台发送 FileSignListChanged 信号 -> 触发 refresh 函数 -> refresh 函数中调用此函数
void FileSign::updateTableData(const QStringList& fileList)
{
    FileSignRecordMap newData;
    for (const auto& file : fileList)
    {
        if (file.startsWith('/'))
        {
            newData[file] = {false, file, m_dbusProxy->GetFileMLSLabel(file).value(), m_dbusProxy->GetFileKICLabel(file).value()};
        }
        else
        {
            newData[file] = {false, file, m_dbusProxy->GetUserMLSLabel(file).value(), ""};
        }
    }
    m_ui->m_fileSignTable->updateData(newData);
}

void FileSign::initConnection()
{
    connect(m_ui->m_search, SIGNAL(textChanged(const QString&)), m_ui->m_fileSignTable, SLOT(searchTextChanged(const QString&)));
    connect(m_ui->m_refresh, &QPushButton::clicked, [this]
            {
                auto reply = m_dbusProxy->GetObjListFromSecuritySign();
                CHECK_ERROR_FOR_DBUS_REPLY(reply);
                updateTableData(reply.value());
            });
    connect(m_ui->m_selectFile, &QPushButton::clicked, this, &FileSign::openFileDialog);
    connect(m_ui->m_selectUser, &QPushButton::clicked, [this](bool)
            {
                m_inputUsers = new AddUserDialog(this);
                connect(m_inputUsers, &AddUserDialog::accepted, this, [this]
                        {
                            auto userList = m_inputUsers->getUserList();
                            auto reply = m_dbusProxy->AddObjToSecuritySign(userList);
                            CHECK_ERROR_FOR_DBUS_REPLY(reply);
                        });
                auto x = window()->x() + window()->width() / 2 - m_inputUsers->width() / 2;
                auto y = window()->y() + window()->height() / 2 - m_inputUsers->height() / 2;
                m_inputUsers->move(x, y);
                m_inputUsers->show();
            });
    connect(m_ui->m_clean, &QPushButton::clicked, [this](bool)
            {
                auto reply = m_dbusProxy->RemoveObjFromSecuritySign(m_ui->m_fileSignTable->getSelectData());
                CHECK_ERROR_FOR_DBUS_REPLY(reply);
            });
    m_ui->m_tips->setText(tr("A total of %1 records").arg((m_ui->m_fileSignTable->getData().size())));
    connect(m_ui->m_fileSignTable, &FileSignTable::dataSizeChanged, [this]
            {
                m_ui->m_tips->setText(tr("A total of %1 records").arg(m_ui->m_fileSignTable->getData().size()));
            });
    connect(m_ui->m_fileSignTable, &FileSignTable::clicked, this, &FileSign::popModifySecurityContext);
    connect(m_dbusProxy, &ToolBoxDbusProxy::FileSignListChanged, [this]
            {
                auto reply = m_dbusProxy->GetObjListFromSecuritySign();
                CHECK_ERROR_FOR_DBUS_REPLY(reply);
                updateTableData(reply.value());
            });
}

void FileSign::popModifySecurityContext(const QModelIndex& index)
{
    RETURN_IF_TRUE(index.column() != FileSignField::FILE_SIGN_FIELD_OPERATE);
    auto data = this->m_ui->m_fileSignTable->getData();
    auto oldIterator = data.begin() + index.row();

    m_modifySecurityContext = new ModifySecurityContext(this);
    m_modifySecurityContext->setSecurityContext(oldIterator->fileSeContext);
    m_modifySecurityContext->setIntegrityLabel(oldIterator->fileCompleteLabel);
    m_modifySecurityContext->setFilePath(oldIterator->filePath);
    connect(m_modifySecurityContext, &ModifySecurityContext::accepted, this, &FileSign::acceptedSecurityContext, Qt::UniqueConnection);

    auto x = window()->x() + window()->width() / 2 - m_modifySecurityContext->width() / 2;
    auto y = window()->y() + window()->height() / 2 - m_modifySecurityContext->height() / 2;
    m_modifySecurityContext->move(x, y);
    m_modifySecurityContext->show();
}

void FileSign::acceptedSecurityContext()
{
    RETURN_IF_TRUE(m_modifySecurityContext->getFilePath().isEmpty());
    if (m_modifySecurityContext->getFilePath().startsWith('/'))
    {
        auto reply = m_dbusProxy->SetFileMLSLabel(m_modifySecurityContext->getFilePath(), m_modifySecurityContext->getSecurityContext());
        CHECK_ERROR_FOR_DBUS_REPLY(reply);
        reply = m_dbusProxy->SetFileKICLabel(m_modifySecurityContext->getFilePath(), m_modifySecurityContext->getIntegrityLabel());
        CHECK_ERROR_FOR_DBUS_REPLY(reply);
        return;
    }
    auto reply = m_dbusProxy->SetUserMLSLabel(m_modifySecurityContext->getFilePath(), m_modifySecurityContext->getSecurityContext());
    CHECK_ERROR_FOR_DBUS_REPLY(reply);
}
}  // namespace ToolBox
}  // namespace KS
