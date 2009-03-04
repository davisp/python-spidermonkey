import t
@t.cx("to python unicode")
def test(cx):
    t.eq(cx.execute('"spam";'), "spam")
    t.eq(isinstance(cx.execute('"spam";'), unicode), True)

