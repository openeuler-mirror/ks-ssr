# -*- coding: utf-8 -*-

import traceback

try:
    import klog
except Exception:

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
    try:
        try:
            klog.debug(
                " ".join(
                    map(lambda x: x.encode(encoding="utf-8", errors="ignore"), args)
                )
            )
        except:
            klog.debug(" ".join(map(lambda x: x, args)))
    except:
        klog.error(str(traceback.format_exc()))


def info(*args):
    try:
        try:
            klog.info(
                " ".join(
                    map(lambda x: x.encode(encoding="utf-8", errors="ignore"), args)
                )
            )
        except:
            klog.info(" ".join(map(lambda x: x, args)))
    except:
        klog.error(str(traceback.format_exc()))


def warning(*args):
    try:
        try:
            klog.warning(
                " ".join(
                    map(lambda x: x.encode(encoding="utf-8", errors="ignore"), args)
                )
            )
        except:
            klog.warning(" ".join(map(lambda x: x, args)))
    except:
        klog.error(str(traceback.format_exc()))


def error(*args):
    try:
        try:
            klog.error(
                " ".join(
                    map(lambda x: x.encode(encoding="utf-8", errors="ignore"), args)
                )
            )
        except:
            klog.error(" ".join(map(lambda x: x, args)))
    except:
        klog.error(str(traceback.format_exc()))


def fatal(*args):
    try:
        try:
            klog.fatal(
                " ".join(
                    map(lambda x: x.encode(encoding="utf-8", errors="ignore"), args)
                )
            )
        except:
            klog.fatal(" ".join(map(lambda x: x, args)))
    except:
        klog.error(str(traceback.format_exc()))
