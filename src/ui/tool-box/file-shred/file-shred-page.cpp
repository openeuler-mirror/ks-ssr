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
 * Author:     chendingjian <chendingjian@kylinos.com.cn>
 */

#include "file-shred-page.h"
#include <QDir>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QTreeView>
#include <QListView>
#include <QWidgetAction>
#include "include/ssr-i.h"
#include "src/ui/ui_file-shred-page.h"
#include "src/ui/tool-box/file-shred/files-dirs-dilog.h"
#include "src/ui/common/user-prompt-dialog.h"
#include "ssr-marcos.h"


#define FILE_SHRED_ICON_NAME "/images/file-shred"

namespace KS
{
namespace ToolBox
{
FileShredPage::FileShredPage(QWidget* parent)
    : Page(parent),
      m_ui(new Ui::FileShredPage)
{
    m_ui->setupUi(this);
    initUI();
}

FileShredPage::~FileShredPage()
{
    delete m_ui;
}

QString FileShredPage::getNavigationUID()
{
    return tr("Tool Box");
}

QString FileShredPage::getSidebarUID()
{
    return tr("File Shred");
}

QString FileShredPage::getSidebarIcon()
{
    return ":" FILE_SHRED_ICON_NAME;
}

QString FileShredPage::getAccountRoleName()
{
    return SSR_ACCOUNT_NAME_SECADM;
}

void FileShredPage::addFiles(bool checked)
{
    Q_UNUSED(checked);
    // 添加文件或目录
    auto fileDialog = new FilesDirsDilog(this);
    fileDialog->setOption(QFileDialog::Option::DontUseNativeDialog);

    // 多选
    auto listView = fileDialog->findChild<QListView *>();
    if (listView)
    {
        listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }
    auto treeView = fileDialog->findChild<QTreeView *>();
    if (treeView)
    {
        treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }
    auto buttonBox = fileDialog->findChild<QDialogButtonBox *>();
    disconnect(buttonBox, &QDialogButtonBox::accepted, fileDialog, nullptr);
    connect(buttonBox, &QDialogButtonBox::accepted, fileDialog, &FilesDirsDilog::selectedAccept);

    QStringList names;
    if (fileDialog->exec() == QDialog::Accepted)
    {
        names = fileDialog->selectedFiles();
    }

    RETURN_IF_TRUE(names.isEmpty());
    m_ui->m_table->addFiles(names);
}

void FileShredPage::initUI()
{
    // 更新表格右上角提示信息
    auto text = QString(tr("A total of %1 records")).arg(m_ui->m_table->getFileShredInfosSize());
    m_ui->m_tips->setText(text);

    // TODO:需要绘制颜色
    auto searchButton = new QPushButton(m_ui->m_search);
    searchButton->setObjectName("searchButton");
    searchButton->setIcon(QIcon(":/images/search"));
    searchButton->setIconSize(QSize(16, 16));
    auto action = new QWidgetAction(m_ui->m_search);
    action->setDefaultWidget(searchButton);
    m_ui->m_search->addAction(action, QLineEdit::ActionPosition::LeadingPosition);

    connect(m_ui->m_search, &QLineEdit::textChanged, this, [this](const QString& text)
            {
                m_ui->m_table->setSearchText(text);
            });
    connect(m_ui->m_add, SIGNAL(clicked(bool)), this, SLOT(addFiles(bool)));
    connect(m_ui->m_remove, &QPushButton::clicked, this, [this]
            {
                m_ui->m_table->delFiles();
            });
    connect(m_ui->m_shred, &QPushButton::clicked, this, [this]
            {
                auto shredNotify = new UserPromptDialog(this);
                shredNotify->setNotifyMessage(tr("File shred"), tr("After crushing, files or folders cannot be recovered."
                                                                   "Are you sure you want to crush them?"));
                auto x = window()->x() + window()->width() / 2 - shredNotify->width() / 2;
                auto y = window()->y() + window()->height() / 2 - shredNotify->height() / 2;
                shredNotify->move(x, y);
                shredNotify->show();
                connect(shredNotify, &UserPromptDialog::accepted, this, [this]{
                    m_ui->m_table->shredFiles();
                });
            });
    connect(m_ui->m_table, &FileShredTable::tableUpdated, this, [this](int total)
            {
                // 更新表格右上角提示信息
                auto text = QString(tr("A total of %1 records")).arg(QString::number(total));
                m_ui->m_tips->setText(text);
            });
}

}  // namespace ToolBox
}  // namespace KS
