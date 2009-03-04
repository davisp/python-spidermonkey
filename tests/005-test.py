import t
@t.cx("to python float")
def test(cx):
    t.eq(cx.execute("42.5;"), 42.5)
