from szimulacio import Szimulacio

tn = ['G', 'G',
      'B', 'B',
      'B', 'B']

sz = Szimulacio(tn, generations=20)

print(sz.tn)


sz.run()
print(sz.statisztikak)


