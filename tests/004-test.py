import t
@t.cx("to python int")
def test(cx):
    t.eq(cx.execute("42;"), 42)
