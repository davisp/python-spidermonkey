import t
@t.cx("test function return")
def test(cx):
    def bar():
        return u"\u0042"
    cx.add_global("bound", bar)
    t.eq(cx.execute("bound();"), u"\u0042")
