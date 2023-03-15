/**
 * @file          /ks-sc/src/daemon/daemon-manager.h
 * @brief         
 * @author        chendingjian <chendingjian@kylinos.com>
 * @copyright (c) 2023 KylinSec. All rights reserved.
 */
#ifndef DAEMONMANAGER_H
#define DAEMONMANAGER_H

#include <QObject>
#include "src/daemon/box/box-manager.h"

namespace KS
{
class DaemonManager : public QObject
{
    Q_OBJECT
public:
    DaemonManager();
    virtual ~DaemonManager();

    static DaemonManager *getInstance()
    {
        return m_instance;
    };

    static void globalInit()
    {
        m_instance = new DaemonManager();
        m_instance->init();
    };

    static void globalDeinit() { delete m_instance; };

    BoxManager *boxManger;

private:
    void init();

private:
    static DaemonManager *m_instance;
};
}  // namespace KS

#endif  // DAEMONMANAGER_H
