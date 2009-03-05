import t
@t.cx("test basic unicode")
def test(cx):
    t.eq(cx.execute(u"\"\u0042\";"), u"\u0042")
