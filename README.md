# ks-ssr

这是一款提供可信保护、文件保护、网络安全和安全加固等功能的安全中心产品。

# 缩略语
| 缩写 | 全称 | 描述 |
| --- | --- | --- |
| ssr | system security reinforcement | 系统安全加固 |
| br | base reinforcement | 基线加固 |
| ra | reinforcement argument | 加固参数 |
| rs | reinforcement standard | 加固标准 |
| dm | device manager | 设备管理 |
| kss | kylinsec security subsystem | 麒麟信安安全子系统 |
| fp | file protected | 文件保护 |
| tp | trusted protection | 可信保护 |
| spm | software package manager | 软件包管理 |


# 目录结构说明

├── cmake                        自定义的 cmake 变量和函数 \
├── data                         配置文件以及相关非运行时工具 \
│   ├── box                     私密保险箱的数据 \
│   ├── dbus                    dbus相关配置文件 \
│   ├── runtime                 提供将软件运行时依赖库文件进行打包的脚本 \
│   └── services                服务启动配置文件 \
├── include                      对外暴露的接口文件，主要用于IPC和RPC，进程内部的接口一般不放到此目录 \
├── lib                          静态库 \
│   ├── base                    提供了一些比较基础的函数和类 \
│   ├── dbus                    对dbus接口的封装 \
│   └── widgets                 自定义控件 \
├── plugins                      插件 \
│   ├── daemon                  后端服务的插件  \
│   │   ├── br                  基线加固模块 \
│   │   ├── dm                  设备管理模块 \
│   │   ├── kss                 可信和文件保护模块 \
│   │   ├── private-box         私密保险箱模块 \
│   │   ├── tool-box            安全工具箱模块 \
│   │   └── vulnerability       漏洞管理模块 \
│   ├── gui                      前端界面的插件 \
│   │   ├── br                  基线加固模块 \
│   │   ├── dm                  设备管理模块 \
│   │   ├── fp                  文件保护模块 \
│   │   ├── log                 日志审计模块 \
│   │   ├── private-box         私密保险箱模块 \
│   │   ├── tool-box            安全工具箱模块 \
│   │   ├── tp                  可信保护模块 \
│   │   └── vulnerability       漏洞管理模块 \
│   └── reinforcements           基线加固的插件 \
│       ├── bash                 bash加固项插件，暂未使用 \
│       ├── cpp                  c++语言加固项插件，暂未使用 \
│       └── python               python语言加固项插件 \
├── resources                     资源文件 \
├── src                           二进制程序的源码目录 \
│   ├── daemon                   后端服务程序 \
│   ├── gui                      前端界面程序 \
│   ├── notify                   通知程序 \
│   └── tool                     相关工具程序 \
│       ├── command              非图形工具，在非图形系统下作为gui程序的替代品 \
│       ├── config               文件操作工具，文件操作时会添加文件锁，避免多线程访问文件时发生错误，提供给python语言加固项插件使用 \
│       ├── crypto               文件加密工具，用于基线加固中加固标准配置文件的加密和解密 \
│       └── timeshift            备份还原工具，提供给漏洞修复模块使用 \
├── test                          测试代码 \
└── translations                  翻译文件