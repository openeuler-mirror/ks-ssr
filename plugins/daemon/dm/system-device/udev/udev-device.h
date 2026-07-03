#ifndef UDEV_DEVICE_DEVICE_H
#define UDEV_DEVICE_DEVICE_H

#include <QObject>

class udev_device;
namespace KS
{
namespace DM
{
class UdevDevice : public QObject
{
    Q_OBJECT
public:
    UdevDevice(udev_device *device);
    UdevDevice(const UdevDevice& device);
    UdevDevice(const QString& syspath);
    UdevDevice operator=(const UdevDevice& device);
    virtual ~UdevDevice();

public:
    QString getSyspath() const;
    QString getSubsystem() const;
    QString getDevtype() const;
    // udev 中没有 get_dev_name 接口， 并且此接口没有被使用，暂且忽略
    QString getDevname() const;
    QString getSysname() const;
    QString getSysattrValue(const QString &attr) const;
    QString getDevNode() const;
    void trigger();

private:
    udev_device *m_device;
};
}  // namespace DM
}  // namespace KS

#endif