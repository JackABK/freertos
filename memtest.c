#include "FreeRTOS.h"
#include "clib.h"
struct slot {
    void *pointer;
    unsigned int size;                                                                                
    unsigned int lfsr;
};
/* I set to 100KB of RAM , the maximum size of buffer about 6920 , .bss section?? */
#define CIRCBUFSIZE 6900 
unsigned int write_pointer, read_pointer;


static struct slot slots[CIRCBUFSIZE];
static unsigned int lfsr = 0xACE1;

static unsigned int circbuf_size(void)
{
    return (write_pointer + CIRCBUFSIZE - read_pointer) % CIRCBUFSIZE;
}

static void write_cb(struct slot foo)
{
    if (circbuf_size() == CIRCBUFSIZE - 1) {
        printf("circular buffer overflow\n\r");
		//exit(1);
    }
    slots[write_pointer++] = foo;
    write_pointer %= CIRCBUFSIZE;
}

static struct slot read_cb(void)
{
    struct slot foo;
    if (write_pointer == read_pointer) {
        // circular buffer is empty
        return (struct slot){ .pointer=NULL, .size=0, .lfsr=0 };
    }
    foo = slots[read_pointer++];
    read_pointer %= CIRCBUFSIZE;
    return foo;
}

// Get a pseudorandom number generator from Wikipedia
static int prng(void)
{
	/*original code by Standard C*/
	#if 0 
    static unsigned int bit;
    /* taps: 16 14 13 11; characteristic polynomial: x^16 + x^14 + x^13 + x^11 + 1 */
    bit  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
    lfsr =  (lfsr >> 1) | (bit << 15);
    return lfsr & 0xffff;
	#endif

	/*inline assmebler*/
	asm("mov r0,%[input]		\n"
		"eor r1,r0,r0,lsr #2	\n" /*r1 = (r0>>0) ^ (r0>>2)*/
		"eor r1,r1,r0,lsr #3	\n"/*r1 = r1 ^ (r0>>3)*/
		"eor r1,r1,r0,lsr #5 	\n"/*r1 = r1 ^ (r0>>5)*/
		"and r1,#1				\n"/*r1=r1 & 1*/
		"lsl r1,#15				\n"/*r1=r1<<15*/			
		"orr r0,r1,r0,lsr #1    \n"/*r0 = (r0>>1) | r1*/
		"mov %[output] , r0 	\n"/*output from r0*/
		:[output]	"=r" (lfsr)	/*output operand*/
		:[input]	"r"	 (lfsr)	/*input operand*/
		: "r1","r0"
	);
    return lfsr & 0xffff;
}

void memtest_cmd(void)
{
    int i, size;
    char *p;

    while (1) {
        size = prng() & 0x7FF;
        printf("try to allocate %d bytes\n\r", size);
        p = (char *) pvPortMalloc(size);
        printf("malloc returned %p\n\r", p);
        if (p == NULL) {
            // can't do new allocations until we free some older ones
            while (circbuf_size() > 0) {
                // confirm that data didn't get trampled before freeing
                struct slot foo = read_cb();
                p = foo.pointer;
                lfsr = foo.lfsr;  // reset the PRNG to its earlier state
                size = foo.size;
                printf("free a block, size %d\n\r", size);
                for (i = 0; i < size; i++) {
                    unsigned char u = p[i];
                    unsigned char v = (unsigned char) prng();
                    if (u != v) {
                        printf("OUCH: u=%02X, v=%02X\n\r", u, v);
                        return 1;
                    }
                }
                vPortFree(p);
                if ((prng() & 1) == 0) break;
            }
        } else {
            printf("allocate a block, size %d\n\r", size);
            write_cb((struct slot){.pointer=p, .size=size, .lfsr=lfsr});
            for (i = 0; i < size; i++) {
                p[i] = (unsigned char) prng();
            }
        }
    }
    return 0;
}
