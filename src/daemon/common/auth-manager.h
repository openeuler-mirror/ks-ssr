#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

//#include <polkit-qt5-1/PolkitQt1/Authority>

namespace KS
{

class AuthManager
{
public:
    inline static AuthManager *instance(){
        static AuthManager instance;
        return &instance;
    }

//    bool checkAuthorization(const QString &actionID, qint16 applicationPID);
private:
    AuthManager(){};
    ~AuthManager(){};

//    Q_DISABLE_COPY(AuthManager)
};
}  // namespace KS
#endif // AUTHMANAGER_H
