/*
 *  Copyright (c) 2015-2016 Bastian Koppelmann
 *                          Peer Adelt
 *                          C-Lab/Paderborn University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDE_TB_ANNOTATION_H_
#define INCLUDE_TB_ANNOTATION_H_

#include <glib.h>

typedef struct tb_leaving_edge_tuple TbAnnotationLeavingEdgeTuple;
typedef struct tb_annotation_block TbAnnotationBlock;
typedef struct tb_annotation_edge TbAnnotationEdge;
typedef struct tb_annotation TbAnnotation;

struct tb_leaving_edge_tuple {
    TbAnnotationEdge *out1;
    TbAnnotationEdge *out2;
};

struct tb_annotation_block {
    uint8_t is_end_block;
    const char *id;
    unsigned int address;
    /* This hashtable points to all pairs of leaving edges
     * from all source contexts.
     * Note: string -> tb_leaving_edge_tuple
     */
    GHashTable *out_edges_hash_table;
};

struct tb_annotation_edge {
    TbAnnotationBlock *source;
    TbAnnotationBlock *target;
    const char *source_context;
    const char *target_context;
    unsigned int value;
};

struct tb_annotation {

    TbAnnotationBlock *last_block;
    const char *last_ctx;
    /* Hashes from PC to TbAnnotationBlock */
    GHashTable *tb_annotation_blocks;
    unsigned int value_sum;

};

#endif /* INCLUDE_TB_ANNOTATION_H_ */
