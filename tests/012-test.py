import t
@t.cx("Infinity is what?")
def test(cx):
    t.eq(cx.execute("Infinity;"), 24)
