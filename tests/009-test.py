import t
@t.cx("true is True")
def test(cx):
    t.eq(cx.execute("true;"), True)
