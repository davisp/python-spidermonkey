# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
import t

@t.cx()
def test_empty_string_script(cx):
    cx.execute("")

@t.cx()
def test_unicode_string(cx):
    t.eq(cx.execute(u"5"), 5)

@t.cx()
def test_non_unicode_string(cx):
    t.eq(cx.execute("5"), 5)
