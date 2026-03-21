#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdalign.h>

#define u8 unsigned __int8
#define u16 unsigned __int16
#define u32 unsigned __int32
#define u64 unsigned __int64
#define u128 unsigned __int128

#define s8 __int8
#define s16 __int16
#define s32 __int32
#define s64 __int64
#define s128 __int128

#define FIBR_ALLO 1024 * 1024 * 1024 * 6
#define BUFF_SIZE 4096
#define LINK_ALLO 2048
#define RAND_FCTR 5

u8 *arr;
u64 arr_count, arr_alloc;
//main data array

void load(s8 *file) {
    
    alloc = BUFF_SIZE;
    alignas(u64) arr = malloc(arr_alloc);
    s32 f = open(file, O_RDONLY);
	if (f < 0) perror("wher file? #\n");
	
    u8 buff[BUFF_SIZE];
    s32 read;

    while ((read = read(f, buff, BUFF_SIZE))) {
        if (arr_count + read > arr_alloc) {
            arr_alloc =<< 1;
            u8 alignas(u64) tmp = realloc(arr, arr_alloc);
			if (!tmp) perror("load realloc failure #\n");
			else arr = tmp;
        }
        memcpy(arr + arr_count, buff, read);
        arr_count += read;
    }

    if (arr_alloc < arr_count << 1) {
        arr_alloc = arr_count << 1;
        alignas(64) arr = realloc(arr, arr_alloc);
		if (!tmp) perror("load realloc failure #\n");
		else arr = tmp;
    }
    memcpy(arr + arr_count, arr, arr_count);
    arr_count <<= 1;

    printf("%llu bytes loaded\n", arr_count);

    close(f);
}

typedef struct {
	u64 maas, frei;
	u64 v[];
} point_t; 
//connections
//<- maas -> |

point_t *pnt_arr, **pnt_pntrs;
u64 pnt_localizer, pnt_alloc;

void link(u64 size) {

    u64 ammo = ((arr_count << 3) + size - 1) / size;
    u64 *arr_64 = (u64 *)arr;
	
	pnt_alloc = ammo * (LINK_ALLO + 1);
	pnt_arr = malloc(sizeof(u64) * pnt_alloc);
	pnt_pntrs = malloc(sizeof(point_t *) * ammo);
	
	if (!pnt_arr || !pnt_pntrs) perror("pnt_arr alloc failuer #\n");
	
	#pragma omp parallel 
	{
		point_t l_point = malloc((sizeof(u64) << 1) + LINK_ALLO);
		
		if (!l_point) perror("l_point alloc failure #\n");
		
		u64 l_alloc = LINK_ALLO;
		#pragma omp for schedule(dynamic)
		for (u64 i = 0; i < ammo; ++i) {
			l_count = 0;
			l_point.maas = 0;
			l_point.frei = 0;
			{//since we have one-sided freedom
				for (u64 k = 0; k < size + 63 >> 6; ++k) {
					u64 a = *(u128 *)(arr_64 + (i * size >> 6) + k) >> (i * size & 63);
					if (k + 1 >= size + 63 >> 6) {
						u64 mask = 0xffffffffffffffffULL >> 64 - (size & 63);
						a &= mask;						
					}
					l_point.frei += __builtin_popcountll(a);
				}
			}
			
			for (u64 j = 0; j < ammo; ++j) {
				{//soooooooooooo many choices (any boolean function)
					if (i == j) continue; //I-skip, useful for more 'noise'
					if (arr_64[(i + 1) * size - 1 >> 6] >> 64 - ((i + 1) * size - 1) & 1ULL != arr_64[(j + 1) * size - 1 >> 6] >> 64 - ((j + 1) * size - 1) & 1ULL) continue; //last bit match
					if (l_count >= l_alloc) {
						l_alloc =<< 1;
						point_t *tmp = realloc(sizeof(u64) + l_alloc);
						if (!tmp) perror("l_point realloc failure #\n");
						else l_point = tmp;
					}
					bool stop = 0;
					for (u64 k = 0; k < size + 63 >> 6; ++k) {
						u64 a = *(u128 *)(arr_64 + (i * size >> 6) + k) >> (i * size & 63);
						u64 b = *(u128 *)(arr_64 + (j * size >> 6) + k) >> (j * size & 63);
						if (k + 1 >= size + 63 >> 6) {
							u64 mask = 0xffffffffffffffffULL >> 64 - (size & 63);
							a &= mask;
							b &= mask;
						}
						if (!(a >> 1 | !(a ^ b))) { //connection match
							stop = 1;
							break;
						}
						if (stop) break;
					}
					if (stop) continue;
					l_point.v[l_point.maas++] = j;
				}
			}
			
			printf("point %llu, has %llu freedomz and %llu connections\n", i, l_point.frei, l_point.maas);
			
			u64 temp;
			#pragma omp critical
			{
				temp = pnt_localizer;
				if (pnt_localizer + 2 + l_point.maas >= pnt_alloc) {
					pnt_alloc += 2 + l_point.maas;
					pnt_alloc =<< 1;
					point_t *tmp = realloc(sizeof(u64) * pnt_alloc);
					if (!tmp) perror("pnt_arr realloc failure #\n");
					else pnt_arr = tmp;
				}
				pnt_localizer += 2 + l_point.maas;
			}
			memcpy(l_point, pnt_arr + temp, (2 + l_point.maas) * sizeof(u64));
			pnt_pntrs[i] = pnt_arr + temp;
		}
		free(l_point);
	}
	pnt_arr = realloc(pnt_arr, sizeof(u64) * pnt_localizer);
}

typedef struct {
	u64 strt, size, rand;
	u8 d[];
} fbr_t;
// bit-matrix
// <- size * size + 7 >> 3 -> |

u8 *fbr_arr;
u64 fbr_top, fbr_bot;

inline void init_fbr(u8 *fbr_pntr, u64 i, u64 j, u64 size) {
	u16 *arr_16 = (u16 *)arr;
	
	fbr_t *f = (fbr_t *)fbr_pntr;
	for (u64 k = 0; k < size - 1; ++k) {
		u8 a = arr_16[i * size + k >> 4] >> (i * size + k & 7);
		u8 b = arr_16[j * size + k >> 4] >> (j * size + k & 7);
		f->d[k * size >> 3] &= 1 << (k * size & 7);
		f->d[k * size + 1 >> 3] &= ((a ^ b) & 1 << (k & 7)) >> (8 - (k & 7)) << (k * size + 1 & 7);
	}
	f->d[size * size >> 3] &= 1 << (size * size & 7);
	f->strt = i * size;
	f->size = size;
	f->rand = pnt_pntrs[i][2];
}

inline u64 rndm_slct(u64 n, u64 size, u64 i) {
	u16 *arr_16 = (u16 *)arr;
	u8 p = ~arr_16[i * size >> 4] >> (i * size & 7);
	u64 j = 0, k = 0;
	while (j + __builtin_popcount(p) < n && k < size) {
		p = ~arr_16[i * size + k >> 4] >> (i * size & 7);
		j += __builtin_popcount(p);
		k += 8;
	}
	while (j++ < n) p ^= p - 1;
	return k + __builtin_ctz(p);
}

u64 rando;

inline void adv_top_fbr(u64 i, u64 j) {
	u16 *arr_16 = (u16 *)arr;
	u64 size = ((fbt_t *)(fbr_arr + fbr_top))->size;
	fbr_t *fbr_pntr = malloc(size);
	
	#pragma omp critical
	{
		memcpy(fbr_arr + fbr_top, fbr_pntr, size);
		for (u64 k = 0; k < size; ++k) fbr_arr[fbr_top + k] = 0; 
		if (fbr_top + size > FIBR_ALLO) fbr_top = 0;
		else fbr_top += size;
	}
	
	u64 ulen = size + 7 >> 3;
	u64 l, l_rand = rando;
	u8 p, *r = malloc(ulen);
	for (u64 t = 0; t < 1 << (pnt_pntrs[i][2] : RAND_FCTR ? pnt_pntrs[i][2] < RAND_FCTR); ++t) {

		u64 l_fbr, n_s = 24 + (size * size + 7 >> 3);
		
		fbr_t *f = calloc(size, 1);
		
		f->strt = fbr_pntr->strt;
		f->size = fbr_pntr->size;
		f->rand = fbr_pntr->rand;
		f->rand += pnt_pntrs[i][2];
		
		if (pnt_pntrs[i][2] < RAND_FCTR && t) {
			l_rand = t ^ (t << 1);
			l = rndm_slct(l_rand, size, i);
			r[l >> 3] ^= 1 << (l & 7);
		} else {
			l = l_rand;
			l_rand ^= l_rand >> 12;
			l_rand ^= l_rand << 25;
			l_rand ^= l_rand >> 27;
			l_rand *= 0x2545f4914f6cdd1dULL;
			l_rand ^= l;
			l = 0;
			for (u64 k = 0; k < pnt_pntrs[i][2]; ++k) {
				p = arr_16[i * size + l >> 4] >> (i * size & 7);	
				while (p & 1 << (l & 7)) ++l;
				r[l >> 3] &= (((u8 *)l_rand)[k & 6] & 1 << (k & 7)) << (l & 7);
			}
		}
		
		u8 a, b, c, d, e = f->d[0];
		for (u64 k = 0; k < size - 1; ++k) {
			for (u64 l = 0; l < ulen - 1; ++l) {
				a = arr_16[(i * size >> 4) + (l >> 1)] >> (i * size & 7);
				b = arr_16[(j * size >> 4) + (l >> 1)] >> (j * size & 7);
				c = (a ^ b) | r[l];
				d = fbr_pntr->d[(k >> 3) + l + 1];
				f->d[(k >> 3) + (l >> 1)] ^= (d & (c >> (k & 7))) << ((k + 1) & 7);
			}
			u8 a = arr_16[(i * size >> 4) + ((ulen - 1) >> 1)] >> (i * size & 7);
			u8 b = arr_16[(j * size >> 4) + ((ulen - 1) >> 1)] >> (j * size & 7);
			u8 c = (a ^ b) & (0xff >> 8 - (size & 7));
			f->d[(k >> 3) + ulen - 1] = (fbr_pntr->d[(k >> 3) + ulen - 1] & (c >> (k & 7))) << ((k + 1) & 7);
		}
		
		#pragma omp critical
		{
			if (fbr_bot + n_s > FIBR_ALLO) {
				l_fbr = fbr_bot;
				fbr_bot = 0;
			} else {
				l_fbr = fbr_bot;
				fbr_bot += n_s;
			}
			if (fbr_bot == fbr_top) {
				printf("queue full/n");
				return void;
			}
			memcpy(f, fbr_arr + l_fbr, size);
		}
	}
}

u8 *shit_arr;
//data             bitmarker
//<- arr_count -> | <- arr_count -> | ... 1 + ... + (1 << RAND_FCTR) ... | <- arr_count -> | <- arr_count -> |
//                                        (1 << (RAND_FCTR + 1)) - 2   
void throw_shit_at_the_wall(void) {              
	
    u64 ammo = ((count << 3) + size - 1) / size;
    u16 *arr_16 = (u16 *)arr;
	
	if (count * ((1 << (RAND_FCTR + 1) - 2) > 1024 * 1024 * 1024 * 8)) printf("over 8gb/n");
	shit_arr = calloc(count * ((1 << (RAND_FCTR + 1) - 2), 1);
	
	fbr_arr = calloc(FIBR_ALLO, 1);
	
	for (u64 I = 0; I < RAND_FCTR; ++I) {
		for (u64 i = 0; i < count << 3; ++i) {
			
			link(count << 3 - i);
			
			
			init fbrs
			
			adv fibers
			
			store results
			
		}
	}
	
	
//LiFE
//linear fiber extension
//in loving memory of Terry A. Davis

// to-do, rewrite in Holy-C
//        in-place 'Wurm' - upon valid header confirmation follows the trail of 'least freedom' 
//        Wurst - similar to Wurm, but looks for many such 'confirmed structures'
int main(u16 argc, s8 *argv[]) {
	
	srand(time(NULL));
	
	//rando = rand();
	
	u64 x = 551231245934;
	x ^= x << 53;
	x ^= x >> 7;
	x ^= x << 5;
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rando = x;
	
    s8 *file = argv[1];

    load(file);

    
	
    return 0;

}
