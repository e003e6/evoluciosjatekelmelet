import numpy as np
import matplotlib.pyplot as plt
import cmodul

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
            if k < 0 or k > 255:
                raise Exception("A kifizetések értékének 0 és 255 között kell lennie!")


        # globális változók (ezeket adom majd át c-nek mint array)
        self.tn = np.array([self.r_to_num(i) for i in tn], dtype='ubyte')
        self.frstart = np.array([self.s_to_num(i) for i in list(fr[1])], dtype='ubyte')
        self.init = np.array([n, generations, rounds], dtype='ushort')
        self.kifizetesek = np.array([c, b, alpha, beta], dtype='ubyte')
        self.hibak = np.array([mu, epsilon, e_rep, e_act, init_good, fr[0]], dtype='single')
        self.statisztikak = np.ones(generations*11, dtype='single')
        self.generations = generations


    def run(self):

        # meghívom a c modul
        self.statisztikak = self.statisztikak.flatten() # csak 1D listán működik (nem tudom miért...)
        cmodul.run(self.tn, self.frstart, self.init, self.hibak, self.kifizetesek, self.statisztikak)
        self.statisztikak = self.statisztikak.reshape(self.generations, 11)


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


    def plot(self, rangee=(2.7, 3)):
        cssz = ['CC', 'CD', 'CP', 'DC', 'DD', 'DP', 'PC', 'PD', 'PP']
        szinek = ['#45258E', '#00B660', '#FAA100', '#E60090', '#00B8F5', '#CEE813', '#FD0000', '#596164', '#981018']
        nlist = list(range(1, self.generations+1))


        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 4), constrained_layout=True, dpi=300)


        for i in range(9):
            ax1.plot(nlist, self.statisztikak[:, i+2].flatten(), c=szinek[i], lw=0.8, label=cssz[i])

        ax2.plot(nlist, self.statisztikak[:, 0].flatten(), c='red', lw=0.8)


        # DESIGN
        ax1.set_xlim((0, nlist[-1]))
        ax1.set_ylim((0, 1))

        ax2.set_xlim((0, nlist[-1]))
        ax2.set_ylim((rangee[0], rangee[1]))

        ax1.legend(loc='center left', bbox_to_anchor=(1, 0.5))
        plt.show()

