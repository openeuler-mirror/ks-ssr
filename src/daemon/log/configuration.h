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

#include <QHostAddress>
#include <QString>
#include "config.h"

class QSettings;
class QString;

namespace KS
{
namespace Log
{
struct Configurations : public QObject
{
public:
    /**
     * @brief 当日志文件达到 maxLogFile 大小时采取的动作。
     */
    enum class MaxLogFileAction
    {
        // 不采取任何动作
        IGNORE,
        // 向 /var/log/messages 中写入一条警告
        SYSLOG,
        // 不再向审计日志中写入
        SUSPEND,
        // 循环日志文件，最多保存 numLogs 个，小于 2 时不轮转日志
        ROTATE,
        // 轮转日志文件，但是老日志数量不会被 numLogs 所限制
        KEEP_LOGS
    };

    Configurations(const QString& = SSR_BR_INSTALL_DATADIR "/ssr.ini");
    virtual ~Configurations();

    void operator=(const Configurations& other);

    QSettings* m_config;
    const MaxLogFileAction m_maxLogFileAction = MaxLogFileAction::ROTATE;
    // 由于日志轮转标准为日志行数， 所以不允许修改日志行数
    const uint m_maxLogFileLine = m_maxLogFileLineDefaultValue; /* 单个日志文件最大容量，单位为 M, 范围为 3000 <= m_maxLogFileLine < 500000 */
    uint m_numLogs = m_numLogsDefaultValue;                     /* 日志数量,范围为 1 <= m_numLogs < 10 */
    QString m_account;
    QString m_passwd;
    QString m_remotePath;
    QHostAddress m_ip;

private:
    constexpr static const uint m_maxLogFileLineDefaultValue = 1000;
    constexpr static const uint m_numLogsDefaultValue = 5;
};
};  // namespace Log
};  // namespace KS