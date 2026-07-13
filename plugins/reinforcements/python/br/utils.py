# -*- coding: utf-8 -*-

import subprocess
import br.log
import time
import os


def execute_command(cmd):
    child_process = subprocess.Popen(
        cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True
    )
    exit_code = child_process.wait()
    stdout = child_process.stdout.read().strip().decode("utf-8")
    stderr = child_process.stderr.read().strip().decode("utf-8")
    if exit_code != 0:
        br.log.debug(
            "Failed to execute command: %s, exit code: %d, output: %s"
            % (cmd, exit_code, stdout + stderr)
        )
    return [not bool(exit_code), stdout, stderr]


def subprocess_not_output(args, ignore_exception=False):
    br.log.debug(args)
    child_process = subprocess.Popen(
        args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True
    )

    exit_code = child_process.wait()

    error = child_process.stderr.read().strip().decode("utf-8")
    if exit_code != 0 and len(error) > 0:
        if ignore_exception:
            br.log.warning(error)
        else:
            raise RuntimeError(error)


def subprocess_has_output(args):
    br.log.debug(args)
    child_process = subprocess.Popen(
        args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True
    )

    exit_code = child_process.wait()

    error = child_process.stderr.read().strip().decode("utf-8")
    if exit_code != 0 and len(error) > 0:
        raise RuntimeError(error)
    # TODO:cmd执行get时命令执行结果为0,但输出在stderr上，暂时不清楚内部逻辑，后续要研究一下
    stdout = child_process.stdout.read().strip().decode("utf-8")

    if len(stdout) == 0:
        stdout = error

    return stdout


def execute_compound_commands(cmd_list):
    # 组合命令
    compound_command = "(" + " ; ".join(cmd_list) + ")"
    return subprocess_has_output(compound_command)


def subprocess_has_output_ignore_error_handling(args):
    br.log.debug(args)
    child_process = subprocess.Popen(
        args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True
    )

    exit_code = child_process.wait()

    error = child_process.stderr.read().strip().decode("utf-8")
    if exit_code != 0 and len(error) > 0:
        br.log.warning(error)
        return error

    return child_process.stdout.read().strip().decode("utf-8")


def is_cent_os_6():
    return (
        os.path.exists("/etc/redhat-release")
        and len(
            br.utils.subprocess_has_output("cat /etc/redhat-release | grep -E '\s6\.'")
        )
        > 0
    )


def is_cent_os_7():
    return (
        os.path.exists("/etc/redhat-release")
        and len(
            br.utils.subprocess_has_output("cat /etc/redhat-release | grep -E '\s7\.'")
        )
        > 0
    )


def is_cent_os_8():
    return (
        os.path.exists("/etc/redhat-release")
        and len(
            br.utils.subprocess_has_output("cat /etc/redhat-release | grep -E '\s8\.'")
        )
        > 0
    )
