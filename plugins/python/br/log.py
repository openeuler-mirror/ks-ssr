try:
    import klog
except:

    class klog:
        @staticmethod
        def debug(msg):
            print("[DEBUG] " + str(msg))

        @staticmethod
        def info(msg):
            print("[INFO] " + str(msg))

        @staticmethod
        def warning(msg):
            print("[WARN] " + str(msg))

        @staticmethod
        def error(msg):
            print("[ERROR] " + str(msg))

        @staticmethod
        def fatal(msg):
            print("[FATAL] " + str(msg))


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
