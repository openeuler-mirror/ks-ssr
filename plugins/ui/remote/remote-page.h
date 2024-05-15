/**
 * Copyright (c) 2020 ~ 2024 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#pragma once

#include <page.h>
#include "actuator-wrapper.h"

namespace Ui
{
class RemotePage;
}

namespace KS
{
namespace Remote
{
class RemotePage : public Page
{
    Q_OBJECT
public:
    RemotePage(QWidget* parent = nullptr);
    virtual ~RemotePage();

    NavigationIndex getNavigationIndex();
    QString getSidebarUID() override;
    QString getSidebarIcon() override;
    QString getAccountRoleName() override;

private slots:
    void onSelectFile();
    void onCancel();
    void onExecute();
    void onReturnToConfig();

    void onActuatorStarted(int totalCount);
    void onActuatorEntryCompleted(int currentCount);
    void onActuatorFinished(bool isSuccessed, const QString& msg, const QString& logPath);
    void onActuatorEchoLog(const QString& log);

private:
    Ui::RemotePage* m_ui;
    ActuatorWrapper* m_actuator;
};
}  // namespace Remote
}  // namespace KS