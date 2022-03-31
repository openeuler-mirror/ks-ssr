# 系统安全加固run包制作
该目录为生成run包的脚本目录，包括ks-ssr-online-generate-run.sh和ks-ssr.sh

## 生成run包条件

使用ks-ssr-online-generate-run.sh需通内网

```
使用ks-ssr-online-generate-run.sh生成的run包仅对当前系统版本生效，如需适配多个版本请查看下一小节--生成run包
```
## 生成run包

生成run包方式有两种，一是使用ks-ssr-online-generate-run.sh在线生成，二是手动上传*.tar.gz文件，使用cat ks-ssr.sh *.tar.gz > *.run手动生成

```
### ./ks-ssr-online-generate-run.sh

会从源中拉取最新版本的rpm包，然后写入run包中去，执行此脚本会在当前目录下生成一个ks-ssr-1.0.run文件
```
ks-ssr.sh为写入run包的脚本，必须与ks-ssr-online-generate-run.sh在相同目录下
```
### cat ks-ssr.sh *.tar.gz > *.run

适配多个系统版本可手动整理各版本所需的rpm包，制作成*.tar.gz包
```
手动生成后需手动给run包执行权限chmod 0755 *.run
```

