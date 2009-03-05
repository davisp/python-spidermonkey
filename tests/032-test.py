import t
@t.cx("function with dict arg")
def test(cx):
    func = cx.execute("function(doc) {if(doc.data) return doc.data;};")
    t.eq(func({"data": 2}), 2)
    t.eq(func({}), None)
