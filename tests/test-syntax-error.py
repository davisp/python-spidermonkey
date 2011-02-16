# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
import t
import traceback

@t.cx()
def test_syntax_error(cx):
    t.raises(t.JSError, cx.execute, "function(asdf;")

@t.cx()
def test_invalid_octal(cx):
    t.raises(t.JSError, cx.execute, "09;")

