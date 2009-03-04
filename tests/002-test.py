import t
@t.rt("creating a new context")
def test(rt):
    t.eq(isinstance(rt.new_context(), t.spidermonkey.Context), True)
