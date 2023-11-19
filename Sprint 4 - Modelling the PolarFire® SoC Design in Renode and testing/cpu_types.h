#ifndef CPU_TYPES_H
#define CPU_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*------------------------------------------------------------------------------
 * addr_t: address type.
 * Used to specify the address of peripherals present in the processor's memory
 * map.
 */
typedef uint64_t addr_t;

/*------------------------------------------------------------------------------
 * psr_t: processor state register.
 * Used by HAL_disable_interrupts() and HAL_restore_interrupts() to store the
 * processor's state between disabling and restoring interrupts.
 */
typedef unsigned int psr_t;

#ifdef __cplusplus
}
#endif

#endif  /* CPU_TYPES_H */

