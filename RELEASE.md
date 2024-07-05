# Release 1.3.12-1 (2024-07-05)
---

+ 漏洞修复前端
  + 不使用 queueConnection 方式来连接信号， 避免 reload dbus 时 Core 掉。

+ 漏洞修复后端
  + 降低漏洞修复时进度信号发送频率。