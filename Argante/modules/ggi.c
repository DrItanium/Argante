/*
    Argante Virtual OS
    ------------------

    Low-level graphics module using GGI, version 0.4 (26-02-2001).
    Copyright (C) Adam Chodorowski

    NOTE: This code is mostly complete, but most functionality hasn't
          been tested. There shouldn't be any problems, but there might...

    TODO:
    - Implement the remaining syscalls:
      - SYSCALL_GGI_DBGETBUFFER
      - SYSCALL_GGI_DBGETNUMBUFFERS
      - SYSCALL_GGI_GETPIXELFORMAT
      - SYSCALL_GGI_RESOURCEACQUIRE
      - SYSCALL_GGI_RESOURCEMUSTACQUIRE
      - SYSCALL_GGI_RESOURCERELEASE

      I haven't really decided what to do about them. Even if I can map
      real memory into Argante space, I have to mangle the structures.
      Either I won't implement this, or it'll be a slightly different
      interface than GGI uses.
*/

#include <ggi/ggi.h>

#include "config.h"
#include "task.h"
#include "bcode.h"
#include "module.h"
#include "memory.h"
#include "console.h"
#include "syscall.h"
#include "acman.h"
#include "evaluate.h"

// This should really be in an include file (evaluate.h?)
extern struct vcpu_struct *curr_cpu_p;
extern int curr_cpu;

/*** Defines *****************************************************************/

#define MAX_VISUALS       8

/*** Structures **************************************************************/

struct Visual {
    int           v_id;     // Identification number
    char         *v_name;   // String identifier
    char         *v_access; // HAC object string (for access control (basically cache))
    int           v_count;  // Open count (ie. how many times it has been OPEN:ed)
    ggi_visual_t  v_real;   // The real GGI visual
};

/*** Global variables ********************************************************/

struct Visual *visuals[MAX_VISUALS];

// Temporary variables used when converting to and from GGI structures.
ggi_color     tmp_color;
ggi_pixel     tmp_pixel;

/*** Macros for validating access ********************************************/

#define THROW(e)            { non_fatal(e,GETMSG(e),c); failure = 1; return; }

#define VALIDATEVISUAL(id)  if( id < 0 || id >= MAX_VISUALS ) THROW(ERROR_GGI_INVALID_VISUAL); \
                            if( visuals[id] == NULL )         THROW(ERROR_GGI_INVALID_VISUAL); \
                            if( visuals[id]->v_real == NULL ) THROW(ERROR_GGI_INVALID_VISUAL);

/*** Support functions *******************************************************/

inline char *GETMSG( int id ) {
    switch( id ) {
        case ERROR_GGI_INVALID_VISUAL: return "GGI: Invalid visual";
        case ERROR_GGI_OPEN:           return "GGI: Could not open visual";
        case ERROR_GGI_CONTROL:        return "GGI: Control function failure";
        case ERROR_GGI_OUTPUT:         return "GGI: Output function failure";
        case ERROR_GGI_INPUT:          return "GGI: Input function failure";
        case ERROR_PROTFAULT:          return "GGI: Access to protected memory";
        case ERROR_NOMEM:              return "GGI: Could not allocate memory";
        default:                       return "GGI: Unknown error";
    }
}

/*** IO Handlers *************************************************************/

int getc_handler( int c ) {
    if( ggiKbhit( visuals[UREG(0)]->v_real ) ) {
        UREG(1) = ggiGetc( visuals[UREG(0)]->v_real );
        return 1;
    } else {
        return 0;
    }
}

int eventread_handler( int c ) {
    // Note: This must already have succeded in the syscall,
    //       so I don't bother checking for failure here.
    void *realmem = verify_access( c, UREG(2), (sizeof( gii_event )+3)/4,
                                   MEM_FLAG_WRITE );

    if( ggiEventsQueued( visuals[UREG(0)]->v_real, UREG(1) ) > 0 ) {
        SREG(0) = ggiEventRead( visuals[UREG(0)]->v_real,
                                (gii_event *) realmem, UREG(1) );

        if( SREG(0) <= 0 ) non_fatal( ERROR_GGI_INPUT, GETMSG( ERROR_GGI_INPUT ), c );

        return 1;
    } else {
        return 0;
    }
}

/*** Functions ***************************************************************/

void syscall_load( int *x ) {
    int i;

    if( ggiInit() == 0 ) {
        *(x++)=SYSCALL_GGI_ADDEVENTMASK;
        *(x++)=SYSCALL_GGI_ADDFLAGS;
        *(x++)=SYSCALL_GGI_CHECKGRAPHMODE;
        *(x++)=SYSCALL_GGI_CHECKMODE;
        *(x++)=SYSCALL_GGI_CHECKSIMPLEMODE;
        *(x++)=SYSCALL_GGI_CHECKTEXTMODE;
        *(x++)=SYSCALL_GGI_CLOSE;
        *(x++)=SYSCALL_GGI_COPYBOX;
        *(x++)=SYSCALL_GGI_CROSSBLIT;
        *(x++)=SYSCALL_GGI_DBGETBUFFER;
        *(x++)=SYSCALL_GGI_DBGETNUMBUFFERS;
        *(x++)=SYSCALL_GGI_DRAWBOX;
        *(x++)=SYSCALL_GGI_DRAWHLINE;
        *(x++)=SYSCALL_GGI_DRAWVLINE;
        *(x++)=SYSCALL_GGI_DRAWLINE;
        *(x++)=SYSCALL_GGI_DRAWPIXEL;
        *(x++)=SYSCALL_GGI_EVENTREAD;
        *(x++)=SYSCALL_GGI_EVENTSEND;
        *(x++)=SYSCALL_GGI_EVENTSQUEUED;
        *(x++)=SYSCALL_GGI_FILLSCREEN;
        *(x++)=SYSCALL_GGI_FLUSH;
        *(x++)=SYSCALL_GGI_FLUSHREGION;
        *(x++)=SYSCALL_GGI_GETBOX;
        *(x++)=SYSCALL_GGI_GETDISPLAYFRAME;
        *(x++)=SYSCALL_GGI_GETEVENTMASK;
        *(x++)=SYSCALL_GGI_GETFLAGS;
        *(x++)=SYSCALL_GGI_GETGCBACKGROUND;
        *(x++)=SYSCALL_GGI_GETGCCLIPPING;
        *(x++)=SYSCALL_GGI_GETGCFOREGROUND;
        *(x++)=SYSCALL_GGI_GETGAMMA;
        *(x++)=SYSCALL_GGI_GETGAMMAMAP;
        *(x++)=SYSCALL_GGI_GETHLINE;
        *(x++)=SYSCALL_GGI_GETMODE;
        *(x++)=SYSCALL_GGI_GETPALETTE;
        *(x++)=SYSCALL_GGI_GETPIXEL;
        *(x++)=SYSCALL_GGI_GETPIXELFORMAT;
        *(x++)=SYSCALL_GGI_GETREADFRAME;
        *(x++)=SYSCALL_GGI_GETVLINE;
        *(x++)=SYSCALL_GGI_GETWRITEFRAME;
        *(x++)=SYSCALL_GGI_GETC;
        *(x++)=SYSCALL_GGI_KBHIT;
        *(x++)=SYSCALL_GGI_MAPCOLOR;
        *(x++)=SYSCALL_GGI_OPEN;
        *(x++)=SYSCALL_GGI_PACKCOLORS;
        *(x++)=SYSCALL_GGI_PUTBOX;
        *(x++)=SYSCALL_GGI_PUTHLINE;
        *(x++)=SYSCALL_GGI_PUTPIXEL;
        *(x++)=SYSCALL_GGI_GETORIGIN;
        *(x++)=SYSCALL_GGI_PUTVLINE;
        *(x++)=SYSCALL_GGI_PUTC;
        *(x++)=SYSCALL_GGI_PUTS;
        *(x++)=SYSCALL_GGI_REMOVEEVENTMASK;
        *(x++)=SYSCALL_GGI_REMOVEFLAGS;
        *(x++)=SYSCALL_GGI_RESOURCEACQUIRE;
        *(x++)=SYSCALL_GGI_RESOURCEMUSTACQUIRE;
        *(x++)=SYSCALL_GGI_RESOURCERELEASE;
        *(x++)=SYSCALL_GGI_SETCOLORFULPALETTE;
        *(x++)=SYSCALL_GGI_SETDISPLAYFRAME;
        *(x++)=SYSCALL_GGI_SETEVENTMASK;
        *(x++)=SYSCALL_GGI_SETFLAGS;
        *(x++)=SYSCALL_GGI_SETGCBACKGROUND;
        *(x++)=SYSCALL_GGI_SETGCCLIPPING;
        *(x++)=SYSCALL_GGI_SETGCFOREGROUND;
        *(x++)=SYSCALL_GGI_SETGAMMA;
        *(x++)=SYSCALL_GGI_SETGAMMAMAP;
        *(x++)=SYSCALL_GGI_SETGRAPHMODE;
        *(x++)=SYSCALL_GGI_SETMODE;
        *(x++)=SYSCALL_GGI_SETORIGIN;
        *(x++)=SYSCALL_GGI_SETPALETTE;
        *(x++)=SYSCALL_GGI_SETREADFRAME;
        *(x++)=SYSCALL_GGI_SETSIMPLEMODE;
        *(x++)=SYSCALL_GGI_SETTEXTMODE;
        *(x++)=SYSCALL_GGI_SETWRITEFRAME;
        *(x++)=SYSCALL_GGI_UNMAPPIXEL;
        *(x++)=SYSCALL_GGI_UNPACKPIXELS;
        *x=SYSCALL_ENDLIST;

        for( i = 0; i < MAX_VISUALS; i++ ) {
            visuals[i] = NULL;
        }

        printk( ">> GGI: Module succesfully initialized.\n" );
    } else {
        printk( ">> GGI: Could not initialize module!\n" );
    }
}

void syscall_unload() {
    int i;

    for( i = 0; i < MAX_VISUALS; i++ ) {
        if( visuals[i] != NULL ) {
            ggiClose( visuals[i]->v_real );

            free( visuals[i]->v_name );
            free( visuals[i]->v_access );
            free( visuals[i] );
        }
    }

    ggiExit();
}

void syscall_handler( int c, int num ) {
    void *realmem;
    void *realmemII;

    switch( num ) {
        /* For those syscalls that require it, u0 always contains the visual
           id as returned by GGI_OPEN. All syscalls make sure to preserve u0
           at all times, so once you've put a visual in u0 you can quickly
           do a sequence of operations on that visual. */

        case SYSCALL_GGI_OPEN:
            /* Input:  u1 - pointer to name
                       u2 - length of name
               Output: u0 - visual ID */
            {
                struct Visual *visual;
                char          *name, *access;
                int            i, namelen, accesslen;

                if( !(realmem = verify_access( c, UREG(1), (UREG(2)+3)/4, MEM_FLAG_READ )) ) {
                    THROW( ERROR_PROTFAULT );
                }

                namelen   = UREG(2);
                accesslen = UREG(2) + strlen( "ggi/visual/" ) + 1;

                if( !(name = malloc( namelen + 1 )) )     THROW( ERROR_NOMEM );
                if( !(access = malloc( accesslen + 1 )) ) THROW( ERROR_NOMEM );

                memcpy( name, realmem, namelen );
                name[namelen] = '\0';
                snprintf( access, accesslen, "ggi/visual/%s", name );

                printf( "GGI: Opening visual name = %s, access = %s\n", name, access );

                VALIDATE( c, access, "ggi/control/open" );

                /* Check if the named visual is already opened. If so, just
                   increment it's open count and return it's id. */
                for( i = 0; i < MAX_VISUALS; i++ ) {
                    if( visuals[i] != NULL ) {
                        if( strcmp( name, visuals[i]->v_name ) == 0 ) {
                            visuals[i]->v_count++;
                            UREG(0) = i;
                            return;
                        }
                    }
                }

                /* We didn't find any visual with that name, so we'll try
                   to open a new one. First, we need to find a free slot. */
                for( i = 0; i < MAX_VISUALS; i++ ) {
                    if( visuals[i] == NULL ) {
                        /* OK, now allocate necessary memory and open the
                           visual. Also fill in the information we need to save. */
                        if( (visual = malloc( sizeof( struct Visual ) )) ) {
                            visual->v_id     = i;
                            visual->v_name   = name;
                            visual->v_access = access;
                            visual->v_count  = 1;
                            visual->v_real   = ggiOpen( NULL );

                            // ggiOpen() failed, inform the user.
                            if( !visual->v_real ) THROW( ERROR_GGI_OPEN );

                            visuals[i] = visual;

                            UREG(0) = i;
                            return;
                        } else {
                            /* Crap. Deallocate everything before giving up... */
                            free( name );
                            free( access );

                            THROW( ERROR_NOMEM );
                        }
                    }
                }

                THROW( ERROR_GGI_OPEN );
            }
        break;

        case SYSCALL_GGI_CLOSE:
            /* Input:  u0 - visual ID
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/close" );

            visuals[UREG(0)]->v_count--;

            if( visuals[UREG(0)]->v_count <= 0 ) {
                SREG(0) = ggiClose( visuals[UREG(0)]->v_real );

                free( visuals[UREG(0)]->v_name );
                free( visuals[UREG(0)]->v_access );
                free( visuals[UREG(0)] );
                visuals[UREG(0)] = NULL;

                if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
            }
        break;

        case SYSCALL_GGI_SETFLAGS:
            /* Input:  u0 - visual ID
                       u1 - flags
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/set/flags" );

            SREG(0) = ggiSetFlags( visuals[UREG(0)]->v_real, UREG(1) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_GETFLAGS:
            /* Input:  u0 - visual ID
               Output: u1 - flags */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/get/flags" );

            UREG(1) = ggiGetFlags( visuals[UREG(0)]->v_real );
        break;

        case SYSCALL_GGI_ADDFLAGS:
            /* Input:  u0 - visual ID
                       u1 - flags
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/get/flags" );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/set/flags" );

            SREG(0) = ggiAddFlags( visuals[UREG(0)]->v_real, UREG(1) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_REMOVEFLAGS:
            /* Input:  u0 - visual ID
                       u1 - flags
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/get/flags" );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/set/flags" );

            SREG(0) = ggiRemoveFlags( visuals[UREG(0)]->v_real, UREG(1) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_SETMODE:
            /* Input:  u0 - visual ID
                       u1 - pointer to ggi_mode structure
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/set/mode" );

            realmem = verify_access( c, UREG(1), (sizeof( ggi_mode )+3)/4,
                                     MEM_FLAG_READ | MEM_FLAG_WRITE );

            SREG(0) = ggiSetMode( visuals[UREG(0)]->v_real, (ggi_mode *) realmem );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_GETMODE:
            /* Input:  u0 - visual ID
                       u1 - pointer to ggi_mode structure
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/get/mode" );

            realmem = verify_access( c, UREG(1), (sizeof( ggi_mode )+3)/4, MEM_FLAG_WRITE );

            if( !realmem ) THROW( ERROR_PROTFAULT );

            SREG(0) = ggiGetMode( visuals[UREG(0)]->v_real, (ggi_mode *) realmem );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_CHECKMODE:
            /* Input:  u0 - visual ID
                       u1 - pointer to ggi_mode structure
               Output: s0 - status code (0 for ok, not 0 otherwise) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/check/mode" );

            realmem = verify_access( c, UREG(1), (sizeof( ggi_mode )+3)/4,
                                     MEM_FLAG_READ | MEM_FLAG_WRITE );

            if( !realmem ) THROW( ERROR_PROTFAULT );

            SREG(0) = ggiCheckMode( visuals[UREG(0)]->v_real, (ggi_mode *) realmem );
        break;

        case SYSCALL_GGI_SETSIMPLEMODE:
            /* Input:  u0 - visual ID
                       u1 - width
                       u2 - heigth
                       u3 - number of frames
                       u4 - graphtype
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/set/mode" );

            SREG(0) = ggiSetSimpleMode( visuals[UREG(0)]->v_real, UREG(1), UREG(2),
                                                                  UREG(3), UREG(4) );
            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_CHECKSIMPLEMODE:
            /* Input:  u0 - visual ID
                       u1 - width
                       u2 - height
                       u3 - number of frames
                       u4 - graphtype
                       u5 - pointer to ggi_mode structure (or NULL)
               Output: s0 - status code (0 for ok, not 0 otherwise) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/check/mode" );

            if( UREG(5) != 0 ) {
                realmem = verify_access( c, UREG(5), (sizeof( ggi_mode )+3)/4,
                                         MEM_FLAG_WRITE );

                if( !realmem ) THROW( ERROR_PROTFAULT );
            } else {
                realmem = NULL;
            }

            SREG(0) = ggiCheckSimpleMode( visuals[UREG(0)]->v_real,
                                          UREG(1), UREG(2), UREG(3), UREG(4),
                                          (ggi_mode *) realmem );
        break;

        case SYSCALL_GGI_SETGRAPHMODE:
            /* Input:  u0 - visual ID
                       u1 - width
                       u2 - heigth
                       u3 - virtual width
                       u4 - virtual height
                       u5 - graphtype
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/set/mode" );

            SREG(0) = ggiSetGraphMode( visuals[UREG(0)]->v_real, UREG(1), UREG(2),
                                       UREG(3), UREG(4), UREG(5) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_CHECKGRAPHMODE:
            /* Input:  u0 - visual ID
                       u1 - width
                       u2 - height
                       u3 - virtual width
                       u4 - virtual height
                       u5 - graphtype
                       u6 - pointer to ggi_mode structure (or NULL)
               Output: s0 - status code (0 for ok, not 0 otherwise) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/check/mode" );

            if( UREG(6) != 0 ) {
                realmem = verify_access( c, UREG(6), (sizeof( ggi_mode )+3)/4,
                                         MEM_FLAG_WRITE );

                if( !realmem ) THROW( ERROR_PROTFAULT );
            } else {
                realmem = NULL;
            }

            SREG(0) = ggiCheckGraphMode( visuals[UREG(0)]->v_real,
                                         UREG(1), UREG(2), UREG(3), UREG(4),
                                         UREG(5), (ggi_mode *) realmem );
        break;

        case SYSCALL_GGI_SETTEXTMODE:
            /* Input:  u0 - visual ID
                       u1 - cols
                       u2 - rows
                       u3 - virtual cols
                       u4 - virtual rows
                       u5 - fontx
                       u6 - fonty
                       u7 - graphtype
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/set/mode" );

            SREG(0) = ggiSetTextMode( visuals[UREG(0)]->v_real,
                                      UREG(1), UREG(2), UREG(3), UREG(4),
                                      UREG(5), UREG(6), UREG(7) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_CHECKTEXTMODE:
            /* Input:  u0 - visual ID
                       u1 - cols
                       u2 - rows
                       u3 - virtual cols
                       u4 - virtual rows
                       u5 - fontx
                       u6 - fonty
                       u7 - graphtype
                       u8 - pointer to ggi_mode structure (or NULL)
               Output: s0 - status code (0 for ok, not 0 otherwise) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/check/mode" );

            if( UREG(8) != 0 ) {
                realmem = verify_access( c, UREG(8), (sizeof( ggi_mode )+3)/4,
                                         MEM_FLAG_WRITE );

                if( !realmem ) THROW( ERROR_PROTFAULT );
            } else {
                realmem = NULL;
            }

            SREG(0) = ggiCheckTextMode( visuals[UREG(0)]->v_real,
                                        UREG(1), UREG(2), UREG(3), UREG(4),
                                        UREG(5), UREG(6), UREG(7),
                                        (ggi_mode *) realmem );
        break;

        case SYSCALL_GGI_SETORIGIN:
            /* Input:  u0 - visual ID
                       s1 - x
                       s2 - y
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/set/origin" );

            SREG(0) = ggiSetOrigin( visuals[UREG(0)]->v_real, SREG(1), SREG(2) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_GETORIGIN:
            /* Input:  u0 - visual ID
               Output: s1 - x
                       s2 - y
                       s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/get/origin" );

            SREG(0) = ggiGetOrigin( visuals[UREG(0)]->v_real, &SREG(1), &SREG(2) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_SETDISPLAYFRAME:
            /* Input:  u0 - visual ID
                       u1 - frame number
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/set/frame/display" );

            SREG(0) = ggiSetDisplayFrame( visuals[UREG(0)]->v_real, UREG(1) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_GETDISPLAYFRAME:
            /* Input:  u0 - visual ID
               Output: u1 - frame number */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/get/frame/display" );

            UREG(1) = ggiGetDisplayFrame( visuals[UREG(0)]->v_real );
        break;

        case SYSCALL_GGI_SETREADFRAME:
            /* Input:  u0 - visual ID
                       u1 - frame number
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/set/frame/read" );

            SREG(0) = ggiSetReadFrame( visuals[UREG(0)]->v_real, UREG(1) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_GETREADFRAME:
            /* Input:  u0 - visual ID
               Output: u1 - frame number */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/get/frame/read" );

            UREG(1) = ggiGetReadFrame( visuals[UREG(0)]->v_real );
        break;

        case SYSCALL_GGI_SETWRITEFRAME:
            /* Input:  u0 - visual ID
                       u1 - frame number
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/set/frame/write" );

            SREG(0) = ggiSetWriteFrame( visuals[UREG(0)]->v_real, UREG(1) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_GETWRITEFRAME:
            /* Input:  u0 - visual ID
               Output: u1 - frame number */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/get/frame/write" );

            UREG(1) = ggiGetWriteFrame( visuals[UREG(0)]->v_real );
        break;

        case SYSCALL_GGI_SETPALETTE:
            /* Input:  u0 - visual ID
                       u1 - pointer to array of ggi_color
                       u2 - array length
                       u3 - start index
               Output: s0 - first entry changed if positive, error code otherwise */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/set/palette" );

            realmem = verify_access( c, UREG(1), ((UREG(2) * sizeof( ggi_color ))+3)/4,
                                     MEM_FLAG_READ );

            if( !realmem ) THROW( ERROR_PROTFAULT );

            SREG(0) = ggiSetPalette( visuals[UREG(0)]->v_real, UREG(3), UREG(2),
                                     (ggi_color *) realmem );

            if( SREG(0) < 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_GETPALETTE:
            /* Input:  u0 - visual ID
                       u1 - pointer to array of ggi_color
                       u2 - array length
                       u3 - start index
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/get/palette" );

            realmem = verify_access( c, UREG(1), ((UREG(2) * sizeof( ggi_color ))+3)/4,
                                     MEM_FLAG_WRITE );

            if( !realmem ) THROW( ERROR_PROTFAULT );

            SREG(0) = ggiGetPalette( visuals[UREG(0)]->v_real, UREG(3), UREG(2),
                                     (ggi_color *) realmem );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_SETCOLORFULPALETTE:
            /* Input:  u0 - visual ID
               Output: s0 - first entry changed if positive, error code if negative */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/set/palette" );

            SREG(0) = ggiSetColorfulPalette( visuals[UREG(0)]->v_real );

            if( SREG(0) < 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_MAPCOLOR:
            /* Input:  u0 - visual ID
                       u1 - red component
                       u2 - green component
                       u3 - blue component
                       u4 - alpha component
               Output: u1 - pixel value */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/map/color" );

            tmp_color.r = UREG(1);
            tmp_color.g = UREG(2);
            tmp_color.b = UREG(3);
            tmp_color.a = UREG(4);

            UREG(1) = ggiMapColor( visuals[UREG(0)]->v_real, &tmp_color );
        break;

        case SYSCALL_GGI_UNMAPPIXEL:
            /* Input:  u0 - visual ID
                       u1 - pixel value
               Output: u1 - red component
                       u2 - green component
                       u3 - blue component
                       u5 - alpha component
                       s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/map/pixel" );

            SREG(0) = ggiUnmapPixel( visuals[UREG(0)]->v_real, UREG(1), &tmp_color );

            UREG(1) = tmp_color.r;
            UREG(2) = tmp_color.g;
            UREG(3) = tmp_color.b;
            UREG(4) = tmp_color.a;

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_PACKCOLORS:
            /* Input:  u0 - visual ID
                       u1 - pointer to array of pixel values (dwords)
                       u2 - pointer to array of ggi_color
                       u3 - length of arrays (both must be same length)
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/map/colors" );

            realmem   = verify_access( c, UREG(1), UREG(3), MEM_FLAG_WRITE );
            realmemII = verify_access( c, UREG(2), ((UREG(3) * sizeof( ggi_color ))+3)/4,
                                       MEM_FLAG_READ );

            if( !realmem || !realmemII ) THROW( ERROR_PROTFAULT );

            SREG(0) = ggiPackColors( visuals[UREG(0)]->v_real,
                                     realmem, (ggi_color *) realmemII, UREG(3) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_UNPACKPIXELS:
            /* Input:  u0 - visual ID
                       u1 - pointer to array of pixel values (dwords)
                       u2 - pointer to array of ggi_color
                       u3 - length of arrays (both must be same length)
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/map/pixels" );

            realmem   = verify_access( c, UREG(1), UREG(3), MEM_FLAG_READ );
            realmemII = verify_access( c, UREG(2), ((UREG(3) * sizeof( ggi_color ))+3)/4,
                                       MEM_FLAG_WRITE );

            if( !realmem || !realmemII ) THROW( ERROR_PROTFAULT );

            SREG(0) = ggiUnpackPixels( visuals[UREG(0)]->v_real,
                                       realmem, (ggi_color *) realmemII, UREG(3) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_SETGAMMA:
            /* Input:  u0 - visual ID
                       f0 - red component
                       f1 - green component
                       f2 - blue component
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/set/gamma" );

            SREG(0) = ggiSetGamma( visuals[UREG(0)]->v_real,
                                   (ggi_float) cpu[c].fregs[0],
                                   (ggi_float) cpu[c].fregs[1],
                                   (ggi_float) cpu[c].fregs[2] );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_GETGAMMA:
            /* Input:  u0 - visual ID
               Output: f0 - red component
                       f1 - green component
                       f2 - blue component
                       s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/get/gamma" );

            SREG(0) = ggiGetGamma( visuals[UREG(0)]->v_real,
                                   (ggi_float *) &cpu[c].fregs[0],
                                   (ggi_float *) &cpu[c].fregs[1],
                                   (ggi_float *) &cpu[c].fregs[2] );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_SETGAMMAMAP:
            /* Input:  u0 - visual ID
                       u1 - pointer to array of ggi_color
                       u2 - length of array
                       u3 - start index
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/set/gamma" );

            realmem = verify_access( c, UREG(1), ((UREG(2) * sizeof( ggi_color ))+3)/4,
                                     MEM_FLAG_READ );

            if( !realmem ) THROW( ERROR_PROTFAULT );

            SREG(0) = ggiSetGammaMap( visuals[UREG(0)]->v_real, UREG(3), UREG(2),
                                      (ggi_color *) realmem );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_GETGAMMAMAP:
            /* Input:  u0 - visual ID
                       u1 - pointer to array of ggi_color
                       u2 - length of array
                       u3 - start index
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/get/gamma" );

            realmem = verify_access( c, UREG(1), ((UREG(2) * sizeof( ggi_color ))+3)/4,
                                     MEM_FLAG_WRITE );

            if( !realmem ) THROW( ERROR_PROTFAULT );

            SREG(0) = ggiGetGammaMap( visuals[UREG(0)]->v_real, UREG(3), UREG(2),
                                      (ggi_color *) realmem );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_SETGCFOREGROUND:
            /* Input:  u0 - visual ID
                       u1 - pixel value
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/set/gc/foreground" );

            SREG(0) = ggiSetGCForeground( visuals[UREG(0)]->v_real, UREG(1) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_GETGCFOREGROUND:
            /* Input:  u0 - visual ID
               Output: u1 - pixel value
                       s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/get/gc/foreground" );

            SREG(0) = ggiGetGCForeground( visuals[UREG(0)]->v_real, &tmp_pixel );
            UREG(1) = tmp_pixel;

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_SETGCBACKGROUND:
            /* Input:  u0 - visual ID
                       u1 - pixel value
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/set/gc/background" );

            SREG(0) = ggiSetGCBackground( visuals[UREG(0)]->v_real, UREG(1) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_GETGCBACKGROUND:
            /* Input:  u0 - visual ID
               Output: u1 - pixel value
                       s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/get/gc/background" );

            SREG(0) = ggiGetGCBackground( visuals[UREG(0)]->v_real, &tmp_pixel );
            UREG(1) = tmp_pixel;

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_SETGCCLIPPING:
            /* Input:  u0 - visual ID
                       s1 - left
                       s2 - top
                       s3 - right
                       s4 - bottom
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/set/gc/clipping" );

            SREG(0) = ggiSetGCClipping( visuals[UREG(0)]->v_real,
                                        SREG(1), SREG(2), SREG(3), SREG(4) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_GETGCCLIPPING:
            /* Input:  u0 - visual ID
               Output: s1 - left
                       s2 - top
                       s3 - right
                       s4 - bottom
                       s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/get/gc/clipping" );

            SREG(0) = ggiGetGCClipping( visuals[UREG(0)]->v_real,
                                        &SREG(1), &SREG(2), &SREG(3), &SREG(4) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_CONTROL );
        break;

        case SYSCALL_GGI_FLUSH:
            /* Input:  u0 - visual ID
               Output: none */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/flush/screen" );

            ggiFlush( visuals[UREG(0)]->v_real );
        break;

        case SYSCALL_GGI_FLUSHREGION:
            /* Input:  u0 - visual ID
                       s1 - x
                       s2 - y
                       s3 - width
                       s4 - height
               Output: none */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/control/flush/region" );

            ggiFlushRegion( visuals[UREG(0)]->v_real, SREG(1), SREG(2), SREG(3), SREG(4) );
        break;

        case SYSCALL_GGI_FILLSCREEN:
            /* Input:  u0 - visual ID
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/output/fillscreen" );

            SREG(0) = ggiFillscreen( visuals[UREG(0)]->v_real );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_OUTPUT );
        break;

        case SYSCALL_GGI_PUTPIXEL:
            /* Input:  u0 - visual ID
                       s1 - x
                       s2 - y
                       u1 - pixel value
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/output/put/pixel" );

            SREG(0) = ggiPutPixel( visuals[UREG(0)]->v_real, SREG(1), SREG(2), UREG(1) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_OUTPUT );
        break;

        case SYSCALL_GGI_GETPIXEL:
            /* Input:  u0 - visual ID
                       s1 - x
                       s2 - y
               Output: s0 - error code (or 0 for success)
                       u1 - pixel value */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/output/get/pixel" );

            SREG(0) = ggiGetPixel( visuals[UREG(0)]->v_real, SREG(1), SREG(2), &tmp_pixel );
            UREG(1) = tmp_pixel;

            if( SREG(0) != 0 ) THROW( ERROR_GGI_OUTPUT );
        break;

        case SYSCALL_GGI_DRAWPIXEL:
            /* Input:  u0 - visual ID
                       s1 - x
                       s2 - y
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/output/draw/pixel" );

            SREG(0) = ggiDrawPixel( visuals[UREG(0)]->v_real, SREG(1), SREG(2) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_OUTPUT );
        break;

        case SYSCALL_GGI_PUTHLINE:
            /* Input:  u0 - visual ID
                       s1 - x
                       s2 - y
                       s3 - width
                       u1 - pointer to buffer
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/output/put/hline" );

            realmem = verify_access( c, UREG(1), SREG(3), MEM_FLAG_READ );

            if( !realmem ) THROW( ERROR_PROTFAULT );

            SREG(0) = ggiPutHLine( visuals[UREG(0)]->v_real,
                                   SREG(1), SREG(2), SREG(3), realmem );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_OUTPUT );
        break;

        case SYSCALL_GGI_GETHLINE:
            /* Input:  u0 - visual ID
                       s1 - x
                       s2 - y
                       s3 - width
                       u1 - pointer to buffer
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/output/put/hline" );

            realmem = verify_access( c, UREG(1), SREG(3), MEM_FLAG_WRITE );

            if( !realmem ) THROW( ERROR_PROTFAULT );

            SREG(0) = ggiGetHLine( visuals[UREG(0)]->v_real,
                                   SREG(1), SREG(2), SREG(3), realmem );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_OUTPUT );
        break;

        case SYSCALL_GGI_DRAWHLINE:
            /* Input:  u0 - visual ID
                       s1 - x
                       s2 - y
                       s3 - width
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/output/draw/hline" );

            SREG(0) = ggiDrawHLine( visuals[UREG(0)]->v_real,
                                    SREG(1), SREG(2), SREG(3) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_OUTPUT );
        break;

        case SYSCALL_GGI_PUTVLINE:
            /* Input:  u0 - visual ID
                       s1 - x
                       s2 - y
                       s3 - height
                       u1 - pointer to buffer
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/output/put/vline" );

            realmem = verify_access( c, UREG(1), SREG(3), MEM_FLAG_READ );

            if( !realmem ) THROW( ERROR_PROTFAULT );

            SREG(0) = ggiPutVLine( visuals[UREG(0)]->v_real,
                                   SREG(1), SREG(2), SREG(3), realmem );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_OUTPUT );
        break;

        case SYSCALL_GGI_GETVLINE:
            /* Input:  u0 - visual ID
                       s1 - x
                       s2 - y
                       s3 - height
                       u1 - pointer to buffer
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/output/get/vline" );

            realmem = verify_access( c, UREG(1), SREG(3), MEM_FLAG_WRITE );

            if( !realmem ) THROW( ERROR_PROTFAULT );

            SREG(0) = ggiGetVLine( visuals[UREG(0)]->v_real,
                                   SREG(1), SREG(2), SREG(3), realmem );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_OUTPUT );
        break;

        case SYSCALL_GGI_DRAWVLINE:
            /* Input:  u0 - visual ID
                       s1 - x
                       s2 - y
                       s3 - height
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/output/draw/vline" );

            SREG(0) = ggiDrawVLine( visuals[UREG(0)]->v_real,
                                    SREG(1), SREG(2), SREG(3) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_OUTPUT );
        break;

        case SYSCALL_GGI_DRAWLINE:
            /* Input:  u0 - visual ID
                       s1 - x
                       s2 - y
                       s3 - xe
                       s4 - ye
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/output/draw/line" );

            SREG(0) = ggiDrawLine( visuals[UREG(0)]->v_real,
                                   SREG(1), SREG(2), SREG(3), SREG(4) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_OUTPUT );
        break;

        case SYSCALL_GGI_PUTBOX:
            /* Input:  u0 - visual ID
                       s1 - x
                       s2 - y
                       s3 - width
                       s4 - height
                       u1 - pointer to buffer
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/output/put/box" );

            realmem = verify_access( c, UREG(1), SREG(3) * SREG(4), MEM_FLAG_READ );

            if( !realmem ) THROW( ERROR_PROTFAULT );

            SREG(0) = ggiPutBox( visuals[UREG(0)]->v_real,
                                 SREG(1), SREG(2), SREG(3), SREG(4), realmem );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_OUTPUT );
        break;

        case SYSCALL_GGI_GETBOX:
            /* Input:  u0 - visual ID
                       s1 - x
                       s2 - y
                       s3 - width
                       s4 - height
                       u1 - pointer to buffer
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/output/get/box" );

            realmem = verify_access( c, UREG(1), SREG(3) * SREG(4), MEM_FLAG_WRITE );

            if( !realmem ) THROW( ERROR_PROTFAULT );

            SREG(0) = ggiGetBox( visuals[UREG(0)]->v_real, SREG(1), SREG(2),
                                 SREG(3), SREG(4), realmem );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_OUTPUT );
        break;

        case SYSCALL_GGI_DRAWBOX:
            /* Input:  u0 - visual ID
                       s1 - x
                       s2 - y
                       s3 - width
                       s4 - height
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/output/draw/box" );

            SREG(0) = ggiDrawBox( visuals[UREG(0)]->v_real,
                                  SREG(1), SREG(2), SREG(3), SREG(4) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_OUTPUT );
        break;

        case SYSCALL_GGI_COPYBOX:
            /* Input:  u0 - visual ID
                       s1 - x
                       s2 - y
                       s3 - width
                       s4 - height
                       s5 - new x
                       s6 - new y
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/output/get/box" );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/output/put/box" );

            SREG(0) = ggiCopyBox( visuals[UREG(0)]->v_real, SREG(1), SREG(2),
                                  SREG(3), SREG(4), SREG(5), SREG(6) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_OUTPUT );
        break;

        case SYSCALL_GGI_CROSSBLIT:
            /* Input:  u0 - src visual ID
                       s1 - src x
                       s2 - src y
                       s3 - src width
                       s4 - src height
                       u1 - dst visual ID
                       u5 - dst x
                       u6 - dst y
               Output: s0 - error code (or 0 for success) */

               VALIDATEVISUAL( UREG(0) );
               VALIDATEVISUAL( UREG(1) );
               VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/output/get/box" );
               VALIDATE( c, visuals[UREG(1)]->v_access, "ggi/output/put/box" );

               SREG(0) = ggiCrossBlit( visuals[UREG(0)]->v_real,
                                       SREG(1), SREG(2), SREG(3), SREG(4),
                                       visuals[UREG(1)]->v_real,
                                       SREG(5), SREG(6) );

               if( SREG(0) != 0 ) THROW( ERROR_GGI_OUTPUT );
        break;

        case SYSCALL_GGI_PUTC:
            /* Input:  u0 - visual ID
                       s1 - x
                       s2 - y
                       u1 - character value
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/output/put/char" );

            SREG(0) = ggiPutc( visuals[UREG(0)]->v_real, SREG(1), SREG(2), UREG(1) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_OUTPUT );
        break;

        case SYSCALL_GGI_PUTS:
            /* Input:  u0 - visual ID
                       s1 - x coordinate
                       s2 - y coordinate
                       u1 - pointer to string
                       u2 - string length
               Output: s0 - error code (or 0 for success) */
            {
                char *tmp;

                VALIDATEVISUAL( UREG(0) );
                VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/output/put/string" );

                realmem = verify_access( c, UREG(1), (UREG(2)+3)/4, MEM_FLAG_READ );

                if( !realmem )                       THROW( ERROR_PROTFAULT );
                if( !(tmp = malloc( UREG(2) + 1 )) ) THROW( ERROR_NOMEM );

                memcpy( tmp, realmem, UREG(2) );
                tmp[UREG(2)] = '\0';

                SREG(0) = ggiPuts( visuals[UREG(0)]->v_real, SREG(1), SREG(2), tmp );

                free( tmp );

                if( SREG(0) != 0 ) THROW( ERROR_GGI_OUTPUT );
            }
        break;

        case SYSCALL_GGI_KBHIT:
            /* Input:  u0 - visual ID
               Output: s0 - if no key has been recieved then 0, else not 0. */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/input/kbhit" );

            SREG(0) = ggiKbhit( visuals[UREG(0)]->v_real );
        break;

        case SYSCALL_GGI_GETC:
            /* Input:  u0 - visual ID
               Output: u1 - unicode character */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/input/get/char" );

            // Is there a char ready ready?
            if( ggiKbhit( visuals[UREG(0)]->v_real ) ) {
                // Return immediately
                UREG(1) = ggiGetc( visuals[UREG(0)]->v_real );
            } else {
                // Wait for a char
                ENTER_IOWAIT( c, 0, getc_handler );
            }
        break;

        case SYSCALL_GGI_SETEVENTMASK:
            /* Input:  u0 - visual ID
                       u1 - event mask
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/input/set/eventmask" );

            SREG(0) = ggiSetEventMask( visuals[UREG(0)]->v_real, UREG(1) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_INPUT );
        break;

        case SYSCALL_GGI_GETEVENTMASK:
            /* Input:  u0 - visual ID
               Output: u1 - event mask */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/input/get/eventmask" );

            UREG(1) = ggiGetEventMask( visuals[UREG(0)]->v_real );
        break;

        case SYSCALL_GGI_ADDEVENTMASK:
            /* Input:  u0 - visual ID
                       u1 - event mask
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/input/get/eventmask" );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/input/set/eventmask" );

            SREG(0) = ggiAddEventMask( visuals[UREG(0)]->v_real, UREG(1) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_INPUT );
        break;

        case SYSCALL_GGI_REMOVEEVENTMASK:
            /* Input:  u0 - visual ID
                       u1 - event mask
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/input/get/eventmask" );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/input/set/eventmask" );

            SREG(0) = ggiRemoveEventMask( visuals[UREG(0)]->v_real, UREG(1) );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_INPUT );
        break;

        case SYSCALL_GGI_EVENTSQUEUED:
            /* Input:  u0 - visual ID
                       u1 - event mask
               Output: s0 - number of events */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/input/get/eventsqueued" );

            SREG(0) = ggiEventsQueued( visuals[UREG(0)]->v_real, UREG(1) );
        break;

        case SYSCALL_GGI_EVENTREAD:
            /* Input:  u0 - visual ID
                       u1 - event mask
                       u2 - pointer to gii_event structure
               Output: s0 - size of returned event (or 0 for error) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/input/get/event" );

            realmem = verify_access( c, UREG(2), (sizeof( gii_event )+3)/4, MEM_FLAG_WRITE );

            if( ggiEventsQueued( visuals[UREG(0)]->v_real, UREG(1) ) > 0 ) {
                // Return immediately
                SREG(0) = ggiEventRead( visuals[UREG(0)]->v_real,
                                         (gii_event *) realmem, UREG(1) );

                if( SREG(0) <= 0 ) THROW( ERROR_GGI_INPUT );
            } else {
                // Wait for an event
                ENTER_IOWAIT( c, 0, eventread_handler );
            }
        break;

        case SYSCALL_GGI_EVENTSEND:
            /* Input:  u0 - visual ID
                       u1 - pointer to gii_event structure
               Output: s0 - error code (or 0 for success) */

            VALIDATEVISUAL( UREG(0) );
            VALIDATE( c, visuals[UREG(0)]->v_access, "ggi/input/put/event" );

            realmem = verify_access( c, UREG(1), (sizeof( gii_event )+3)/4, MEM_FLAG_READ );

            SREG(0) = ggiEventSend( visuals[UREG(0)]->v_real, (gii_event *) realmem );

            if( SREG(0) != 0 ) THROW( ERROR_GGI_INPUT );
        break;
    }
}

