from enum import Enum


class FBufferType(Enum):
    PRIVATE = 1
    LOCAL = 2
    GLOBAL = 3
    ALL = 4


class FBuffer:
    def __init__(self,
                 buffer_type: FBufferType,
                 data_type: str,
                 name: str,
                 size: int):
        self.buffer_type = buffer_type
        self.data_type = data_type
        self.name = name
        self.size = size

    def get_declaration(self):
        if self.buffer_type == FBufferType.GLOBAL:
            ret = '__global ' + self.data_type + ' * ' + self.name
        elif self.buffer_type == FBufferType.LOCAL:
            ret = '__local ' + self.data_type + ' * ' + self.name
        elif self.buffer_type == FBufferType.PRIVATE:
            ret = self.data_type + ' * ' + self.name
        else:
            ret = ''

        return ret
