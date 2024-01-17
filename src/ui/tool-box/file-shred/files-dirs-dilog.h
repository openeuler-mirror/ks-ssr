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

#pragma once
#include <QFileDialog>

namespace KS
{
namespace ToolBox
{
// 继承QFileDialog，自定义accept时的处理（关闭对话框）用于同时选中文件以及文件夹使用
class FilesDirsDilog : public QFileDialog
{
    Q_OBJECT
public:
    FilesDirsDilog(QWidget *parent) : QFileDialog(parent)
    {
    }
    virtual ~FilesDirsDilog(){};

public slots:
    void selectedAccept(){ QDialog::accept(); };
};
}  // namespace ToolBox
}  // namespace KS
