from szimulacio import Szimulacio

tn = ['G', 'B',
      'B', 'G',
      'B', 'B']


sz = Szimulacio(tn)

sz.run()

print(sz.statisztikak)

sz.plot()


