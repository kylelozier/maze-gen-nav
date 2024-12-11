#include "ezmemory.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

struct memory_stats{
    u64 total_allocated;
    u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];
};

static const char* memory_tag_strings[MEMORY_TAG_MAX_TAGS] = {
    "UNKNOWN  ",
    "ARRAY" ,
    "LINEAR_ALLC",
    "DARRAY",
    "DICT",
    "RING_QUEUE",
    "BST",
    "STRING",
    "APPLICATION",
    "JOB",
    "TEXTURE",
    "MAT_INST",
    "RENDERER",
    "GAME",
    "TRANSFORM",
    "ENTITY",
    "ENTITY_NODE",
    "SCENE" };

static struct memory_stats stats;

void initialize_memory() {
    memset(&stats, 0, sizeof(stats));
}
void shutdown_memory(){

}

/*MEMORY FUNCTION WRAPPERS*/

void* ezallocate(u64 size, memory_tag tag){
    if(tag == MEMORY_TAG_UNKNOWN) {
        printf("\n%s", "Warning memory tag unknown using callocate().");
    }

    stats.total_allocated += size;
    stats.tagged_allocations[tag] += size;

    //TODO: MEMORY ALIGNMENT !!!
    void* block = malloc(size);
    memset(block, 0, size);
    return block;
}

void ezfree(void* block, u64 size, memory_tag tag){
        if(tag == MEMORY_TAG_UNKNOWN) {
        printf("\n%s", "Warning memory tag unknown using ezfree().");
    }

    stats.total_allocated -= size;
    stats.tagged_allocations[tag] -= size;

    //TODO: MEMORY ALIGNMENT
    free(block);
}

void* ezzero_memory(void* block, u64 size){
    return memset(block, 0, size);
}

void* ezcopy_memory(void* dest, const void* source, u64 size){
    return memcpy(dest, source, size);
}

void* ezset_memory(void* dest, i32 value, u64 size){
    return memset(dest, value, size);
}

char* get_memory_usage_str(){
    const u64 gib = 1024 * 1024 * 1024;
    const u64 mib = 1024 * 1024;
    const u64 kib = 1024;

    char buffer[8000] = "System memory use (tagged:\n)";
    u64 offset = strlen(buffer);
    for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) {
        char unit[4] = "XiB";
        float amount = 1.0f;
        if(stats.tagged_allocations[i] >= gib) {
            unit[0] = 'G';
            amount = stats.tagged_allocations[i] / (float)gib;
        } else if (stats.tagged_allocations[i] >= mib) {
            unit[0] = 'M';
            amount = stats.tagged_allocations[i] / (float)mib;
        } else if (stats.tagged_allocations[i] >= kib) {
            unit[0] = 'K';
            amount = stats.tagged_allocations[i] / (float)kib;
        } else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)stats.tagged_allocations[i];
        }

        i32 length = snprintf(buffer + offset, 8000, " %s: %.2f%s\n", memory_tag_strings[i], amount, unit);
        offset += length;
    }
    char* out_string = _strdup(buffer);
    return out_string;
}

/*DARRAY FUNCTIONS*/

void* _darray_create(u64 length, u64 stride) {
    u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 array_size = length * stride;
    u64* new_array = ezallocate(header_size + array_size, MEMORY_TAG_DARRAY);
    ezset_memory(new_array, 0, header_size + array_size);
    new_array[DARRAY_CAPACITY] = length;
    new_array[DARRAY_LENGTH] = 0;
    new_array[DARRAY_STRIDE] = stride;
    return (void*)(new_array + DARRAY_FIELD_LENGTH);
}

void _darray_destroy(void* array) {
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 total_size = header_size + header[DARRAY_CAPACITY] * header[DARRAY_STRIDE];
    ezfree(header, total_size, MEMORY_TAG_DARRAY);
}

u64 _darray_field_get(void* array, u64 field) {
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    return header[field];
}

void _darray_field_set(void* array, u64 field, u64 value) {
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    header[field] = value;
}

void* _darray_resize(void* array) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    void* temp = _darray_create(
        (DARRAY_RESIZE_FACTOR * darray_capacity(array)), stride);
    ezcopy_memory(temp, array, length * stride);

    _darray_field_set(temp, DARRAY_LENGTH, length);
    _darray_destroy(array);
    return temp;
}

void* _darray_push(void* array, const void* value_ptr) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    if(length >= darray_capacity(array)) {
        array = _darray_resize(array);
    }

    u64 addr = (u64)array;
    addr += (length * stride);
    ezcopy_memory((void*)addr, value_ptr, stride);
    _darray_field_set(array, DARRAY_LENGTH, length + 1);
    return array;
}

void _darray_pop(void* array, void* dest) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    u64 addr = (u64)array;
    addr += ((length -1) * stride);
    ezcopy_memory(dest, (void*)addr, stride);
    _darray_field_set(array, DARRAY_LENGTH, length - 1);
}

void* _darray_pop_at(void* array, u64 index, void* dest) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    if(index >= length) {
        printf("\n%s%llu %llu", "Index outside bounds of this array. Length, Index: ", length, index);
        return array;
    }

    u64 addr = (u64)array;
    ezcopy_memory(dest, (void*)(addr + (index * stride)), stride);

    if(index != length - 1) {
        ezcopy_memory(
            (void*)(addr + (index * stride)),
            (void*)(addr + ((index + 1) * stride)),
            stride * (length - index));
    }

    _darray_field_set(array, DARRAY_LENGTH, length - 1);
    return array;
}

void* _darray_insert_at(void* array, u64 index, void* value_ptr) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    if(index >= length) {
        printf("\n%s%llu %llu", "Index outside bounds of this array. Length, Index: ", length, index);
        return array;
    }
    if(length >= darray_capacity(array)) {
        array = _darray_resize(array);
    }

    u64 addr = (u64)array;

    if(index != length - 1) {
        ezcopy_memory(
            (void*)(addr + ((index + 1) * stride)),
            (void*)(addr + (index  * stride)),
            stride * (length - index));
    }

    ezcopy_memory((void*)(addr + (index * stride)), value_ptr, stride);

    _darray_field_set(array, DARRAY_LENGTH, length + 1);
    return array;
}
