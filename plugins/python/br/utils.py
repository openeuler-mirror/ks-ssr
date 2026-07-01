# -*- coding: utf-8 -*-

import subprocess
import br.log
import time

def subprocess_not_output(args, ignore_exception=False):
    br.log.debug(args)
    child_process = subprocess.Popen(
        args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)

    exit_code = child_process.wait()
    # 暂时通过添加sleep方式解决cpu占用率过高的问题（#57871）
    # time.sleep(0.05)

    error = child_process.stderr.read().strip().decode('utf-8')
    if exit_code != 0 and len(error) > 0:
        if ignore_exception:
            br.log.debug(error)
        else:
            raise RuntimeError(error)


def subprocess_has_output(args):
    br.log.debug(args)
    child_process = subprocess.Popen(
        args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)

    exit_code = child_process.wait()

    error = child_process.stderr.read().strip().decode('utf-8')
    if exit_code != 0 and len(error) > 0:
        raise RuntimeError(error)
    # TODO:cmd执行get时命令执行结果为0,但输出在stderr上，暂时不清楚内部逻辑，后续要研究一下
    stdout = child_process.stdout.read().strip().decode('utf-8')
    if len(stdout) == 0:
        stdout = error

    return stdout

def subprocess_has_output_ignore_error_handling(args):
    br.log.debug(args)
    child_process = subprocess.Popen(
        args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)

    exit_code = child_process.wait()
    # 暂时通过添加sleep方式解决cpu占用率过高的问题（#57871）
    # time.sleep(0.05)

    error = child_process.stderr.read().strip().decode('utf-8')
    if exit_code != 0 and len(error) > 0:
        br.log.debug(error)
        return error

    return child_process.stdout.read().strip().decode('utf-8')