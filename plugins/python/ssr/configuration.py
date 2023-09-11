#--coding:utf8 --
import ssr.utils

SSR_CONFIG_PATH = '/usr/bin/ks-ssr-config'


class KV:
    def __init__(self, config_path, split_pattern='\\s+', join_string='\t', comment='#'):
        self.config_path = config_path
        self.split_pattern = split_pattern
        self.join_string = join_string
        self.comment = comment

    def set_value(self, key, value):
        command = '{0} --type=KV --method=SETVAL --key="{1}" --value="{2}" --split-pattern="{3}" --join-str="{4}" --comment="{5}" {6}'.format(
            SSR_CONFIG_PATH, key, value, self.split_pattern, self.join_string, self.comment, self.config_path)
        ssr.utils.subprocess_not_output(command)

    def set_all_value(self, key, value):
        command = '{0} --type=KV --method=SETVALALL --key="{1}" --value="{2}" --split-pattern="{3}" --join-str="{4}" --comment="{5}" {6}'.format(
            SSR_CONFIG_PATH, key, value, self.split_pattern, self.join_string, self.comment, self.config_path)
        ssr.utils.subprocess_not_output(command)

    def get_value(self, key):
        command = '{0} --type=KV --method=GETVAL --key="{1}" --split-pattern="{2}" --comment="{3}" {4}'.format(
            SSR_CONFIG_PATH, key, self.split_pattern, self.comment, self.config_path)
        return ssr.utils.subprocess_has_output(command)

    def del_record(self, key):
        command = '{0} --type=KV --method=DELVAL --key="{1}" --split-pattern="{2}" --comment="{3}" {4}'.format(
            SSR_CONFIG_PATH, key, self.split_pattern, self.comment, self.config_path)
        return ssr.utils.subprocess_not_output(command)


class Table:
    def __init__(self, config_path, split_pattern='\\s+', join_string='\t'):
        self.config_path = config_path
        self.split_pattern = split_pattern
        self.join_string = join_string

    def set_value(self, match_rule, value):
        command = '{0} --type=TABLE --method=SETVAL --key="{1}" --value="{2}" --split-pattern="{3}" --join-str="{4}" {5}'.format(
            SSR_CONFIG_PATH, match_rule, value, self.split_pattern, self.join_string, self.config_path)
        ssr.utils.subprocess_not_output(command)

    def get_value(self, match_rule):
        command = '{0} --type=TABLE --method=GETVAL --key="{1}" --split-pattern="{2}" {3}'.format(SSR_CONFIG_PATH, match_rule, self.split_pattern,
                                                                                                  self.config_path)
        return ssr.utils.subprocess_has_output(command)

    def del_record(self, match_rule):
        command = '{0} --type=TABLE --method=DELVAL --key="{1}" --split-pattern="{2}" {3}'.format(SSR_CONFIG_PATH, match_rule, self.split_pattern,
                                                                                                  self.config_path)
        return ssr.utils.subprocess_not_output(command)


class PAM:
    def __init__(self, config_path, line_match_pattern):
        self.config_path = config_path
        self.line_match_pattern = line_match_pattern

    def set_value(self, key, split_pattern, value, join_string):
        command = '{0} --type=PAM --method=SETVAL --line-match-pattern="{1}" --key="{2}" --value="{3}" --split-pattern="{4}" --join-str="{5}" {6}'.format(
            SSR_CONFIG_PATH, self.line_match_pattern, key, value, split_pattern, join_string, self.config_path)
        ssr.utils.subprocess_not_output(command)

    def get_value(self, key, split_pattern):
        command = '{0} --type=PAM --method=GETVAL --line-match-pattern="{1}" --key="{2}" --split-pattern="{3}" {4}'.format(
            SSR_CONFIG_PATH, self.line_match_pattern, key, split_pattern, self.config_path)
        return ssr.utils.subprocess_has_output(command)

    def del_record(self, key, split_pattern):
        command = '{0} --type=PAM --method=DELVAL --line-match-pattern="{1}" --key="{2}" --split-pattern="{3}" {4}'.format(
            SSR_CONFIG_PATH, self.line_match_pattern, key, split_pattern, self.config_path)
        return ssr.utils.subprocess_not_output(command)

    def has_key(self, key):
        command = '{0} --type=PAM --method=GETVAL --line-match-pattern="{1}" --key="{2}" {3}'.format(SSR_CONFIG_PATH, self.line_match_pattern, key,
                                                                                                     self.config_path)
        output = ssr.utils.subprocess_has_output(command)
        return (output == 'true')

    def set_line(self, newline, next_line_match_pattern):
        command = '{0} --type=PAM --method=SETLINE --line-match-pattern="{1}" --new-line="{2}" --next-line-match-pattern="{3}" {4}'.format(
            SSR_CONFIG_PATH, self.line_match_pattern, newline, next_line_match_pattern ,self.config_path)
        ssr.utils.subprocess_not_output(command)

    def del_line(self):
        command = '{0} --type=PAM --method=DELLINE --line-match-pattern="{1}" {2}'.format(SSR_CONFIG_PATH, self.line_match_pattern, self.config_path)
        ssr.utils.subprocess_not_output(command)

    def get_line(self):
        command = '{0} --type=PAM --method=GETLINE --line-match-pattern="{1}" {2}'.format(SSR_CONFIG_PATH, self.line_match_pattern, self.config_path)
        return ssr.utils.subprocess_has_output(command)

    # 为了避免设置值时匹配行不存在，这里先设置一个默认行，如果不存在则使用默认行
    # def set_value_with_line(self, key, split_pattern, value, join_string, newline):
    #     self.set_line(newline)
    #     self.set_value(key, split_pattern, value, join_string)