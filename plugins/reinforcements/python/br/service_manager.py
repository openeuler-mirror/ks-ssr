# -*- coding: utf-8 -*-
import json
import br.log
import br.utils
import time
import abc

SERVICE_MANAGER_LIST = ["systemctl", "service"]
SERVICE_AUTOSTART = "autostart"


class ServiceManager:
    __metaclass__ = abc.ABCMeta

    def __init__(self, service):
        self.service = service

    @abc.abstractmethod
    def exist(self):
        pass

    @abc.abstractmethod
    def is_active(self):
        pass

    @abc.abstractmethod
    def start(self):
        pass

    @abc.abstractmethod
    def stop(self):
        pass

    @abc.abstractmethod
    def mask(self):
        pass

    @abc.abstractmethod
    def unmask(self):
        pass

    @abc.abstractmethod
    def restart(self):
        pass

    @abc.abstractmethod
    def reload(self):
        pass

    @abc.abstractmethod
    def is_enable(self):
        pass

    @abc.abstractmethod
    def enable(self):
        pass

    @abc.abstractmethod
    def disable(self):
        pass


class Systemd(ServiceManager):

    def __call_noresult(self, action):
        command = "systemctl {0} {1}.service".format(action, self.service)
        br.utils.subprocess_not_output(command)

    def __call_result(self, action):
        command = "systemctl {0} {1}.service".format(action, self.service)
        return br.utils.subprocess_has_output(command)

    def exist(self):
        command = "systemctl list-unit-files | grep {0}\.service | wc -l".format(
            self.service
        )
        try:
            num = int(br.utils.subprocess_has_output(command))
        except:
            num = 0

        return num >= 1

    def is_active(self):
        actived = self.__call_result("is-active")

        # 防止其他加固项正在 reload sshd 导致误判 sshd 没有启动。
        for i in range(9):
            if actived != "reloading":
                break
            time.sleep(0.3)
            actived = self.__call_result("is-active")
        return actived == "active"

    def start(self):
        if self.is_active():
            return
        self.__call_noresult("start")

    def stop(self):
        if not self.is_active():
            return
        self.__call_noresult("stop")

    def mask(self):
        try:
            output = self.__call_result("mask")
            br.log.debug(output)
        except Exception as e:
            br.log.debug(e)
            return (False, str(e))

    def unmask(self):
        try:
            output = self.__call_result("unmask")
            br.log.debug(output)
        except Exception as e:
            br.log.debug(e)
            return (False, str(e))

    def restart(self):
        self.__call_noresult("restart")

    def reload(self):
        self.__call_noresult("reload")

    def is_enable(self):
        actived = self.__call_result("is-enabled")
        return actived == "enabled"

    def enable(self):
        if self.is_enable():
            return
        self.__call_noresult("enable")

    def disable(self):
        if not self.is_enable():
            return
        self.__call_noresult("disable")


class Initd(ServiceManager):

    def __call_noresult(self, action):
        command = "service {1} {0}".format(action, self.service)
        br.utils.subprocess_not_output(command)

    def __call_result(self, action):
        command = "service {1} {0}".format(action, self.service)
        return br.utils.subprocess_has_output(command)

    def exist(self):
        sysvInitCommand = "chkconfig --list {0} 2>&1 | grep -o 'error reading information on service'".format(
            self.service
        )
        sysvInitOutput = br.utils.subprocess_has_output(sysvInitCommand)
        return len(sysvInitOutput) == 0

    def is_active(self):
        isSuccess, stdout, stderr = br.utils.execute_command(
            "service {0} status".format(self.service)
        )
        is_running_flag = not (
            ("Service not running" in stdout) or ("is stopped" in stdout)
        )
        # service $SERVER_NAME status 返回值为 0 时才判断是否有非运行状态输出， 否则直接认为服务没有运行。
        # 有些服务的 status 接口不完整，当服务没有运行时返回值也是 0 ,此时根据输出判断服务是否运行。
        return isSuccess and is_running_flag

    def start(self):
        if self.is_active():
            return
        self.__call_noresult("start")

    def stop(self):
        if not self.is_active():
            return
        self.__call_noresult("stop")

    # initd 中实现 mask/umask 功能比较麻烦， 现有加固项暂时没有使用，暂不实现
    def mask(self):
        pass

    # initd 中实现 mask/umask 功能比较麻烦， 现有加固项暂时没有使用，暂不实现
    def unmask(self):
        pass

    def restart(self):
        self.__call_noresult("restart")

    def reload(self):
        self.__call_noresult("reload")

    def is_enable(self):
        return (
            len(
                br.utils.subprocess_has_output(
                    "chkconfig --list {0} 2>&1 | grep -P -o '[0-9]:on'".format(
                        self.service
                    )
                )
            )
            > 0
        )

    def enable(self):
        br.utils.subprocess_not_output("chkconfig --add {0}".format(self.service))
        br.utils.subprocess_not_output("chkconfig {0} on".format(self.service))

    def disable(self):
        br.utils.subprocess_not_output("chkconfig --del {0}".format(self.service))


class SwitchBase(object):
    def __init__(self, service, key="enabled"):
        self.systemd_proxy = ServiceManagerProxy(service)
        self.key = key

    def get(self):
        retdata = dict()
        retdata[self.key] = self.systemd_proxy.is_active()
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        # 也可以不用捕获异常，后台框架会对异常进行处理
        try:
            if args[self.key]:
                if self.systemd_proxy.exist():
                    self.systemd_proxy.enable()
                    self.systemd_proxy.start()
            else:
                if self.systemd_proxy.exist():
                    if self.systemd_proxy.stop():
                        # if not self.systemd_proxy.service_stop():
                        return (False, "Unable to stop service!")
                    # self.systemd_proxy.kill()
                    self.systemd_proxy.disable()
            return (True, "")
        except Exception as e:
            br.log.error(str(e))
            return (False, "Abnormal service!")

    def backup(self):
        return self.get()

    def rollback(self, args_json):
        return self.set(args_json)


class ServiceManagerProxy(ServiceManager):
    def __init__(self, service):
        self.serviceManager = ServiceManagerProxy.get_service_manager(service)

    @staticmethod
    def __get_service_control():
        for service_ctl in SERVICE_MANAGER_LIST:
            if (
                len(
                    br.utils.subprocess_has_output(
                        "which {0} 2>&1 | grep -o 'no {0}'".format(service_ctl)
                    )
                )
                == 0
            ):
                return service_ctl
        raise Exception("No service control command found!")

    @staticmethod
    def get_service_manager(service):
        service_control = ServiceManagerProxy.__get_service_control()
        if service_control == "systemctl":
            return Systemd(service)
        elif service_control == "service":
            return Initd(service)
        else:
            raise Exception("No support service manager: {0}".format(service_control))

    def exist(self):
        return self.serviceManager.exist()

    def is_active(self):
        return self.serviceManager.is_active()

    def start(self):
        return self.serviceManager.start()

    def stop(self):
        return self.serviceManager.stop()

    def mask(self):
        return self.serviceManager.mask()

    def unmask(self):
        return self.serviceManager.unmask()

    def restart(self):
        return self.serviceManager.restart()

    def reload(self):
        return self.serviceManager.reload()

    def is_enable(self):
        return self.serviceManager.is_enable()

    def enable(self):
        return self.serviceManager.enable()

    def disable(self):
        return self.serviceManager.disable()
