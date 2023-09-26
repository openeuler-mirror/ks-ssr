/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiranwidgets-qt5 is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
 */
 
#ifndef QAPPLICATIONDEMO_KIRAN_APPLICATION_H
#define QAPPLICATIONDEMO_KIRAN_APPLICATION_H

#include <QApplication>

/* 重新定义qApp宏，转换QCoreApplication全局单例为Application, 只在Application时有效 */
#if defined(qApp)
#undef qApp
#endif
#define qApp (static_cast<Application*>(QCoreApplication::instance()))

namespace KS
{

class Application;
class ApplicationPrivate;
/**
 * @brief Kiran对于QApplication的一层封装
 * 使用说明
 * 　安装 **kiranwidgets-qt5-examples**,详情见/usr/share/kiranwidgets-qt5/examples/kiran-application/
 *  @since 2.1.0
 *  @see QApplication
 */
class Q_DECL_EXPORT Application : public QApplication {
    Q_OBJECT
    Q_DECLARE_PRIVATE(Application)
    Q_DISABLE_COPY(Application)
public:
    /**
     * @brief Application构造方法
     * @param argc     同QApplication::QApplication(int &, char **)中argc
     * @param argv     同QApplication::QApplication(int &, char **)中argv
     * @param appFlags 同QApplication::QApplication(int &, char **)中appFlags
     * @see QApplication::QApplication(int &, char **)
     */
    Application(int &argc, char **argv, int appFlags= ApplicationFlags);

    /**
     * @brief Application析构方法
     */
    ~Application();
private:
    /* ApplicationPrivate私有类指针 */
    ApplicationPrivate *d_ptr;
};
}  // namespace KS
#endif //KIRAN_APPLICATION_H
