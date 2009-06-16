
#ifndef STDBOOL_H
#define STDBOOL_H

#ifdef __cplusplus
  #define _Bool     bool
  #define true      true
  #define false     false
#else
  #define bool      _Bool
  #define true      1
  #define false     0
#endif  /* __cplusplus */

#define __bool_true_false_are_defined 1

#endif /* STDBOOL_H */


