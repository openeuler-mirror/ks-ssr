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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#pragma once

#include <kylin-license/license-i.h>
#include <QObject>
#include <QString>

#define LICENSE_OBJECT_NAME "KSSC"
#define TIMEOUT_MS 5000
#define METHOD_GET_LICENSE "GetLicense"
#define METHOD_GET_LICENSE_OBJECT "GetLicenseObject"
#define METHOD_ACTIVATE_BY_ACTIVATION_CODE "ActivateByActivationCode"
#define SIGNAL_LICENSE_CHANGED "LicenseChanged"

namespace KS
{
class LicenseDBus : public QObject
{
    Q_OBJECT
public:
    LicenseDBus(QObject* parent = 0);
    static QSharedPointer<LicenseDBus> getDefault();
    virtual ~LicenseDBus();

public:
    /**
     * @brief getObjectPath: 获取激活对象DBus路径
     * @param objectName:激活对象名称
     * @return 激活对象DBus路径
     */
    QString getObjectPath(const QString& objectName);

    /**
     * @brief updateLicense:更新授权信息
     */
    void updateLicense();

    /**
     * @brief activateByActivationCode:通过激活码授权
     * @param activation_Code:激活码
     * @param errorMsg:错误信息
     * @return true: 授权成功
     *         false:授权失败
     */
    bool activateByActivationCode(const QString& activation_Code, QString& errorMsg);

    /**
     * @brief isActivate:判断是否授权
     * @return true： 已授权
     *         false：未授权/授权已过期
     */
    bool isActivated();

    /**
     * @brief getActivationCode:获取激活码
     * @return 激活码
     */
    QString getActivationCode();

    /**
     * @brief getMachineCode 获取机器码
     * @return 机器码
     */
    QString getMachineCode();

    /**
     * @brief getExpiredTime 获取质保期时间戳
     * @return 质保期时间戳
     */
    time_t getExpiredTime();

signals:
    void licenseChanged();

private slots:
    void licenseChange(bool);

private:
    bool m_isActivated;
    QString m_objectPath;
    QString m_machineCode;
    QString m_activationCode;
    time_t m_expiredTime;
};
}  // namespace KS
