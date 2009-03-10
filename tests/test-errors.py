# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
import t

@t.cx()
def test_raise_js_error(cx):
    t.raises(Exception, cx.execute, 'throw("foo");')

@t.cx()
def test_raise_js_error_in_function(cx):
    func = cx.execute("function(doc) {throw(\"error\");};")
    t.raises(Exception, func)

@t.cx()
def test_propogate_from_py(cx):
    def do_raise():
        raise SystemExit()
    cx.add_global("do_raise", do_raise)
    t.raises(SystemExit, cx.execute, "do_raise();")
