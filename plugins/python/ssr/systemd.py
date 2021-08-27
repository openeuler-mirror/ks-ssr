import ssr.utils


class Proxy:
    def __init__(self, service):
        self.service = service

    def is_active(self):
        actived = self.__call_result('is-active')
        return actived == 'active'

    def start(self):
        if self.is_active():
            return
        self.__call_noresult('start')

    def stop(self):
        if not self.is_active():
            return
        self.__call_noresult('stop')

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

    def __call_noresult(self, action):
        command = 'systemctl {0} {1}.service'.format(action, self.service)
        ssr.utils.subprocess_not_output(command)

    def __call_result(self, action):
        command = 'systemctl {0} {1}.service'.format(action, self.service)
        return ssr.utils.subprocess_has_output(command)