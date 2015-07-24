SET LIBTMXSRC=libtmx-parser/src
SET TINYXMLSRC=libtmx-parser/libs/tinyxml2
cl tmxconvert.cpp %LIBTMXSRC%/base64.cpp %LIBTMXSRC%/tmxparser.cpp %TINYXMLSRC%/tinyxml2.cpp -I%LIBTMXSRC% -I%TINYXMLSRC% 