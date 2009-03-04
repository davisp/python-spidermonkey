import t
@t.cx("nested object equality")
def test(cx):
    t.eq(
        cx.execute('["foo", 2, {"bar": 2.3, "spam": [1,2,3]}];'),
        [u"foo", 2, {u"bar": 2.3, u"spam": [1,2,3]}]
    )
