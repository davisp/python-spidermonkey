import t
@t.cx("test NaN is a float and not equal itself.")
def test(cx):
    nan = cx.execute("NaN;")
    t.eq(type(nan), float)
    t.ne(nan, nan)
