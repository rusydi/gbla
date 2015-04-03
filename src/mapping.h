/* gbla: Gröbner Basis Linear Algebra
 * Copyright (C) 2015 Christian Eder <ederc@mathematik.uni-kl.de>
 * This file is part of gbla.
 * gbla is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * gbla is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with gbla . If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file mapping.h
 * \brief Interfaces for sparse matrix index maps used for subdividing the matrix
 * in Faugère-Lachartre style
 *
 * \author Christian Eder <ederc@mathematik.uni-kl.de>
 */

#ifndef GB_MAPPING_H
#define GB_MAPPING_H

#include <gbla_config.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <matrix.h>
#include <elimination.h>
#include <math.h>
#include <omp.h>

/**
 * \brief Indexer for subdividing sparse matrix into 4 parts as described by
 * Faugère and Lachartre in http://dx.doi.org/10.1145/1837210.1837225.
 *
 *                 A | B
 * M     ---->     --+--
 *                 C | D
 * In the subdivision the following dimensions hold:
 * A->nrows = B->nrows = map->npiv // number of pivots found
 * C->nrows = D->nrows = M->nrows - map->npiv // non-pivots
 * A->ncols = C->ncols = map->npiv
 * B->ncols = D->ncols = M->ncols - map->npiv
 */

typedef struct map_fl_t {
  ri_t npiv;      /*!<  number of pivots found in input matrix */
  ci_t *pc;       /*!<  map of pivot columns: from input matrix M to
                        submatrix A; has length M->ncols, maps non-pivot
                        columns to __GB_MINUS_ONE_32 */
  ci_t *npc;      /*!<  map of non-pivot columns: from input matrix M to
                        submatrix B; has length M->ncols, maps pivot
                        columns to __GB_MINUS_ONE_32 */
  ci_t *pc_rev;   /*!<  reverse map of pivot columns: from submatrices
                        A and B to input matrix M */
  ci_t *npc_rev;  /*!<  reverse map of non-pivot columns: from submatrices
                        A and B to input matrix M */
  ri_t *pri;      /*!<  has length M->nrows, maps pivot columns to
                        their corresponding row index, maps non-pivot
                        columns to __GB_MINUS_ONE_32 */
  ri_t *npri;     /*!<  indices of non-pivot rows */

  int nthrds;     /*!<  number of threads to be used for the indexer */
} map_fl_t;


/**
 * \brief Initializes a map for the input matrix M with all entries set
 * to __GB_MINUS_ONE_8.
 *
 * \param size of columns col_size
 *
 * \param size of rows row_size
 *
 * \param map to be initialized
 *
 */
/* static */ /* inline */ void init_fl_map_sizes(uint32_t col_size, uint32_t row_size, map_fl_t *map);

/**
 * \brief Initializes a map for the input matrix M with all entries set
 * to __GB_MINUS_ONE_8.
 *
 * \param original matrix M
 *
 * \param map to be initialized
 *
 */
/* static */ /* inline */ void init_fl_map(sm_t *M, map_fl_t *map) ;

/**
 * \brief Process multiline matrix and applies mapping for it
 *
 * \param multiline matrix A
 *
 * \param map to be defined map
 *
 * \param block height of the corresponding block matrices bheight
 */
void process_matrix(sm_fl_ml_t *A, map_fl_t *map, const bi_t bheight);

/**
 * \brief Combines the mappings from the outer input matrix A and the
 * echelonized multiline matrix D
 *
 * \param mapping of the input matrix M outer_map
 *
 * \param pointer to mapping of the echelonized multiline matrix D. Will be
 * freed at the end. inner_map_in
 *
 * \param column dimension of input matrix M outer_coldim
 *
 * \param column dimension of multiline matrix D inner_coldim
 *
 * \param parameter that defines if only the new pivots are remap, i.e. no
 * columns are considered. This is done when the echelon form is wanted and not
 * the RREF
 */
void combine_maps(map_fl_t *outer_map, map_fl_t **inner_map_in,
    const ci_t outer_coldim, const ci_t inner_coldim,
    int only_rows);

/**
 * \brief Reconstructs matrix M after elimination process
 *
 * \param input and output matrix M
 *
 * \param block sub matrix A
 *
 * \param block sub matrix B
 *
 * \param multiline submatrix D
 *
 * \param outer mapping map
 *
 * \param column dimension of M coldim
 *
 * \param parameter to set/unset freeing of sub matrices free_matrices
 *
 * \param paramter indicating if M was already freed M_freed
 *
 * \param paramter indicating if A was already freed A_freed
 *
 * \param paramter indicating if D was already freed D_freed
 *
 * \param number of threads
 */
void reconstruct_matrix_block(sm_t *M, sbm_fl_t *A, sbm_fl_t *B, sm_fl_ml_t *D,
    map_fl_t *map, const ci_t coldim, int free_matrices,
    int M_freed, int A_freed, int D_freed, int nthrds);

/**
 * \brief Reconstructs matrix M after full reduced row echelon process
 *
 * \param input and output matrix M
 *
 * \param block sub matrix A
 *
 * \param block sub matrix B2
 *
 * \param block sub matrix D2
 *
 * \param outer mapping map
 *
 * \param column dimension of M coldim
 *
 * \param parameter to set/unset freeing of sub matrices free_matrices
 *
 * \param paramter indicating if M was already freed M_freed
 *
 * \param paramter indicating if A was already freed A_freed
 *
 * \param paramter indicating if D was already freed D_freed
 *
 * \param number of threads
 */
void reconstruct_matrix_block_reduced(sm_t *M, sbm_fl_t *A, sbm_fl_t *B2, sbm_fl_t *D2,
    map_fl_t *map, const ci_t coldim, int free_matrices,
    int M_freed, int A_freed, int D_freed, int nthrds);

/**
 * \brief Reconstructs matrix M after elimination process
 *
 * \param input and output matrix M
 *
 * \param multiline sub matrix A
 *
 * \param block sub matrix B
 *
 * \param multiline submatrix D
 *
 * \param outer mapping map
 *
 * \param column dimension of M coldim
 *
 * \param parameter to set/unset freeing of sub matrices free_matrices
 *
 * \param paramter indicating if M was already freed M_freed
 *
 * \param paramter indicating if A was already freed A_freed
 *
 * \param paramter indicating if D was already freed D_freed
 *
 * \param number of threads
 */
void reconstruct_matrix_ml(sm_t *M, sm_fl_ml_t *A, sbm_fl_t *B, sm_fl_ml_t *D,
    map_fl_t *map, const ci_t coldim, int free_matrices,
    int M_freed, int A_freed, int D_freed, int nthrds);

/**
 * \brief Reallocates memory for the rows of the multiline during the splicing of
 * the input matrix. The buffer size buffer_A is doubled during this process
 *
 * \param multiline sub matrix A
 *
 * \param multiline index mli
 *
 * \param buffer size buffer_A
 */
/* static */ /* inline */ void realloc_rows_ml(sm_fl_ml_t *A, const mli_t mli,
    const bi_t init_buffer_A, mli_t *buffer_A);

/**
 * \brief Reallocates memory for the rows of the blocks during the splicing of
 * the input matrix. The buffer size buffer_A is doubled during this process
 *
 * \param block matrix A
 *
 * \param row block index rbi in A
 *
 * \param block index in row bir
 *
 * \param block index in row bir
 *
 * \param line index in block lib
 *
 * \param buffer size buffer_A
 *
 */
/* static */ /* inline */ void realloc_block_rows(sbm_fl_t *A, const ri_t rbi, const ci_t bir,
    const bi_t lib, const bi_t init_buffer_A, bi_t *buffer_A) ;

/**
 * \brief Swaps data arrangement in leftsided block matrices
 *
 * \param block matrix A
 *
 * \param number of blocks in the corresponding row clA
 *
 * \param current row index for blocks rbi
 *
 * \param current line in block lib
 *
 */
/* static */ /* inline */ void swap_block_data(sbm_fl_t *A, const ci_t clA, const bi_t rbi,
    const bi_t cvb) ;


/**
 * \brief Inserts elements from input matrix M in multiline rows of A corresponding
 * to the given splicing and mapping precomputed. This is the version for multi
 * line rows inserting one entry in the first field, 0 in the second field.
 *
 * \param multiline sub matrix A
 *
 * \param original matrix M
 *
 * \param current multiline index mli
 *
 * \param position of the element in line of the block eil
 *
 * \param row index of corresponding element in M bi1
 *
 * \param index in row bi1 of corresponding element in M i1
 *
 */
/* static */ /* inline */ void insert_row_data_ml_1_1(sm_fl_ml_t *A, const sm_t *M,
    const mli_t mli, const ci_t eil, const ci_t bi1, const ci_t i1) ;

/**
 * \brief Inserts elements from input matrix M in multiline rows of A corresponding
 * to the given splicing and mapping precomputed. This is the version for multi
 * line rows inserting one entry in the second field, 0 in the first field.
 *
 * \param multiline sub matrix A
 *
 * \param original matrix M
 *
 * \param current multiline index mli
 *
 * \param position of the element in line of the block eil
 *
 * \param row index of corresponding element in M bi2
 *
 * \param index in row bi1 of corresponding element in M i2
 *
 */
/* static */ /* inline */ void insert_row_data_ml_1_2(sm_fl_ml_t *A, const sm_t *M,
    const mli_t mli, const ci_t eil, const ci_t bi2, const ci_t i2) ;

/**
 * \brief Inserts elements from input matrix M in multiline rows of A corresponding
 * to the given splicing and mapping precomputed. This is the version for multi
 * line ml inserting two entries.
 *
 * \param multiline sub matrix A
 *
 * \param original matrix M
 *
 * \param current multiline index mli
 *
 * \param position of the element in line of the block eil
 *
 * \param row index of corresponding element in M bi1
 *
 * \param index in row bi1 of corresponding element in M i1
 *
 * \param row index of corresponding element in M bi2
 *
 * \param index in row bi1 of corresponding element in M i2
 *
 */
/* static */ /* inline */ void insert_row_data_ml_2(sm_fl_ml_t *A, const sm_t *M,
    const mli_t mli, const ci_t eil, const ci_t bi1, const ci_t i1,
    const ci_t bi2, const ci_t i2) ;


/**
 * \brief Inserts elements from input matrix M in block rows of A corresponding
 * to the given splicing and mapping precomputed. This is the version for multi
 * line blocks inserting one entry in the first field, 0 in the second field.
 *
 * \param block matrix A
 *
 * \param original matrix M
 *
 * \param current row index for blocks rbi
 *
 * \param column index for the block in the current block row bir
 *
 * \param current line in block lib
 *
 * \param position of the element in line of the block eil
 *
 * \param row index of corresponding element in M bi1
 *
 * \param index in row bi1 of corresponding element in M i1
 *
 */
/* static */ /* inline */ void insert_block_row_data_ml_1_1(sbm_fl_t *A, const sm_t *M,
    const bi_t rbi, const bi_t bir, const bi_t lib, const bi_t eil,
    const ci_t bi1, const ci_t i1) ;

/**
 * \brief Inserts elements from input matrix M in block rows of A corresponding
 * to the given splicing and mapping precomputed. This is the version for multi
 * line blocks inserting one entry in the second field, 0 in the first field.
 *
 * \param block matrix A
 *
 * \param original matrix M
 *
 * \param current row index for blocks rbi
 *
 * \param column index for the block in the current block row bir
 *
 * \param current line in block lib
 *
 * \param position of the element in line of the block eil
 *
 * \param row index of corresponding element in M bi2
 *
 * \param index in row bi1 of corresponding element in M i2
 *
 */

/* static */ /* inline */ void insert_block_row_data_ml_1_2(sbm_fl_t *A, const sm_t *M,
    const bi_t rbi, const bi_t bir, const bi_t lib, const bi_t eil,
    const ci_t bi2, const ci_t i2) ;

/**
 * \brief Inserts elements from input matrix M in block rows of A corresponding
 * to the given splicing and mapping precomputed. This is the version for multi
 * line blocks inserting two entries.
 *
 * \param block matrix A
 *
 * \param original matrix M
 *
 * \param current row index for blocks rbi
 *
 * \param column index for the block in the current block row bir
 *
 * \param current line in block lib
 *
 * \param position of the element in line of the block eil
 *
 * \param row index of corresponding element in M bi1
 *
 * \param index in row bi1 of corresponding element in M i1
 *
 * \param row index of corresponding element in M bi2
 *
 * \param index in row bi1 of corresponding element in M i2
 *
 */
/* static */ /* inline */ void insert_block_row_data_ml_2(sbm_fl_t *A, const sm_t *M,
    const bi_t rbi, const bi_t bir, const bi_t lib, const bi_t eil,
    const ci_t bi1, const ci_t i1, const ci_t bi2, const ci_t i2) ;

/**
 * \brief Constructs a mapping for splicing the BD part for the second round of
 * FL reduction when performing a reduced row echelon form computations
 *
 * \param new indexer map map
 *
 * \param indexer map from D map_D
 *
 * \param number of rows of BD rows_BD
 *
 * \param rank of D rank_D
 *
 * \param column dimension of B and D coldim
 *
 * \param number of threads nthreads
 */
void construct_fl_map_reduced(map_fl_t *map, map_fl_t *map_D, ri_t rows_BD,
    ri_t rank_D, ci_t coldim, int nthreads);

/**
 * \brief Constructs an indexer map for a Faugère-Lachartre decomposition of the
 * original matrix M
 *
 * \param original matrix M
 *
 * \param indexer map for M
 */
void construct_fl_map(sm_t *M, map_fl_t *map);

/**
 * \brief Constructs the subdivision of M into ABCD in the
 * Faugère-Lachartre style
 *
 *                 A | B
 * M     ---->     --+--
 *                 C | D
 * In the subdivision the following dimensions hold:
 * A->nrows = B->nrows = map->npiv // number of pivots found
 * C->nrows = D->nrows = M->nrows - map->npiv // non-pivots
 * A->ncols = C->ncols = map->npiv
 * B->ncols = D->ncols = M->ncols - map->npiv
 *
 *  \param original matrix M
 *
 *  \param block submatrix A
 *
 *  \param block submatrix B
 *
 *  \param block submatrix C
 *
 *  \param block submatrix D
 *
 *  \param indexer mapping map
 *
 *  \param dimension of blocks block_dim
 *
 *  \param number of rows per multiline rows_multiline
 *
 *  \param destructing input matrix on the go? destruct_input_matrix
 *
 *  \param number of threads to be used nthreads
 *
 *  \param level of verbosity
 *
 *  \param checks if map was already defined outside map_defined
 */
void splice_fl_matrix(sm_t *M, sbm_fl_t *A, sbm_fl_t *B, sbm_fl_t *C, sbm_fl_t *D,
                      map_fl_t *map, ri_t complete_nrows, ci_t complete_ncols,
                      int block_dim, int rows_multiline,
                      int nthreads, int destruct_input_matrix, int verbose,
                      int map_defined);

/**
 * \brief Constructs the subdivision of M into ABCD in the
 * Faugère-Lachartre style
 *
 *                 A | B
 * M     ---->     --+--
 *                 C | D
 * In the subdivision the following dimensions hold:
 * A->nrows = B->nrows = map->npiv // number of pivots found
 * C->nrows = D->nrows = M->nrows - map->npiv // non-pivots
 * A->ncols = C->ncols = map->npiv
 * B->ncols = D->ncols = M->ncols - map->npiv
 *
 *  \note Submatrix A is in multiline format.
 *
 *  \param original matrix M
 *
 *  \param multiline submatrix A
 *
 *  \param block submatrix B
 *
 *  \param block submatrix C
 *
 *  \param block submatrix D
 *
 *  \param indexer mapping map
 *
 *  \param dimension of blocks block_dim
 *
 *  \param number of rows per multiline rows_multiline
 *
 *  \param destructing input matrix on the go? destruct_input_matrix
 *
 *  \param number of threads to be used nthreads
 *
 *  \param level of verbosity
 */
void splice_fl_matrix_ml_A(sm_t *M, sm_fl_ml_t *A, sbm_fl_t *B, sbm_fl_t *C, sbm_fl_t *D,
                      map_fl_t *map, int block_dim, int rows_multiline,
                      int nthreads, int destruct_input_matrix, int verbose);

/**
 * \brief Constructs the subdivision of M into ABCD in the
 * Faugère-Lachartre style
 *
 *                 A | B
 * M     ---->     --+--
 *                 C | D
 * In the subdivision the following dimensions hold:
 * A->nrows = B->nrows = map->npiv // number of pivots found
 * C->nrows = D->nrows = M->nrows - map->npiv // non-pivots
 * A->ncols = C->ncols = map->npiv
 * B->ncols = D->ncols = M->ncols - map->npiv
 *
 *  \note Submatrices A and C are in multiline format.
 *
 *  \param original matrix M
 *
 *  \param multiline submatrix A
 *
 *  \param block submatrix B
 *
 *  \param multiline submatrix C
 *
 *  \param block submatrix D
 *
 *  \param indexer mapping map
 *
 *  \param dimension of blocks block_dim
 *
 *  \param number of rows per multiline rows_multiline
 *
 *  \param destructing input matrix on the go? destruct_input_matrix
 *
 *  \param number of threads to be used nthreads
 *
 *  \param level of verbosity
 */
void splice_fl_matrix_ml_A_C(sm_t *M, sm_fl_ml_t *A, sbm_fl_t *B, sm_fl_ml_t *C,
                      sbm_fl_t *D, map_fl_t *map, int block_dim, int rows_multiline,
                      int nthreads, int destruct_input_matrix, int verbose);


/**
 * \brief Writes corresponding entries of original matrix M into the block
 * submatrices A and B. The entries are defined by the mappings from M given by
 * rihb, crb and rbi:
 * parts of M --> A|B
 *
 * \param original matrix M
 *
 * \param block submatrix A (left side)
 *
 * \param block submatrix B (right side)
 *
 * \param splicer mapping map  that stores pivots and non pivots
 *
 * \param row indices in horizonal block rihb
 *
 * \param current row block crb
 *
 * \param row block index rbi
 *
 * \param level of density of initial buffer for submatrix splicing
 * density_level: If set to 0 then we assume the upper part of the splicing is
 * done, which means a very sparse left part and denser right part. If set to 1
 * then we assume that the lower part of the splicing is done, with a hybrid
 * left part and a very dense right part.
 */
void write_blocks_lr_matrix(sm_t *M, sbm_fl_t *A, sbm_fl_t *B, map_fl_t *map,
        ri_t *rihb, const ri_t cvb, const ri_t rbi, const bi_t density_level);


/**
 * \brief Writes corresponding entries of original matrix M into the mutiline
 * submatrix A and the block submatrix B. The entries are defined by the
 * mappings from M given by rihb, crb and rbi:
 * parts of M --> A|B
 *
 * \note The main differene to the function write_blocks_lr_matrix() is that the
 * lefthand side sub matrix A is in multiline row format and not in block format.
 * The handling of the righthand side block sub matrix B is exactly the same.
 *
 * \param original matrix M
 *
 * \param multiline submatrix A (left side)
 *
 * \param block submatrix B (right side)
 *
 * \param splicer mapping map  that stores pivots and non pivots
 *
 * \param row indices in horizonal block rihb
 *
 * \param current row block crb
 *
 * \param row block index rbi
 *
 * \param level of density of initial buffer for submatrix splicing
 * density_level: If set to 0 then we assume the upper part of the splicing is
 * done, which means a very sparse left part and denser right part. If set to 1
 * then we assume that the lower part of the splicing is done, with a hybrid
 * left part and a very dense right part.
 */
void write_lr_matrix_ml(sm_t *M, sm_fl_ml_t *A, sbm_fl_t *B, map_fl_t *map,
        ri_t *rihb, const ri_t cvb, const ri_t rbi, const bi_t density_level);
#endif

/* vim:sts=2:sw=2:ts=2:
 */
