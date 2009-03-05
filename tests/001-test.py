import t
@t.rt("creating runtime")
def test(rt):
    t.ne(rt, None)
