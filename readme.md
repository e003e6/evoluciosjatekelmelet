## v2.1

- Modell 1 átdolgozása: gyorsabb Python futás (csak Numpy és Matplotlib csomagok használatával)

- Szimulációs leírás (lásd lentebb)

----

<br>

# The basic model

Van egy fix méretű populáció (N), ami játékosokból áll. 

- Minden játékos kap egy kezdeti hírnevet, ami egyenlő valószínűséggel jó vagy rossz (a hírnév egy ágens tulajdonság, ami a játék alatt folyton változik). 

- Minden játékos a 9 lehetséges cselekvési szabály egyikét kapja (ez egy fix ágnes tulajonság).

- Minden játékos vall egy társadalmi normát (ez egy fix tulajdonság, ami vagy megegyezik a populáció összes tagjánál vagy minden ágens egyedi normát vall - a szimuláció módjától függően).

### A játékszabályok

A játkékosokat a játák körök alatt mindig véletlenszerűen párba álítjuk. Minden párban az egyik játékos donorként, a másik recipiensként játszik. 

A donornak három viselkedési, döntési lehetősége van: 

- **együttműködik (C)**: a donor számára költséggel (c), a recipiens számára pedig haszonnal (b) jár
- **dezertál (D)**: nincs költsége és nem jár haszonnal
- **büntet (P)**: a donor számára (α) költsége van és (β) költsége a recipiens számára

Mindenk játékos bináris hírnévvel rendelkezik: jó (G) vagy rossz (B).

**A donor döntését a cselekévési szabálya határozza meg, a recipiens hírnevének függvényében!**

Minden interakció után a donor hírneve a populáció társadalmi normájának megfelelően frissül (a recipiens hírneve változatlan marad). 

### Action rules

Minden játékosnak van egy cselekvési szabálya (s). A cselekvési szabályát mindig a donor akalmazza játék közben. A cselekévis szabály (függvény) bemenete a recipiens hírneve. Ez a függvény határozza meg, hogy a donor milyen cselekvést választ a játékban.

`s(x) -> C, D, P`

Cselekvési szabály jó hírnévvel rendelekző recipiens felé: `s(G)`, rossz címzett felé pedig `s(B)`.

$3^2 = 9$ lehetséges cselekvési szabály van: 

`s(G)s(B) = CC, CD, CP, DC, DD, DP, PC, PD, PP`

Ha a donor például `CP` cselekévési szabállyal rendelkezik akkor jó hírnevű játákos ellen mindig C (együttműködés), rossz hírvevű játékos ellen mindig P (büntetés) cselekévést választ.

### Social norms

A recipiensek hírnevének frissítésére társadalmi normát használunk. A populáció összes játékosa azonos normát oszt (ez a populáció tulajdonsága, nem az ágensé).

Egy példa a társadalmi normára (xy norm):

| *n(J, X)*        | jó (G) | rossz (B) |
| ---------------- |:------:|:---------:|
| **kooperál (C)** | G      | G         |
| **dezertál (D)** | B      | B         |
| **büntet (P)**   | B      | B         |

A horizontális tengely a recipiens hírnve, a vertikális tengely a donor cselekvése. A táblázat értékei a két változó alapján megadják a donor új hírnvét. 

A jelenlegi társadalmi normában a recipiens hírnevétől független minden koperálást elismer és minden nem koperálást elitél a populáció.

### Egy generáció

A generáció elnevezés kissé félrevezető, mivel a reprodukció nem minden esetben a generáció végén történik meg. 

Egy generáció pszeudó kódban:

```pseudo
loop gen:
    - generáció számlálók nullázás
    - egyedi kifizetések nullázása
    
    loop round:
        - párba állítjuk a játékosokat

        loop par:
            - a pár játszik egymással
            - frissül a donor hírneve

    - reprodukció

    - generáció értékeinek mentése
```

<br>

## A modell három változata

1. **shared norm, public reputation** (`MODE = 1`): a társadalmi norma és a reputáció is populáció tulajdonság. 

2. **shared norm, privat reputation** (`MODE = 2`): a társadalmi norma populáció tulajdonság, de minden ágens saját vélemény listát tart fent a többi ágensről és a saját véleménye alapján cselekszik vele szemben a játék alatt. A játékot megfigyelő többi ágens a saját, privát véleménylistája alapján ítéli meg a játékosokat. 

3. **privat norm, privat reputation** (`MODE = 3`): a társadalmi norma és a reputáció is privát ágens tulajdonság. Mindenki a saját normája alapján ítéli meg a játékosok cselekedetét, és a saját magának fenttartott véleméyn alapján cselekszik a többi játékossal szemben. (Itt persze felmerül az a logikai ellentmondás, hogy a játkos a saját cselekvési szabályát elitéli.)

<br>

### Reprodukció

Az `UPDATE_MODE` határozza meg a populáció frissítésének (evolúciós dinamikájának) típusát. A két módszer:

1. Wright–Fisher process (`UPDATE_MODE = 1`): **Egyszerre egy teljes generáció cserélődik ki**. Minden generáció elején minden játékos egyszerre "úszületik", azaz stratégiát kap egy szülőtől, akit a szaporodási sikerességük (payoff) alapján választanak ki.

2. Moran process (`UPDATE_MODE = 2`): folyamatos, egyedi szintű szelekciós modell. **Egyetlen egyed frissül egy lépésben**. Tehát minden generációban `N` lépés van, minden egyes játékosra külön `total_steps = N*gen`. Minden lépésben egy véletlenszerű játékos "meghal", és helyére egy új kerül, akinek szülőjét a payoff alapján választjuk.

#### Moran process

A folyamat egyszerre mindig egyetlen egyedet cserél ki a populációban: egy egyed meghal, és a helyét egy új, szaporodással létrejövő egyed veszi át.

Minden kieső játékosért egy új egyén lép be. Alap hírnévét aszerint kapja, hogy a jelenlegi populációban milyen arányban vannak jó és rossz játékosok. (Így a populáció mérete állandó marad.)

A sikeresebb (nagyobb payoffal rendelkező) egyedek gyakrabban örökítik tovább a stratégiájukat, de ugyan akkora valószínűsggel halnak meg.

1. **Egy haldokló egyed kiválasztása.** Véletlenszerűen kiválasztunk egy egyedet. Minden egyed halálozási esélye egyenlő.

2. **Meghalt egyed pódlása.** Szülő egyed kiválasztása: a szülőt a játék körök alatt gyűjtött kifizetés (payoff) szerint választjuk ki. Véletlenszámmal választjuk ki a reprodukáló egyedet úgy, hogy a payoff  szerint súlyozott a választás. (A magasabb payoffú egyedek nagyobb valószínűséggel szaporodnak.)

3. Az új egyed (aki átveszi a meghalt egyed helyét) örökli a reprodukáló szülő stratégiáját.

4. De kis valószínűséggel mutáció (epsilon) következik be, ami a stratégiát véletlenszerűen újra állítja.

5. Az újonnan született egyed véletlenszerű kezdeti reputációt kapnak.

Generáció pszeudó kód Morgen process esetében:

```pseudo
loop gen:
    - generáció számlálók nullázás
    
    loop step (N):
        - egyedi kifizetések nullázása

        loop round:
            - párba állítjuk a játékosokat

            loop par:
                - a pár játszik egymással
                - frissül a donor hírneve

        - Moran process (egy játákos reprodukciója)

    - generáció értékeinek mentése
```

#### Wright–Fisher process

...

#### CDF payoff

...

## Mode2

Minden játákos saját reputációt tart fen. Ez egy `[N][N]` elemű mátrix, ahol `reputation[m][n]` azt jelenti, hogy `m` indexű játékos mit gondol `n` indexű játákosról.

### Kezdeti hírnév beállítás

Az `INITIAL_CORRELATION` változó azt szabályozza, hogy az egyének kezdeti reputációja (jó vagy rossz) **megegyezzen-e a populáció tagjai között**.

- `INITIAL_CORRELATION = 0` : Minden játékos minden másik játékosról függetlenül vélekedik. Minden `reputation[m][n]` érték **önállóan véletlenszerűen** kerül beállításra a `INITIAL_GOOD` szerint.

- `INITIAL_CORRELATION = 1`: Minden játékosról egységes vélemény alakul ki a játék kezdetén. Mindenki ugyanúgy látja `n` játékost, az `INITIAL_GOOD` alapján.

### Communication round

Azt a folyamatot modellezi, amikor a játékosok "beszélnek egymással"" és **megosztják a véleményüket** más játékosokról.

Minden játékos privát reputációt tart fenn másokról. A kommunikációs körök során ezek a vélemények terjednek a játékosok között, azaz az egyik játékos átveszi a másik véleményét.

A kommunikációra a lépések végén kerül sor.

`COMMUNICATION`: hány kommunikációs interakció történjen egy játékosra vetítve minden egyes lépés során Minden játékos átlagosan `COMMUNICATION` alkalommal hall valamit egy másikról.

- `l`: a hallgató (aki frissíti a véleményét)

- `m`: az informátor (`l` azt gondolja, hogy megbízható vagy nem)

- `n`: a témaszemély, akiről az információ szól

`COM_MODE`: a kommunikáció milyen szabály alapján történik

- `COM_MODE = 1`: Mindenkinek hiszünk: `l` kérdés nélkül átveszi `m` véleményét `n`-ről

- `COM_MODE = 2`: Csak annak hiszünk, akit jónak tartunk. `l` játékos csak akkor veszi át `m` véleményét, ha őt jónak tartja (`reputation[l][m] == 0`).  Ez azt modellezi, hogy az emberek csak akkor fogadnak el információt másoktól, ha megbízhatónak tartják őket.

```
loop N*COMMUNICATION:
    - l, m, n = rand

    - COM_MODE
```

## Python megvalósítás

## Init

A Python osztálynál egy kivételével mindenhol be van állítva alapértelmezett érték, így egyedül a társadalma norma megadása után indítható a szimuláció.

```python
def __init__(self, tn, n=100, generations=1000, rounds=10, fr=(0.8, 'CD'), init_good=0.5,
             mu=0.02, epsilon=0.01, e_rep=0, e_act=0, c=3, b=9, alpha=1, beta=4):
    ...
```

- **tn:** társadalmi norma, egy listában megadott mátrix

```python
tn = ['G', 'G',
      'B', 'B',
      'B', 'B']
```

- **n:** populáció mérete (csak páros szám lehet, hogy párba lehessen állítani az ágenseket)

- **genereations:** generációk száma

- **rounds:** játékkörök száma

- **fr:** kezdeti frekvencia. Valószínűségi érték (p: 0-1) és cselekvési szabály. Mekkora valószínűséggel inicializálja a medadott cselekvési szabálat (pl. 80% eselyel `CD`, maradák 20% eséllyel vélelenszerűen bármi más).

- **init_good:** mekkora arányban legyen jó reputációja az új játékosoknak (p: 0-1)

- **mu, epsilon, e_rep, e_act:** hibák valószínűségei (p: 0-1)

- **c, b, alpha, beta:** kifizetési értékek

- action a: 0=cooperation, 1=defection, 2=punishment

- reputation j: 0=good, 1=bad

A megvalósított modellben nagy szerepet fordítunk a vektorosított megoldásoknak. 

```python
for agens in range(n):
    rec_hirnev_f = reputation[agens][recipientidx]          # MODE2: rec hírneve a megfigyelő ágens szerint
    uj_velemeny = public_norm[donor_action][rec_hirnev_f]   # a cselvés fix, de az alapján itélem meg, hogy milyennek gondolom a recipiens hírnevét

        # de lehet, hogy hibásan
        if np.random.rand() < mu: # ha VAN hiba
            uj_velemeny = 0 if uj_velemeny == 1 else 1

    reputation[agens][donor] = uj_velemeny
```

*(A minta kód `uj_velemeny` valtozója csak a könyebb érthetőség miatt létezik)*

```python
# minden ágens véleménye a recipiensről egy listába
rec_hirnev = reputation[:, recipientidx]

# az új donorról kialakított vélemény kiszámítása minden egyes ágens számára
uj_velemenyek = public_norm[donor_action][rec_hirnev]

# hibák generálása minden ágensre külön
hibak = np.random.rand(n) < mu

# ahol van hiba, ott megfordul a véleményt
uj_velemenyek[hibak] = 1 - uj_velemenyek[hibak]

# frissítjük a donor reputációját minden ágens számára
reputation[:, donoridx] = uj_velemenyek

```

