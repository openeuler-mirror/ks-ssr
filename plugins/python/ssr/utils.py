import subprocess
from ssr import log


def subprocess_not_output(args):
    log.debug(args)
    child_process = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)

    exit_code = child_process.wait()
    if exit_code != 0:
        raise Exception(child_process.stderr.read())


def subprocess_has_output(args):
    log.debug(args)
    child_process = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)

    exit_code = child_process.wait()
    if exit_code != 0:
        raise Exception(child_process.stderr.read())

    return child_process.stdout.read().strip().decode('utf-8')