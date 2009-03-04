import t
@t.cx("simple execution")
def test(cx):
    t.eq(cx.execute("2;"), 2)
