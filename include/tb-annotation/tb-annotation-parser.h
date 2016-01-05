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

#ifndef INCLUDE_TB_ANNOTATION_PARSER_H_
#define INCLUDE_TB_ANNOTATION_PARSER_H_

#include "tb-annotation/tb-annotation.h"

void tb_annotation_xml_init(const char *filename);
void tb_annotation_xml_close(void);
TbAnnotation *tb_annotation_parse(const char *filename);

#endif /* INCLUDE_TB_ANNOTATION_PARSER_H_ */
