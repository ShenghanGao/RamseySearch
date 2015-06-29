#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "list.h"
#include "fifo.h"	/* for taboo list */
#include "time.h"
#include "clique_count.h"
#include "search.h"
#include "graph.h"
#include "recursion.h"
#include "client_transfer.h"
#define TABOOSIZE (500)
#define BIGCOUNT (1000)
#define SA_THRESHOLD (90)
/*
 * example of very simple search for R(7, 7) counter 
 * examples starts with a small randomized graph and 
 * works its way up to successively larger graphs one 
 * at a time
 * uses a taboo list of size #TABOOSIZE# to hold and 
 * encoding of and edge (i, j) + clique_count
 */

int BFsearch(int *g, int gsize) {
	int count, i, j, best_count = BIGCOUNT,
			best_i, best_j;
	int *new_g;
	void *taboo_list = FIFOInitEdge(TABOOSIZE);
	// best_ever is the goal we want to achieve,
	// it is lower than THRESHOLD
	CliqueCountCreateCache(g, gsize);
	// see how many clique 7 we have left. If there are still 
	// a lot, we will need to clean it up.
	if(cache_7.length == 0) {
		printf("Eureka! Counter-example found!\n");
		new_g = (int *) malloc((gsize + 1) * (gsize + 1) * sizeof(int));
		// copy the old graph into the new graph leaving 
		// the last row and last column alone
		CopyGraph(g, gsize, new_g, gsize + 1);
		// throw away the old graph and make new one 
		free(g);
		g = new_g;
		// randomly assigned value for last column
		for(i = 0; i < gsize + 1; i ++) {
			if(rand() % 100 > 50) {
				new_g[i * (gsize + 1) + gsize] = 1; // last column
				new_g[gsize * (gsize + 1) + i] = 1;	
			}
			else {
				new_g[i * (gsize + 1) + gsize] = 0; // last column
				new_g[gsize * (gsize + 1) + i] = 0;	
			}
		}
		gsize = gsize + 1;
		CliqueCountCreateCache(g, gsize);
		while(cache_7.length > best_ever) {
		// reset the taboo list for the new graph
			for(i = 0; i < gsize; i ++) {
				for(j = i + 1; j < gsize; j ++) {
      		g[i * gsize + j] = 1 - g[i * gsize + j];
					count = CliqueCountUseCache(g, gsize, i, j, best_count);
					if(count != -1 && !FIFOFindEdgeCount(taboo_list, i, j, count)) {
    	  		best_count = count;
          	best_i = i;
          	best_j = j;
					}
        	g[i * gsize + j] = 1 - g[i * gsize + j];
				}
			}
			if(recv_flag) {
				FIFODelete(taboo_list); 
				return (0);	 
			}
			printf("BACKTRACKING size: %d, best_6_count: %d, best_count: %d, best edge: (%d, %d), new color: %d\n",
					 gsize, cache_6.length, best_count, best_i, best_j, 
					 g[best_i * gsize + best_j]); 
			// keep the best flip we saw. 
			g[best_i * gsize + best_j] = 1 - g[best_i * gsize + best_j];
			CliqueCountCreateCache(g, gsize);
			FIFOInsertEdgeCount(taboo_list, best_i, best_j, best_count);
			best_count = BIGCOUNT;
			PrintGraph(g, gsize);
		}
	} 
	if(cache_7.length <= best_ever) {
		if(!compare_graph(g, gsize)) {
			PrintGraphNew(g, gsize);
			send_file(ip_addr);
		}
		//return (0);
	}
	for(i = 0; i < gsize; i ++) {
		for(j = i + 1; j < gsize; j ++) {
     	g[i * gsize + j] = 1 - g[i * gsize + j];
			count = CliqueCountUseCache(g, gsize, i, j, best_count);
			if(count != -1 && !FIFOFindEdgeCount(taboo_list, i, j, count)) {
    	 	best_count = count;
        best_i = i;
        best_j = j;
			}
      g[i * gsize + j] = 1 - g[i * gsize + j];
		}
	}
	recursiveSearch(g, gsize, RECURSION_DEPTH, best_ever, 
					best_i, best_j, taboo_list);
	FIFODelete(taboo_list);
	free(g); 
	return (0);
}

int DFsearch(int *g, int gsize) {
	int count, i, j, best_count = BIGCOUNT, best_i, best_j;
	void *taboo_list = FIFOInitEdge(TABOOSIZE);
	int last_i, last_j;

	CliqueCountCreateCache(g, gsize);
	// this is the first phase of DFsearch, that is to find
	// a good graph whose clique count is less than best_ever
	while(!recv_flag && cache_7.length > best_ever) {
		// do the second round of clean up by breaking the ties
		// since best_count is initialized to BIGCOUNT, we won't
		// have 
		for(i = 0; i < gsize; i ++) {
			for(j = i + 1; j < gsize; j ++) {
        g[i * gsize + j] = 1 - g[i * gsize + j];
				count = CliqueCountUseCache(g, gsize, i, j, best_count);
			//	if(count != -1 && (count <= best_count || rand() % 100 > SA_THRESHOLD + (count - best_ever))
			//			&& !FIFOFindEdgeCount(taboo_list, i, j, count)) {
				if(count != -1 && count <= best_count && !FIFOFindEdgeCount(taboo_list, i, j, count)) {
    	    best_count = count;
          best_i = i;
          best_j = j;
				}
        g[i * gsize + j] = 1 - g[i * gsize + j];
			}
		} 
							
		printf("BACKTRACKING size: %d, best_6_count: %d, best_count: %d, best edge: (%d, %d), new color: %d\n",
					 gsize, cache_6.length, best_count, best_i, best_j, 
					 g[best_i * gsize + best_j]); 
		// keep the best flip we saw.
		g[best_i * gsize + best_j] = 1 - g[best_i * gsize + best_j];
		FIFOInsertEdgeCount(taboo_list, best_i, best_j, best_count);
		CliqueCountCreateCache(g, gsize);
		best_count = BIGCOUNT;
		PrintGraph(g, gsize);
	}
	// this is the second phase of DFSearch, that is to 
	// generate as many seed as possible
	while(!recv_flag) {
		CliqueCountCreateCache(g, gsize);
		// see how many clique 7 we have left. If there are still 
		// a lot, we will need to clean it up.
		if(cache_7.length <= best_ever) {
			if(!compare_graph(g, gsize)) {
				PrintGraphNew(g, gsize);
				if(send_file(ip_addr) != 0)
					printf("Failed to send graph!\n");
			}
		}
		// do the second round of clean up by breaking the ties
		// since best_count is initialized to BIGCOUNT, we won't
		// have 
		for(i = 0; i < gsize; i ++) {
			for(j = i + 1; j < gsize; j ++) {
        g[i * gsize + j] = 1 - g[i * gsize + j];
				count = CliqueCountUseCache(g, gsize, i, j, best_count + 10);
				if(count != -1 && last_i != i && last_j != j && 
					(count <= best_count || rand() % 100 > SA_THRESHOLD + (count - best_ever))
						&& !FIFOFindEdgeCount(taboo_list, i, j, count)) {
				//if(count != -1 && last_i != i && last_j != j 
				//	&& (count <= best_count) && !FIFOFindEdgeCount(taboo_list, i, j, count)) {
    	    best_count = count;
          best_i = i;
          best_j = j;
				}
        g[i * gsize + j] = 1 - g[i * gsize + j];
			}
		} 
							
		printf("BACKTRACKING size: %d, best_6_count: %d, best_count: %d, best edge: (%d, %d), new color: %d\n",
					 gsize, cache_6.length, best_count, best_i, best_j, 
					 g[best_i * gsize + best_j]); 
		// keep the best flip we saw. 
		last_i = best_i;
		last_j = best_j;
		g[best_i * gsize + best_j] = 1 - g[best_i * gsize + best_j];
		FIFOInsertEdgeCount(taboo_list, best_i, best_j, best_count);
		best_count = BIGCOUNT;
		PrintGraph(g, gsize);
	}
	FIFODelete(taboo_list);
	free(g); 
	return (-1);
}

bool compare_graph(int *g, int gsize) {
	int *old_g;
	int old_gsize, i, j;
	int diff_count = 0;
	if(!ReadGraph("../../file/client/new_graph", &old_g, &old_gsize))
		return false;
	if(gsize != old_gsize) return false;
	for(i = 0; i < gsize; i ++) {
		for(j = i + 1; j < gsize; j ++) {
			if(g[i * gsize + j] != old_g[i * gsize + j]) {
				diff_count ++;
				if(diff_count > 2)
					return false;
			}
		}
	}
	free(old_g);
	printf("duplicate graph!\n");
	return true;
}

int *load_graph(int* gsize) {
	int *g;
	ReadGraph("../../file/client/old_graph", &g, gsize);
	return g;
}
