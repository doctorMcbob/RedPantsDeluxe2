#ifndef BENCHMARKS
#define BENCHMARKS

#define STRING_MACHINE_MODE 0
#define DRAWING_MODE 1
#define COLLISION_MODE 2
#define INTERPERETER_MODE 3

void switch_benchmark_mode(int mode);
void print_benchmark_data();
int get_benchmark_mode();
#endif
