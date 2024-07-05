# ks-ssr

这是一款提供可信保护、文件保护、网络安全和安全加固等功能的安全中心产品。

# 目录层级

├── cmake
├── data
│   ├── box
│   ├── dbus
│   ├── distribution-actuator
│   ├── runtime
│   └── services
├── include
├── lib
│   ├── base
│   ├── dbus
│   └── widgets
├── plugins
│   ├── daemon
│   ├── gui
│   └── reinforcements
├── resources
│   ├── icons
│   ├── images
│   └── styles
├── src
│   ├── daemon
│   ├── gui
│   ├── notify
│   └── tool
├── test
└── translations

## cmake

此文件中包含 ks-ssr 项目中自定义的 cmake 变量和函数。

## data

此目录包含 ks-ssr 的配置文件，以及相关非运行时工具。

## include

此目录包含 ks-ssr 一些全局的定义， 方便前后端和每个模块使用。

## lib

此目录包含一些通用的工具类， 一般将其编译成静态库来方便其他模块使用。

## plugins

此目录包含 ks-ssr 的实际功能实现的代码， 目前来说有以下几个模块。

### br

主机安全加固模块， 提供主机安全扫描和加固的功能， 也是最核心的功能。
扫描和加固也是由一个个的配置项来完成实际工作， 配置项位于 plugins/reinforcement 目录下， 目前主要是以 python 来编写加固项， 还支持 bash 和 cpp 作为加固项。

### dm

全名 device-manager， 是外设管控模块提供设备管理， 接口管理等功能。

### kss

可信模块， 提供可信相关功能。

### private-box

私密保险箱， 通过加密文件系统提供加密文件等功能。

### tool-box

安全工具箱， 依赖于 selinux 和 xfs 文件系统， 提供读取和修改文件安全元数据等功能。

### vulnerability

漏洞管理模块， 提供漏洞扫描和漏洞修复等功能， 需要手动配置 yum 源。

## resource

资源文件， 包括 logo 和 图标等。

## src

### daemon & gui

包含前后端插件基础框架代码。

### notify

为了实现弹窗， 而编写的一个会被会话拉起的进程， 前后端通过 dbus 与其通信， 由它来负责弹窗。

### tool

相关运行时工具。

#### command

命令行工具， 提供基线加固和漏洞管理等功能。

#### config

基线加固修改配置文件的工具， 通过正则来查找和修改文件。

#### crypto

加密二进制工具， 在编译时加密基线加固标准文件。

#### test

测试用例， 暂时未启用。

#### translations

翻译文件。
