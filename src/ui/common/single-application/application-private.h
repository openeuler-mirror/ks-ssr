/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd. 
 * ks-sc is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
 */

#pragma once

#include "application.h"

#include <QFont>
#include <QObject>
namespace KS
{
class ApplicationPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(Application);

public:
    ApplicationPrivate(Application* ptr);
    ~ApplicationPrivate();

public:
    void init();

private:
    void setupTranslations();

private:
    Application* q_ptr;
    bool m_adaptiveAppFontEnable = false;
};
}  // namespace KS
