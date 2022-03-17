#ifndef BENCH_HPP
#define BENCH_HPP
#include <numeric>
#include <chrono>
#include <vector>
#include <limits>
inline unsigned long long _bm_nanoTime(){
    using namespace std;
    using namespace chrono;
    return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
}
inline uint64_t _bm_rdtsc(){
    uint64_t tick;
    unsigned c, d;
    asm volatile("rdtsc" : "=a" (c), "=d" (d));
    tick = (((uint64_t)c) | (((uint64_t)d) << 32));
    return tick;
}
struct benchmark_info{
    double meantime, vartime, meanclocks, varclocks;
    operator double(){
        return meantime;
    }
};
template<unsigned int run_length = 300, typename Function, typename... Ts>
benchmark_info bench_function(Function f, Ts&&... args){
    unsigned int since_best = 0;
    uint64_t best_time = std::numeric_limits<uint64_t>::max();
    uint64_t accum = 0;
    uint64_t accum_clocks = 0;
    double var_time;
    double var_clocks;
    while(true){
        auto t1 = _bm_nanoTime();
        uint64_t c1 = _bm_rdtsc();
        f(std::forward<Ts>(args)...);
        uint64_t c2 = _bm_rdtsc();
        auto t2 = _bm_nanoTime();
        if(t2 - t1 < best_time){
            best_time = t2 - t1;
            since_best = 0;
            accum = 0;
            accum_clocks = 0;
            accum_clocks += (c2 - c1);
            accum += (t2 - t1);
            since_best++;
            var_time = 0;
            var_clocks = 0;
        }
        else{
            since_best++;
            accum += (t2 - t1);
            accum_clocks += (c2 - c1);
            double delta_time2 = (t2 - t1) - double(accum) / since_best;
            double delta_clocks2 = (c2 - c1) - double(accum_clocks) / since_best;
            var_time += delta_time2 * delta_time2;
            var_clocks += delta_clocks2 * delta_clocks2;
        }
        if(since_best >= run_length){
            break;
        }
    }
    return benchmark_info{accum / double(run_length), var_time / double(run_length), accum_clocks / double(run_length), var_clocks / double(run_length)};
}

#endif
