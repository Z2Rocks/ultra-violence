#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
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

#define FIBR_LIMT 1024 * 1024
#define BUFF_SIZE 4096
#define LINK_ALLO 2048
#define PATH_ALLO 64
#define RAND_FCTR 4

u8 *arr;
u64 arr_count, arr_alloc;

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
        u8 p, q = 0;
		for (u64 i = 0; i < read; ++i) {
			p = buff[i];
			for (u8 j = 0; j < 4; ++j) {
				if (p & 0x01) q &= 0x01 << (j << 1);
				p =>> 1;
			}
			arr[++arr_count] = q;
			q = 0;
			for (u8 j = 0; j < 4; ++j) {
				if (p & 0x01) q &= 0x01 << (j << 1);
				p =>> 1;
			}
			arr[++arr_count] = q;
			q = 0;
		}
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
			{//since we have one sided freedom
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
	u64 a, b, c;
	u8 d[];
} eqn_t;

// <- a -> | <- b -> | <- c -> 

//x0+x1+x2+x3+x4
//01
//01111100
//00 11 11 11 11

//(x0+x1+x2+x3)(x5)+(x0+x1+x2)(x4)
//01 01
//01111010 01111000 
//00 10 11 11 11 01 10 01 11 10 11 11 01 10 01

//(x0+x1+x2+x3)(x5)+(x8+x11+x12)(x14)
//10 01 
//01111010-01001101 
//00 10 11 11 11 01 10 01 11 10 11 11 01 10 01

//(x0+x1+x2)(x3)(x5)+(x35+x37)(x44)(x67+x97)(x1000+x1001+x1002+x1003+x1004)
//01 11 10 01 10 11 01
//01111010 11000100 01010000-00100000 00010111-11000000 01111100
//00 10 11 11 01 10 01 10 01 11 10 11 01 10 01 10 11 01 10 11 11 11 11 01

//(x0+x1)(x2)(x3)
//01
//01111000
//00 10 11 01 10 01 10 01

//I+x
//01
//11000000
//11

//00 - filler
//sadly I acts as a normalization block, (I + x)(I + y) can not be simplified
//(a meta identifier at which gives the tree structure would make it optimal a-howManyBitsOf->b)
inline bool slct(u8 r, u8 s, u64 j, u64 k, u64 l, u64 m, u8 *a, u8 *b, u64 *trgt) {
	l = 0;
	chain:
	switch (q & 0x03) {
		case 1:
			++l;
			break;
		case 2:
			++l;
			while (!(s & 1)) {
				s =>> 1;
				++k;
				if (!(k & 7) && l) {
					s = b[++m];
					--l;
				}
			}
			break;
		case 3:
			k = 0;
			switch (l) {
				case 0:
					n = b[m++];
					break;
				case 1:
					n = *(u16 *)(b + m);
					m += 2;
					break;
				case 2:
					n = (*(u32 *)(b + m)) & 0x00ffffffUL;
					m += 3;
					break;
				case 3:
					n = *(u32 *)(b + m);
					m += 4;
			s = b[m];
			l = 0;
		}
	}
	q =>> 2;
	if (!(r-- & 3)) q = a[++j];
	if (l) goto chain;
	if (k) return (bool)(trgt[k + n >> 6] >> (k + n & 63) & 1ULL);
	else return true;
}

inline bool rslv(eqn_t *x, u64 *trgt) {
	u8 *a = x->d, *b = x->d + x->a;
	u8 *c = x->d + x->a + x->b;
	bool ret = 0, cls = 0, skp = 0;
	u8 p, q = a[0], r = 3, s = b[0];
	u64 i = 0, j = 0, k = 0, l, m = 0;
	ret = slct(r, s, j, k, l, m, *a, *b, *trgt);
	for (; i < x->c; ++i) {
		for (u8 t = 0; t < 4; ++t) {
			p = c[i];
			switch (p & 0x03) {
				case 1:
					if (!cls) ret = slct(r, s, j, k, l, m, *a, *b, *trgt);
					cls = 0;
					break;
				case 2: 
					cls = 1;
					if (!ret) skp = 1;
					break;
				case 3:
					if (skp) skp = 0;
					ret ^= slct(r, s, j, k, l, m, *a, *b, *trgt);
			}
			p =>> 2;
		}
	}
	return ret;
}

inline void one(eqn_t *x) {
	u64 i = 0, j = x->a, l = 0;
	u8 p = x->d[i], q;
	while (i < x->a) {
		switch (p & 0x03) {
			case 1:
				++l;
				break;
			case 2:
				goto end:
			case 3:
				j += l + 1;
				l = 0;
		}
		p =>> 2;
		if (!p) p = x->d[++i]; 
	}
	end:
	x->d[j] ^= 0x01;
	x->d[x->a + x->b] ^= 0x03;
}

//(I) is not allowed.
inline void norm(bool *ntr, u64 *x_n, u64 *y_n, u64 *y_r, eqn_t *x, eqn_t *y) {
	if (!ntr && y->d[y->a + y->b] & 1) return false;
	
	u64 i, j, l_1, l_2;
	u8 p, q;
	
	i = 0, j = y->a, l_1 = 0, *y_n = 0;
	while (i < y->a) {
		p = y->d[i];
		switch (p & 0x03) {
			case 1:
				++l_1;
				break;
			case 2:
				j += l_1;
				goto y_found;
			case 3:
				switch (l_1) {
					case 0:
						*y_n = y->d[j++];
						break;
					case 1:
						*y_n = *(u16 *)(y->d + j);
						j += 2;
						break;
					case 2:
						*y_n = *(u32 *)(y->d + j) & 0x00ffffffUL;
						j += 3;
						break;
					case 3:
						*y_n = *(u32 *)(y->d + j);
						j += 4;
				}
				*y_r = j;
				l_1 = 0;
		}
		p =>> 2;
		if (!p) ++i;
	}
	y_found:
	q = y->d[j];
	*y_n += 7 - __builtin_clz(q);
	*y_n += l_1 << 3;
    
	i = x->a - 1, j = x->a + x->b - 1, l_1 = 0, l_2 = 0;
	while (i + 1 > 0) {
		p = x->d[i];
		if (p & 0xc0) {
			p =<< 2;
			--j;
			if (!p) p = x->d[--i];
			while (p & 0xc0 == 0x40) {
				++l_1;
				p =<< 2;
				--j;
				if (!p) p = x->d[--i];
			}
			switch (l_1) {
				case 0:
					*x_n = x->d[j];
					goto x_found;
				case 1:
					*x_n = *(u16 *)(x->d + j);
					goto x_found;
				case 2:
					*x_n = *(u32 *)(x->d + j) & 0xffffff00UL;
					goto x_found;
				case 3:
					*x_n = *(u32 *)(x->d + j);
					goto x_found;
			}
		} else if (p & 0xc0) ++l_2;
		p =<< 2;
		if (p & 0xc0) --j;
		if (!p) --i;
	}
	x_found:
	q = x->d[x->a + x->b - 1];
	*x_n += 7 -__builtin_clz(q);
	*x_n += l_2 << 3;
	
	if (*y_n > *x_n) {
		if (*y_n < 0x100) {
			return *x_n - *y_n < 8;
		} else if (*y_n < 0x10000) {
			return *x_n - *y_n < 16;
		} else if (*y_n < 0x1000000UL) {
			return *x_n - *y_n < 24;
		} else if (*y_n < 0x100000000ULL) {
			return *x_n - *y_n < 32;
		} else return false;
	} else return false;
}

inline void add(eqn_t *x, eqn_t *y, eqn_t *z) {
	u64 i, j, l, x_n, y_n, y_r, x_s = 0, y_s = 0;
	u8 p, q;
	bool adI, nrm;
	
	nrm = norm(true, &x_n, &y_n, &y_r, x, y);
	
	memcpy(x->d, z->d, x->a);
	z->a += x->a;
	
	p = z->d[x->a - 1];
	while (!(p & 0xc0)) {
		x_s += 2;
		p =<< 2;
	}
	
	//W.I.P.
	
	if (x_s) {
		
		
		
		z->d[z->a - 1] &= y->d[0] << 8 - x_s;
		for (u64 t = 1; t < y->a, ++t) z->d[z->a - 1 + t] = *(u16 *)(y->d + t) >> x_s; 
		z->d[z->a + y->a - 2] &= 0xff >> x_s;
		if (y->b & 7 < x_s >> 1) --z->a;
	} else memcpy(y->d, z->d + z->a, y->a);
	z->a += y->a;
	
	memcpy(x->d + x->a, z->d + z->a, x->b);
	z->a += x->b;
	
	i = 0, j = y->a, l = 0;
	p = y->d[i];
	while (i < y->a) {
		switch (p & 0x03) {
			case 1:
				++l;
				break
			case 2:
				goto one_found;
			case 3:
				j += l + 1;
		}
		++j;
		p =>> 2;
		if (!p) p = y->d[++i];
	}
	one_found:
	if (y->d[j] & 0x01) adI = true;
	
	if (adI) {
		i = 0, j = z->a, l = 0;
		p = z->d[i];
		while (i < z->a) {
			switch (p & 0x03) {
				case 1:
					++l;
					break;
				case 2:
					goto add_Ia;
				case 3:
					j += l + 1;
			}
			++j;
			p =>> 2;
			if (!p) p = z->d[++i];
		}
		add_Ia:
		z->d[j] ^= 0x01;
	}
	
	if (nrm) {
		then we incorporate the first y bitvector into the last x bitvector while masking I
		oh fk
		
	} else memcpy(y->d + y->a, z->d + z->a, y->b);
	z->b += y->b;
	
	memcpy
	
	//(x0+x1+x2+x3)(x5)+(x8+x11+x12)(x14)
	//10 01 
	//01111010-01001101 
	//00 10 11 11 11 01 10 01 11 10 11 11 01 10 01

}

inline void add_fst(eqn_t *x, eqn_t *y, eqn_t *z) {
	memcpy(x->d, z->d, x->a);
	z->a += x->a;
	memcpy(y->d, z->d + z->a, y->a);
	z->a += y->a;
	memcpy(x->d + x->a, z->d + z->a, x->b);
	z->b += x->b;
	memcpy(y->d + y->a, z->d + z->a + z->b, y->b);
	z->b += y->b;
	memcpy(x->d + x->a + x->b, z->d + z->a + z->b, x->c);
	z->c += x->c;
	z->d[z->a + z->b + z->c++] = 0x03;
	memcpy(y->d + y->a + y->b, z->d + z->a + z->b + z->c, y->c);
	z->c += y->c;
}

inline void mlt_fst(eqn_t *x, eqn_t *y, eqn_t *z) {
	memcpy(x->d, z->d, x->a);
	z->a += x->a;
	memcpy(y->d, z->d + z->a, y->a);
	z->a += y->a;
	memcpy(x->d + x->a, z->d + z->a, x->b);
	z->b += x->b;
	memcpy(y->d + y->a, z->d + z->a + z->b, y->b);
	z->b += y->b;
	z->d[z->a + z->b++] = 0x02;
	memcpy(x->d + x->a + x->b, z->d + z->a + z->b, x->c);
	z->c += x->c;
	z->d[z->a + z->b + z->c++] = 0x06;
	memcpy(y->d + y->a + y->b, z->d + z->a + z->b + z->c, y->c);
	z->c += y->c;
	z->d[z->a + z->b + z->c++] = 0x01;
}

typedef struct {
	u64 strt, size, free;
	u8 d[];
} fbr_t;
//matrix                      equations in order     
//<- size * size + 7 >> 2 -> | <- FIBR_LIMT * size * (size - 1) -> | <- growth < ~5*3^n <?=> 1024 * 1024 => ~10 stacked mlt add steps per

inline fbr_t *init_fbr(u64 i, u64 j, u64 size) {
	u16 *arr_16 = (u16 *)arr;
	fbr_t *f = calloc(24 + (size * size + 7 >> 2) + FIBR_LIMT * size * (size - 1), 1);
	f->strt = i * size;
	f->size = size;
	
	for (u64 k = 0; k < size - 1; ++k) {
		u8 a = arr_16[i * size + k >> 4] >> (i * size + k & 7);
		u8 b = arr_16[j * size + k >> 4] >> (j * size + k & 7);
		if (!(a & 1 << (k & 7)))
			f->d[((k + 1) * size >> 2)] &= 0x03 << ((k + 1 << 1) & 7);
	    } else if ((a ^ b) & 1 << (k & 7)) {
			f->d[((k + 1) * size >> 2)] &= 0x01 << ((k + 1 << 1) & 7);
		}
		f->d[(k * size >> 2)] &= 0x01 << ((k << 1) * size & 7);		
	}
	
	u64 t = size * size + 7 >> 2;
	
	f->free = pnt_pntrs[i].frei;
	eqn_t *e;
	for (u64 k = 0; k < f->free; ++k) {
		if (k < 7) {
			e = (eqn_t *)(f->d + t + k * 27);
			e->a = 1;
			e->b = 1;
			e->c = 0;
			e->d[0] = 2;
			e->d[1] = 1 << k + 1;
		} else if (k < 15) {
			e = (eqn_t *)(f->d + t + 189 + k * 28);
			e->a = 1;
			e->b = 2;
			e->c = 0;
			e->d[0] = 9;
			e->d[2] = 1 << k - 7;
		} else if (k < 0x100) {
			e = (eqn_t *)(f->d + t + 405 + k * 28);
			e->a = 1;
			e->b = 2;
			e->c = 0;
			e->d[0] = 11;
			e->d[1] = k - 1;
			e->d[2] = 2;
		} else if (k < 0x10000) {
			e = (eqn_t *)(f->d + t + 405 + 0xff * 28 + k * 28);
			e->a = 1;
			e->b = 3;
			e->c = 0;
			e->d[0] = 0x2d;
			*(u16 *)(e->d + 1) = k - 1;
			e->d[3] = 2;
		} else printf("maybe you should think about what you're doing...");
	}
	return f;
}

inline void adv_fbr(fbr_t *f, fbr_t *n, u64 i, u64 j) {
	u16 *arr_16 = (u16 *)arr;
	
	u8 p, q;
	u64 n = f->free, m = 0;
	for (u64 k = 0; k < f->size * f->size; ++k) {
		
		p = f->d[k >> 2];hmm
		
		
		
		
		
		
		
				
		
		
		u8 a = arr_16[i * f->size + m >> 4] >> (i * f->size + m & 7);
		u8 b = arr_16[j * f->size + m >> 4] >> (j * f->size + m & 7);
		
		if (!(a & (1 << (m & 7))) {
			eqn_t *e = calloc(28, 1);
			if (n < 7) {
				e->a = 1;
				e->b = 1;
				e->c = 0;
				e->d[0] = 2;
				e->d[1] = 1 << n + 1;
			} else if (n < 15) {
				e->a = 1;
				e->b = 2;
				e->c = 0;
				e->d[0] = 9;
				e->d[2] = 1 << n - 7;
			} else if (n < 0x100) {
				e->a = 1;
				e->b = 2;
				e->c = 0;
				e->d[0] = 11;
				e->d[1] = n - 1;
				e->d[2] = 2;
			} else if (n < 0x10000) {
				e->a = 1;
				e->b = 3;
				e->c = 0;
				e->d[0] = 0x2d;
				*(u16 *)(e->d + 1) = n - 1;
				e->d[3] = 2;
			} else printf("but, algebruh");
			
		}
		
		
		
		
		
		
		if (++m == size) m = 0;
	}
	
}

1 a01 a02 a03 
0 1 a12 a13 
0 0 1 a23 
0 0 0 1       

1 c1  0  0   
0  1  x  0  
0 0 1  c2 
0  0  0  1

1 a01+(c1) a02+b1a01 a03+(c2)a02 0 1 a12+b1 a13+(c2)a12 0 0 1 a23+c2 0 0 0 1

inline void rtr_fbr(fbr_t *f, u64 i, u64 j) {
	
}

inline void smpl_fbr(fbr_t *f, u64 *trgt) {
	
}


//<- size * size + 7 >> 3 -> | <- size * size + 7 >> 3 -> | <- FIBR_LIMT * size * (size - 1) -> | <- growth < ~5*3^n <?=> 1024 * 1024 => ~10 stacked mlt add steps per

// typedef struct {
// u64 a, b, c;
// u8 d[];
// } eqn_t;

// <- a -> | <- b -> | <- c -> 
  
  // typedef struct {
	// u64 maas, frei;
	// u64 v[];
// } point_t; 

// point_t *pnt_arr, **pnt_pntrs;
// u64 pnt_localizer, pnt_alloc;
  
typedef struct {
	u64 start, size, free, lenght;
	u64 p[];
} path_t;

path_t *path_arr, **path_pntrs;
u64 path_count, path_alloc;

void grow(u64 size) {              

    u64 ammo = ((count << 3) + size - 1) / size;
    u64 *arr_64 = (u64 *)arr;
    
    #pragma omp parallel
    {
		
		
        #pragma omp for schedule(dynamic)
		for (u64 i = 0; i < ammo; ++i) {
			if (!pnt_pntrs[i][0]) continue;
			c_bits[i >> 3] &= 1 << (i & 7);
			w_p.path->p[w_p.path->lenght] = i;
			
			
			for (u64 f = 0; f < size; ++f) {
				for (u64 j = 0; j < path_pntrs[i][
				if (pnt_pntrs[i]
				
				c_bits[i >> 3] &= 1 << (i & 7);
				w_p.path->p[w_p.path->lenght] = i;
				
				
				
				u64 temp = 0;
				for (u64 k = 0; k < size - 1; ++k) {
					u64 a = arr_64[i * size + k >> 6] >> (i * size + k & 63);
					u64 b = arr_64[j * size + k >> 6] >> (j * size + k & 63);
					w_p[w_p_c].mtrx[temp * size >> 6] &= 1 << (temp * size & 63);
					w_p[w_p_c].mtrx[temp * size + 1 >> 6] &= ((a ^ b) & 1 << (k & 63)) >> (64 - (k & 63)) << (temp++ * size + 1 & 63);
				}
				
				
				
				while (p) {
					again:
					u64 e = pnt_pntrs[nds[p]][3 + pth[p]];
					if (pth[p] < pnt_pntrs[nds[p]][0]) {
						if (!(bit[e >> 3] & 1 << (e & 7))) {
							
							pth_degree += pnt_pntrs[nds[p]][3 + pnt_pntrs[nds[p]][0] + pth[p]];
							
							bit[e >> 3] &= 1 << (e & 7);
							
							nds[p + 1] = pnt_pntrs[nds[p]][3 + pth[p++]++];
							
							
				
							}
							
							
							
							
						} else {
							++pth[p];
							goto again;
						}
					} else {
						bit[e >> 3] &= ~(1 << (e & 7))  ;
						pth[p--] = 0;
						++pth[p];
							
							





void compose() {
	
}

//LiFE
//linear fiber extension

int main(u16 argc, s8 *argv[]) {

    s8 *file = argv[1];

    if (GROW_FCTR > 8) return 1;

    load(file);

    scan();
	

	//next, a snek (or Wurst)
    return 0;

}
