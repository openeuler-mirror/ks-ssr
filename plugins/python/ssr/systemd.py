# -*- coding: utf-8 -*-
import json
import ssr.utils
from ssr.translation import _


class Proxy:
    def __init__(self, service):
        self.service = service

    # 判断服务是否存在
    def exist(self):
        command = 'systemctl list-unit-files {0}.service | grep {1} | wc -l'.format(
            self.service, self.service)
        num = ssr.utils.subprocess_has_output(command)
        return (num == '1')

    def is_active(self):
        actived = self.__call_result('is-active')
        return actived == 'active'

    def start(self):
        if self.is_active():
            return
        self.__call_noresult('start')

    def stop(self):
        try:
            output = self.__call_result('stop')
            ssr.log.debug(output)
        except Exception as e:
            ssr.log.debug(e)
            return (False, str(e))

    def kill(self):
        if not self.is_active():
            return
        self.__call_noresult('kill')

    def mask(self):
        try:
            output = self.__call_result('mask')
            ssr.log.debug(output)
        except Exception as e:
            ssr.log.debug(e)
            return (False, str(e))

    def unmask(self):
        try:
            output = self.__call_result('unmask')
            ssr.log.debug(output)
        except Exception as e:
            ssr.log.debug(e)
            return (False, str(e))

    def service_save(self):
        try:
            output = self.__call_service_result('save')
            ssr.log.debug(output)
        except Exception as e:
            ssr.log.debug(e)
            return (False, str(e))

    def service_stop(self):
        output = self.__call_service_result('stop')
        if str(output).find('FAILED') < 0:
            return True
        else:
            return False

    def service_restart(self):
        self.__call_service_noresult('restart')

    def service_reload(self):
        self.__call_service_noresult('reload')

    def restart(self):
        self.__call_noresult('restart')

    def reload(self):
        self.__call_noresult('reload')

    def is_enable(self):
        actived = self.__call_result('is-enabled')
        return actived == 'enabled'

    def enable(self):
        if self.is_enable():
            return
        self.__call_noresult('enable')

    def disable(self):
        if not self.is_enable():
            return
        self.__call_noresult('disable')

    def __call_service_noresult(self, action):
        command = 'service {0} {1}'.format(self.service, action)
        ssr.utils.subprocess_not_output(command)

    def __call_service_result(self, action):
        command = 'service {0} {1}'.format(self.service, action)
        return ssr.utils.subprocess_has_output(command)

    def __call_noresult(self, action):
        command = 'systemctl {0} {1}.service'.format(action, self.service)
        ssr.utils.subprocess_not_output(command)

    def __call_result(self, action):
        command = 'systemctl {0} {1}.service'.format(action, self.service)
        return ssr.utils.subprocess_has_output(command)


class SwitchBase(object):
    def __init__(self, service, key='enabled'):
        self.systemd_proxy = Proxy(service)
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
                        return (False, _('Unable to stop service!\t'))
                    # self.systemd_proxy.kill()
                    self.systemd_proxy.disable()
            return (True, '')
        except Exception as e:
            ssr.log.error(str(e))
            return (False, _("Abnormal service!\t"))
