#--coding:utf8 --

try:
    import configparser
except:
    import ConfigParser as configparser

import os
import stat
import br.utils
import json
import br.vars

if os.path.exists('/etc/logrotate.d/rsyslog'):
    LOGFILE_CONF_FILEPATH = '/etc/logrotate.d/rsyslog'
else:
    LOGFILE_CONF_FILEPATH = '/etc/logrotate.d/syslog' 

LOGFILE_INI_FILEPATH = br.vars.SSR_BR_PLUGIN_PYTHON_ROOT_DIR + "/br/audit/logfile.ini"
LOGFILE_GROUP_PERMISSIONS = "Permissions"
# LPK: Logfile Permissions Key
LPK_MODE_FILE_LIST = "ModeFileList"
LPK_APPEND_FILE_LIST = "AppendFileList"

EXCLUDE_MODE = stat.S_IWGRP | stat.S_IXGRP | stat.S_IWOTH | stat.S_IXOTH | stat.S_IXUSR

PERMISSIONS_ARG_MODE_PERMISSIONS_LIMIT = "mode-permissions-limit"
PERMISSIONS_ARG_APPEND_PERMISSIONS_LIMIT = "append-permissions-limit"

LOGFILE_ROTETE_CONF = '### KSBRManager ###\n\
/var/log/messages\n\
{\n\
    missingok\n\
    sharedscripts\n\
    prerotate\n\
        sudo /usr/bin/chattr -a /var/log/messages\n\
    endscript\n\
    postrotate\n\
        sudo /usr/bin/systemctl kill -s HUP rsyslog.service >/dev/null 2>&1 || true\n\
        sudo /usr/bin/chattr +a /var/log/messages\n\
    endscript\n\
}\n\
### KSBRManager ###'


# 系统日志文件和配置的权限控制
class Permissions:
    def __init__(self):
        self.conf = configparser.ConfigParser()
        self.conf.read(LOGFILE_INI_FILEPATH)
        try:
            self.mode_filelist = self.conf.get(LOGFILE_GROUP_PERMISSIONS, LPK_MODE_FILE_LIST).split(';')
            self.append_filelist = self.conf.get(LOGFILE_GROUP_PERMISSIONS, LPK_APPEND_FILE_LIST).split(';')
        except Exception as e:
            self.mode_filelist = list()
            self.append_filelist = list()
            br.log.debug(str(e))

    def get(self):
        retdata = dict()

        mode_permissions_limit = True
        append_permissions_limit = True

        for mode_file in self.mode_filelist:
            if not os.access(mode_file, os.F_OK):
                continue
            mode = os.stat(mode_file).st_mode
            if (mode & EXCLUDE_MODE) != 0:
                mode_permissions_limit = False
                break
        retdata[PERMISSIONS_ARG_MODE_PERMISSIONS_LIMIT] = mode_permissions_limit

        br.log.debug(str(self.mode_filelist))
        br.log.debug(str(self.append_filelist))

        # for append_file in self.append_filelist:
        #     if not os.access(append_file, os.F_OK):
        #         continue
        #     file_attrs = br.utils.subprocess_has_output('lsattr -l {0}'.format(append_file))
        #     br.log.debug(file_attrs)
        #     if file_attrs.find("Append_Only") == -1:
        #         append_permissions_limit = False
        #         break
        # br.log.debug(append_permissions_limit)
        #retdata[PERMISSIONS_ARG_APPEND_PERMISSIONS_LIMIT] = append_permissions_limit

        output = br.utils.subprocess_has_output('grep -r "{0}" {1}'.format('/var/log/messages',LOGFILE_CONF_FILEPATH))
        output_ksbrmanager = br.utils.subprocess_has_output('grep -r "{0}" {1}'.format('### KSBRManager ###', LOGFILE_CONF_FILEPATH))
        if len(output) != 0 and len(output_ksbrmanager) != 0:
            retdata[PERMISSIONS_ARG_APPEND_PERMISSIONS_LIMIT] = True
        else:
            retdata[PERMISSIONS_ARG_APPEND_PERMISSIONS_LIMIT] = False

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if args[PERMISSIONS_ARG_MODE_PERMISSIONS_LIMIT]:
            for mode_file in self.mode_filelist:
                if not os.access(mode_file, os.F_OK):
                    continue
                mode = os.stat(mode_file).st_mode
                if mode != (mode & ~EXCLUDE_MODE):
                    br.utils.subprocess_not_output('sudo chattr -a {0}'.format(mode_file))
                    os.chmod(mode_file, mode & ~EXCLUDE_MODE)
                    if args[PERMISSIONS_ARG_APPEND_PERMISSIONS_LIMIT]:
                        br.utils.subprocess_not_output('sudo chattr +a {0}'.format(mode_file))

        # if args[PERMISSIONS_ARG_APPEND_PERMISSIONS_LIMIT]:
        #     for append_file in self.append_filelist:
        #         if not os.access(append_file, os.F_OK):
        #             continue
        #         br.utils.subprocess_not_output('chattr +a {0}'.format(append_file))

        if args[PERMISSIONS_ARG_APPEND_PERMISSIONS_LIMIT]:
            output = br.utils.subprocess_has_output('grep -r "{0}" {1}'.format('/var/log/messages',LOGFILE_CONF_FILEPATH))
            output_ksbrmanager = br.utils.subprocess_has_output('grep -r "{0}" {1}'.format('### KSBRManager ###', LOGFILE_CONF_FILEPATH))
            if len(output) == 0 and len(output_ksbrmanager) == 0:
                br.utils.subprocess_not_output('echo \'{0}\'    >> {1}'.format( LOGFILE_ROTETE_CONF, LOGFILE_CONF_FILEPATH))
            elif len(output_ksbrmanager) == 0 and len(output) != 0:
                br.utils.subprocess_not_output('sed -i \'s/{0}/ /g\' {1}'.format("\/var\/log\/messages", LOGFILE_CONF_FILEPATH))
                br.utils.subprocess_not_output('echo \'{0}\'    >> {1}'.format( LOGFILE_ROTETE_CONF, LOGFILE_CONF_FILEPATH))

            br.utils.subprocess_not_output('sudo chattr +a {0}'.format('/var/log/messages'))
        else:
            output = br.utils.subprocess_has_output('grep -rn "{0}" {1} | cut -f1 -d:'.format('### KSBRManager ###', LOGFILE_CONF_FILEPATH))
            if len(output) != 0:
                line = output.split()
                br.utils.subprocess_not_output('sed -i \'{0},{1}d\' {2}'.format(line[0], line[1], LOGFILE_CONF_FILEPATH))
                br.utils.subprocess_not_output('sed -i "1i{0}" {1}'.format('/var/log/messages', LOGFILE_CONF_FILEPATH))

            br.utils.subprocess_not_output('sudo chattr -a {0}'.format('/var/log/messages'))

        return (True, '')