/* gbla: Gröbner Basis Linear Algebra
 * Copyright (C) 2015 Brice Boyer <brice.boyer@lip6.fr>
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


#ifndef __GBLA_types_H
#define __GBLA_types_H


/* OLD FORMAT */
#define elem_o       uint16_t
#define stor_t       uint32_t  /* Element representation mod p on file */
#define larg_t       uint64_t  /* Element representation mod p on file */
/* NEW FORMAT */
#define elemt_s  uint16_t /* field element storage */
#define elemt_m  uint16_t /* modulo storage */
#define elemt_t  double   /* field element type in computations  */

#define SAME_READ_TYPE 1 /* elemt_s == elemt_t */

#define index_t       uint64_t  /* indexing elements */
#define dimen_t       uint32_t /* size_t */


#define NEGMASK (1U<<31)
#define VERMASK (1U<<31)
#define OLDMASK (0xFFFF0000)
/* #define MARKER32 UINT32_MAX */
/* #define MARKER32 (~0) */
/* #define MARKER32 ((uint32_t)-1)*/
#define MARKER32 (0xFFFFFFFF)
#define MARKER16 (0x0000FFFF)
#define MARKER08 (0x000000FF)





#endif /* __GBLA_types_H */
/* vim: set ft=c: */
