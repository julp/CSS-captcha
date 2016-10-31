#pragma once

#ifdef __GNUC__
# define GCC_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
#else
# define GCC_VERSION 0
#endif /* __GNUC__ */

#ifndef __has_attribute
# define __has_attribute(x) 0
#endif /* !__has_attribute */

#ifndef __has_builtin
# define __has_builtin(x) 0
#endif /* !__has_builtin */

#if GCC_VERSION || __has_attribute(deprecated)
# define DEPRECATED __attribute__((deprecated))
#else
# define DEPRECATED
#endif /* DEPRECATED */

#if GCC_VERSION || __has_attribute(unused)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#else
# define UNUSED
#endif /* UNUSED */

#if GCC_VERSION >= 3004 || __has_attribute(warn_unused_result)
# define WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
# define WARN_UNUSED_RESULT
#endif /* WARN_UNUSED_RESULT */

#if GCC_VERSION >= 2004 || __has_attribute(const)
# define CONST __attribute__((const))
#else
# define CONST
#endif /* CONST */

#if GCC_VERSION >= 2004 || __has_attribute(pure)
# define PURE __attribute__((pure))
#else
# define PURE
#endif /* PURE */

#if GCC_VERSION >= 2096 || __has_attribute(malloc)
# define MALLOC __attribute__((malloc))
#else
# define MALLOC
#endif /* MALLOC */

#if GCC_VERSION >= 4003
# define ALLOC_SIZE(...) __attribute__((alloc_size(__VA_ARGS__)))
#else
# define ALLOC_SIZE(...)
#endif /* ALLOC_SIZE */

#if (GCC_VERSION >= 3003 || __has_attribute(nonnull))
# define NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#else
# define NONNULL(...)
#endif /* NONNULL */

#if GCC_VERSION || __has_attribute(sentinel)
# define SENTINEL __attribute__((sentinel))
#else
# define SENTINEL
#endif /* SENTINEL */

#if GCC_VERSION || __has_attribute(deprecated)
# define DEPRECATED __attribute__((deprecated))
#else
# define DEPRECATED
#endif /* DEPRECATED */

#if GCC_VERSION >= 2003 || __has_attribute(format)
# define FORMAT(archetype, string_index, first_to_check) __attribute__((format(archetype, string_index, first_to_check)))
# define PRINTF(string_index, first_to_check) FORMAT(__printf__, string_index, first_to_check)
#else
# define FORMAT(archetype, string_index, first_to_check)
# define PRINTF(string_index, first_to_check)
#endif /* FORMAT,PRINTF */

#if __has_builtin(__builtin_expect)
# define EXPECTED(condition)   __builtin_expect(!!(condition), 1)
# define UNEXPECTED(condition) __builtin_expect(!!(condition), 0)
#else
# define EXPECTED(condition)   (condition)
# define UNEXPECTED(condition) (condition)
#endif /* __builtin_expect */
