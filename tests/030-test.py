import t
@t.cx("raise JS error")
def test(cx):
    t.raises(Exception, cx.execute, 'throw("foo");')
