import t
@t.cx("basic array equality")
def test(cx):
    t.eq(cx.execute("[1,2,3];"), [1, 2, 3])
