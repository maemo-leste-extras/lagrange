import lldb

class PtrSet_SynthProvider:

    def __init__(self, valobj, dict):
        self.valobj = valobj
        self.update()

    def num_children(self):
        return self.elem_count

    def get_child_index(self, name):
        try:
            return int(name.lstrip('[').rstrip(']'))
        except:
            return -1

    def get_child_at_index(self, index):
        if index < 0:
            return None
        if index >= self.num_children():
            return None
        try:
            offset = (self.range_start.GetValueAsUnsigned(0) + index) * \
                self.valobj.GetProcess().GetAddressByteSize()
            return self.values_data.CreateValueFromExpression(
                '[' + str(index) + ']',
                '*(void**) &%s[%u]' % (self.values_data.get_expr_path(), offset)
            )
        except:
            return None

    def update(self):
        try:
            self.values_data = self.valobj.GetChildMemberWithName('values') \
                                          .GetChildMemberWithName('data')
            values_range = self.valobj.GetChildMemberWithName('values') \
                                      .GetChildMemberWithName('range')
            self.range_start = values_range.GetChildMemberWithName('start')
            self.range_end = values_range.GetChildMemberWithName('end')
            self.elem_count = self.range_end.GetValueAsUnsigned(0) \
                - self.range_start.GetValueAsUnsigned(0)            
        except Exception as x:
            print("Problem:", x)

    def has_children(self):
        return True


#def PtrSet_SummaryProvider(valobj, dict):
#    prov = PtrSet_SynthProvider(valobj, None)
#    return 'size=' + str(prov.num_children()) + " cmp=" + \
#        str(valobj.GetChildMemberWithName('cmd'))

