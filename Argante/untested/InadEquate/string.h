/*
 * String hash table.
 * 
 * This is for ensuring only one copy of "hit me"
 * is around at the same time. We'll only ever need
 * to add strings to this.
 */

typedef const char *string;
