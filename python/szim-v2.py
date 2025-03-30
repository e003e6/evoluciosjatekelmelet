import numpy as np
import matplotlib.pyplot as plt

'''
shared norm, public reputation
Moran process reputation
'''

class Szimulacio:

    def __init__(self, tn, n=100, generations=1000, rounds=10, fr=(0.8, 'CD'), init_good=0.5,
                 mu=0.02, epsilon=0.01, e_rep=0, e_act=0, c=3, b=9, alpha=1, beta=4):

        # társadalmi vélemény mátrix beállítása (listából np mátrix)
        public_norm = np.array([self.r_to_num(t) for t in tn], dtype=np.uint8).reshape(3, 2)

        self.statisztikak = []
        self.values = [public_norm, n, generations, rounds, fr, init_good, mu, epsilon, e_rep, e_act, c, b, alpha, beta]


    def run(self):

        # változók kicsomagolása
        public_norm, n, generations, rounds, fr, init_good, mu, epsilon, e_rep, e_act, c, b, alpha, beta = self.values

        # kezdeti frekvencia beállítása
        strategy = np.zeros((n, 2), dtype=np.uint8)  # len = N, [[0 1], [2 0] ...]
        th = int(n * fr[0])
        # megadott %-ú ágenshez beállítom a kezdetben többségi viselkedési szabályt
        strategy[:th, 0] = self.s_to_num(fr[1][0])
        strategy[:th, 1] = self.s_to_num(fr[1][1])
        # maradék ágensek véletlenszerű cselekvési szabályt kapnak
        strategy[th:, :] = np.random.randint(0, 3, size=(n - th, 2))

        # véletlenszerű kezdeti hírnevek
        public_reputation = (np.random.rand(n) <= init_good).astype(np.uint8)


        for gen in range(generations):

            # minden generáció elején elmentem a frekvenciát
            freakvenciak = {}
            for action in ['CC', 'CD', 'CP', 'DC', 'DD', 'DP', 'PC', 'PD', 'PP']:
                ker = (strategy[:, 0] == self.s_to_num(action[0])) & (strategy[:, 1] == self.s_to_num(action[1]))
                freakvenciak[action] = np.sum(ker)/n

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
                        rec_hirnev = public_reputation[recipientidx]

                        # hiba a recipiens hírnevének felidézésében
                        if np.random.rand() > e_rep: # ha NINCSEN hiba
                            donor_action = strategy[donoridx][0] if (rec_hirnev == 0) else strategy[donoridx][1]
                        else:
                            donor_action = strategy[donoridx][0] if (rec_hirnev == 1) else strategy[donoridx][1]

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
                        uj_velemeny = public_norm[donor_action][rec_hirnev]

                        # de lehet, hogy hibásan
                        if np.random.rand() > mu:
                            public_reputation[donoridx] = uj_velemeny
                        else:
                            public_reputation[donoridx] = 0 if uj_velemeny == 1 else 1

                # minden lépés végén mentem a kifizetésket a generációba
                average_payoff += (sum(payoff)/n)/rounds

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
                public_reputation[dying] = 0 if np.random.rand() < init_good else 1

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

    sz = Szimulacio(tn, generations=20)
    sz.run()
    sz.plot()
