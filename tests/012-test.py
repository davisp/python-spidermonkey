import t
@t.cx("Infinity is 1E500*1E500")
def test(cx):
    t.eq(cx.execute("Infinity;"), 1E500*1E500)
