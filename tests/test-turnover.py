# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
import t

#def test_churn_runtimes():
#    for i in range(1000):
#        rt = t.spidermonkey.Runtime()


class Session(object):
    def __init__(self):
        self.key = None
        
    def set(self, key):
        self.key = key
        return self

@t.rt()
def test_churn_contexts(rt):
    for i in range(1000):
        cx = rt.new_context()
        cx.add_global('session', Session)
        data = cx.execute('new session().set("foo");')
        t.eq(data.key, "foo")

