/* AIX requires this to be the first thing in the file.  */
#ifdef __BORLANDC__
#include <malloc.h>
/* If Borland optimises away the stackframe, its alloca crashes. Fix. */  
#define ALLOCA_STACK char make_real_stackframe[1]; make_real_stackframe[0]=0
#else
#define ALLOCA_STACK
#ifdef __GNUC__
# define alloca __builtin_alloca
#else
# ifdef _MSC_VER
#  include <malloc.h>
#  define alloca _alloca
# else
#  if HAVE_ALLOCA_H
#   include <alloca.h>
#  else
#   ifdef _AIX
 #pragma alloca
#   else
#    ifndef alloca /* predefined by HP cc +Olibcalls */
char *alloca ();
#    endif
#   endif
#  endif
# endif
#endif
#endif
