#ifndef __GB_io_H
#define __GB_io_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include "matrix.h"

#define max(a,b) \
	({ __typeof__ (a) _a = (a); \
	 __typeof__ (b) _b = (b); \
	 _a > _b ? _a : _b; })

#define ERROR_READING \
	if (verbose > 0) { \
		printf("Error while reading file '%s'\n",fn); \
		fclose(fh); \
		return -1 ; \
	}

#define SWAP(a,b)  \
	t = a ; \
        a = b ; \
        b = t

void insert_sort(uint32_t * liste, uint32_t  size)
{
	uint32_t d , c = 1 , t ;
	for ( ; c < size ; ++c) {
		d = c;
		while ( d > 0 && liste[d] < liste[d-1]) {
			SWAP(liste[d],liste[d-1]);
			d--;
		}
	}
}


void insert_sort(uint32_t * liste, uint32_t  size, uint32_t * copain)
{
	uint32_t d , c = 1 , t ;
	for ( ; c < size ; ++c) {
		d = c;
		while ( d > 0 && liste[d] < liste[d-1]) {
			SWAP(liste[d],liste[d-1]);
			SWAP(copain[d],copain[d-1]);
			d--;
		}
	}
}
#undef SWAP


void copy( uint32_t * to , uint32_t * from, uint32_t size)
{
	uint32_t i = 0 ;
	for ( ; i < size ; ++i) {
		to[i] = from[i] ;
	}
}


/* buffer:
 */
void getSparsestRows(uint32_t * colid
		, uint64_t * start
		, uint32_t row
		, uint32_t * pivots /* pivots[j] is the sparsest row with j as pivot */
		// , uint32_t * sparse
		// , uint32_t * discard /* discard[k] = 1 iff this row is not sparsest */
		// , uint32_t * discarded
		// , uint32_t i
		)
{
	SAFE_MALLOC_DECL(sparse,row,uint32_t);
	uint32_t j = 0;
	for ( ; j < row ; ++j) {
		uint32_t pivot = colid[start[j]] ; /* first column in each row */
		uint32_t creux = start[j+1]-start[j] ; /* length of the row */
		if (sparse[j] > creux) { /* this row is sparsest */
			sparse[j] = creux ;
			pivots[j]  = pivot ;
		}
	}

	return ;
}

#if 0
void insert_row(CSR_zo * A, uint32_t * colid, uint32_t nnz, uint32_t pol)
{
	A->row += 1 ;
	A->map_zo_pol = (uint32_t*) realloc(A->map_zo_pol,A->row*sizeof(uint32_t));
	A->map_zo_pol[A->row-1] = pol ;
	uint32_t last = A->nnz();
	A->start_zo = (uint32_t *) realloc(A->start_zo,(A->row+1)*sizeof(uint32_t));
	A->start_zo[A->row] = last+nnz ;
	A->colid_zo = (uint32_t *) realloc(A->colid_zo, A->nnz() *sizeof(uint32_t));
	for (uint32_t i = 0 ; i < nnz ; ++i) {
		A->colid_zo[last+i] = colid[i] ;
	}
	return ;
}

void fix_last_row(
		GBMatrix_t * A
		GBMatrix_t * C
		)
{
	CSR_zo * A_mat = A->matrix_zo[A->matrix_nb-1] ;
	CSR_zo * B_mat = C->matrix_zo[C->matrix_nb-1] ;
	if (B_mat->row == MAT_ROW_BLOCK) {
		B_mat->matrix_nb += 1 ;
		B_mat->matrix_zo = (CSR_zo  *) realloc(B_mat->matrix_zo, B_mat->matrix_nb * sizeof(CSR_zo));
		B_mat.init();
	}
	uint32_t last_row_size = A_mat->nnz() ;

	insert_row(B_mat,A_mat->getRow(A_mat->row),last_row_size, A_mat->map_zo_pol[A_mat->row]);

	A_mat->row -= 1 ;
}

#endif

void splitRowsUpBottom(
		GBMatrix_t * A
		, GBMatrix_t * C
		, uint32_t  * colid
		, uint64_t * start
		// , uint64_t nnz
		, uint32_t row
		, uint32_t * pivots
		, uint32_t * pols
		)
{
	// checkFull(A_init)  ;
	// checkFull(B_init);

	uint32_t last_pivot=0;
	uint32_t i = 0 ;
	for ( ; i < row ; ++i)  {
		if (i == pivots[last_pivot]) {
			appendRow(A,colid+start[i],start[i+1]-start[i],pols[row]);
			++last_pivot;
		}
		else {
			appendRow(C,colid+start[i],start[i+1]-start[i],pols[row]);
		}
		// until next non discarded
		// populate C ;
		// is discarded
		// populate A
	}
	return ;
}


/* matrix reader and row splitter
 * A_init [out] the top part with upper triangular left
 * B_init [out] the bottom part
 * polys [out] the polynomials used in the matrices (shared)
 * fh [in] the file in appropriate format
 */
int read_file(struct GBMatrix_t * A_init, struct GBMatrix_t * B_init
		, struct CSR_pol * polys
		, FILE * fh
		// , int verbose = 0
		)
{
	uint32_t i = 0 ;

	/* sanity */
	assert(fh);

	/* format */
	SAFE_READ_DECL_V(b,uint32_t,fh);
	/* XXX set TYPE here and C++-ise*/

	/* READ in row col nnz mod */
	SAFE_READ_DECL_V(m,uint32_t,fh);
	SAFE_READ_DECL_V(n,uint32_t,fh);
	SAFE_READ_DECL_V(mod,uint32_t,fh);
	assert(mod > 1);
	SAFE_READ_DECL_V(nnz,uint32_t,fh);


	/* READ in ZO start */
	SAFE_READ_DECL_P(start_zo,m+1,uint64_t,fh);

	/* largest row */
	uint32_t big_row = 0 ;
	for (i = 0 ; i < m ; ++i)
		big_row = max(big_row,start_zo[i]);

	/* pol/zo correspondance */
	SAFE_READ_DECL_P(map_zo_pol,m,uint32_t,fh);

	SAFE_MALLOC_DECL(map_pol_zo_A,m,uint32_t);
	uint32_t map_pol_zo_A_size = 0 ;

	SAFE_MALLOC_DECL(map_pol_zo_B,m,uint32_t);
	uint32_t map_pol_zo_B_size = 0 ;


	/* colid in ZO */
	SAFE_READ_DECL_V(colid_size,uint64_t,fh);
	SAFE_MALLOC_DECL(buffer,colid_size,uint32_t); /* buffer has the matrix */

	uint32_t last_start = 0 ;
	i = 0 ;
	/* FIXME */
	{
		SAFE_MALLOC_DECL(pivots,m,uint32_t);

		getSparsestRows(buffer,start_zo,m, pivots);
		SAFE_MALLOC_DECL(A,1,GBMatrix_t);
		SAFE_MALLOC_DECL(C,1,GBMatrix_t);
		// GROW A,C with seleted pivots
		splitRowsUpBottom(A,C,buffer,start_zo,m,pivots,map_zo_pol);
		// ADD POLYS
	}


	free(buffer);

	// size of nnz for pols:
	SAFE_READ_DECL_V(size_pol,uint64_t,fh);

	// create GBpolynomials shared by A_init and B_init
	SAFE_READ_DECL_P(start_pol,m,uint32_t,fh);
	// populate A and C

	SAFE_READ_DECL_P(vals_pol,size_pol,uint32_t,fh);
	// populate A and C

	return 0 ;
}

uint32_t get_permute_columns(
		GBMatrix_t * A
		// , GBMatrix_t * C
		, uint32_t *  perm
		)
{
	uint32_t trans = 0 ;
	SAFE_MALLOC_DECL(col_size,A->col,uint32_t);
	SAFE_MALLOC_DECL(last_elt,A->row,uint32_t);
	// copy last columns of A,C to B,D

	for (uint32_t k = 0 ; k < A->matrix_nb ; ++k ) {
		CSR_zo * A_k = &(A->matrix_zo[k]) ;

		for (uint32_t i = 0 ; i < A_k->row ; ++i) {
			for (uint32_t j = A_k->start_zo[i] ; j < A_k->start_zo[i+1] ; ++j) {
				col_size[A_k->colid_zo[j]] += 1 ;
				last_elt[A_k->colid_zo[j]] = i ;
			}
		}
	}
	for (uint32_t j = A->row ; j < A->col ; ++j) {
		uint32_t k = last_elt[j] ;
		if (perm[k] != 0 && (col_size[perm[k]] > col_size[j]));
		perm[k] = j ;
		++trans;
	}
}


void do_permute_columns_up(
		GBMatrix_t * A_init,
		GBMatrix_t * A,
		GBMatrix_t * B
		, uint32_t * perm_i
		, uint32_t * perm_j
	)
{

	for (uint32_t blk = 0 ; blk < A_init->matrix_nb ; ++blk) {
		CSR_zo * A_k = &(A_init->matrix_zo[blk]);

		CSR_zo * Ad = getLastMatrix(A);
		/* apply permutations (the rows keep the same length : in place */
		uint32_t here = 0 ;

		for (uint32_t i = 0 ; i < A_k->row ; ++i) {
			for (uint32_t j = A_k->start_zo[i] ; j < A_k->start_zo[i+1] ; ++j) {
				if (A_k->colid_zo[j] == perm_i[here])
					A_k->colid_zo[j] = perm_j[here++] ;
			}
		}
		for (uint32_t i = 0 ; i < A_k->row ; ++i) {
			uint32_t j0 = A_k->start_zo[i] ;
			uint32_t j1 = A_k->start_zo[i+1] ;
			insert_sort(&A_k->colid_zo[j0], j1-j0) ;
		}

		/* get begining of B */
		SAFE_MALLOC_DECL(start_b,A_k->row+1,uint32_t);
		for (uint32_t i = 0 ; i < A_k->row ; ++i) {
			uint32_t j0 = A_k->start_zo[i] ;
			uint32_t j1 = A_k->start_zo[i+1] ;
			while(j0 < j1) /* and A_k->start_zo[j0] < k) */
				++j0;
			start_b[i+1] = j0 ;
		}
		for (uint32_t i = 0 ; i < A_k->row ; ++i) {
			uint32_t b = start_b[i+1]-A_k->start_zo[i];
			A_k->start_zo[i+1]+= b;
			for (uint32_t j = A_k->start_zo[i] ; j < start_b[i+1] ; ++j) {
				copy(A_k->colid_zo+A_k->start_zo[i], A_k->colid_zo+A_k->start_zo[i], b); /* faux */
			}
		}

		uint32_t k = A_init->row ; /* faux */

		CSR_zo * Bt = getLastMatrix(B);
		uint32_t size = A_k->nnz - A_k->nnz;
		for (uint32_t i = 0 ; i < A_k->row ; ++i) {
			for (uint32_t j = start_b[i+1] ; j < start_b[i+1] ; ++j) {
				Bt->start_zo[A_k->colid_zo[j]-k] += 1 ;

			}
		}
		for (uint32_t i = 0 ; i < A_k->col-k ; ++i) {
			Bt->start_zo[i+1] += Bt->start_zo[i] ;
		}

		SAFE_CALLOC_DECL(done_col,A_k->row,uint32_t);
		for (uint32_t nextlig = 1 ; nextlig <= A_k->row ; ++nextlig) {
			uint32_t i = start_b[nextlig];
			while (i < A_k->start_zo[nextlig]){
				uint32_t cur_place = Bt->start_zo[A_k->colid_zo[i]-k];
				cur_place  += done_col[A_k->colid_zo[i]-k] ;
				Bt->colid_zo[ cur_place ] =  nextlig-1 ;
				done_col[A_k->colid_zo[i]-k] += 1 ;
				++i;
			}
		}

	}
}

#if 0

void do_permute_columns_lo(
		GBMatrix_t * C,
		GBMatrix_t * D
		, uint32_t * perm
	)
{
}

#endif



// transforms perm where perm[i] = j into two lists
// such that ther permutations are (perm_i[k],perm_j[k]) for perm and perm^(-1).
// perm_i is increasing.
uint32_t split_permutations(
		uint32_t * perm
		, uint32_t perm_size
		, uint32_t * perm_i
		, uint32_t * perm_j)
{
	uint32_t here ;
	for (uint32_t i = 0 ; i < perm_size ; ++i)
		if (perm[i] != i) {
			perm_i[here] = i ;
			perm_j[here] = perm[i] ;
			++here;
		}
	for (uint32_t i = 0 ; i < perm_size ; ++i)
		if (perm[i] != i) {
			perm_i[here] = perm[i] ;
			perm_j[here] = i ;
			++here;
		}

	/* the last ones are not necessarily in order */
	insert_sort(perm_i+here/2,here/2,perm_j+here/2);
	return here ;
}

int split_columns(
		GBMatrix_t * A_init
		, GBMatrix_t * C_init
		, GBMatrix_t * A
		, GBMatrix_t * Bt
		, GBMatrix_t * C
		, DenseMatrix_t * D
		)
{
	/* search for columns to permute to make A sparser */
	SAFE_MALLOC_DECL(perm,A->row,uint32_t); /* what columns the first row ones should be exchanged with */
	for (uint32_t i = 0 ; i < A->row ; ++i)
		perm[i] = i ;
	uint32_t trans = get_permute_columns(A,perm);
	SAFE_MALLOC_DECL(perm_i,2*trans,uint32_t);
	SAFE_MALLOC_DECL(perm_j,2*trans,uint32_t);
	split_permutations(perm,A->row,perm_i,perm_j);

	// copy last columns of A,C to B,D in proper format
	do_permute_columns_up(A_init,A,C,perm_i, perm_j);
	// do_permute_columns_lo(B,D,perm);
}


#endif // __GB_io_H
