import log


def debug(*args):
    log.debug(' '.join(map(str, args)))