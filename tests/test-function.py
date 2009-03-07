import t

@t.cx()
def test_call_js_func(cx):
    t.eq(cx.execute('function() {return "yipee";};')(), "yipee")

@t.cx()
def test_call_js_with_arg(cx):
    func = cx.execute("function(v) {return v * 2;};")
    t.eq(func(2), 4)

@t.cx()
def test_function_with_dict_arg(cx):
    func = cx.execute("function(doc) {if(doc.data) return doc.data;};")
    t.eq(func({"data": 2}), 2)
    t.eq(func({}), None)

@t.cx()
def test_function_returning_unicode(cx):
    def bar():
        return u"\u0042"
    cx.add_global("bound", bar)
    t.eq(cx.execute("bound();"), u"\u0042")

@t.cx()
def test_global_function(cx):
    def meander():
        return "Meandering enthusiastically!"
    cx.add_global("meander", meander)
    t.eq(cx.execute("meander();"), "Meandering enthusiastically!")
