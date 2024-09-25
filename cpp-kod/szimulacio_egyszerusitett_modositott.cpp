#include<stdio.h>
#include<stdlib.h>
#include<time.h>

// MODE=1 (shared norm, public reputation)
// UPDATE_MODE=2

// action a: 0=cooperation, 1=defection, 2=punishment
// reputation j: 0=good, 1=bad


// ------------- Constants -------------

const int N = 100;
const int GENERATION = 1000;
const int ROUND = 10;

// public norm (0=good, 1=bad)
const int PUBLIC_NORM_CG = 0;
const int PUBLIC_NORM_CB = 1;
const int PUBLIC_NORM_DG = 1;
const int PUBLIC_NORM_DB = 0;
const int PUBLIC_NORM_PG = 1;
const int PUBLIC_NORM_PB = 1;

// kezdeti frekvencia (jelenleg: 80% CD)
const int MAJOR_STRATEGY_NUMBER = 80;
const int MAJOR_STRATEGY_G = 0;
const int MAJOR_STRATEGY_B = 1;

// initial fraction of good players (50-50)
const double INITIAL_GOOD = 0.5;

// payoff parameters
const double B = 9.0;
const double C = 3.0;
const double ALPHA = 1.0;
const double BETA = 4.0;

// accumulating errors (error rate in assigining reputation)
const double MU = 0.02;

// mutation rates (mutation rate in action rules) (fi)
const double MUTATION_RATE = 0.01;

// temporal errors
const double E_ACT = 0.00;   // error rate in executing intended action
const double E_REP = 0.00;   // error rate in recalling recipient’s reputation (temporal, not accumulating)


// ------------- Functions -------------

// calculating frequency of a given strategy
double frequency(int** strategy, int a1, int a2) {
	int n;
	int total = 0;

	for (n=0; n<N; n++) {
		if (strategy[n][0]==a1 && strategy[n][1]==a2) total++;
	}

	return ( (double) total/N);
}

// calculating average payoff per round
double average_per_round(double* payoff) {
	int n;
	double total = 0.0;

	for (n=0; n<N; n++) total += payoff[n];

	total = total / (double) N;
	total = total / (double) ROUND;
	
	return (total);
}

// shifting the minimum payoff to zero
void minimum_to_zero(double* payoff) {
	int n;
	double min;

	min = payoff[0];

	for (n=1; n<N; n++) {
	  if (payoff[n]<min) min = payoff[n];
	}

	for (n=0; n<N; n++) payoff[n] -= min;
}

// generating a cummulative distribution function (cdf)
void payoff_to_cdf(double* payoff) {
	int n;
	double total = 0.0;

	for (n=0; n<N; n++) total += payoff[n];

	if (total>0.0) {
		for (n=0; n<N; n++) payoff[n] = payoff[n]/total;
		for (n=1; n<N; n++) payoff[n] = payoff[n-1]+payoff[n];
	} else {
		for (n=0; n<N; n++) payoff[n] = (double) (n+1)/N;
	}

	payoff[N-1] = 1.0;
}


// -------------- MAIN --------------

int main(void) {
	int n, j, a, a1, a2, gen, steps, round, pair;   // ciklus számlálók
	int reproduce, stack_length, donor_action;  
	int times_of_cooperation;
	double average_payoff;
	int rd_int;                 // random variable (int)
	double rd_double;           // random variable (double)

	double *payoff;             // payoff[n] = player n’s payoff
	int **strategy;             // strategy[n][j] = player n’s action towards j
	int **public_norm;          // public_norm[a][j] = evaluation of action a towards j
	int *public_reputation;     // public_reputation[n] = player n’s public reputation

	// for donor-recipient matching
	int *stack;
	int *donor;                 // donor[pair]: the donor in #pair
	int *recipient;             // recipient[pair]: the recipient in #pair

	// seed for a random variable
	srand( (unsigned) time(NULL));


	// dynamic memory allocation

	payoff = new double [N];
	strategy = new int* [N]; 
	for (n=0; n<N; n++) strategy[n] = new int [2]; 

	public_norm = new int* [3];
	for (a=0; a<3; a++) public_norm[a] = new int [2];
	
	public_reputation = new int [N];

	stack = new int [N];
	donor = new int [N/2];
	recipient = new int [N/2];


	// kezdeti frekvencia beállítása
	for(n=0; n<N; n++) {
		for (j=0; j<2; j++) {
			// initialization of strategy[n][j]: 0=cooperation, 1=defection; 2=punishment
			if (n<MAJOR_STRATEGY_NUMBER) {                   // MAJOR_STRATEGY_NUMBER = 80
				if(j==0) strategy[n][j] = MAJOR_STRATEGY_G;  // 0=cooperation
				if(j==1) strategy[n][j] = MAJOR_STRATEGY_B;  // 1=defection

			} else strategy[n][j] = rand()%3;       // strategy is assigned randomly to the others
		} 
	}

	// társadalmi vélemény mátrix beállítása
	for (a=0; a<3; a++) {
		for (j=0; j<2; j++) {
			// initialization of public_norm[a][j]: 0=good, 1=bad
			if (a==0 && j==0) public_norm[a][j] = PUBLIC_NORM_CG;
			if (a==0 && j==1) public_norm[a][j] = PUBLIC_NORM_CB;
			if (a==1 && j==0) public_norm[a][j] = PUBLIC_NORM_DG;
			if (a==1 && j==1) public_norm[a][j] = PUBLIC_NORM_DB;
			if (a==2 && j==0) public_norm[a][j] = PUBLIC_NORM_PG;
			if (a==2 && j==1) public_norm[a][j] = PUBLIC_NORM_PB;
		} 
	}
	
	// véletlenszerűen kiosztom a kezdeti hírnevet
	for (n=0; n<N; n++) {
		// initialization of public_reputation[n]: 0=good, 1=bad
		if ( (double) rand()/RAND_MAX<=INITIAL_GOOD) public_reputation[n] = 0;
		else public_reputation[n] = 1;
	}


	//  ********** Start of simulation ************

	for (gen=0; gen<GENERATION; gen++) {

   		// minden generáció elején kiirom a generáció számát és a frekvenciát
		printf("gen = %4d |", gen);

		for (a1=0; a1<3; a1++) {
			for (a2=0; a2<3; a2++) {
				printf(" %2.2f", frequency(strategy, a1, a2));
			}
		}
		
		// minden generáció elején lenullázom a számlálókat
		times_of_cooperation = 0;
		average_payoff = 0.0;


		for (steps=0; steps<N; steps++) {

			// minden lépés elött lenullázom a kifizetéséket
			for (n=0; n<N; n++) payoff[n] = 0.0;  

			// minden lépésben van 10 (ROUND) játék
			for (round=0; round<ROUND; round++) {    // start of one round
				
				// minden játék kör elején párba állítom az ágenseket

				for (n=0; n<N; n++) stack[n] = n;
				stack_length = N;

				for (pair=0; pair<N/2; pair++) {
					rd_int = rand()%stack_length;
					donor[pair] = stack[rd_int];
					stack[rd_int] = stack[stack_length-1];
					stack_length--;

					rd_int = rand()%stack_length;
					recipient[pair] = stack[rd_int];
					stack[rd_int] = stack[stack_length-1];
					stack_length--;
				}


				// minden egyes pár játszik egymással (a giving game and payoff calculation)
				for (pair=0; pair<N/2; pair++) {

					// error in recalling recipient’s reputation
					if ( (double) rand()/RAND_MAX<E_REP) { // if error occurs
						donor_action = strategy[donor[pair]][1-public_reputation[recipient[pair]]];
					} else {    // if error does not occur
						donor_action = strategy[donor[pair]][public_reputation[recipient[pair]]];
					}

					// error in executing intended action
					if ( (double) rand()/RAND_MAX<E_ACT) {   // if error occurs
					
						// one of the other two actions is taken randomly
						if ( (double) rand()/RAND_MAX<0.5) donor_action = (donor_action+1)%3;
						else donor_action = (donor_action+2)%3;
					}


					if (donor_action==0) times_of_cooperation++;

					// kifizetések
					switch (donor_action) {
						case 0: payoff[donor[pair]] -= C; payoff[recipient[pair]] += B; break;
						case 1: break;
						case 2: payoff[donor[pair]] -= ALPHA; payoff[recipient[pair]] -= BETA; break;
					}

					// a játék után firssül a donor hírneve
					public_reputation[donor[pair]] = public_norm[donor_action][public_reputation[recipient[pair]]];

					// error in assignment
					if ( (double) rand()/RAND_MAX<MU) {
						public_reputation[donor[pair]] = 1-public_reputation[donor[pair]];
					}
					
				} // end of game for each pair
			} //end of one step

			average_payoff += average_per_round(payoff);


			// **** reproduction: Moran process (UPDATE_MODE==2) ****

			minimum_to_zero(payoff);
			payoff_to_cdf(payoff);

			// chosing a dying individual
			n = rand()%N;

			// choosing a parent proportionally to payoff
			rd_double = (double) rand()/RAND_MAX;   // (p=0-1)
			reproduce = 0;

			while (rd_double>payoff[reproduce]) reproduce++;


			// inheritance of strategy
			for(j=0; j<2; j++) {
				strategy[n][j] = strategy[reproduce][j];
			}
			
			// mutation
			if ( (double) rand()/RAND_MAX<MUTATION_RATE) {

				for (j=0; j<2; j++) strategy[n][j] = rand()%3;
			}

			// assigning a new reputation to the new-born
			if ( (double) rand()/RAND_MAX<=INITIAL_GOOD) public_reputation[n] = 0;
			else public_reputation[n] = 1;
				
		} 

		// displaying C% and average payoff per generation
		printf(" | C%% = %5.3f", (double) times_of_cooperation/(ROUND*N*N/2));
		printf(" | payoff = %6.2f\n", (double) average_payoff/N);
	} // end of simulation


	// deleting dynamic memory
	delete[] payoff;
	
	for (n=0; n<N; n++) delete[] strategy[n];
	delete[] strategy; 

	for (a=0; a<3; a++) delete[] public_norm[a];
	delete[] public_norm;
	
	delete[] public_reputation;
	
	delete[] stack;
	delete[] donor;
	delete[] recipient;

	// a program lefutott hibakód nélkül...
	return (0);
} // end of the main
