/*
 * Tiny Code Generator for QEMU
 *
 * Copyright (c) 2008 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <stdint.h>
#include "qemu/host-utils.h"

/* This file is compiled once, and thus we can't include the standard
   "exec/helper-proto.h", which has includes that are target specific.  */

#include "exec/helper-head.h"

#ifdef CONFIG_TB_ANNOTATION
#include "tb-annotation/tb-annotation.h"
#endif

#define DEF_HELPER_FLAGS_2(name, flags, ret, t1, t2) \
  dh_ctype(ret) HELPER(name) (dh_ctype(t1), dh_ctype(t2));

#include "tcg-runtime.h"


/* 32-bit helpers */

int32_t HELPER(div_i32)(int32_t arg1, int32_t arg2)
{
    return arg1 / arg2;
}

int32_t HELPER(rem_i32)(int32_t arg1, int32_t arg2)
{
    return arg1 % arg2;
}

uint32_t HELPER(divu_i32)(uint32_t arg1, uint32_t arg2)
{
    return arg1 / arg2;
}

uint32_t HELPER(remu_i32)(uint32_t arg1, uint32_t arg2)
{
    return arg1 % arg2;
}

/* 64-bit helpers */

uint64_t HELPER(shl_i64)(uint64_t arg1, uint64_t arg2)
{
    return arg1 << arg2;
}

uint64_t HELPER(shr_i64)(uint64_t arg1, uint64_t arg2)
{
    return arg1 >> arg2;
}

int64_t HELPER(sar_i64)(int64_t arg1, int64_t arg2)
{
    return arg1 >> arg2;
}

int64_t HELPER(div_i64)(int64_t arg1, int64_t arg2)
{
    return arg1 / arg2;
}

int64_t HELPER(rem_i64)(int64_t arg1, int64_t arg2)
{
    return arg1 % arg2;
}

uint64_t HELPER(divu_i64)(uint64_t arg1, uint64_t arg2)
{
    return arg1 / arg2;
}

uint64_t HELPER(remu_i64)(uint64_t arg1, uint64_t arg2)
{
    return arg1 % arg2;
}

uint64_t HELPER(muluh_i64)(uint64_t arg1, uint64_t arg2)
{
    uint64_t l, h;
    mulu64(&l, &h, arg1, arg2);
    return h;
}

int64_t HELPER(mulsh_i64)(int64_t arg1, int64_t arg2)
{
    uint64_t l, h;
    muls64(&l, &h, arg1, arg2);
    return h;
}

#ifdef CONFIG_TB_ANNOTATION
static inline void take_final_edge(TbAnnotation *env, TbAnnotationEdge *edge)
{
    /* Store current context and block */
    env->last_ctx = edge->target_context;
    env->last_block = edge->target;
    /* Accumulate value */
    env->value_sum += edge->value;
}

static inline void take_edge(TbAnnotation *env, TbAnnotationEdge *edge)
{
    TbAnnotationLeavingEdgeTuple *out;

    /* Store current context and block */
    env->last_ctx = edge->target_context;
    env->last_block = edge->target;
    /* Accumulate value */
    env->value_sum += edge->value;

    /* Check whether we are at the end of our analysis... */
    if (env->last_block->out_edges_hash_table != NULL) {
        out = g_hash_table_lookup(env->last_block->out_edges_hash_table,
                                  env->last_ctx);
        if (out != NULL && out->out1->target->is_end_block) {
            take_final_edge(env, out->out1);
        }
    }
}

void HELPER(annotation)(uint64_t pc, void *opaque)
{
    TbAnnotation *env = (TbAnnotation *) opaque;
    TbAnnotationBlock *b;
    TbAnnotationLeavingEdgeTuple *out;

    if (!env) {
        return;
    }

    /* does the block corresponding to pc exist? */
    if (!g_hash_table_contains(env->tb_annotation_blocks, &pc)) {
        return;
    }
    /* if last_block == NULL we're in the first block */
    if (env->last_block == NULL) {

        b = (TbAnnotationBlock *)g_hash_table_lookup(env->tb_annotation_blocks,
                                                     &pc);
        env->last_block = b;

    } else {
        /* while not reached block with current pc (target)
         * take the next distinct edge if it exists
         * otherwise we're one edge away from the target and
         * take the edge directly leading to the target
         */
        out = g_hash_table_lookup(env->last_block->out_edges_hash_table,
                                  env->last_ctx);

        while (out != NULL && out->out2 == NULL) {
            /* We found a distinct path to "out1" */
            take_edge(env, out->out1);

            /* Have we reached our target? */
            if (env->last_block->address == pc) {
                return;
            }

            /* Get the current out edge tuple */
            out = g_hash_table_lookup(env->last_block->out_edges_hash_table,
                                      env->last_ctx);
        }

        /* If we get here, we are on a branch block.
           Take the edge leading to the target. */
        if (out != NULL && out->out1->target->address == pc) {

            /* Take this edge */
            take_edge(env, out->out1);


        } else if (out != NULL && out->out2->target->address == pc) {

            /* Take the other edge */
            take_edge(env, out->out2);

        } else {

            /* Something went terribly wrong here... */
        }
    }
        }
#endif
