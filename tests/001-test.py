import t
@t.rt("creating runtime")
def test(rt):
    t.neq(rt, None)
