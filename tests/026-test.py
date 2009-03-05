import t
@t.cx("unicode in global name")
def test(cx):
    cx.add_global(u"\u00FC", 234)
    t.eq(cx.execute(u"\u00FC;"), 234) 
