/* Define this if you want war commands enabled */
#define WAR

/* The tsunami string .. :) :)
 *
 * \002 = Bold
 * \007 = Beep
 * \022 = Inverse
 */
/*
#define TSU "\002\007\007\007\007^\031]\031\007^\031]\031\007^\031]\031\007^\031]\031\007\002"
*/
/*
#define TSU "\002^\022]\022^\007\022]\022^\022]\022^\022]\002^\022]\022^\022]\022^\007\022]\022\002"
*/
#define TSU "^]^]^]^]^]^]^]^]^]^]^]^]"  

/* Counters ------------------------------------------------
   Note: This is number pet whatever-TIME
   NICKCNT		Number of nickfloods before kick
   CAPSCNT		Number of times user can TYPE IN CAPS
   DEOPCNT		Number of deops allowed before mass
   PUBCNT		Number of publics before kick
 */
#define NICKCNT 5
#define CAPSCNT 3
#define DEOPCNT 4
#define PUBCNT 6

/* Counters ------------------------------------------------
   Note: This is number pet whatever-TIME
   NICKTIME		Number of nickfloods before kick
   CAPSTIME		Number of times user can TYPE IN CAPS
   DEOPTIME		Number of deops allowed before mass
   PUBTIME		Number of publics before kick
 */
#define NICKTIME 10
#define CAPSTIME 15
#define DEOPTIME 10
#define PUBTIME 10
