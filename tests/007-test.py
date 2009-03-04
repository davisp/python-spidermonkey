import t
@t.cx("undefined is None")
def test(cx):
    t.eq(cx.execute("undefined;"), None)
