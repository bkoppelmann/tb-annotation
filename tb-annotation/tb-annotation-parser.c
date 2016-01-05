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

#include <assert.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/tree.h>

#include "tb-annotation/tb-annotation.h"
#include "tb-annotation/tb-annotation-parser.h"

xmlDocPtr tb_annotation_doc;
xmlXPathContextPtr tb_annotation_xpathctx;
xmlXPathObjectPtr tb_annotation_xpath;

void tb_annotation_xml_init(const char *filename)
{

    /* Load XML file */
    tb_annotation_doc = xmlParseFile(filename);
    assert(tb_annotation_doc);

    /* Create XPath evaluation context */
    tb_annotation_xpathctx = xmlXPathNewContext(tb_annotation_doc);
    assert(tb_annotation_xpathctx);
}

void tb_annotation_xml_close(void)
{

    xmlXPathFreeObject(tb_annotation_xpath);
    xmlXPathFreeContext(tb_annotation_xpathctx);
    xmlFreeDoc(tb_annotation_doc);
}

static int32_t tb_annotation_xml_evalxpath(const char *xpath,
        xmlNodeSetPtr *nodes)
{

    /* Evaluate the expression */
    tb_annotation_xpath = xmlXPathEvalExpression(BAD_CAST xpath,
            tb_annotation_xpathctx);
    assert(tb_annotation_xpath);

    /* Set the return value */
    *nodes = tb_annotation_xpath->nodesetval;
    return tb_annotation_xpath->nodesetval->nodeNr;
}

TbAnnotation *tb_annotation_parse(const char *filename)
{
    int32_t i;
    xmlNodeSetPtr nodes = NULL;
    TbAnnotationBlock *cur_block;
    TbAnnotationLeavingEdgeTuple *cur_leaving_edge_tuple;
    TbAnnotationEdge *cur_edge;
    GHashTable *id_to_block = g_hash_table_new(g_str_hash, g_str_equal);
    TbAnnotation *tba = g_new0(TbAnnotation, 1);
    tba->tb_annotation_blocks = g_hash_table_new(g_int_hash, g_int_equal);

    /* Open XML file */
    tb_annotation_xml_init(filename);

    /* Parse Blocks */
    tb_annotation_xml_evalxpath("/QEMU-annotation/Blocks/Block", &nodes);
    for (i = 0; i < nodes->nodeNr; i++) {

        /* Read XML description of current block */
        const char *id = (const char *) xmlGetNoNsProp(
                nodes->nodeTab[i], BAD_CAST "id");
        const char *address_str = (const char *) xmlGetNoNsProp(
                nodes->nodeTab[i], BAD_CAST "address");

        /* Create the block */
        cur_block = g_new0(TbAnnotationBlock, 1);
        cur_block->out_edges_hash_table = g_hash_table_new(
                g_str_hash, g_str_equal);
        cur_block->id = id;
        cur_block->address = -1;

        if (g_strcmp0("None", address_str) == 0) {

            cur_block->is_end_block = 1;

        } else {

            cur_block->is_end_block = 0;
            sscanf(address_str, "%x", &cur_block->address);

            /* Store current block in TbAnnotations PC -> block map */
            g_hash_table_insert(tba->tb_annotation_blocks, &cur_block->address,
                    cur_block);
        }

        /* Store in id -> block map for edge creation */
        g_hash_table_insert(id_to_block, (gpointer) id, cur_block);
    }

    /* Parse Edges (also adding out_edges to blocks) */
    tb_annotation_xml_evalxpath("/QEMU-annotation/Edges/Edge", &nodes);
    for (i = 0; i < nodes->nodeNr; i++) {

        /* Read XML description of current edge */
        const char *source_id = (const char *) xmlGetNoNsProp(
                nodes->nodeTab[i], BAD_CAST "source");
        const char *source_ctx = (const char *) xmlGetNoNsProp(
                nodes->nodeTab[i], BAD_CAST "source_context");
        const char *target_id = (const char *) xmlGetNoNsProp(
                nodes->nodeTab[i], BAD_CAST "target");
        const char *target_ctx = (const char *) xmlGetNoNsProp(
                nodes->nodeTab[i], BAD_CAST "target_context");
        const char *value_str = (const char *) xmlGetNoNsProp(
                nodes->nodeTab[i], BAD_CAST "value");

        /* Create the edge */
        cur_edge = g_new0(TbAnnotationEdge, 1);
        cur_edge->source = g_hash_table_lookup(id_to_block, source_id);
        cur_edge->target = g_hash_table_lookup(id_to_block, target_id);
        cur_edge->source_context = source_ctx;
        cur_edge->target_context = target_ctx;
        sscanf(value_str, "%u", &cur_edge->value);

        /* Store this edge in source blocks leaving_edge_tuple */
        cur_leaving_edge_tuple = g_hash_table_lookup(
                cur_edge->source->out_edges_hash_table,
                cur_edge->source_context);
        if (cur_leaving_edge_tuple == NULL) {
            cur_leaving_edge_tuple = g_new0(TbAnnotationLeavingEdgeTuple, 1);
            g_hash_table_insert(cur_edge->source->out_edges_hash_table,
                    (gpointer) cur_edge->source_context,
                    cur_leaving_edge_tuple);
        }
        /* There must be free space in the leaving_edge_tuple ... */
        assert(cur_leaving_edge_tuple->out1 == NULL ||
               cur_leaving_edge_tuple->out2 == NULL);
        if (cur_leaving_edge_tuple->out1 == NULL) {
            cur_leaving_edge_tuple->out1 = cur_edge;
        } else {
            cur_leaving_edge_tuple->out2 = cur_edge;
        }
    }

    /* Parse Startcontext */
    tb_annotation_xml_evalxpath("/QEMU-annotation/Start-context", &nodes);
    assert(nodes->nodeNr == 1);
    tba->last_ctx = (const char *) xmlGetNoNsProp(nodes->nodeTab[0],
    BAD_CAST "value");

    /* Destroy temporary block id -> block map */
    g_hash_table_destroy(id_to_block);

    /* Close XML file */
    tb_annotation_xml_close();

    /* Return TbAnnotation data */
    return tba;
}
