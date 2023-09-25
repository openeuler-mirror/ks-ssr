#ifndef KSS_H
#define KSS_H

#include <QObject>
#include <QProcess>

namespace KS
{
enum TRUSTED_FILE_TYPE
{
    // 未知文件类型
    UNKNOWN_TYPE = 0,
    // 可执行文件
    EXECUTABLE_FILE,
    // 动态库
    DYNAMIC_LIBRARY,
    // 内核模块
    KERNEL_MODULE,
    // 可执行脚本
    EXECUTABLE_SCRIPT
};

class Kss : public QObject
{
    Q_OBJECT
public:
    Kss(QObject *parent = nullptr);
    virtual ~Kss(){};

public:
    // 可信保护
    // 添加文件
    void addTrustedFile(const QString &filePath);
    // 移除文件
    void removeTrustedFile(const QString &filePath);
    // 获取内核白名单
    QString getModuleFiles();
    // 防卸载功能开关
    void prohibitUnloading(bool prohibited, const QString &filePath);
    // 获取程序白名单
    QString getExecuteFiles();

    // 文件保护
    // 添加文件
    void addFile(const QString &fileName, const QString &filePath, const QString &insertTime);
    // 移除文件
    void removeFile(const QString &filePath);
    // 获取文件保护列表
    QString getFiles();

    // 搜索 传入要搜索的列表，此处fileList为一个json字符串，可通过get*Files()获取
    QString search(const QString &pathKey, const QString &fileList);
public Q_SLOTS:
    void onProcessExit(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void execute(const QString &cmd);

private:
    QProcess *m_process;
    QString m_processOutput;
    QString m_errorOutput;
};
}
#endif // KSS_H
