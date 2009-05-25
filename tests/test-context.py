# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
import t
import time

@t.rt()
def test_no_provided_runtime(rt):
    t.raises(TypeError, t.spidermonkey.Context)

@t.rt()
def test_invalid_runtime(rt):
    t.raises(TypeError, t.spidermonkey.Context, 0)

@t.rt()
def test_creating_new_context(rt):
    t.eq(isinstance(rt.new_context(), t.spidermonkey.Context), True)

@t.cx()
def test_basic_execution(cx):
    t.eq(cx.execute("var x = 4; x * x;"), 16)
    t.lt(cx.execute("22/7;") - 3.14285714286, 0.00000001)
    
@t.cx()
def test_reentry(cx):
    cx.execute("var x = 42;")
    t.eq(cx.execute("x;"), 42)

@t.cx()
def test_null(cx):
    cx.execute("x = null;")
    t.eq(cx.execute("x;"), None)

@t.cx()
def test_get_set_limits(cx):
    t.eq(cx.max_time(), 0)
    t.eq(cx.max_memory(), 0)
    t.eq(cx.max_time(10), 0) # Accessors return previous value.
    t.eq(cx.max_time(), 10)
    t.eq(cx.max_memory(10), 0)
    t.eq(cx.max_memory(), 10)

@t.cx()
def test_exceed_time(cx):
    script = """
        var time = function() {return (new Date()).getTime();};
        var start = time();
        while((time() - start) < 2000) {}
    """
    cx.max_time(1)
    t.raises(SystemError, cx.execute, script)

@t.cx()
def test_does_not_exceed_time(cx):
    cx.max_time(1)
    func = cx.execute("function() {return 1;}")
    time.sleep(1.1)
    cx.execute("var f = 2;");
    time.sleep(1.1)
    func()
    time.sleep(1.1)
    cx.execute("f;");

@t.cx()
def test_exceed_memory(cx):
    cx.max_memory(10000)
    script = "var f = []; var b = 1000000; while(b-- > 0) f[f.length] = b*0.9;"
    t.raises(MemoryError, cx.execute, script)

@t.cx()
def test_small_limit(cx):
    cx.max_memory(1)
    t.raises(MemoryError, cx.execute, "var f = []; while(true) f.push(2.3);");

@t.cx()
def test_does_not_exceed_memory(cx):
    cx.max_memory(10000)
    script = "var f = 2; f;"
    cx.execute(script)

