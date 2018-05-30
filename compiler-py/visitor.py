import struct

def intbytes_8(n:int):
    return [int(b) for b in bytearray(struct.pack("@q", n))]
def doublebytes(d:float):
    return [int(b) for b in bytearray(struct.pack("@d", d))]


class NodeVisitor(object):
    def visit(self, node):
        method_name = "visit_" + type(node).__name__
        visitor = getattr(self, method_name, self.generic_visit)
        return visitor(node)
    def generic_visit(self, node):
        raise Exception("No visit_%s method: %s" % (type(node).__name__, node))