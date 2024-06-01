#ifndef UDEV_DEVICE_ENUMERATOR_H
#define UDEV_DEVICE_ENUMERATOR_H

#include <QObject>

template <typename T>
class QList;
class udev;
class udev_enumerate;

namespace KS
{
namespace DM
{
class UdevDevice;

class UdevDeviceEnumerator : public QObject
{
    Q_OBJECT
public:
    UdevDeviceEnumerator();
    virtual ~UdevDeviceEnumerator();
    QList<UdevDevice*> getDevices() const;

private:
    udev_enumerate* m_deviceEnum;
    QList<UdevDevice*> m_devices;
};
}  // namespace DM
}  // namespace KS

#endif