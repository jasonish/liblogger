liblogger - simple logging module
------------------------------------------------------------------------------

USAGE

    For usage please refer to demo.c.

LOGGER TYPES

    callback

        Passes the assembled log message to an application provided
        callback.

    file pointer

        Writes log messages to the provided file pointer.

    file

        Writes log messages to a named file.  The file can either be
        overwritten or appended to.

    rotating

        Write log messages to the specified file.  When the file
        reaches a specified size it will be rolled over and a new file
        is created.  The number of rolled over log files to keep is
        configurable to control disk usage.

TODO

    Make thread safe.

