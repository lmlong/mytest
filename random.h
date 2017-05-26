#include <stdint.h>
#include <vector>

uint32_t random_seed();
int gen_uniform_int(int min, int max);
void shuffle_vecint(std::vector< int > &vecnums);
