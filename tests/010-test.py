import t
@t.cx("false is False")
def test(cx):
    t.eq(cx.execute("false;"), False)
