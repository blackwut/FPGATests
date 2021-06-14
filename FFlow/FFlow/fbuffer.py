import sys
from enum import Enum

# TODO: ricontrollare tutto perchè porcodio


# Global FBuffer access mode from device
class FBufferAccess(Enum):
    READ = 1        # Each replica has its own buffer in read mode
    WRITE = 2       # Each replica has its own buffer in write mode
    RW = 3          # Each replica has its own buffer in read-write mode
    READ_ALL = 4    # All replicas share the same buffer in read mode
    WRITE_ALL = 5   # All replicas share the same buffer in write mode
    RW_ALL = 6      # All replicas share the same buffer in read-write mode


class FBuffer:
    def __init__(self,
                 datatype: str,
                 name: str,
                 size: int = 1):
        assert datatype
        assert name
        assert size > 0

        self.datatype = datatype
        self.name = name
        self.size = size
        self.visibility = ''

    def is_private(self):
        return False

    def is_local(self):
        return False

    def is_global(self):
        return False

    def declare(self):
        raise RuntimeError('has to be implemented in a subclass')

    def parameter(self):
        raise RuntimeError('has to be implemented in a subclass')

    def use(self):
        raise RuntimeError('has to be implemented in a subclass')


class FBufferPrivate(FBuffer):
    def __init__(self,
                 datatype: str,
                 name: str,
                 size: int = 1,
                 value=None,
                 ptr: bool = False):    # set ptr = True if this buffer has to passed by ref to a function even if size == 1
        super().__init__(datatype, name, size)
        self.value = value
        self.ptr = ptr
        self.visibility = '__private'

    def is_ptr_parameter(self):
        return self.size > 1 or self.ptr

    def is_private(self):
        return True

    def declare(self):
        d = ' '.join([self.visibility,
                      self.datatype,
                      self.name])
        d += ('[' + str(self.size) + ']' if self.size > 1 else '')
        if self.value:
            if type(self.value) in (list, tuple, set):
                d += ' = {' + ', '.join(self.value) + '}'
        return d

    def parameter(self):
        return self.visibility + ' ' + self.datatype + (' * ' if self.is_ptr_parameter() else ' ') + self.name

    def use(self):
        if self.size > 1 or not self.ptr:
            return self.name
        return '&' + self.name


class FBufferLocal(FBuffer):
    def __init__(self,
                 datatype: str,
                 name: str,
                 size: int = 1,
                 value=None):
        super().__init__(datatype, name, size)
        self.value = value
        self.visibility = '__local'

    def is_ptr_parameter(self):
        return self.size > 1

    def is_local(self):
        return True

    def declare(self):
        d = ' '.join([self.visibility,
                      self.datatype,
                      self.name])
        d += ('[' + str(self.size) + ']' if self.size > 1 else '')
        if self.value:
            if type(self.value) in (list, tuple, set):
                d += ' = {' + ', '.join(self.value) + '}'
        return d

    def parameter(self):
        return self.visibility + ' ' + self.datatype + (' * ' if self.is_ptr_parameter() else ' ') + self.name

    def use(self):
        if self.is_ptr_parameter():
            return self.name
        return '&' + self.name


class FBufferGlobal(FBuffer):
    def __init__(self,
                 datatype: str,
                 name: str,
                 size: int = 1,
                 access: FBufferAccess = FBufferAccess.READ_ALL,
                 ptr: bool = True):    # set ptr = False if you need to pass a single value to kernel
        super().__init__(datatype, name, size)
        self.ptr = ptr
        if not self.ptr and self.size > 1:
            sys.exit(self.name + ' FBufferGlobal has to be of size = 1 if ptr is False')
        self.visibility = '__global'

    def is_ptr_parameter(self):
        return self.size > 1

    def is_global(self):
        return True

    def declare(self):
        raise RuntimeError('never call this function on FBufferGlobal')

    def parameter(self):
        if self.is_ptr_parameter():
            return ' '.join([self.visibility,
                             self.datatype,
                             '*',
                             'restrict',
                             self.name])
        else:
            return ' '.join([self.datatype,
                             self.name])

    def use(self):
        return self.name
