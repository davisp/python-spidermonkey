import t
@t.cx("mapping equality")
def test(cx):
    t.eq(cx.execute('var f={"foo": "bar"}; f;'), {"foo": "bar"})
