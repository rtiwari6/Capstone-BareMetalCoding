#ifndef HAL_ASSERT_HEADER
#define HAL_ASSERT_HEADER


#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * ASSERT() implementation.
 ******************************************************************************/
#if defined ( __GNUC__ )

#define ASSERT(CHECK) \
    do { \
        if (!(CHECK)) \
        { \
            __asm volatile ("ebreak"); \
        }\
    } while(0);

#else
/* Disable assertions if we do not recognize the compiler. */
#define ASSERT(CHECK)

#endif

#if defined(NDEBUG)
/***************************************************************************//**
 * HAL_ASSERT() is defined out when the NDEBUG symbol is used.
 ******************************************************************************/
#define HAL_ASSERT(CHECK)

#else
/***************************************************************************//**
 * Default behaviour for HAL_ASSERT() macro:
 *------------------------------------------------------------------------------
  The behaviour is toolchain specific and project setting specific.
 ******************************************************************************/
#define HAL_ASSERT(CHECK)     ASSERT(CHECK);

#endif  /* NDEBUG */

#ifdef __cplusplus
}
#endif

#endif  /* HAL_ASSERT_HEADER */

