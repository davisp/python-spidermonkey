import t
@t.cx("null is None")
def test(cx):
    t.eq(cx.execute("null;"), None)
