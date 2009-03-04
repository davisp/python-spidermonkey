import t
@t.cx("test NaN is what?")
def test(cx):
    t.eq(cx.execute("NaN;"), 13)
