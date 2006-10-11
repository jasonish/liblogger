cdef extern from "Python.h":
    ctypedef struct PyObject

    void Py_INCREF(PyObject *)
    void Py_DECREF(PyObject *)

cdef extern from "../logger.h":
    cdef enum Levels:
        LOG_LEVEL_DEBUG
        LOG_LEVEL_INFO

    ctypedef struct log_handler_t

    void logger_init(int log_level)
    void logger_remove_handler(log_handler_t *handler)
    void log_info(char *fmt, ...)

    log_handler_t *logger_add_callback_handler(void (*cb)(char *msg,
                                                          void *arg),
                                               void *arg)
    log_handler_t *logger_add_rotating_handler(char *filename, int size,
                                               int count)

# Make the log levels accessible to Python callers.
DEBUG = LOG_LEVEL_DEBUG
INFO = LOG_LEVEL_INFO

cdef class LogHandler:
    cdef log_handler_t *handler

    def __dealloc__(self):
        if self.handler:
            logger_remove_handler(self.handler)

cdef class CallbackHandler(LogHandler):
    def __init__(self, callback):
        Py_INCREF(<PyObject *>callback)
        self.handler = logger_add_callback_handler(CallbackHandler_on_callback,
                                                   <void *>callback)

cdef void CallbackHandler_on_callback(char *msg, void *arg):
    (<object>arg)(msg)

class FileobjHandler:
    def __init__(self, fileobj):
        self.fileobj = fileobj
        self.handler = CallbackHandler(self.callback)

    def callback(self, msg):
        self.fileobj.write("%s\n" % msg)

    def add(self):
        self.handler.add()

cdef class RotatingHandler(LogHandler):

    def __init__(self, filename, size, count):
        self.handler = logger_add_rotating_handler(filename, size, count)

def add_callback_handler(callback):
    return CallbackHandler(callback)

def add_fileobj_handler(fileobj):
    return FileobjHandler(fileobj)

def add_rotating_handler(filename, size, count):
    return RotatingHandler(filename, size, count)

def info(msg):
    log_info(msg)

def init(level):
    logger_init(level)
