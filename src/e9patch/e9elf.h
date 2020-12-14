/*
 * e9elf.h
 * Copyright (C) 2020 National University of Singapore
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __E9ELF_H
#define __E9ELF_H

#include <cstdint>

#include "e9alloc.h"
#include "e9mapping.h"
#include "e9patch.h"

void parseElf(Allocator &allocator, const char *filename, uint8_t *data,
    size_t size, Mode mode, ElfInfo &info);
size_t emitElf(const Binary *B, const MappingSet &mappings,
    size_t mapping_size);

#endif
