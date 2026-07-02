#ifndef TPUTILS_H
#define TPUTILS_H

#include <QObject>
namespace KS
{
enum TrustedFileType
{
    // 未知文件类型
    TRUSTED_FILE_TYPE_NONE = 0,
    // 可执行文件
    TRUSTED_FILE_TYPE_EXECUTABLE_FILE,
    // 动态库
    TRUSTED_FILE_TYPE_DYNAMIC_LIBRARY,
    // 内核模块
    TRUSTED_FILE_TYPE_KERNEL_MODULE,
    // 可执行脚本
    TRUSTED_FILE_TYPE_EXECUTABLE_SCRIPT
};

enum TrustedFileStatus
{
    // 异常 (未认证/被篡改)
    TRUSTED_FILE_STATUS_ILLEGAL = 0,
    // 正常（已认证）
    TRUSTED_FILE_STATUS_NORMAL,
};

struct TrustedRecord
{
    // 是否被选中
    bool selected;
    // 文件路径
    QString filePath;
    // 文件类型
    QString type;
    // 状态
    QString status;
    // 是否开启防卸载
    // bool
};

class TPUtils : public QObject
{
    Q_OBJECT
public:
    TPUtils(QObject *parent = nullptr);
    virtual ~TPUtils();

    static QString fileTypeEnum2Str(int fileType);
    static QString fileStatusEnum2Str(int fileStatus);
signals:

};

}  // namespace KS

#endif // TPUTILS_H
