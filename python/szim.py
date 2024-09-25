import numpy as np

'''
shared norm, public reputation
Moran process reputation
'''

class Szimulacio:

    def __init__(self, tn, n=100, generations=1000, rounds=10, fr=(0.8, 'CD'), init_good=0.5,
                 mu=0.02, epsilon=0.01, e_rep=0, e_act=0, c=3, b=9, alpha=1, beta=4):
        self.stats = []
        self.generations = generations

        self._run(tn, n, generations, rounds, fr, init_good, mu, epsilon, e_rep, e_act, c, b, alpha, beta)


    def _run(self, tn, n, generations, rounds, fr, init_good, mu, epsilon, e_rep, e_act, c, b, alpha, beta):

        # kezdeti frekvencia beállítása
        strategy = np.zeros((n, 2), dtype=np.uint8)
        for i in range(n):
            if i<n*fr[0]:
                strategy[i, 0] = self.s_to_num(fr[1][0])
                strategy[i, 1] = self.s_to_num(fr[1][1])
            else:
                strategy[i, 0] = np.random.randint(0, 3)
                strategy[i, 1] = np.random.randint(0, 3)

        # társadalmi vélemény mátrix beállítása
        public_norm = np.array([self.r_to_num(t) for t in tn], dtype=np.uint8).reshape(3, 2)

        # véletlenszerű kezdeti hírnevek
        public_reputation = np.array([0 if (np.random.rand() > init_good) else 1 for n in range(n)], dtype=np.uint8)

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

                    # minden játék kör elején véletlenszerűen párba állítom az ágenseket
                    parok = np.arange(0, n, dtype=np.uint8); np.random.shuffle(parok)
                    parok = parok[np.newaxis, :].reshape(int(n/2), 2)

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
                            l = [0, 1, 2]; l.remove(donor_action)
                            donor_action = np.random.choice(l)

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
                            public_reputation[donoridx] = np.abs(uj_velemeny-1)

                # minden lépés végén mentem a kifizetésket a generációba
                average_payoff += (sum(payoff)/n)/rounds

                # Moran process

                min_to_zero = payoff - min(payoff)
                cdf = np.cumsum(min_to_zero/np.sum(min_to_zero))

                # chosing a dying individual
                dying = np.random.randint(0, n)

                # choosing a parent proportionally to payoff
                r = np.random.rand()
                szulo = 0
                while r > cdf[szulo]:
                    szulo += 1

                # mutáció
                if np.random.rand() > epsilon:
                    strategy[dying] = strategy[szulo]
                else:
                    strategy[dying] = np.array([np.random.randint(0, 3), np.random.randint(0, 3)])

                # assigning a new reputation to the new-born
                if np.random.rand() < init_good:
                    public_reputation[dying] = 0
                else:
                    public_reputation[dying] = 1

            stat = (gen, freakvenciak, average_payoff/n, (times_of_cooperation/(rounds*n*n/2)))
            #print(stat)
            self.stats.append(stat)


    def s_to_num(self, s):
        match s:
            case 'C':
                return 0
            case 'D':
                return 1
            case 'P':
                return 2

    def r_to_num(self, r):
        match r:
            case 'G':
                return 0
            case 'B':
                return 1