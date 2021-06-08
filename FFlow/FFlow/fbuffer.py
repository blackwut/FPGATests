from enum import Enum


class FBufferKind(Enum):
    PRIVATE = 1
    LOCAL = 2
    GLOBAL = 3
    ALL = 4


class FBufferAccess(Enum):
    READ = 1
    WRITE = 2
    RW = 3


class FBuffer:
    def __init__(self,
                 kind: FBufferKind,
                 datatype: str,
                 name: str,
                 size: int = 1,
                 ptr: bool = True):
        assert kind
        assert datatype
        assert name
        assert size > 0

        self.kind = kind
        self.datatype = datatype
        self.name = name
        self.size = size

        self.ptr = True
        if self.kind == FBufferKind.PRIVATE:
            self.ptr = ptr

        self.visibility = '__private'
        if self.kind is FBufferKind.LOCAL:
            self.visibility = '__local'
        elif self.kind is FBufferKind.GLOBAL:
            self.visibility = '__global'

    def is_private(self):
        return self.kind == FBufferKind.PRIVATE

    def is_local(self):
        return self.kind == FBufferKind.LOCAL

    def is_global(self):
        return self.kind == FBufferKind.GLOBAL

    def declare(self):
        if self.kind is FBufferKind.GLOBAL:
            return self.visibility + ' ' + self.datatype + (' * ' if self.ptr else ' ') + self.name
        else:
            return self.visibility + ' ' + self.datatype + ' ' + self.name + ('[' + str(self.size) + ']' if self.size > 1 else '')

    def parameter(self):
        return self.visibility + ' ' + self.datatype + (' * ' if self.ptr else ' ') + self.name

    def use(self):
        if self.is_private() and self.ptr:
            return '&' + self.name
        return self.name
