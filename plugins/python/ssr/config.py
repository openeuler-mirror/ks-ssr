import ssr.utils

CONFIG_PATH = '/usr/bin/kiran-ssr-config'


class Plain:
    def __init__(self, config_path, split_regex='\\s+', join_string='\t'):
        self.config_path = config_path
        self.split_regex = split_regex
        self.join_string = join_string

    def set_value(self, key, value):
        command = '{0} --type=PLAIN --set={1} --value={2} --split-regex="{3}" --join-string "{4}" {5}'.format(
            CONFIG_PATH, key, value, self.split_regex, self.join_string, self.config_path)
        ssr.utils.subprocess_not_output(command)

    def get_value(self, key):
        command = '{0} --type=PLAIN --get={1} --split-regex="{2}" {3}'.format(CONFIG_PATH, key, self.split_regex, self.config_path)
        return ssr.utils.subprocess_has_output(command)