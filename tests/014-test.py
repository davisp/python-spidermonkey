import t
@t.cx("mapping equality")
def test(cx):
    js = 'var d = {0: 0, "a": 1, 2: "b", "c": "d", "blah": 2.5}; d;'
    py = {0: 0, "a": 1, 2: "b", "c": "d", "blah": 2.5}
    t.eq(cx.execute(js), py)
