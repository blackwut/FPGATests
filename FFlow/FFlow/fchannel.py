class FChannel:
    def __init__(self,
                 data_type: str,
                 name: str,
                 depth: int = 1,
                 o_degree: int = 1,
                 i_degree: int = 1):
        self.tuple_type = 'tuple_' + name
        self.data_type = data_type
        self.name = name
        self.depth = depth
        self.o_degree = o_degree
        self.i_degree = i_degree

        self.i = self.o_degree
        self.j = self.i_degree
