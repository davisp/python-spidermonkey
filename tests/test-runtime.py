import t

@t.rt()
def test_creating_runtime(rt):
    t.ne(rt, None)
