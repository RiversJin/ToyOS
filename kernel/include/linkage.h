#ifndef LINKAGE_H
#define LINKAGE_H


#ifdef __ASSEMBLY__

    #ifndef ENTRY
    #define ENTRY(name) \
        .global name \
        name:
    #endif

    #ifndef WEAK
    #define WEAK(name) \
        .weak name \
        name:
    #endif

    #ifndef END
    #define END(name) \
        .size name, .-name
    #endif

    #ifndef ENDPROC
    #define ENDPROC(name) \
        .type name, @function \
        END(name)
    #endif

#endif

#endif /* LINKAGE_H */