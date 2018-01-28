

class Env(object):
    def __init__(self, parent=None):
        self.vars = {}
        self.parent = parent
    def __len__(self):
        if self.parent is None:
            return len(self.vars)
        else:
            return len(self.vars) + len(self.parent)
    def __contains__(self, var:str):
        if var in self.vars: return True
        elif self.parent is None: return False
        else: return self.parent.__contains__(var)
    def __getitem__(self, var:str):
        if var in self.vars:
            return self.vars[var]
        elif self.parent is None:
            return None
        else:
            return self.parent.__getitem__(var)
    def __setitem__(self, key, value):
        self.vars[key] = value
    def decl_var(self, var:str ):
        self.vars[var] = len(self)
