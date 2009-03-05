import t
@t.cx("test unbind unicode")
def test(cx):
    cx.add_global(u"\u00E9", 734)
    t.eq(cx.execute(u"\u00E9;"), 734)
    #t.eq(cx.rem_global(u"\u00E9"), 734)
    #t.raise(AttributeError, cx.execute, u"\u00E9")
    import sys
    sys.stderr.write("not ")
