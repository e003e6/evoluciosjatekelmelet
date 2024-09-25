#include <stdlib.h> 
#include <time.h> 
#include <stdio.h>


// pszeudo véletlenszám generátor: valószínűség 0-1 között (p)
short veletlen_start = 0;

float randomp(void) {
	
	if (veletlen_start == 0) {
		veletlen_start = 1;
		srand(time(NULL));
		rand();
	}
		
return ((float) rand()/RAND_MAX);	
}

// pszeudo véletlen int generátor (beállított határok között)
short randomint(short min, short max){
		short velint;

		velint = (short) (min+(randomp()*(max-min))+0.5);

return (velint);	
}

// véletlenszerűvé rendezi át a lista elemeket
void shuffle(unsigned short len, unsigned short lista[]) {
    unsigned short i, ran_i, value;

    for (i=0; i<len; i++) {
        ran_i = randomint(0, len-1); 
        // kicserélem i és ran_i értékét
        value = lista[i];
        lista[i] = lista[ran_i];
        lista[ran_i] = value;
    }

return; 
}


// elmenti az aktuális cselekvési szabály megoszlást
void frekvenciamentes(unsigned short aktgen, unsigned short n, unsigned char strategy[n][2], float * statisztikak) {
    unsigned short g, b, nn, total, sz=2; 

    for (g=0; g<3; g++) {
			for (b=0; b<3; b++) {
                total = 0;
                for (nn=0; nn<n; nn++) {
                    if (strategy[nn][0]==g && strategy[nn][1]==b) { total++; }
                }
                statisztikak[aktgen*11+sz] = ((float)total)/n;
                sz++;
			}
	}
       
return;    
}


void parbaallitas(unsigned short n, unsigned short indexek[n], unsigned short parok[n/2][2]) {
    unsigned short i, nfel;
    nfel = n/2;

    // megkeverem az indexeket
    shuffle(n, indexek); 

    // feltöltom a párok listát az új kevert indexekkel
    for (i=0; i<nfel; i++) {
        parok[i][0] = indexek[i];
        parok[i][1] = indexek[nfel+i];
    }

return;
}


void cumsum(unsigned short size, float arr[size]) {
		int i;
		float min, osszeg;
		
		min = arr[0];
		for (i=1; i<size; i++) {
				if (arr[i] < min) {
					min = arr[i];
				}
		}
		
		if (min < 0) {
			for (i=0; i<size; i++) {
				arr[i] -= min;
			}
		}
				
		for (i=osszeg=0; i<size; i++) {
			osszeg += arr[i];
		}
	
		for (i=0; i<size; i++) {
			arr[i] = arr[i]/osszeg;
        }
		
		for (i=1; i<size; i++) {
       arr[i] = arr[i] + arr[i-1]; 
     }

return;       
}


float sum(float lista[], unsigned short len) {
    unsigned short i;
    float sum;

    for (i=sum=0; i<len; i++) {
        sum += lista[i];
    }

return (sum);
} 


void szimulacio(unsigned char tn[6], unsigned char fr_start[2], unsigned short init[3], float hibak[6], unsigned char kifizetesek[4], float * statisztikak) {
    
    // hozott változók
    const unsigned short n=init[0], gen=init[1], round=init[2];
    const float mu=hibak[0], ep=hibak[1], erep=hibak[2], eact=hibak[3], igood=hibak[4];
    const unsigned char c=kifizetesek[0], b=kifizetesek[1], alpha=kifizetesek[2], beta=kifizetesek[3];

    // új változók
    unsigned short i, aktgen, aktstep, aktround, par;   // ciklus változók
    unsigned short donor_idx, recipient_idx, dying_idx, szulo_idx, indexek[n], parok[n/2][2];
    unsigned char donor_action, uj_action, strategy[n][2], public_reputation[n];
    float times_of_cooperation, average_payoff, p, payoff[n];


    // kezdeti frekvencia beállítása
    {
        unsigned short fr_fo = n*hibak[5];

        for (i=0; i<fr_fo; i++) {
            strategy[i][0] = fr_start[0];
            strategy[i][1] = fr_start[1];
        }
        for (i=i; i<n; i++) {
            strategy[i][0] = randomint(0, 2);
            strategy[i][1] = randomint(0, 2);
        }
    }
    
    // véletlenszerű kezdeti hírnevek beállítása
    for (i=0; i<n; i++) {
        if (randomp() > igood) { public_reputation[i] = 0; }
        else {public_reputation[i] = 1; }
    }

    // indexek lista feltöltése
    for (i=0; i<n; i++) {
        indexek[i] = i;
    }

    // szimuláció indítása
    for (aktgen=0; aktgen<gen; aktgen++) {
        
        // minden generáció elején elmentem azt a frekvenciát amivel kezd a generáció
        frekvenciamentes(aktgen, n, strategy, statisztikak);

        // nullázom a generáció értékeket
        times_of_cooperation = 0.0; 
        average_payoff = 0.0;

        // minden generáció n darab lépésből áll
        for (aktstep=0; aktstep<n; aktstep++) {

            // minden lépés elején lenullázom a kifizetéséket
            for (i=0; i<n; i++) { payoff[i] = 0.0; }

            // minden lépés 10 játékkörből áll
            for (aktround=0; aktround<round; aktround++) {

                // minden kör elején véletlenszerűen párba állítom az ágenseket
                parbaallitas(n, indexek, parok);

                // minden pár játszik egymással
                for (par=0; par<n/2; par++) {

                    donor_idx = parok[par][0];
                    recipient_idx = parok[par][1];

                    // a donor eldönti mit lépjen ( de lehet hiba a recipiens hírnevének felidézésében)
                    if (randomp() > erep) { // ha NINCSEN hiba
                        donor_action = strategy[donor_idx][public_reputation[recipient_idx]];
                    } else {
                        donor_action = strategy[donor_idx][abs(public_reputation[recipient_idx]-1)];
                    }

                    // hiba a tervezett művelet végrehajtásában
                    if (randomp() < eact) { // ha VAN hiba
                        do { uj_action = randomint(0, 2); } 
                        while (uj_action == donor_action);
                        donor_action = uj_action;
                    }

                    if (donor_action == 0) { times_of_cooperation++; }

                    // a donor lép
                    switch (donor_action) {
                    case 0:
                        payoff[donor_idx] -= c;
                        payoff[recipient_idx] += b;
                        break;
                    case 1:
                        break;
                    case 2:
                        payoff[donor_idx] -= alpha;
                        payoff[recipient_idx] -= beta;
                    }

                    // a játék után firssül a donor hírneve (de lehet, hogy hibásan emlékszik rá a populáció)
                    if (randomp() > mu) {  // ha NINCSEN hiba
                        public_reputation[donor_idx] = tn[donor_action*2+public_reputation[recipient_idx]]; 
                    } else {
                        public_reputation[donor_idx] = abs(tn[donor_action*2+public_reputation[recipient_idx]]-1);
                    }
                }
            }
        
            // minden lépés végén hozzáadom a gen payoff-hoz az átlagos payoff/round értékét
            average_payoff += (sum(payoff, n)/n)/round;

            // minden lépés végén jön a reprodukció
            cumsum(n, payoff);
            
            dying_idx = randomint(0, n-1);
        
            p = randomp();
            szulo_idx = 0;
            while (p > payoff[szulo_idx]) { szulo_idx++; }
            
            strategy[dying_idx][0] = strategy[szulo_idx][0];
            strategy[dying_idx][1] = strategy[szulo_idx][1];

            if (randomp() < ep) { // ha VAN mutáció
                strategy[dying_idx][0] = randomint(0, 2);
                strategy[dying_idx][1] = randomint(0, 2);
            }

            if (randomp() < igood) {
                public_reputation[dying_idx] = 0;
            } else {
                public_reputation[dying_idx] = 1;
            }
        }

        statisztikak[aktgen*11+0] = average_payoff/n;
        statisztikak[aktgen*11+1] = times_of_cooperation/(round*n*n/2);

    }

return;
}
