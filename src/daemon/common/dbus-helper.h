#include <qt5-log-i.h>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusContext>
#include <QDBusMessage>

class DBusHelper
{
public:
    inline static QString getCallerUniqueName(const QDBusContext* _this)
    {
        if (_this == nullptr || !_this->calledFromDBus())
        {
            return QString();
        }
        return _this->message().service();
    }

    inline static pid_t getCallerPid(const QDBusContext* _this)
    {
        if (_this == nullptr || !_this->calledFromDBus())
        {
            return -1;
        }
        auto dbusConn = _this->connection();
        auto dbusMsg = _this->message();

        return dbusConn.interface()->servicePid(dbusMsg.service());
    }
};