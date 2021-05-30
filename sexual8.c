#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//#include <sys/time.h>
#include <time.h>
//#include "nrutil.h"
/* compile using 
 * gcc -g sexual8.c  poidev.c ran1.c -lm -o sexual8
 */

// #define RUNS 10
// #define ITERATES 11
#define GENERATIONS 101
#define MAX_SIZE 200000
#define INITIAL_SIZE 1000
// #define NO_LOCI 1
// #define NO_LOCI 2
// #define NO_LOCI 4
#define NO_LOCI 20
#define MUT_RATE 1.0e-08     // rate per locus
// #define RECOMB_RATE 0.5  // rate per individual
#define AVG_ALLELE_FREQ 0.01 // initial advantagous allele freq
#define FECUNDITY 2 // fecundity
// selection coefficent for beneficial allele 0.1


extern double ran1(long *iseed);
extern double poidev(double xm, long *idum);
extern double gammln(double xx);
extern void quicksort(int *list, int i, int j);
long *iseed, idum;
time_t tp;

struct {
   int sex; 
   double fitness;
   double relativeFitness;
   int maternal_allele[NO_LOCI];
   int paternal_allele[NO_LOCI];
} n[MAX_SIZE], m[MAX_SIZE], tmp, tmp0;
int  exceeded=1;
int  i, j, jj, k, l, iter, numberNew, gen, numberInd;
int  tmp_allele, save_k;
double arandom;
double fit_sexual;
double max;
double avgAlleles;
double sum;
int noHetero;
int noHomo;
int no_sexual, no_recomb=0;
int count, noGoodAlleles;
int maxJ;

void usage(void);

int main(int argc, char *argv[]) {
    iseed = &idum;
    idum = (unsigned) time(&tp);
    idum = -idum;
//    idum = -12345;
// *****   initialize   *******************************************
//    
    tmp0.sex = 0; tmp0.fitness=0.0; // set up a null value tmp0
    for(i=0; i<NO_LOCI; i++) { 
	tmp0.paternal_allele[i]=0; 
	tmp0.maternal_allele[i]=0; 
    }
    for(i=0; i<INITIAL_SIZE; i++) { 
	n[i].sex=0;                         // sexual is 0 
	n[i].fitness=1.0; 
	for(j=0; j<NO_LOCI; j++) {
	    count=0;
	    if(ran1(iseed) < AVG_ALLELE_FREQ) {
		n[i].paternal_allele[j]=1;  // advantagous is 1 else 0
		count++;
	    } else { n[i].paternal_allele[j]=0; }
	    if(ran1(iseed) < AVG_ALLELE_FREQ) {
		n[i].maternal_allele[j]=1;  // advantagous is 1 else 0
		count++;
	    } else { n[i].maternal_allele[j]=0; }
	    n[i].fitness=n[i].fitness*(1.0+ (double)count*0.1);  // multiplicative fitness; dominance
	}
    }
    numberInd=INITIAL_SIZE;
    gen =0; 
    // calculate avg sexual fitness
    fit_sexual=0.0;  
    max=0.0;
    no_sexual=0;
    for(j=0; j<numberInd; j++) { if(n[j].fitness > max) max = n[j].fitness; }
    for(j=0; j<numberInd; j++) { 
	n[j].relativeFitness = n[j].fitness/max;
        fit_sexual+=n[j].fitness; 
        no_sexual++;
    }
    fit_sexual=(double) fit_sexual/(double)no_sexual;
    max=0.0; 
    avgAlleles=0.0;
    for(j=0; j<numberInd; j++) { 
        sum=0; 
        for(k=0;k<NO_LOCI;k++) {
            sum+=n[j].maternal_allele[k];
            sum+=n[j].paternal_allele[k];
        }
        avgAlleles+=sum;
        if(n[j].fitness > max) { 
            max=n[j].fitness;
            noGoodAlleles=0; 
            noHetero=0; 
            noHomo=0;
            maxJ=j;
            for(k=0;k<NO_LOCI;k++) {
                noGoodAlleles+=n[j].maternal_allele[k];
                noGoodAlleles+=n[j].paternal_allele[k];
                if(n[j].maternal_allele[k]==1 && n[j].paternal_allele[k]==0) noHetero++; 
                if(n[j].maternal_allele[k]==0 && n[j].paternal_allele[k]==1) noHetero++; 
                if(n[j].maternal_allele[k]==1 && n[j].paternal_allele[k]==1) noHomo++; 
            } 
        } 
    } 
    avgAlleles=avgAlleles/numberInd;

    //---------------------------------------------
    fprintf(stdout,"\nSexual Model with selection (random number based on\n");
    arandom=MUT_RATE;
    fprintf(stdout,"relative fitness to determine who survives).  Mutation rate is %10.5g,\n",arandom);
    // arandom=RECOMB_RATE;
    // fprintf(stdout,"Recomb rate is ONE per sexual individual per generation\n");
    fprintf(stdout,"Starting values at generation %3d are; population size %6d,\n",gen, numberInd);
    fprintf(stdout,"with avg fitness of %10.5f.\n",fit_sexual);
    fprintf(stdout,"\n");


    fprintf(stdout,"gen  popSize    avgFit     maxFit maxAlleles avgAlleles \n");
    fprintf(stdout,"%3d  %6d  %10.5f %10.5f %6d %10.4f\n",gen,numberInd,fit_sexual,max,noGoodAlleles,avgAlleles);
    //---------------------------------------------
    
// *****   start body   *******************************************
//    
    for(gen=1; gen<GENERATIONS; gen++) {
	// MUTATE
	for(i=0; i<numberInd; i++) {
	    for(j=0; j<NO_LOCI; j++) {
		if(ran1(iseed) < MUT_RATE) n[i].paternal_allele[j]=0;
		if(ran1(iseed) < MUT_RATE) n[i].maternal_allele[j]=0;
	    }
	}
	// CALCULATE FITNESS
	// calculate avg sexual fitness
	fit_sexual=0.0;
	no_sexual=0;
	max=0.0;
	for(i=0; i<numberInd; i++) { 
	    n[i].fitness=1.0;
	    for(j=0; j<NO_LOCI; j++) { 
		count=0;
		if(n[i].paternal_allele[j]==1) count++;
		if(n[i].maternal_allele[j]==1) count++;
		n[i].fitness=n[i].fitness*(1.0+ (double)count*0.1);
	    }
	    if(n[i].fitness > max) max = n[i].fitness;
	}
	// make fitnesses relative 
	for(i=0; i<numberInd; i++) { 
            n[i].relativeFitness = n[i].fitness/max;
            fit_sexual+=n[i].fitness; 
            no_sexual++;
	}
	if(no_sexual>0) { fit_sexual=fit_sexual/no_sexual; } else fit_sexual=0;
        max=0.0; 
        avgAlleles=0.0;
        for(j=0; j<numberInd; j++) { 
            sum=0; 
            for(k=0;k<NO_LOCI;k++) {
                sum+=n[j].maternal_allele[k];
                sum+=n[j].paternal_allele[k];
            }
            avgAlleles+=sum;
            if(n[j].fitness > max) { 
                max=n[j].fitness;
                noGoodAlleles=0; 
                noHetero=0; 
                noHomo=0;
                maxJ=j;
                for(k=0;k<NO_LOCI;k++) {
                    noGoodAlleles+=n[j].maternal_allele[k];
                    noGoodAlleles+=n[j].paternal_allele[k];
                    if(n[j].maternal_allele[k]==1 && n[j].paternal_allele[k]==0) noHetero++; 
                    if(n[j].maternal_allele[k]==0 && n[j].paternal_allele[k]==1) noHetero++; 
                    if(n[j].maternal_allele[k]==1 && n[j].paternal_allele[k]==1) noHomo++; 
                } 
            } 
        }
        avgAlleles=avgAlleles/numberInd;
        fprintf(stdout,"%3d  %6d  %10.5f %10.5f %6d %10.4f\n",gen,numberInd,fit_sexual,max,noGoodAlleles,avgAlleles);
	//
	//
	// SURVIVAL (survive if relative-w > random #)
	count=numberInd;
	for(i=0, j=0; i<count; i++) { 
	    if(n[i].relativeFitness > ran1(iseed)) {
		n[j]=n[i];
		j++;
	    } else { numberInd--; }
	}

	// RECOMBINE  
        // Apr 2021 -- no recombination
        // Apr 2021 -- rather completely unlinked
	// Apr 2021 // for(i=0; i<numberInd; i++) {
	// Apr 2021 //     if(n[i].sex==0) { // sexual
	// Apr 2021 //	       k=(int) (NO_LOCI-1)*ran1(iseed); // choose a spot for recomb
	// Apr 2021 //	       for(j=k; j<NO_LOCI; j++) {
	// Apr 2021 //	           tmp_allele=n[i].paternal_allele[j];
	// Apr 2021 //	           n[i].paternal_allele[j]=n[i].maternal_allele[j];
	// Apr 2021 //	           n[i].maternal_allele[j]=tmp_allele;
	// Apr 2021 //	       }
	// Apr 2021 //     }
	// Apr 2021 // }
	//
	//
	// MATE
	// at random choose two sexual ind's mix/match
	numberNew=0;
	for(i=0; i<numberInd; i++) { 
	    if(n[i].sex==0) { // choose a new sexual from gamete union
		for(jj=numberNew; jj<numberNew+FECUNDITY; jj++) {
		    m[jj]=n[i]; // start it off the same; to retain half the compliment
		    k = (int) numberInd*ran1(iseed);
		    save_k = k; // remember the value
		    while(k<numberInd) {
			if(k!=i) { // no self mating!
                            // i and k mate. Choosing genes with
                            // unlinked 20 loci. Arbitrarily we can
                            // call the i^th individual the paternal
                            // individual and the k^th individual
                            // the maternal.
                            if(n[k].sex==0) {
                                for(j=0; j<NO_LOCI; j++) {
                                    arandom=ran1(iseed);
                                    if(arandom < 0.5) {
                                        m[jj].paternal_allele[j]=n[i].paternal_allele[j];
                                    } else {                            
                                        m[jj].paternal_allele[j]=n[i].maternal_allele[j];
                                    }
                                }
                                for(j=0; j<NO_LOCI; j++) {
                                    arandom=ran1(iseed);
                                    if(arandom < 0.5) {
                                        m[jj].maternal_allele[j]=n[k].paternal_allele[j];
                                    } else {                            
                                        m[jj].maternal_allele[j]=n[k].maternal_allele[j];
                                    }
                                }
                                break;
                            }
                        }
                        k++;
                        if(k>=numberInd) k=0;
                        if(k==save_k) break; // no other sexuals
                    }
                }
                numberNew=jj;
            } else { printf("Have a problem here\n"); exit(0); }
        }
        // Now have new collection of m genotypes, reset to n
        numberInd=numberNew;
        for(i=0; i<numberInd; i++) { n[i]=m[i]; }
        //
        //
        // SELECT A NEW POP BY SAMPLE WITHOUT REPLACMENT
        // Do this via a permutation of all samples
        // and pick the first n.
        //
        // Permute
        for(i=0; i<numberInd; i++) { 
            j = (int) numberInd * ran1(iseed);
            tmp = n[i];
            n[i] = n[j];
            n[j] = tmp;
        }
        if(numberInd > INITIAL_SIZE) { 
            for(i=INITIAL_SIZE+1; i<numberInd; i++) { n[i]=tmp0; } // any beyond are null
            numberInd=INITIAL_SIZE; 
        } 
    }
}

void usage(void) {
    fprintf(stderr,"\n");
    fprintf(stderr," This program does drift-over-time with \n");
    fprintf(stderr," with viability selection on individuals.\n");
    fprintf(stderr,"\n");
}
