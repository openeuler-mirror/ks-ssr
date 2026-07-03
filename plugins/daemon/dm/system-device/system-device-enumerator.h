#ifdef WITH_SYSTEMD_UDEV
#include "systemd-udev/sd-device-enumerator.h"
#else
#include "udev/udev-device-enumerator.h"
#endif

namespace KS
{
namespace DM
{
using SystemDeviceEnumerator
#ifdef WITH_SYSTEMD_UDEV
    = SDDeviceEnumerator;
#else
    = UdevDeviceEnumerator;
#endif

}  // namespace DM
}  // namespace KS