#pragma once

#include <unistd.h>
#include <sys/mman.h>
#include <stddef.h>
#include <stdint.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>

namespace stdlike {

#define SMALL_CHUNKS_STEP 16
#define MMAP_THRESHOLD (128 * 1024)
#define MAX_SMALL_CHUNK_SIZE 512
#define NUM_SMALL_SIZES (MAX_SMALL_CHUNK_SIZE / SMALL_CHUNKS_STEP)
#define NUM_LARGE_SIZES 8

struct Chunk {
    size_t size;
    Chunk* prev = nullptr;
    Chunk* next = nullptr;
};

size_t CalcChunkSize(size_t size) {
    return ((size + SMALL_CHUNKS_STEP - 1) / SMALL_CHUNKS_STEP) * SMALL_CHUNKS_STEP;
}

void MarkUsed(Chunk* chunk) {
    chunk->size |= 1;

    size_t size = chunk->size;
    size_t* footer = reinterpret_cast<size_t*>((char*)chunk + sizeof(size_t) + CurChunkSize(size));
    *footer = size;
}

void MarkUnused(Chunk* chunk) {
    chunk->size &= ~1;

    size_t size = chunk->size;
    size_t* footer = reinterpret_cast<size_t*>((char*)chunk + sizeof(size_t) + CurChunkSize(size));
    *footer = size;
}

bool IsFree(size_t size) {
    return (size & 1) == 0;
}

size_t CurChunkSize(size_t size) {
    return size & ~1;
}

Chunk* small_bins[NUM_SMALL_SIZES] = {nullptr};
Chunk* large_bins[NUM_LARGE_SIZES] = {nullptr};

size_t GetSmallBinIndex(size_t size) {
    return (size / SMALL_CHUNKS_STEP) - 1;
}

inline size_t GetLargeBinIndex(size_t size) {
    size_t index = 0;
    size_t cur_size = MAX_SMALL_CHUNK_SIZE;
    while (cur_size < size && index < NUM_LARGE_SIZES - 1) {
        cur_size *= 2;
        ++index;
    }
    return index;
}

void RemoveFromBin(Chunk*& bin, Chunk* chunk) {
    if (chunk->prev != nullptr) {
        chunk->prev->next = chunk->next;
    } else {
        bin = chunk->next;
    }

    if (chunk->next != nullptr) {
        chunk->next->prev = chunk->prev;
    }

    chunk->next = chunk->prev = nullptr;
}

void AddToBin(Chunk*& bin, Chunk* chunk) {
    chunk->next = bin; 
    chunk->prev = nullptr;

    if (bin != nullptr) {
        bin->prev = chunk;
    }

    bin = chunk;
}

Chunk* ExtendHeap(size_t size) {
    void* block = sbrk(size);
    if (block == (void*)-1) {
        return nullptr;
    }

    Chunk* chunk = static_cast<Chunk*>(block);
    chunk->size = size;
    size_t* footer = reinterpret_cast<size_t*>(
        (char*)chunk + CurChunkSize(chunk->size) - sizeof(size_t));
    *footer = chunk->size;

    return chunk;
}

Chunk* UnionChunks(Chunk* chunk) {
    Chunk* prev_chunk = nullptr;
    Chunk* next_chunk = nullptr;

    if ((char*)chunk > (char*)sbrk(0)) {
        size_t* prev_footer = (size_t*)((char*)chunk - sizeof(size_t));
        if (IsFree(*prev_footer)) {
            prev_chunk = (Chunk*)((char*)chunk - CurChunkSize(*prev_footer));
        }
    }

    size_t* next_header = (size_t*)((char*)chunk + CurChunkSize(chunk->size));
    if (IsFree(*next_header)) {
        next_chunk = (Chunk*)(next_header);
    }

    if (prev_chunk != nullptr) {
        RemoveFromBin(small_bins[GetSmallBinIndex(CurChunkSize(prev_chunk->size))], prev_chunk);
        chunk->size += CurChunkSize(prev_chunk->size);
        prev_chunk->size = chunk->size;
        *(next_header - sizeof(size_t)) = chunk->size; 
        chunk = prev_chunk;
    }

    if (next_chunk != nullptr) {
        RemoveFromBin(small_bins[GetSmallBinIndex(CurChunkSize(next_chunk->size))], next_chunk);
        chunk->size += CurChunkSize(next_chunk->size);
        size_t* new_footer = (size_t*)(next_header + CurChunkSize(next_chunk->size) - sizeof(size_t));
        *new_footer = chunk->size;
    }
}

void Free(void* ptr) {
    if (ptr == nullptr) return;

    size_t* size = (size_t*)((char*)ptr - sizeof(size_t));

    if (*size > MMAP_THRESHOLD) {
        if (munmap(ptr, *size + sizeof(size_t)) != 0) {
            std::cout << "munmap failed";
            abort();
        }
        return;
    }

    Chunk* chunk = (Chunk*)((char*)ptr - sizeof(size_t));
    size_t chunk_size = CurChunkSize(chunk->size);

    if (IsFree(chunk->size)) {
        std::cout << "double free" << std::endl;
        abort();
    }

    MarkUnused(chunk);
    chunk = UnionChunks(chunk);

    Chunk** bins = (*size <= MAX_SMALL_CHUNK_SIZE) ? small_bins : large_bins;
    size_t bin_index = (*size <= MAX_SMALL_CHUNK_SIZE) ? GetSmallBinIndex(*size) : GetLargeBinIndex(*size);
    AddToBin(bins[bin_index], chunk);
}

void* Calloc(size_t num, size_t size) {
    size_t total_size = num * size;
    void* ptr = Malloc(total_size);
    if (ptr != nullptr) {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

void* Realloc(void* ptr, size_t size) {
    if (ptr == nullptr) {
        return Malloc(size);
    }

    Chunk* chunk = (Chunk*)((char*)ptr - sizeof(size_t));
    size_t old_size = CurChunkSize(chunk->size);

    if (old_size > MMAP_THRESHOLD) {
        void* new_ptr = mremap(chunk, old_size, size + sizeof(size_t), MREMAP_MAYMOVE);
        if (new_ptr == MAP_FAILED) {
            std::cout << "mremap failed" << std::endl;
            abort();
        }
        return (void*)((char*)new_ptr + sizeof(size_t));
    }

    if (size + 2 * sizeof(size_t) <= old_size) {
        return ptr;
    }

    MarkUnused(chunk);
    Chunk* new_chunk = UnionChunks(chunk);

    if (new_chunk->size >= 2 * sizeof(size_t) + size) {
        return (void*)((char*)new_chunk + sizeof(size_t));
    }

    void* new_ptr = Malloc(size);
    if (new_ptr != nullptr) {
        memcpy(new_ptr, ptr, old_size);
        Free(ptr);
    }
    return new_ptr;
}

void* Malloc(size_t size) {
    if (size == 0) return nullptr;

    size_t chunk_size = CalcChunkSize(size + 2 * sizeof(size_t));

    if (chunk_size > MMAP_THRESHOLD) {
        return mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    }

    Chunk** bins = (chunk_size <= MAX_SMALL_CHUNK_SIZE) ? small_bins : large_bins;
    size_t bin_index = (chunk_size <= MAX_SMALL_CHUNK_SIZE) ? GetSmallBinIndex(chunk_size) : GetLargeBinIndex(chunk_size);

    Chunk* current_bin = bins[bin_index];
    while (current_bin != nullptr) {
        if (CurChunkSize(current_bin->size) >= chunk_size) {
            RemoveFromBin(bins[bin_index], current_bin);
            MarkUsed(current_bin);
            return (void*)((char*)current_bin + sizeof(size_t));
        }
        current_bin = current_bin->next;
    }

    Chunk* new_chunk = ExtendHeap(chunk_size);
    if (new_chunk == nullptr) {
       return nullptr;
    } 

    MarkUsed(new_chunk);
    return (void*)((char*)new_chunk + sizeof(size_t));
}

}
