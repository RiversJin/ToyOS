#ifndef COMPILER_ATTRIBUTES_H
#define COMPILER_ATTRIBUTES_H

#define   noinline                      __attribute__((__noinline__))
#define __always_inline                 inline __attribute__((__always_inline__))

typedef __u8  __attribute__((__may_alias__))  __u8_alias_t;
typedef __u16 __attribute__((__may_alias__)) __u16_alias_t;
typedef __u32 __attribute__((__may_alias__)) __u32_alias_t;
typedef __u64 __attribute__((__may_alias__)) __u64_alias_t;

static __always_inline void __read_once_size(const volatile void *p, void *res, int size)
{
	switch (size) {
	case 1: *(__u8_alias_t  *) res = *(volatile __u8_alias_t  *) p; break;
	case 2: *(__u16_alias_t *) res = *(volatile __u16_alias_t *) p; break;
	case 4: *(__u32_alias_t *) res = *(volatile __u32_alias_t *) p; break;
	case 8: *(__u64_alias_t *) res = *(volatile __u64_alias_t *) p; break;
	default:
		barrier();
		__builtin_memcpy((void *)res, (const void *)p, size);
		barrier();
	}
}

static __always_inline void __write_once_size(volatile void *p, void *res, int size)
{
	switch (size) {
	case 1: *(volatile  __u8_alias_t *) p = *(__u8_alias_t  *) res; break;
	case 2: *(volatile __u16_alias_t *) p = *(__u16_alias_t *) res; break;
	case 4: *(volatile __u32_alias_t *) p = *(__u32_alias_t *) res; break;
	case 8: *(volatile __u64_alias_t *) p = *(__u64_alias_t *) res; break;
	default:
		barrier();
		__builtin_memcpy((void *)p, (const void *)res, size);
		barrier();
	}
}

#define READ_ONCE(x)					\
({							\
	union { typeof(x) __val; char __c[1]; } __u =	\
		{ .__c = { 0 } };			\
	__read_once_size(&(x), __u.__c, sizeof(x));	\
	__u.__val;					\
})

#define WRITE_ONCE(x, val)				\
({							\
	union { typeof(x) __val; char __c[1]; } __u =	\
		{ .__val = (val) }; 			\
	__write_once_size(&(x), __u.__c, sizeof(x));	\
	__u.__val;					\
})


#endif // COMPILER_ATTRIBUTES_H