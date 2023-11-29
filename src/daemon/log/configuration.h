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
    bool isValid();

    QSettings* m_config;
    const MaxLogFileAction m_maxLogFileAction = MaxLogFileAction::ROTATE;
    uint m_maxLogFile = 8; /* 单个日志文件最大容量，单位为 M, 范围为 5 <= m_maxLogFile < 50 */
    uint m_numLogs = 5;    /* 日志数量,范围为 1 <= m_numLogs < 10 */
    QString m_account;
    QString m_passwd;
    QHostAddress m_ip;
#pragma message("TODO: SCP 还需要对应服务端的路径")
};
};  // namespace Log
};  // namespace KS