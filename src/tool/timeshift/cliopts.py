# coding: utf-8
import argparse
import sys


class CommandLineArgs:
    def __init__(self):
        self.parser = argparse.ArgumentParser(description="KylinSec Security Reinforcement CLI",
                                              usage="%(prog)s [command] [options]",
                                              epilog="Use '%(prog)s --help' for more information.")

    def add_arguments(self):
        # 创建互斥参数组
        group = self.parser.add_mutually_exclusive_group()
        group.add_argument("--check", action='store_true',
                           help="Perform system check process")
        group.add_argument("--backup", action='store_true',
                           help="Perform system backup process")
        group.add_argument("--rollback", action='store_true',
                           help="Perform system rollback process")

    def parse_args(self, args):
        return self.parser.parse_args(args)

    def print_help(self):
        self.parser.print_help()


args_handler = CommandLineArgs()
args_handler.add_arguments()
if not sys.argv[1:]:
    args_handler.print_help()
    sys.exit(1)

tooloptions = args_handler.parse_args(sys.argv[1:])
