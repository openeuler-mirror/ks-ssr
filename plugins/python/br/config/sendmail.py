#--coding:utf8 --

import json
import br.systemd


class SendMailProxy:
    def __init__(self):
        self.sendmail_service = 'sendmail'
        self.postfix_service =  'postfix'
        self.key = 'enabled'
        self.sendmail = False
        self.postfix = False

    # 判断服务是否存在
    def exist(self, service):
        command = 'systemctl list-unit-files {0}.service | grep {1} | wc -l'.format(service, service)
        num = br.utils.subprocess_has_output(command)
        return (num == '1')

    def is_active(self, service):
        actived = self.__call_result('is-active', service)
        return actived == 'active'

    def start(self, service):
        if self.is_active(service):
            return
        self.__call_noresult('start', service)

    def stop(self, service):
        if not self.is_active(service):
            return
        self.__call_noresult('stop', service)

    def restart(self, service):
        self.__call_noresult('restart', service)

    def is_enable(self, service):
        actived = self.__call_result('is-enabled', service)
        return actived == 'enabled'

    def enable(self, service):
        if self.is_enable(service):
            return
        self.__call_noresult('enable', service)

    def disable(self, service):
        if not self.is_enable(service):
            return
        self.__call_noresult('disable', service)

    def __call_noresult(self, action, service):
        command = 'systemctl {0} {1}.service'.format(action, service)
        br.utils.subprocess_not_output(command)

    def __call_result(self, action, service):
        command = 'systemctl {0} {1}.service'.format(action, service)
        return br.utils.subprocess_has_output(command)

class Switch(SendMailProxy):
    def get(self):
        retdata = dict()
        if self.exist(self.sendmail_service):
            self.sendmail = True

        if self.exist(self.postfix_service):
            self.postfix = True

        if self.sendmail and self.postfix:
            retdata[self.key] = self.is_active(self.sendmail_service) or self.is_active(self.postfix_service)

        elif self.sendmail:
            retdata[self.key] = self.is_active(self.sendmail_service)

        elif self.postfix:
            retdata[self.key] = self.is_active(self.postfix_service)

        else:
            retdata[self.key] = False   

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        try:
            if args[self.key]:
                if self.exist(self.sendmail_service):
                    self.enable(self.sendmail_service)
                    self.start(self.sendmail_service)

                if self.exist(self.postfix_service):
                    self.enable(self.postfix_service)
                    self.start(self.postfix_service)
            else:
                if self.exist(self.sendmail_service):
                    self.disable(self.sendmail_service)
                    self.stop(self.sendmail_service)

                if self.exist(self.postfix_service):
                    self.disable(self.postfix_service)
                    self.stop(self.postfix_service)
            return (True, '')
        except Exception as e:
            return (False, str(e))