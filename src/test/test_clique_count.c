#include "clique_count.h"
#include "graph.h"
#include "stdio.h"
int main() {
	int* g;
	int gsize, i, j;
	ReadGraph("ce", &g, &gsize);
	PrintGraph(g, gsize);
	
	List *cache_6 = list_init(6);
	List *cache_7 = list_init(7);

	CliqueCountCreateCache(g, gsize, cache_6, cache_7);
	printf("# of clique_6:\t%d\n", cache_6->length);
	printf("# of clique_7:\t%d\n", cache_7->length);
/*	i = 40, j = 40;
	printf("Preflip (%d, %d)\n", i, j);
	g[i * gsize + j] = 1 - g[i * gsize + j];
	i = 30, j = 45;
	printf("Preflip (%d, %d)\n", i, j);
	g[i * gsize + j] = 1 - g[i * gsize + j];
	i = 40, j = 41;
	printf("Preflip (%d, %d)\n", i, j);
	g[i * gsize + j] = 1 - g[i * gsize + j];
	i = 50, j = 70;
	printf("Preflip (%d, %d)\n", i, j);
	g[i * gsize + j] = 1 - g[i * gsize + j];
	i = 70, j = 79;
	printf("Preflip (%d, %d)\n", i, j);
	g[i * gsize + j] = 1 - g[i * gsize + j];*/

	int count = CliqueCount(g, gsize);
	printf("# of clique_7 by normal:\t%d\n", count);
	printf("\n\n");
	count = CliqueCountUseCache(g, gsize, i, j, 10000, cache_6, cache_7);
	printf("# of clique_7 by cache:\t%d\n", count);
	free(g);
	return 0;
}
