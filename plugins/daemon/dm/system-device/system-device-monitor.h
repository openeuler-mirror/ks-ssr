#include "include/ssr-i.h"
#ifdef WITH_SYSTEMD_UDEV
#include "systemd-udev/sd-device-monitor.h"
#else
#include "udev/udev-device-monitor.h"
#endif

namespace KS
{
namespace DM
{
using SystemDeviceMonitor
#ifdef WITH_SYSTEMD_UDEV
    = SDDeviceMonitor;
#else
    = UdevDeviceMonitor;
#endif
}  // namespace DM
}  // namespace KS