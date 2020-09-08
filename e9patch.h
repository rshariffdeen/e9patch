/*
 * e9patch.h
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

#ifndef __E9PATCH_H
#define __E9PATCH_H

#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstring>

#include <elf.h>

#include <deque>
#include <map>
#include <vector>

#define NO_RETURN               __attribute__((__noreturn__))
#define NO_INLINE               __attribute__((__noinline__))

#define PAGE_SIZE               ((size_t)4096)

/*
 * States of each virtual memory byte.
 */
#define STATE_UNKNOWN           0x0     // Unknown or "don't care"
#define STATE_INSTRUCTION       0x1     // Used by an instruction.
#define STATE_PATCHED           0x2     // Used by a patched instruction.
#define STATE_FREE              0x3     // Was used by instruction, now free.
#define STATE_OVERFLOW          0x4     // Byte is past the end-of-file.
#define STATE_QUEUED            0x5     // Byte is queued for patching.
#define STATE_LOCKED            0x10    // Byte is locked (read-only).

/*
 * C-string comparator.
 */
struct CStrCmp
{
    bool operator()(const char* a, const char* b) const
    {
        return (strcmp(a, b) < 0);
    }
};

/*
 * Buffer.
 */
struct Buffer
{   
    unsigned i = 0; 
    const unsigned max;
    uint8_t * const bytes;
    
    Buffer(uint8_t * bytes, unsigned max = UINT32_MAX) : bytes(bytes), max(max)
    {
        ;
    }
    
    void push(uint8_t b)
    {
        assert(i < max);
        if (bytes != nullptr)
            bytes[i] = b;
        i++;
    }
    
    void push(const uint8_t *data, unsigned len)
    {
        assert(i + len <= max);
        if (bytes != nullptr)
            memcpy(bytes + i, data, len);
        i += len;
    }
    
    size_t size()
    {
        return (size_t)i;
    }
};

/*
 * Bounds.
 */
struct Bounds
{
    intptr_t lb;                        // Lower bound.
    intptr_t ub;                        // Upper bound.
};

/*
 * Trampoline template entry kind.
 */
enum EntryKind
{
    ENTRY_BYTES,
    ENTRY_ZEROES,
    ENTRY_LABEL,
    ENTRY_MACRO,
    ENTRY_REL8,
    ENTRY_REL32,
    ENTRY_INT8,
    ENTRY_INT16,
    ENTRY_INT32,
    ENTRY_INT64,
    ENTRY_INSTRUCTION,
    ENTRY_INSTRUCTION_BYTES,
    ENTRY_CONTINUE,
    ENTRY_TAKEN,
};

/*
 * Trampoline template entry.
 */
struct Entry
{
    EntryKind kind;                     // Entry kind
    union
    {
        unsigned length;                // Entry length
        bool use_label;                 // Use label for rel8/rel32?
    };
    union
    {
        const uint8_t *bytes;           // Raw bytes
        const char *label;              // Label name
        const char *macro;              // Macro name
        uint8_t uint8;                  // 8bit integer constant
        uint16_t uint16;                // 16bit integer constant
        uint32_t uint32;                // 32bit integer constant
        uint64_t uint64;                // 64bit integer constant
    };
};

/*
 * A trampoline template.
 */
struct Trampoline
{
    const char *name;                   // Name (if applicable)
    int prot:31;                        // Protections.
    int preload:1;                      // Pre-load trampoline?
    unsigned num_entries;               // Number of entries.
                                        // Entries.
    Entry entries[];
};

/*
 * The default evictee trampoline template.
 */
extern const Trampoline *evicteeTrampoline;

/*
 * Metadata representation.
 */
struct Metadata
{
    size_t num_entries;                 // Number of entries.
    Trampoline *entries[];              // Entries.
};

/*
 * Instruction representation.
 */
struct Instr
{
    const size_t offset:48;             // The instruction offset
    const size_t size:4;                // The instruction size (bytes)
    const size_t pcrel32_idx:4;         // 32bit PC-relative imm idx (or 0)
    const size_t pcrel8_idx:4;          // 8bit PC-relative imm idx (or 0)
    const size_t pic:1;                 // PIC? (stored here for convenience)
    const intptr_t addr;                // The address of the instruction
    intptr_t trampoline = INTPTR_MIN;   // The address of any trampoline

    const struct Original
    {
        const uint8_t * const bytes;    // The (unmodified) instruction

        Original(const uint8_t *bytes) : bytes(bytes)
        {
            ;
        }
    } original;

    const struct Patched
    {
        uint8_t * const bytes;          // The (modified/patched) instruction
        uint8_t * const state;          // The instruction state

        Patched(uint8_t *bytes, uint8_t *state) : bytes(bytes), state(state)
        {
            ;
        }

    } patched;

    const Metadata *metadata = nullptr; // The instruction metadata.
    Instr *prev = nullptr;              // The previous instruction.
    Instr *next = nullptr;              // The next instruction.

    Instr(off_t offset, intptr_t addr, size_t size, const uint8_t *original,
            uint8_t *bytes, uint8_t *state,  size_t pcrel32_idx,
            size_t pcrel8_idx, bool pic) :
        offset((size_t)offset), addr(addr), size(size), original(original),
        patched(bytes, state), pcrel32_idx(pcrel32_idx),
        pcrel8_idx(pcrel8_idx), pic(pic)
    {
        ;
    }
};

/*
 * Virtual address space allocation.
 */
struct Alloc
{
    intptr_t lb;                // Allocation lower bound
    intptr_t ub;                // Allocation upper bound
    const Instr *I;             // Instruction.
    const Trampoline *T;        // Trampoline.
};

/*
 * Interval tree.
 */
struct Node;
struct Tree
{
    Node *root;                 // Interval tree root
};

/*
 * Virtual address space allocator.
 */
struct Allocator
{
    Tree tree;                  // Interval tree

    /*
     * Iterators.
     */
    struct iterator
    {
        Node *node = nullptr;

        iterator() = default;
        iterator(const iterator &i) : node(i.node)
        {
            ;
        }
        iterator(Node *node) : node(node)
        {
            ;
        }

        const Alloc *operator*();
        void operator++();
        bool operator!=(const iterator &i)
        {
            return (node != i.node);
        }
        bool operator==(const iterator &i)
        {
            return (node == i.node);
        }
        void operator=(const iterator &i)
        {
            node = i.node;
        }
    };

    iterator begin() const;
    static iterator end()
    {
        iterator i;
        return i;
    }
    iterator find(intptr_t addr) const;

    Allocator()
    {
        tree.root = nullptr;
    }
};

/*
 * The (minimal) ELF info needed for rewriting.
 */
struct ElfInfo
{
    Elf64_Ehdr *ehdr;               // EHDR (Elf header)
    Elf64_Phdr *phdr_note;          // PHDR PT_NOTE to be used for loader.
    Elf64_Phdr *phdr_dynamic;       // PHDR PT_DYNAMIC else nullptr.
    bool pic;                       // Position independent?
};

/*
 * Supported binary modes.
 */
enum Mode 
{
    MODE_EXECUTABLE,                    // Binary is an executable.
    MODE_SHARED_OBJECT                  // Binary is a shared object.
};

/*
 * Binary representation.
 */
typedef std::map<off_t, Instr *> InstrSet;
typedef std::deque<std::pair<Instr *, const Trampoline *>> PatchQueue;
typedef std::map<const char *, Trampoline *, CStrCmp> TrampolineSet;
typedef std::vector<intptr_t> InitSet;
struct Binary
{
    const char *filename;               // The binary's path.
    size_t size;                        // The binary's size.
    ElfInfo elf;                        // ELF information.
    Mode mode;                          // Binary mode.

    struct
    {
        const uint8_t *bytes;           // The original binary bytes.
        int fd;                         // The original binary file descr.
    } original;
    struct
    {
        uint8_t *bytes;                 // The patched binary bytes.
        uint8_t *state;                 // The patched binary state.
        size_t size;                    // The patched binary size.
    } patched;

    intptr_t cursor;                    // Patching cursor.
    PatchQueue Q;                       // Instructions queued for patching.

    InstrSet Is;                        // All (known) instructions.
    TrampolineSet Ts;                   // All current trampoline instrument.
    
    Allocator allocator;                // Virtual address allocation.

    InitSet inits;                      // Initialization functions.
    intptr_t mmap = INTPTR_MIN;         // Mmap function.
};

/*
 * Global options.
 */
extern bool option_is_tty;
extern bool option_debug;
extern bool option_disable_B1;
extern bool option_disable_B2;
extern bool option_disable_T1;
extern bool option_disable_T2;
extern bool option_disable_T3;
extern bool option_experimental;
extern bool option_static_loader;
extern bool option_same_page;
extern bool option_trap_all;
extern bool option_use_stack;
extern intptr_t option_lb;
extern intptr_t option_ub;

/*
 * Global statistics.
 */
extern size_t stat_num_patched;
extern size_t stat_num_failed;
extern size_t stat_num_B1;
extern size_t stat_num_B2;
extern size_t stat_num_T1;
extern size_t stat_num_T2;
extern size_t stat_num_T3;
extern size_t stat_num_virtual_mappings;
extern size_t stat_num_physical_mappings;
extern size_t stat_num_virtual_bytes;
extern size_t stat_num_physical_bytes;
extern size_t stat_input_file_size;
extern size_t stat_output_file_size;

extern void NO_RETURN error(const char *msg, ...);
extern void warning(const char *msg, ...);
extern void debugImpl(const char *msg, ...);

#define debug(msg, ...)                                                 \
    do {                                                                \
        if (__builtin_expect(option_debug, false))                      \
            debugImpl((msg), ##__VA_ARGS__);                            \
    } while (false)

#define ADDRESS_FORMAT              "%s%s0x%lx"
#define ADDRESS(p)                                                      \
    (IS_ABSOLUTE(p)? "[absolute] ": ""),                                \
    ((p) < 0? "-": ""),                                                 \
    std::abs(BASE_ADDRESS(p))

#endif
