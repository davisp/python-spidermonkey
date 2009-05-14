# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
import t
import traceback

ERROR = 'File "<JavaScript>", line 1, in ' \
        'SyntaxError: missing ) after formal parameters'

@t.cx()
def test_syntax_error(cx):
    try:
        cx.execute("function(asdf;")
        t.eq(1, 0)
    except:
        line = traceback.format_exc().split("\n")[-3].strip()
        t.eq(line, ERROR)

@t.cx()
def test_invalid_octal(cx):
    t.raises(t.JSError, cx.execute, "09;")

