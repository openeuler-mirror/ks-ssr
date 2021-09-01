import klog


def debug(*args):
    klog.debug(' '.join(map(str, args)))


def info(*args):
    klog.info(' '.join(map(str, args)))


def warning(*args):
    klog.warning(' '.join(map(str, args)))


def error(*args):
    klog.error(' '.join(map(str, args)))


def fatal(*args):
    klog.fatal(' '.join(map(str, args)))
