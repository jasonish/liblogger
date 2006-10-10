StaticLibrary("logger", "logger.c",
              CCFLAGS = ["-g", "-Wall", "-O2"]
              )

Program("demo", ["demo.c"],
	CCFLAGS = ["-g", "-Wall", "-O2"],
        LIBPATH = ".",
        LIBS = "logger"
        )

