#include "proc.h"
#include "param.h"

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
