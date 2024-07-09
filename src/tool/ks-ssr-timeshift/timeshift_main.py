# coding:utf-8
from cliopts import tooloptions
from utils import CONFIG_PATH
import rollback
import backup


def main():
    config = CONFIG_PATH
    is_backup = tooloptions.backup
    is_rollback = tooloptions.rollback
    if is_backup:
        backup.setup_logger()
        backup.main(config)
    elif is_rollback:
        rollback.setup_logger()
        rollback.main(config)
