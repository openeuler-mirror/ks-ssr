/**
 * Copyright (c) 2022 ~ 2023 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinsec.com.cn>
 */

#pragma once

#include <QDBusMessage>
#include <QObject>
#include <QSharedPointer>
#include <QTimer>
#include <functional>

class QDBusPendingCallWatcher;

namespace KS
{
class PolkitProxy : public QObject
{
    Q_OBJECT
public:
    PolkitProxy();
    virtual ~PolkitProxy(){};

    static QSharedPointer<PolkitProxy> getDefault();

public:
    using checkAuthHandler = std::function<void(const QDBusMessage &)>;

    struct CheckAuthData
    {
        QTimer timer;
        QString cancelString;
        QDBusMessage message;
        checkAuthHandler handler;
    };

    void checkAuthorization(const QString &action,
                            bool userInteraction,
                            const QDBusMessage &message,
                            checkAuthHandler handler);

private:
    void onCancelCheckAuth(QSharedPointer<CheckAuthData> checkAuthData);
    void onFinishCheckAuth(QDBusPendingCallWatcher *watcher, QSharedPointer<CheckAuthData> checkAuthData);

private:
    static QSharedPointer<PolkitProxy> m_instance;
};

}  // namespace KS
