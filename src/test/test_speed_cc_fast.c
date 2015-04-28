#include "clique_count.h"
#include "graph.h"
#include "stdio.h"
int main() {
	int* g;
	int gsize;
	int i, j, count, best_count = 10000000, best_i, best_j;
	int flag = 0;
	List *cache_6, *cache_7;
	ReadGraph("ce", &g, &gsize);
	PrintGraph(g, gsize);
	i = 70, j = 80;
	printf("Preflip (%d, %d)\n", i, j);
	g[i * gsize + j] = 1 - g[i * gsize + j];
	i = 69, j = 70;
	printf("Preflip (%d, %d)\n", i, j);
	g[i * gsize + j] = 1 - g[i * gsize + j];
	i = 64, j = 79;
	printf("Preflip (%d, %d)\n", i, j);
	g[i * gsize + j] = 1 - g[i * gsize + j];
	i = 50, j = 70;
	printf("Preflip (%d, %d)\n", i, j);
	g[i * gsize + j] = 1 - g[i * gsize + j];
	i = 70, j = 79;
	printf("Preflip (%d, %d)\n", i, j);
	g[i * gsize + j] = 1 - g[i * gsize + j];

	while(best_count != 0) {
		cache_6 = list_init(6);
		cache_7 = list_init(7);
		CliqueCountCreateCache(g, gsize, cache_6, cache_7);
		for(i = 0; i < 80; i ++) {
			for(j = i + 1; j < 80; j ++) {
				g[i * gsize + j] = 1 - g[i * gsize + j];
				count = CliqueCountUseCache(g, gsize, i, j, best_count, cache_6, cache_7);
				printf("(%d, %d) count = %d\n", i, j, count);
				if(count != -1) {
					best_count = count;
					best_i = i;
					best_j = j;
				}
				g[i * gsize + j] = 1 - g[i * gsize + j];
			}
		}	
		g[best_i * gsize + best_j] = 1 - g[best_i * gsize + best_j];
		printf("Flip (%d, %d) bestcount = %d \n", i, j, best_count);
		list_delete(cache_6);
		list_delete(cache_7);
	}
	free(g);
	return 0;
}
