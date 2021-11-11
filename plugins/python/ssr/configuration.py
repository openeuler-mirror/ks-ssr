import ssr.utils

SSR_CONFIG_PATH = '/usr/bin/kiran-ssr-config'


class KV:
    def __init__(self, config_path, split_pattern='\\s+', join_string='\t'):
        self.config_path = config_path
        self.split_pattern = split_pattern
        self.join_string = join_string

    def set_value(self, key, value):
        command = '{0} --type=KV --set="{1}" --value="{2}" --split-pattern="{3}" --join-str="{4}" {5}'.format(
            SSR_CONFIG_PATH, key, value, self.split_pattern, self.join_string, self.config_path)
        ssr.utils.subprocess_not_output(command)

    def get_value(self, key):
        command = '{0} --type=KV --get="{1}" --split-pattern="{2}" {3}'.format(SSR_CONFIG_PATH, key, self.split_pattern, self.config_path)
        return ssr.utils.subprocess_has_output(command)


class Table:
    def __init__(self, config_path, split_pattern='\\s+', join_string='\t'):
        self.config_path = config_path
        self.split_pattern = split_pattern
        self.join_string = join_string

    def set_value(self, match_rule, value):
        command = '{0} --type=TABLE --set="{1}" --value="{2}" --split-pattern="{3}" --join-str="{4}" {5}'.format(
            SSR_CONFIG_PATH, match_rule, value, self.split_pattern, self.join_string, self.config_path)
        ssr.utils.subprocess_not_output(command)

    def get_value(self, match_rule):
        command = '{0} --type=TABLE --get="{1}" --split-pattern="{2}" {3}'.format(SSR_CONFIG_PATH, match_rule, self.split_pattern, self.config_path)
        return ssr.utils.subprocess_has_output(command)

    def del_record(self, match_rule):
        command = '{0} --type=TABLE --del="{1}" --split-pattern="{2}" {3}'.format(SSR_CONFIG_PATH, match_rule, self.split_pattern, self.config_path)
        return ssr.utils.subprocess_not_output(command)


class PAM:
    def __init__(self, config_path, line_match_pattern):
        self.config_path = config_path
        self.line_match_pattern = line_match_pattern

    def set_value(self, key, split_pattern, value, join_string):
        command = '{0} --type=PAM --set="{1}" --value="{2}" --split-pattern="{3}" --join-str="{4}" {5}'.format(
            SSR_CONFIG_PATH, key, value, split_pattern, join_string, self.config_path)
        ssr.utils.subprocess_not_output(command)

    def get_value(self, key, split_pattern):
        command = '{0} --type=PAM --get="{1}" --split-pattern="{2}" {3}'.format(SSR_CONFIG_PATH, key, split_pattern, self.config_path)
        return ssr.utils.subprocess_has_output(command)

    def has_key(self, key):
        command = '{0} --type=PAM --get="{1}" {2}'.format(SSR_CONFIG_PATH, key, self.config_path)
        output = ssr.utils.subprocess_has_output(command)
        return (output == 'true')