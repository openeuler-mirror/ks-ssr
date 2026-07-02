import ssr.utils

SSR_CONFIG_PATH = '/usr/bin/kiran-ssr-config'


class Plain:
    def __init__(self, config_path, kv_split_pattern='\\s+', kv_join_string='\t'):
        self.config_path = config_path
        self.kv_split_pattern = kv_split_pattern
        self.kv_join_string = kv_join_string

    def set_value(self, key, value):
        command = '{0} --type=PLAIN --set={1} --value={2} --kv-split-pattern="{3}" --kv-join-str="{4}" {5}'.format(
            SSR_CONFIG_PATH, key, value, self.kv_split_pattern, self.kv_join_string, self.config_path)
        ssr.utils.subprocess_not_output(command)

    def get_value(self, key):
        command = '{0} --type=PLAIN --get={1} --kv-split-pattern="{2}" {3}'.format(SSR_CONFIG_PATH, key, self.kv_split_pattern, self.config_path)
        return ssr.utils.subprocess_has_output(command)


class PAM:
    def __init__(self, config_path, line_match_pattern):
        self.config_path = config_path
        self.line_match_pattern =line_match_pattern

    def set_value(self, key, kv_split_pattern, value, kv_join_string):
        command = '{0} --type=PAM --set={1} --value={2} --kv-split-pattern="{3}" --kv-join-str="{4}" {5}'.format(
            SSR_CONFIG_PATH, key, value, kv_split_pattern, kv_join_string, self.config_path)
        ssr.utils.subprocess_not_output(command)

    def get_value(self, key, kv_split_pattern):
        command = '{0} --type=PAM --get={1} --kv-split-pattern="{2}" {3}'.format(
            SSR_CONFIG_PATH, key, kv_split_pattern, self.config_path)
        return ssr.utils.subprocess_has_output(command)

    def has_key(self, key):
        command = '{0} --type=PAM --get={1} {2}'.format(
            SSR_CONFIG_PATH, key, self.config_path)
        output = ssr.utils.subprocess_has_output(command)
        return (output == 'true')