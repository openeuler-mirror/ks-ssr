#ifdef WITH_SYSTEMD_UDEV
#include "systemd-udev/sd-device.h"
#else
#include "udev/udev-device.h"
#endif

namespace KS
{
namespace DM
{
using SystemDevice
#ifdef WITH_SYSTEMD_UDEV
    = SDDevice;
#else
    = UdevDevice;
#endif
}  // namespace DM
}  // namespace KS