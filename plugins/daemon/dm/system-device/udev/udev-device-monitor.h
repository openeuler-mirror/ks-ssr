#ifndef UDEV_DEVICE_MONITOR_H
#define UDEV_DEVICE_MONITOR_H

#include <QMap>
#include <QObject>
#include <QSharedPointer>
#include "udev-device.h"

class udev_monitor;
class QSocketNotifier;

namespace KS
{
namespace DM
{
class UdevDeviceEnumerator;

class UdevDeviceMonitor : public QObject
{
    Q_OBJECT
public:
    UdevDeviceMonitor();
    virtual ~UdevDeviceMonitor();
    bool isDeviceExisted(const QString& syspath) const;
signals:
    void deviceChanged(UdevDevice* device, int action);

private:
    void initDevices();

private slots:
    void onUdevDeviceChanged(int);

private:
    udev_monitor* m_udevDeviceMonitor;
    QSocketNotifier* m_deviceNotify;
    QMap<QString, QSharedPointer<UdevDevice>> m_devices;
};
}  // namespace DM
}  // namespace KS

#endif