import t
@t.cx("js throws")
def test(cx):
    func = cx.execute("function(doc) {throw(\"error\");};")
    t.raises(Exception, func)
