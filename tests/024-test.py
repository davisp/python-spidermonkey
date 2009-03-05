import t
@t.cx("test reentry")
def test(cx):
    cx.execute("var x = 42;")
    t.eq(cx.execute("x;"), 42)
