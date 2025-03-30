# The basic model

Van egy fix méretű populáció (N), ami játékosokból áll. 

- Minden játékos kap egy kezdeti hírnevet, ami egyenlő valószínűséggel jó vagy rossz (a hírnév egy ágens tulajdonság, ami a játék alatt folyton változik). 

- Minden játékos a 9 lehetséges cselekvési szabály egyikét kapja (ez egy fix ágnes tulajonság).

- Minden játékos ugyanazt a társadalmi normát vallja (ez egy fix populáció tulajdonság).

### A játékszabályok

A játkékosokat a játák körök alatt mindig véletlenszerűen párba álítjuk. Minden párban az egyik játékos donorként, a másik recipiensként játszik. 

A donornak három viselkedési, döntési lehetősége van: 

- **együttműködik (C)**: a donor számára költséggel (c), a recipiens számára pedig haszonnal (b) jár
- **dezertál (D)**: nincs költsége és nem jár haszonnal
- **büntet (P)**: a donor számára (α) költsége van és (β) költsége a recipiens számára

Mindenk játékos bináris hírnévvel rendelkezik: jó (G) vagy rossz (B).

**A donor döntését a cselekévési szabálya határozza meg, a befogadó hírnevének függvényében!**

Minden interakció után a donor hírneve a populáció társadalmi normájának megfelelően frissül, a recipiens hírneve változatlan marad. 

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

A jelenlegi társadalmi normában a recipiens hírnevétől független minden koperálást támagot és minden nem koperálást elitél a populáció.

### Egy generáció

A generáció elnevezés kissé félrevezető, mivel a reprodukció nem a generáció végén történik meg. A generáció inkább csak egy mérféldkő, egy frekvencia, amivel szabályozzuk a szimuláció hosszát és ahol lekérjük az adatokat a szimulációból.

Egy generáció pszeudó kódban:

```pseudo
loop gen:
    - generáció számlálók nullázás
    
    loop step:
        - egyedi kifizetések nullázása

        loop round:
            - párba állítjuk a játékosokat

            loop par:
                - a pár játszik egymással
                - frissül a donor hírneve

        - Moran process (reprodukció)

    - generáció értékeinek mentése
```

### Evolúció (Moran process)

A folyamat egyszerre mindig egyetlen egyedet cserél ki a populációban: egy egyed meghal, és a helyét egy új, szaporodással létrejövő egyed veszi át.

Minden kieső játékosért egy új egyén lép be. Alap hírnévét aszerint kapja, hogy a jelenlegi populációban milyen arányban vannak jó és rossz játékosok. (Így a populáció mérete állandó marad.)

## Python megvalósítás

## Init

A Python osztálynál egy kivételével mindenhol be van állítva alapértelmezett érték, így egyedül a társadalma norma megadása után indítható a szimuláció.

```python
def __init__(self, tn, n=100, generations=1000, rounds=10, fr=(0.8, 'CD'), init_good=0.5,
             mu=0.02, epsilon=0.01, e_rep=0, e_act=0, c=3, b=9, alpha=1, beta=4):
    ...
```

- **tn:** társadalmi norma, egy listában megadott mátrixpython

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





### Moran process

A sikeresebb (nagyobb payoffal rendelkező) egyedek gyakrabban örökítik tovább a stratégiájukat, de ugyan akkora valószínűsggel halnak meg.

1. **Egy haldokló egyed kiválasztása.** Véletlenszerűen kiválasztunk egy egyedet. Minden egyed halálozási esélye egyenlő.

2. **Meghalt egyed pódlása.** Szülő egyed kiválasztása: a szülőt a játék körök alatt gyűjtött kifizetés (payoff) szerint választjuk ki. Véletlenszámmal választjuk ki a reprodukáló egyedet úgy, hogy a payoff  szerint súlyozott a választás. (A magasabb payoffú egyedek nagyobb valószínűséggel szaporodnak.)

3. Az új egyed (aki átveszi a meghalt egyed helyét) örökli a reprodukáló szülő stratégiáját.

4. De kis valószínűséggel mutáció (epsilon) következik be, ami a stratégiát véletlenszerűen újra állítja.

5. Az újonnan született egyed véletlenszerű kezdeti reputációt kapnak.



#### CDF payoff








