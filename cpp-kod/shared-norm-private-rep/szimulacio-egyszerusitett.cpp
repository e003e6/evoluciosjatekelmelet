#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// MODE = 2: shared norm, private reputation
// UPDATE_MODE = 2: Morgen process reprodukcio

// ------------- constanses -------------

const int N = 100;
const int GENERATION = 20;
const int ROUND = 10;

// public norm : 0=good, 1=bad
const int PUBLIC_NORM_CG = 0;
const int PUBLIC_NORM_CB = 1;
const int PUBLIC_NORM_DG = 1;
const int PUBLIC_NORM_DB = 0;
const int PUBLIC_NORM_PG = 1;
const int PUBLIC_NORM_PB = 1;

// major strategy at the beginning: 0=cooperation, 1=defection, 2=punishment
const int MAJOR_STRATEGY_NUMBER = 80;
const int MAJOR_STRATEGY_G = 0;
const int MAJOR_STRATEGY_B = 1;

// initial fraction of good players
const double INITIAL_GOOD = 0.5;
const int INITIAL_CORRELATION = 0;    // correlation in intial private reputation: 0=no, 1=yes

// communication round (for mode 2)
const int COMMUNICATION = 100;        // average number of communication rounds per individual
const int COM_MODE = 1;               // mode of communication : 1=believe all, 2=believe the good

// payoff parameters
const double B = 9.0;
const double C = 3.0;
const double ALPHA = 1.0;
const double BETA = 4.0;

// accumulating errors
const double MU = 0.02;       // error rate in assigining reputation (accumulating; public in mode 1 and private in modes 2 and 3)

// temporal errors
const double E_ACT = 0.00;      // error rate in executing intended action
const double E_REP = 0.00;      // error rate in recalling recipient’s reputation (temporal, not accumulating)

// mutation rates
const double MUTATION_RATE = 0.01;   // mutation rate in action rules (and in social norms for mode 3)



// ------------- functions -------------

// displaying public_norm
void display_public_norm(int** public_norm) {
	int a, j;

	printf("-------\n");
	printf("    G B <-- PUBLIC NORM\n");

	for (a=0; a<3; a++) {
		switch(a) {
			case 0: printf("C : ") ; break;
			case 1: printf("D : ") ; break;
			case 2: printf("P : ") ; break;
		}

	    for (j=0; j<2; j++) {
	      	switch(public_norm[a][j]) {
	        	case 0: printf("G ") ; break;
	        	case 1: printf("B ") ; break;
	        }
		}

		printf("\n") ;
   }

   printf("-------\n") ;
}


// calculating frequency of a given strategy
double frequency(int** strategy, int a1, int a2) {
	int n;
	int total=0;

	for (n=0; n<N; n++) {
		if (strategy[n][0]==a1&&strategy[n][1]==a2) total++;
	}

	return ((double)total/N);
}


// calculating average payoff per round
double average_per_round(double* payoff) {
	int n;
	double total=0.0;

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

	if(total>0.0) {
		for (n=0; n<N; n++) payoff[n] = payoff[n]/total;
		for (n=1; n<N; n++) payoff[n] = payoff[n-1]+payoff[n];
	} else {
		for (n=0; n<N; n++) payoff[n] = (double)(n+1)/N;
	}

	payoff[N-1] = 1.0;
}



// -------------- MAIN --------------

int main(void) {
	int steps, total_steps, round;
	int l, m, n, j, a, a1, a2, reproduce, pair;
	int stack_length;
	int donor_action;
	int times_of_cooperation;
	double average_payoff;
	int rd_int;                 // random variable (int)
	double rd_double;           // random variable (double)

	double *payoff;             // payoff[n] = player n’s payoff
	int **strategy;             // strategy[n][j] = player n’s action towards j
	int **public_norm;          // public_norm[a][j] = evaluation of action a towards j
	int ***norm;                // norm[n][a][j] = player n’s evaluation of action a towards j
	int *public_reputation;     // public_reputation[n] = player n’s public reputation
	int **reputation;           // reputation[m][n] = player n’s reputation in the eyes of m


	// for donor-recipient matching
	int *stack;
	int *donor;                 // donor[pair]: the donor in #pair
	int *recipient;             // recipient[pair]: the recipient in #pair

	int *parent;
	int **old_strategy;
	int ***old_norm;
	

	// seed for a random variable
	srand((unsigned)time(NULL));


	// dynamic memory allocation

	payoff = new double [N];
	strategy = new int* [N]; 
	old_strategy = new int* [N];

	for(n=0; n<N; n++) {
		strategy[n] = new int [2]; 
		old_strategy[n] = new int [2];
	}

	public_norm = new int* [3];
	for (a=0; a<3; a++) public_norm[a] = new int[2];
	
	reputation = new int*[N];
	for (m=0; m<N; m++) reputation[m] = new int[N];
	
	stack = new int[N];
	donor = new int[N/2];
	recipient = new int[N/2];
	parent = new int[N];



	// ******** initialization of variables (for a whole simulation) ********

	for(n=0; n<N; n++) {
		for(j=0; j<2; j++) {
			// initialization of strategy[n][j]: 0=cooperation, 1=defection; 2=punishment
			if (n<MAJOR_STRATEGY_NUMBER) {     // for major strategy
				if(j==0) strategy[n][j] = MAJOR_STRATEGY_G;
				if(j==1) strategy[n][j] = MAJOR_STRATEGY_B;
			} else strategy[n][j] = rand() %3;       // strategy is assigned randomly to the others
		} 
	}

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


	// ******** initialization of reputation  *********

	if (INITIAL_CORRELATION==0) {      // without initial correlation
		for (m=0; m<N; m++) {
			for (n=0; n<N; n++) {
				// initialization of reputation[m][n] (how m thinks n): 0=good, 1=bad
				if ( (double) rand()/RAND_MAX<=INITIAL_GOOD) reputation[m][n] = 0;
				else reputation[m][n] = 1;
			} 
		}
	}

	if(INITIAL_CORRELATION==1) {  // with initial correlation
		for(n=0; n<N; n++) {
			// initialization of reputation[m][n] (how m thinks n): 0=good, 1=bad
			if ( (double) rand()/RAND_MAX<=INITIAL_GOOD) {
				for (m=0; m<N; m++) reputation[m][n] = 0;
			} else {
				for (m=0; m<N; m++) reputation[m][n] = 1;
			}
		} 
	}
	

	//  ********** start of simulation ************
	printf("           |  CC   CD   CP   DC   DD   DP   PC   PD   PP | \n");

	total_steps = GENERATION*N;

	for (steps=0; steps<total_steps; steps++) {

		// ********** displaying generation & frequencies of strategies at the begininng of generation **********
		
		if (steps%N==0) {
			{
			printf("gen = %4d |", steps/N);
			for (a1=0; a1<3; a1++) {
				for (a2=0; a2<3; a2++) {
					printf(" %2.2f", frequency(strategy, a1, a2));
				}
			} 
			}
		} 

		// ******** initialization of variables (for each generation) ********
		if (steps % N==0) {
			times_of_cooperation = 0;
			average_payoff = 0.0;
		}


		// ******** initialization of variables (for each step) ********

		for (n=0; n<N; n++) payoff[n] = 0.0;

		for (round=0; round<ROUND; round++) {    // start of one round
			
			// ******** donor-recipient pair matching ********

			for (n=0; n<N; n++) stack[n] = n;
			stack_length = N;

			for (pair=0; pair<N/2; pair++) {
				rd_int = rand() % stack_length;
				donor[pair] = stack[rd_int];
				stack[rd_int] = stack[stack_length-1];
				stack_length--;
				rd_int = rand()%stack_length;
				recipient[pair] = stack[rd_int];
				stack[rd_int] = stack[stack_length-1];
				stack_length--;
			}


			// ******** a game in each pair ********

			for (pair=0; pair<N/2; pair++) {

				// ******** a giving game and payoff calculation ********

				// error in recalling recipient’s reputation
				if ( (double) rand()/RAND_MAX<E_REP) { // if error occurs
					donor_action = strategy[donor[pair]][1-reputation[donor[pair]][recipient[pair]]];

				} else {    // if error does not occur
					donor_action=strategy[donor[pair]][reputation[donor[pair]][recipient[pair]]];
				}

				// error in executing intended action
				if ( (double) rand()/RAND_MAX<E_ACT) {   // if error occurs
				
					// one of the other two actions is taken randomly
					if ( (double) rand()/RAND_MAX<0.5) donor_action = (donor_action + 1) % 3;
					else donor_action = (donor_action + 2) % 3;
				}

				if (donor_action==0) times_of_cooperation++;

				switch(donor_action) {
					case 0: payoff[donor[pair]] -= C; payoff[recipient[pair]] += B; break;
					case 1: break;
					case 2: payoff[donor[pair]] -= ALPHA; payoff[recipient[pair]] -= BETA; break;
				}


				// ******** updating reputation********
				
				for (m=0; m<N; m++) {
					reputation[m][donor[pair]] = public_norm[donor_action][reputation[m][recipient[pair]]];
					// error in assignment
					if ( (double) rand()/RAND_MAX<MU) {
						reputation[m][donor[pair]] = 1-reputation[m][donor[pair]];
					}
				}
				
			} 
			// end of game for each pair


			// ***** communication round *****

			for (j=0; j<N*COMMUNICATION; j++) {
				l = rand()%N;
				m = rand()%N;
				n = rand()%N;
				
				if (COM_MODE==1) {
					reputation[l][n] = reputation[m][n];  // l immitates m’s opinion on n
				}
		
				if (COM_MODE==2) {
					if (reputation[l][m]==0) reputation[l][n] = reputation [m][n];
					// l immitates m’s opinion on n, only when l thinks m is good
				}
			} 
			
		} 
		//end of one step

		average_payoff += average_per_round(payoff);


		// displaying C% and average payoff per generation

		if (steps%N==N-1) {
			printf(" | C%% = %5.3f", (double)times_of_cooperation/(ROUND*N*N/2));
			printf(" | payoff = %6.2f\n", (double)average_payoff/N);
		}

		// reproduction
		minimum_to_zero(payoff);
		payoff_to_cdf(payoff);


		// Moran process

		// chosing a dying individual (n)
		n = rand()%N;
		// choosing a parent proportionally to payoff (reproduce)
		rd_double = (double) rand()/RAND_MAX;
		reproduce = 0;

		while(rd_double>payoff[reproduce]) reproduce++;

		// ********** inheritance of strategy & norm **********

		for(j=0; j<2; j++) {
			strategy[n][j] = strategy[reproduce][j];
		}
		
		// mutation
		if ( (double) rand()/RAND_MAX<MUTATION_RATE) {
			for (j=0; j<2; j++) strategy[n][j] = rand()%3;
		}

		// assigning a new reputation to the new-born

		if (INITIAL_CORRELATION==0) {   // without initial correlation
			for (m=0; m<N; m++) {
				// initialization of reputation[m][n] (how m thinks n): 0=good, 1=bad
				if ( (double) rand()/RAND_MAX<=INITIAL_GOOD) reputation[m][n] = 0;
				else reputation[m][n] = 1;
			} 
		}

		if (INITIAL_CORRELATION==1) {  // with initial correlation
		
			// initialization of reputation[m][n] (how m thinks n): 0=good, 1=bad
			if ( (double) rand()/RAND_MAX<=INITIAL_GOOD) {

				for(m=0; m<N; m++) reputation[m][n]=0;
			} else {
				for(m=0; m<N; m++) reputation[m][n]=1;
			}
		} 
			
	} 
	// end of simulation

	display_public_norm(public_norm);


	// deleting dynamic memory
	delete[] payoff;
	
	for(n=0; n<N; n++) {
		delete[] strategy[n]; 
		delete[] old_strategy[n];
	}

	delete[] strategy; 
	delete[] old_strategy;

	for(a=0; a<3; a++) delete[] public_norm[a];
	delete[] public_norm;

	for (m=0; m<N; m++) delete[] reputation[m];
	delete[] reputation;
	
	delete[] stack;
	delete[] donor;
	delete[] recipient;
	delete[] parent;
}
// end of the main
