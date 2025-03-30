import numpy as np

# c-ben írt kódot előkészíti futásra

'''
shared norm, public reputation
Moran process reputation
'''


class Szimulacio:

    def __init__(self, tn, n=100, generations=1000, rounds=10, fr=(0.8, 'CD'), init_good=0.5,
                 mu=0.02, epsilon=0.01, e_rep=0, e_act=0, c=3, b=9, alpha=1, beta=4):

        # Hibakeresés, hogy a C-nek nehogy rossz értékeket adjunk át
        if n%2 != 0:
            raise Exception("A populáció mérete csak páros szám lehet!")

        for szam in [mu, epsilon,  e_rep, e_act, init_good, fr[0]]:
            if szam < 0 or szam > 1:
                raise Exception("A valószínűségi értékeknek 0 és 1 között kell lennie!")

        if fr[1] not in ['CC', 'CD', 'CP', 'DC', 'DD', 'DP', 'PC', 'PD', 'PP']:
            raise Exception("A megadott cselekvésiszabály hibás!")

        if len(tn) != 6:
            raise Exception("A társadalmi norna lista nem megfelelő méretű!")

        for t in tn:
            if t != 'G' and t != 'B':
                raise Exception("A társadalmi norna csak 'G' és 'B' elemekből állhat!")

        for k in [c, b, alpha, beta]:
            if k < 0 or k > 100:
                raise Exception("A kifizetések értékének 0 és 100 között kell lennie!")


        # globális változók (ezeket adom majd át c-nek mint array)
        self.tn = np.array([self.r_to_num(i) for i in tn], dtype='ushort')
        self.frstart = np.array([self.s_to_num(i) for i in list(fr[1])], dtype='ushort')
        self.init = np.array([n, generations, rounds], dtype='ushort')
        self.kifizetesek = np.array([c, b, alpha, beta], dtype='ushort')
        self.hibak = np.array([mu, epsilon, e_rep, e_act, init_good, fr[0]], dtype='float')
        self.statisztikak = np.zeros((n, 11), dtype='float')


    def run(self):
        # az eredményeket tároló listát nullázom (de nem csinálok újat!)
        self.statisztikak[:, :] = 0

        # meghívom c modul
        #cmodul.run(self.tn, self.frstart, self.init, self.hibak, self.kifizetesek, self.statisztikak)

    def r_to_num(self, r):
        match r:
            case 'G':
                return 0
            case 'B':
                return 1

    def s_to_num(self, s):
        match s:
            case 'C':
                return 0
            case 'D':
                return 1
            case 'P':
                return 2
