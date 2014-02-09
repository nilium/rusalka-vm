Rusalka Bytecode Format
==============================================================================

This document intends to describe the structure of the Rusalka bytecode format. There's no accepted extension for bytecode, though each compiled bytecode file is considered a single unit, so I'll use "unit" to refer to bytecode files.


Chunks
------------------------------------------------------------------------------

Units are comprised of a series of chunks, and most chunks are comprised mainly of signed 32-bit little endian integers. The only exception to this rule is when dealing with masks, but they only occur in one particular chunk, so I'll get to that in due time. Unless stated otherwise, assume all integers are signed, 32-bit, and little-endian.

Chunks are simple structures, and can be easily represented in pseudo-C like so:

    struct chunk {
        int32_t name;
        int32_t size_bytes;
        char    data[size_bytes - 8];
    };

As such, every chunk has a four-byte integer name and a four-byte size measured in bytes. The size includes both the size_bytes and name members themselves, hence the size_bytes - 8 in the data member above.

The following sections will describe each chunk in a unit and their contents. The heading for each section will contain the long name and the short name. The short name is the sequence of bytes that comprise the name in the above structure. In C, for example, if the short name is `VERS`, you can represent it as an integer with a multi-character literal: `'SREV'` (though you might want to use something that isn't implementation-defined).

All but the first two chunks are always in the same order. There is no requirement that other chunks be presented in the same order or contiguously in a unit, provided the version and offsets table are the first two chunks and follow one after the other. You can use the offsets table, in turn, to seek to different chunks.

However, the `asm2bc` utility will always emit contiguous chunks in the order of the sections presented below. This may change in the future, hence the offsets table, but for now, the actual order of the chunks is always the same.

> __Rationale for preferring signed integers:__ although unsigned integers grant you the one extra bit, it's a pain in the neck to mix signed and unsigned integers in the same program. As such, it seems simpler to mandate that signed integers be preferred unless the integer is _only_ to be used for bitwise operations.


##### Tables

A brief aside on tables: all table chunks are prefixed with a length integer for the number of entries they contain. A table with a count of 62, for example, contains 62 of whatever that table holds (e.g., instructions).


#### `VERS` — Bytecode Version

The `VERS` chunk contains a single integer with the unit's bytecode version number. Right now this is ignored because the bytecode hasn't stabilized, though future implementations of Rusalka or anything working with units may wish to consider differences in versions (i.e., higher versions than those supported are unloadable and previous versions may require optional padding arguments for specific instructions). In general, though, I'd like to avoid any significant changes to the bytecode once this is stable.

At the moment, the bytecode version is always `8`.


#### `OFFS` — Chunk Offsets

The offsets table contains `count` name-offset pairs. Each name refers to a chunk in the unit and the offset is relative to the start of the unit. Because all further chunks are not required to be in the order below (though most likely will be), you should use the offsets in this chunk to always end up at the start of the chunk you want to read.

As such, each pair in the chunk can be represented as follows:

    struct chunk_offset_t {
        int32_t chunk_name;
        int32_t offset_from_start;
    };

Entries in this chunk are ordered according to the chunks in the unit. As such, the VERS and OFFS chunks are always the first two entries, then each chunk after those in the order they are in the unit.

This chunk may contain more entries than there are chunks currently supported by the bytecode. If they're not chunks you care about, you can ignore their offsets since they won't be of any use. All entries must have unique names and offsets, otherwise you can consider the unit corrupt.


### Relocation Tables

All relocation tables follow the same format: they contain `count` instruction pointers and an unsigned 32-bit little-endian integer mask describing which operands of the instruction need to be relocated. How they're relocated depends on the table.

As such, all relocation table entries are represented in C as such:

    struct relocation_entry_t {
        int32_t instruction_pointer;
        uint32_t operands_mask;
    }

The instruction pointer is the zero-based offset, measured in instructions, into the unit's instructions relative to the start of the instructions for that unit. So, a pointer of 30 is the 31st instruction in the unit.

The operands mask is the 32-bit unsigned little-endian integer mask for which operands need to be set. Each bit corresponds to an operand of the intruction. If the first bit in it is set, the first operand needs relocation. If the fifth bit is set, the fifth operand needs relocation. Fairly simple.


#### `EREL` — Extern Relocations Table

The externs relocation table contains `count` instruction pointers and a bitmask of which operands must be relocated once an extern exported label is made available.

All extern labels are invalid values in the unit and must be filled out before the unit is considered valid. They are not required to have any usable value, though currently `asm2bc` gives them a value of zero.

This may happen at link time or the extern label operands can be updated as units are loaded. There's no requirement for _when_ it happens, only that the instructions with incomplete operands be treated as invalid. What that means for the Rusalka VM is up to the implementation.


#### `LREL` — Label Relocations Table

The label relocations table contains `count` entries describing which operands reference a label _defined in the unit_. As such, they are already known positions in the instruction set and must be relocated if the unit is being linked with another unit.

The labels referenced may either be imported or exported. If the label has a negative value, it is an import label and, as such, a host function. Import labels only need relocation if a previously-loaded unit defined the import label with a different position, otherwise it's considered a new import and may be given its own value according to the `IMPT` table.


#### `DREL` - Data Relocations Table

The data relocations table contains `count` entries describing which operands reference data blocks defined in the unit. Each unit gets its own data blocks, so these must always be relocated if the unit isn't to be loaded. VM implementations aren't required to deduplicate data blocks, though it may be useful to do as part of a linker stage. Depends on the size of the data block.


#### `DATA` - Data Blocks Table

The data table contains `count` entries describing data blocks in the unit. These blocks are to be treated as read-only by the VM.

Each entry can be represented as such:

    struct data_block_entry_t {
        int32_t data_id;
        int32_t data_size_bytes;
        char    data[data_size_bytes];
    };

The data ID is the zero-based number of the block. These always count upwards, so the data entries are essentially an array. IDs must be incremented if prior units provide data blocks.

The data itself is a string of `data_size_bytes`. The data is not required to contain anything and is not required to be null-terminated.


#### `IMPT` — Imports Table

The imports table contains `count` entries with the addresses and names of imported labels referenced in the unit. These may need to be relocated.

Each entry is structured as such:

    struct label_table_entry_t {
        int32_t address;
        int32_t name_size_bytes;
        char    name[name_size_bytes];
    };

The address is always negative and count downwards, though may not be in order.

Imported label names are not null-terminated.


#### `EXPT` — Exports Table

The exports table contains `count` entries with the addresses and names of exported labels defined in the unit. This does not include extern labels as they are considered undefined until another unit provides the required labels. As such, these are only labels _in_ the unit. They may require relocation.

Entries in this table are structured the same as the imports table (`IMPT`). See the `label_table_entry_t` definition.

The address of exported labels is always zero or greater. A label with address 0 points to the first instruction in the unit, for example, while an address of 8 points to the 7th instruction.

Exported label names are not null-terminated.


#### `EXTS` — Externs Table

The externs table contains `count` entries with the names of extern labels that are not defined in this unit. Until all entries in this table are provided by other units, the unit with the externs has invalid instruction operands.

Each entry in the table is structured as a size-in-bytes integer followed by a sequence of characters for the name.

Extern label names are not null-terminated.


#### `INST` — Instructions Table

The instructions table contains `count` instructions. Each instruction is an integer opcode followed by its operands.

    struct instruction_entry_t {
        int32_t opcode;
        double operands[ARGC];
    }

Because each opcode has a fixed number of arguments, there is no operand count provided for each instruction.

All operands are little-endian 64-bit floats, or doubles. When treated as integers, these are considered signed 32-bit integers, as otherwise the last 12 bits of a 64-bit integer would be cut off.
