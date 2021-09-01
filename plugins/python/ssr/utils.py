import subprocess
import ssr.log


def subprocess_not_output(args):
    ssr.log.debug(args)
    child_process = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)

    exit_code = child_process.wait()
    error = child_process.stderr.read().strip().decode('utf-8')
    if exit_code != 0 and len(error) > 0:
        raise Exception(error)


def subprocess_has_output(args):
    ssr.log.debug(args)
    child_process = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)

    exit_code = child_process.wait()
    error = child_process.stderr.read().strip().decode('utf-8')
    if exit_code != 0 and len(error) > 0:
        raise Exception(error)

    return child_process.stdout.read().strip().decode('utf-8')