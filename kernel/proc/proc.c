#include "proc.h"
#include "include/param.h"

struct cpu cpus[NCPU];

static volatile uint64_t* _spintable = (uint64_t*)PA2VA(0xD8);
extern void _entry();

void init_awake_ap_by_spintable(){
    for(int i = 1; i < NCPU; ++i){
        _spintable[i] = (uint64_t)VA2PA(&_entry);
    }
    asm volatile(
        "dsb st \n\t"
        "sev"
    );
}


struct cpu* mycpu(void){
    return &cpus[cpuid()];
}

void init_cpu_info(){
    for(int i = 0; i < NCPU; ++i){
        cpus[i].depth_spin_lock = 0;
        cpus[i].cpuid = i;
    }
}