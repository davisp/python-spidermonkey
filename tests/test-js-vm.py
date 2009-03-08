import t

@t.cx()
def test_js_for_in(cx):
    cx.add_global("foo", {"1": 3, "2": 2})
    cx.execute('for(var k in foo) {if(k != foo[k]) throw("foo");}')