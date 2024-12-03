
#ifndef ADORE_EXPORT_H
#define ADORE_EXPORT_H

#ifdef ADORE_STATIC_DEFINE
#  define ADORE_EXPORT
#  define ADORE_NO_EXPORT
#else
#  ifndef ADORE_EXPORT
#    ifdef Adore_EXPORTS
        /* We are building this library */
#      define ADORE_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define ADORE_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef ADORE_NO_EXPORT
#    define ADORE_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef ADORE_DEPRECATED
#  define ADORE_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef ADORE_DEPRECATED_EXPORT
#  define ADORE_DEPRECATED_EXPORT ADORE_EXPORT ADORE_DEPRECATED
#endif

#ifndef ADORE_DEPRECATED_NO_EXPORT
#  define ADORE_DEPRECATED_NO_EXPORT ADORE_NO_EXPORT ADORE_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef ADORE_NO_DEPRECATED
#    define ADORE_NO_DEPRECATED
#  endif
#endif

#endif /* ADORE_EXPORT_H */
