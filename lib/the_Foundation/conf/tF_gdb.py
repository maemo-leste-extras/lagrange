class Int2Printer:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "{%d,%d}" % (self.val['x'], self.val['y'])


class RangePrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "{%s...%s}" % (self.val['start'], self.val['end'])


class RectPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "{pos = %s, size = %s}" % (self.val['pos'], self.val['size'])


class StringPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        size = int(self.val['chars']['i']['size'])
        if size == 0: return "<empty String>"
        return "{\"" + self.val['chars']['i']['data'].string('utf-8').replace('"', '\"') + \
            "\" size = %d" % size  + "}"


class ObjectPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "{instance of %s, refCount = %d}" % (self.val['classObj'], self.val['refCount'])


class PtrSetPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        values = self.val['values']
        set_range = (int(values['range']['start']), int(values['range']['end']))
        set_size = set_range[1] - set_range[0]
        if set_size == 0: return "<empty PtrSet>"
        return "{size = %d}" % (set_size)


def lookup_type(val):
    type_str = str(val.type)
    if type_str.startswith('const '): type_str = type_str[6:]
    if type_str.endswith(' *'): type_str = type_str[:-2]
    if type_str == 'iString':
        return StringPrinter(val)
    if type_str.startswith('iRange'):
        return RangePrinter(val)
    if type_str == 'iInt2':
        return Int2Printer(val)
    if type_str == 'iRect':
        return RectPrinter(val)
    if type_str == 'iObject':
        return ObjectPrinter(val)
    if type_str == 'iPtrSet':
        return PtrSetPrinter(val)
    return None


gdb.pretty_printers.append(lookup_type)
