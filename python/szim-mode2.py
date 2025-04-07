import numpy as np
import matplotlib.pyplot as plt

'''
shared norm, private reputation
Moran process reputation
'''

class Szimulacio:

    def __init__(self, tn, n=100, generations=1000, rounds=10, fr=(0.8, 'CD'), init_good=0.5,
                 mu=0.02, epsilon=0.01, e_rep=0, e_act=0, c=3, b=9, alpha=1, beta=4,
                 communication=100, com_mode=1, init_correlation=0):

        # társadalmi vélemény mátrix beállítása (listából np mátrix)
        public_norm = np.array([self.r_to_num(t) for t in tn], dtype=np.uint8).reshape(3, 2)

        self.statisztikak = []
        self.values = [public_norm, n, generations, rounds, fr, init_good, mu, epsilon, e_rep, e_act, c, b, alpha, beta, communication, com_mode, init_correlation]


    def _set_strategy(self, n, fr):
        '''
        Kezdeti frekvencia beállítása
        A strategy mátrix változó egy N számú lista, ami tartalmazza minden ágens viselkedési szabályát
        [0, 1] = CD
        [[0 1], [2 0] ...], len = N
        '''
        strategy = np.zeros((n, 2), dtype=np.uint8)
        th = int(n * fr[0])

        # megadott %-ú ágenshez beállítom a kezdetben többségi viselkedési szabályt
        strategy[:th, 0] = self.s_to_num(fr[1][0])
        strategy[:th, 1] = self.s_to_num(fr[1][1])
        # maradék ágensek véletlenszerű cselekvési szabályt kapnak
        strategy[th:, :] = np.random.randint(0, 3, size=(n - th, 2))

        return strategy

    def _set_public_reputation(self, n, init_good):
        '''

        '''
        return (np.random.rand(n) <= init_good).astype(np.uint8)

    def _set_private_reputation(self, n, init_good, init_correlation):
        '''
        n*n elemű mátrix
        minden sor egy adott ágens, miniden oszlop, hogy az adott ágens az adot ágensről mit gondol
        0: good, 1: bad
        '''

        if init_correlation==0:
            # Minden játékos minden másik játékosról függetlenül vélekedik. Minden reputation[m][n] érték önállóan véletlenszerű
            return (np.random.rand(n, n) > init_good).astype(np.uint8)

        elif init_correlation==1:
            # Mindenki ugyanúgy látja adott játékost, tehát a sorok (kezdetben) megegyeznek
            return np.repeat((np.random.rand(n) > init_good).astype(np.uint8)[np.newaxis, :], n, axis=0)

        else:
            raise Exception("init_correlation értéke csak 0 vagy 1 lehet!")


    def _get_freakvenciak(self, n, strategy):
        '''
        '''

        freakvenciak = {}

        for action in ['CC', 'CD', 'CP', 'DC', 'DD', 'DP', 'PC', 'PD', 'PP']:
            ker = (strategy[:, 0] == self.s_to_num(action[0])) & (strategy[:, 1] == self.s_to_num(action[1]))
            freakvenciak[action] = np.sum(ker) / n

        return freakvenciak


    def run(self):

        # változók kicsomagolása
        public_norm, n, generations, rounds, fr, init_good, mu, epsilon, e_rep, e_act, c, b, alpha, beta, communication, com_mode, init_correlation = self.values

        # kezdeti frekvencia beállítása
        strategy = self._set_strategy(n, fr)

        # kezdeti hírnevek beállítása
        #public_reputation = self._set_public_reputation(n, init_good)
        reputation = self._set_private_reputation(n, init_good, init_correlation)


        for gen in range(generations):

            # minden generáció elején elmentem a frekvenciát
            freakvenciak = self._get_freakvenciak(n, strategy)

            # minden generáció elején lenullázom a számlálókat
            times_of_cooperation = 0
            average_payoff = 0

            for step in range(n):

                # minden lépés elején lenullázom a kifizetéséket
                payoff = np.zeros(n)

                # minden lépésben van x (ROUND) játék
                for round in range(rounds):

                    # minden játék kör elején véletlenszerűen párba állítom az ágenseket (vagyis az indexeket 0:n-ig)
                    parok = np.random.permutation(n).reshape(-1, 2)

                    # minden egyes pár játszik egymással
                    for par in range(int(n/2)):
                        donoridx = parok[par][0]
                        recipientidx = parok[par][1]
                        #rec_hirnev = public_reputation[recipientidx]    # MODE1: rec hírneve a közös vélemény alapján
                        rec_hirnev = reputation[donoridx][recipientidx]  # MODE2: rec hírneve a donor szerint

                        # hiba a recipiens hírnevének felidézésében
                        donor_action = strategy[donoridx][np.bitwise_xor(rec_hirnev, (np.random.rand() <= e_rep))]

                        # hiba a tervezett művelet végrehajtásában
                        if np.random.rand() < e_act: # ha VAN hiba
                            # one of the other two actions is taken randomly
                            donor_action = np.random.choice(np.delete([0, 1, 2], donor_action))

                        if donor_action == 0:
                            times_of_cooperation += 1

                        # kifizetések
                        match donor_action:
                            case 0:    # ha kooperál
                                payoff[donoridx] -= c
                                payoff[recipientidx] += b
                            case 1:    # ha nem kooperál
                                pass
                            case 2:    # ha büntet
                                payoff[donoridx] -= alpha
                                payoff[recipientidx] -= beta

                        # a játék után firssül a donor hírneve
                        uj_velemenyek = public_norm[donor_action][reputation[:, recipientidx]]

                        # hibák generálása minden ágensre külön
                        hibak = np.random.rand(n) < mu

                        # ahol van hiba, ott megfordítjuk a véleményt
                        uj_velemenyek[hibak] = 1 - uj_velemenyek[hibak]

                        # frissítjük a donor reputációját minden ágens szempontjából
                        reputation[:, donoridx] = uj_velemenyek

                    # miutám minden pár játszott jön a kommunikációs kör
                    #vszamok = np.argsort(np.random.rand(n * communication, n), axis=1)[:, :3]  # (l,m,n), len=n*communication
                    vszamok = np.random.randint(0, n, size=(n * communication, 3))

                    if com_mode == 1:
                        reputation[vszamok[:, 0], vszamok[:, 2]] = reputation[vszamok[:, 1], vszamok[:, 2]]
                    elif com_mode == 2:
                        mask = reputation[vszamok[:, 0], vszamok[:, 1]] == 0  # l hallgaó véleménye az m informátorról, csak azok a sorok ahol good
                        reputation[vszamok[:, 0], vszamok[:, 2]][mask] = reputation[vszamok[:, 1], vszamok[:, 2]][mask]
                    else:
                        raise Exception('A com_mode változó értéke csak 1 vagy 2 lehet!')

                # minden lépés végén mentem a kifizetésket a generációba
                average_payoff += (np.sum(payoff)/n)/rounds

                # Moran process

                # haldokló egyed véletlen kiválasztása
                dying = np.random.randint(0, n)

                # szülő kiválasztása payoff arányosan
                min_to_zero = payoff - np.min(payoff)
                cdf = np.cumsum(min_to_zero/np.sum(min_to_zero))
                szulo = np.searchsorted(cdf, np.random.rand())

                # mutáció
                if np.random.rand() > epsilon:
                    strategy[dying] = strategy[szulo]
                else:
                    strategy[dying] = np.random.randint(0, 3, size=2)

                # új játékos reputációja
                if init_correlation == 0:
                    # minden reputation[m][n] érték önállóan véletlenszerű
                    reputation[:, dying] = (np.random.rand(n) > init_good).astype(np.uint8)

                elif init_correlation == 1:
                    # Mindenki ugyanúgy látja adott játékost, tehát a sorok (kezdetben) megegyeznek
                    reputation[:, dying] = 0 if np.random.rand() < init_good else 1

            stat = (gen, freakvenciak, average_payoff/n, (times_of_cooperation/(rounds*n*n/2)))
            print(stat)
            self.statisztikak.append(stat)


    def s_to_num(self, s):
        match s:
            case 'C':
                return 0
            case 'D':
                return 1
            case 'P':
                return 2

    def r_to_num(self, r):
        return 0 if r == 'G' else 1

    def plot(self, rangee=(2.7, 3)):
        cssz = ['CC', 'CD', 'CP', 'DC', 'DD', 'DP', 'PC', 'PD', 'PP']
        szinek = ['#45258E', '#00B660', '#FAA100', '#E60090', '#00B8F5', '#CEE813', '#FD0000', '#596164', '#981018']
        nlist = list(range(1, self.values[2] + 1))

        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 4), constrained_layout=True, dpi=300)

        for i, cs in enumerate(cssz):
            ax1.plot(nlist, [stat[1][cs] for stat in self.statisztikak], c=szinek[i], lw=0.8, label=cs)

        ax2.plot(nlist, [stat[-2] for stat in self.statisztikak], c='red', lw=0.8)

        # DESIGN
        ax1.set_xlim((0, nlist[-1]))
        ax1.set_ylim((0, 1))

        ax2.set_xlim((0, nlist[-1]))
        ax2.set_ylim((rangee[0], rangee[1]))

        ax1.legend(loc='center left', bbox_to_anchor=(1, 0.5))
        plt.show()


if __name__ == "__main__":

    tn = ['G', 'B',
          'B', 'G',
          'B', 'B']

    sz = Szimulacio(tn, generations=1000)
    sz.run()
    sz.plot()
