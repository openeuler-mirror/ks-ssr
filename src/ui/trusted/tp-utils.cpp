#include "tp-utils.h"
namespace KS
{
TPUtils::TPUtils(QObject *parent) : QObject(parent)
{

}

TPUtils::~TPUtils()
{

}

QString TPUtils::fileTypeEnum2Str(int fileType)
{
    QString type;
    switch (fileType)
    {
    case TrustedFileType::TRUSTED_FILE_TYPE_NONE:
        type = QString(tr("Unknown file"));
        break;
    case TrustedFileType::TRUSTED_FILE_TYPE_EXECUTABLE_FILE:
        type = QString(tr("Executable file"));
        break;
    case TrustedFileType::TRUSTED_FILE_TYPE_DYNAMIC_LIBRARY:
        type = QString(tr("Dynamic library"));
        break;
    case TrustedFileType::TRUSTED_FILE_TYPE_KERNEL_MODULE:
        type = QString(tr("Kernel file"));
        break;
    case TrustedFileType::TRUSTED_FILE_TYPE_EXECUTABLE_SCRIPT:
        type = QString(tr("Executable script"));
        break;
    default:
        break;
    }
    return type;
}

QString TPUtils::fileStatusEnum2Str(int fileStatus)
{
    QString status;
    if (fileStatus == TrustedFileStatus::TRUSTED_FILE_STATUS_NORMAL)
    {
        status = QString(tr("Certified"));
    }
    else
    {
        status = QString(tr("Being tampered with"));
    }
    return status;
}
}  // namespace KS
