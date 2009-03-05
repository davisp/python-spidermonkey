import t
@t.cx("call js function")
def test(cx):
    func = cx.execute("function(v) {return v * 2;};")
    t.eq(func(2), 4)
