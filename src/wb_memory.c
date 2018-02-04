
#define ld_allocate(size, count) malloc((size) * (count))
#define ld_free(ptr) free((ptr))

#ifndef REFLECTED
struct MemoryArena
{
	string name;
	u8* data;
	u8* head;
	u8* temp_head;
	isize size;
};
#endif


static inline 
isize mem_align_4(isize p)
{
	usize mod = p & 3;
	return mod ? p + 4 - mod : p;
}

static inline 
isize mem_align(usize p, isize n)
{
	usize mod = p & (n-1);
	return mod ? p + n - mod : p;
}

void arena_init(MemoryArena* arena, string name, isize size)
{
	arena->name = name;
	arena->size = size;
	arena->data = platform_allocate_memory(size, NULL);
	arena->head = arena->data;
	arena->temp_head = NULL;
}

MemoryArena* arena_bootstrap(string name, isize size)
{
	MemoryArena* arena;
	size += sizeof(MemoryArena) + 64;
	void* data = platform_allocate_memory(size, NULL);
	arena = data;
	arena->data = data;
	arena->head = (u8*)arena->data + sizeof(MemoryArena);
	arena->name = name;
	arena->size = size;
	arena->temp_head = NULL;
	return arena;
}

void* arena_push(MemoryArena* arena, isize size)
{
	u8** local_head = arena->temp_head != NULL ? &arena->temp_head : &arena->head;
	u8* old_head = *local_head;
	u8* new_head = (u8*)mem_align_4((usize)*local_head + size);
	if(new_head > (arena->data + arena->size)) {
		log_error("Error: Arena [%s] was filled with allocation of size %d\n", arena->name, size);
		return NULL;
	}
	*local_head = new_head;

	return old_head;
}

void arena_start_temp(MemoryArena* arena)
{
	arena->temp_head = (u8*)mem_align((usize)arena->head, PageSize);
}

void arena_end_temp(MemoryArena* arena)
{
	if(arena->temp_head == NULL) return;

	u8* temp_start = (u8*)mem_align((usize)arena->head, PageSize);
	isize length = mem_align((isize)(arena->temp_head - temp_start), PageSize);
	platform_decommit_memory(temp_start, &length);
	platform_commit_memory(length, temp_start);

	arena->temp_head = NULL;
}

void arena_clear(MemoryArena** arena)
{
	MemoryArena local = *(*arena);
	platform_decommit_memory(local.data, &local.size);
	platform_commit_memory(local.size, local.data);
	local.head = local.data;
	*arena = arena_push(&local, sizeof(MemoryArena) + 64);
	**arena = local;

}

void* arena_alloc_wrapper(isize size, void* userdata)
{
	return arena_push(userdata, size);
}

Allocator arena_allocator(MemoryArena* arena)
{
	Allocator a;
	a.allocate = arena_alloc_wrapper;
	a.free = platform_no_op_free;
	a.alloc_userdata = arena;
	return a;
}

MemoryArena arena_free(MemoryArena* arena)
{
	MemoryArena local_arena = *arena;
	platform_free_memory(arena->data, NULL);
	return local_arena;
}

#ifndef REFLECTED
struct PoolHandle
{
	struct PoolHandle* next;
};

struct MemoryPool
{
	isize element_size;
	void* slots;
	isize count;
	isize capacity;
	
	string name;

	PoolHandle* free_list;
};
#endif

void pool_init(MemoryPool* pool, string name, isize element_size, isize capacity, MemoryArena* arena)
{
	pool->name = name;
	//cheating.
	if(element_size < sizeof(void*)) element_size = sizeof(void*);
	pool->element_size = element_size;
	pool->count = 0;
	pool->capacity = capacity;
	pool->slots = arena_push(arena, element_size * capacity);
	pool->free_list = pool->slots;

	pool->count = 0;
	PoolHandle* handle = NULL;
	for(isize i = 0; i < capacity; ++i) {
		handle = (void*)((isize)pool->slots + element_size * i);
		handle->next = (void*)((isize)pool->slots + element_size * (i+1));
	}
	handle->next = NULL;

}

MemoryPool* pool_new(string name, isize element_size, isize capacity, MemoryArena* arena)
{
	MemoryPool* pool = arena_push(arena, sizeof(MemoryPool));
	pool_init(pool, name, element_size, capacity, arena);
	return pool;
}

void* pool_retrieve(MemoryPool* pool)
{
	void* ptr = pool->free_list;
	pool->free_list = pool->free_list->next;
	pool->count++;
	return ptr;
}

void pool_release(MemoryPool* pool, void** iptr)
{
	void* ptr = *iptr;
	if(ptr < pool->slots || (char*)ptr > ((char*)pool->slots + pool->capacity * pool->element_size)) {
		return;
	}
	pool->count--;
	*iptr = NULL;
	PoolHandle* handle = ptr;
	handle->next = pool->free_list;
	pool->free_list = ptr;
}

void pool_print(MemoryPool* pool) 
{
	isize size = pool->count * pool->element_size;
	isize cap = pool->capacity * pool->element_size;
	f32 ratio = (f32)size/(f32)cap;
	printf("Pool [%s]: Filled:%.2f Size:%dk/%dk Count:%d Free:%d \n", pool->name, ratio * 100, size/1000, cap/1000, pool->count, pool->capacity - pool->count);
}

void pool_clear(MemoryPool* pool, i8 clear)
{
	if(clear) memset(pool->slots, 0, pool->element_size * pool->capacity);
	pool->count = 0;

	PoolHandle* handle = pool->slots;
	for(isize i = 0; i < (pool->capacity - 1); ++i) {
		handle->next = handle + pool->element_size;
		handle = handle->next;
	}
	handle->next = NULL;
	pool->free_list = pool->slots;
}

/*
#if 0
// Dynamic pool

typedef struct DynamicPoolHandle_
{
	i32 count;
	i32 id;
	i32 bucket_index;
	i32 handle_index;
	struct DynamicPoolHandle_* next;
} DynamicPoolHandle;

typedef struct DynamicPoolBucket_
{
	struct DynamicPoolBucket_* next;
	DynamicPoolHandle* handles;
	DynamicPoolHandle* free_list;
	void* data;
	i32 handle_count;
	i32 count;
	i32 index;
} DynamicPoolBucket;

#define DynamicMemoryPoolMetadataOffset (sizeof(i32)*2)
#define GetDynamicMemoryPoolMetadata(pool, id) (void*)((u8*)((pool)->current_bucket->data) + (pool)->element_size * (id))
#define GetDynamicMemoryPoolData(pool, id) (void*)((u8*)((pool)->current_bucket->data) + (pool)->element_size * (id) + DynamicMemoryPoolMetadataOffset)
typedef struct DynamicMemoryPool_
{
	string name;
	Allocator alloc;
	isize element_size;
	isize raw_element_size;
	isize handle_capacity;
	isize bucket_capacity;

	DynamicPoolBucket* current_bucket;
	DynamicPoolBucket* bucket_list;
	isize bucket_count;
} DynamicMemoryPool;

isize dpool_get_total_count(DynamicMemoryPool* pool)
{
	DynamicPoolBucket* bucket = pool->bucket_list;
	isize total = 0;
	do {
		total += bucket->count;
	} while(bucket = bucket->next);
	return total;
}

void dpool_print(DynamicMemoryPool* pool)
{
	printf("Memory Pool [%s]: ElementSize:%d HandleCap:%d BucketCount:%d MaxBuckets:%d\n",
			pool->name, pool->element_size,
			pool->handle_capacity, pool->bucket_count, pool->bucket_capacity);
	isize total = dpool_get_total_count(pool);
	isize tcap = pool->bucket_count * pool->bucket_capacity;
	f64 ratio = (f64)total / (f64)tcap;
	printf("TotalCount:%d TotalCapacity:%d Percent:%.2f%%\n", total, tcap, ratio * 100);
	DynamicPoolBucket* bucket_head = pool->bucket_list;
	do {
		printf("\tBucket [%d]: Count:%d HandleCount:%d\n", 
				bucket_head->index, bucket_head->count, bucket_head->handle_count);
		DynamicPoolHandle* handle_head = bucket_head->free_list;
		do {
			printf("\t\tHandle [%d]: ID:%d FreeSpaces:%d\n",
					handle_head->handle_index, handle_head->id, handle_head->count);
		} while(handle_head = handle_head->next);
	} while(bucket_head = bucket_head->next);
}

static inline
void _dpool_handle_init(DynamicPoolHandle* handle, i32 id, i32 count, i32 bucket_index, i32 handle_index)
{
	handle->count = count;
	handle->bucket_index = bucket_index;
	handle->handle_index = handle_index;
	handle->id = id;
	handle->next = NULL;
}

DynamicPoolBucket* _dpool_new_bucket(DynamicMemoryPool* pool, i32 index)
{
	DynamicPoolBucket* bucket = AllocatorAlloc(pool->alloc, sizeof(DynamicPoolBucket) + 
			pool->element_size * pool->bucket_capacity + 
			sizeof(DynamicPoolHandle) * pool->handle_capacity);
	bucket->data = (void*)((u8*)bucket + sizeof(DynamicPoolBucket));
	bucket->handles = (void*)((u8*)bucket + sizeof(DynamicPoolBucket) + 
			pool->element_size * pool->bucket_capacity);
	bucket->free_list = bucket->handles;
	_dpool_handle_init(bucket->free_list, 0, pool->bucket_capacity, index, 0);

	bucket->count = 0;
	bucket->index = index;
	bucket->handle_count = 1;
	bucket->next = NULL;

	return bucket;
}


void dpool_init(DynamicMemoryPool* pool, Allocator alloc, string name, isize element_size, isize bucket_capacity)
{
	pool->name = name;
	pool->alloc = alloc;
	pool->raw_element_size = element_size;
	pool->element_size = element_size + sizeof(i32) * 2;
	pool->handle_capacity = bucket_capacity / 2 + 1;
	pool->bucket_capacity = bucket_capacity;

	pool->bucket_list = _dpool_new_bucket(pool, 0);
	pool->current_bucket = pool->bucket_list;
	pool->bucket_count = 1;
}

void dpool_reinit(DynamicMemoryPool* pool)
{
	if(pool->bucket_count != 0) return;
	pool->bucket_list = _dpool_new_bucket(pool, 0);
	pool->current_bucket = pool->bucket_list;
	pool->bucket_count = 1;
}

//TODO(will): This function doesn't do what it says it does.
// It needs to visit all _filled_ slots and extract them
// It can do this by iterating over bucket->data and checking 
// the free list if there's anything (not) there.
#if 0
void dpool_fill_array(DynamicMemoryPool* pool, void* array, isize size)
{
	DynamicPoolBucket* bucket = pool->bucket_list;
	isize i = 0;
	do {
		for(isize j = 0; j < bucket->count; ++j) {
			memcpy((u8*)array + i * pool->raw_element_size, 
					(u8*)bucket->data + j * pool->element_size + DynamicMemoryPoolMetadataOffset, 
					pool->raw_element_size);
			i++;
		}
	} while(bucket = bucket->next);
}
#endif

void dpool_refresh(DynamicMemoryPool* pool)
{
	//This function should:
	//	Clean up all the handles in all buckets
	//	Set the current bucket to the first one with more than 10% free (or the first bucket if there's only one)
	
	DynamicPoolBucket* bucket = pool->bucket_list;
	DynamicPoolBucket* emptiest_bucket = NULL;
	isize bucket_count = pool->bucket_capacity + 1;
	do {
		isize last_used_handle = 0;
		for(isize i = 0; i < pool->handle_capacity; ++i) {
			DynamicPoolHandle* handle = bucket->handles + i;
			if(handle->count == 0 && handle->next == NULL && handle->id == -1) {
				;
			} else {
				last_used_handle = i;
			}
		}
		bucket->handle_count = last_used_handle + 1;
		if(bucket->count <= bucket_count) {
			emptiest_bucket = bucket;
		}
	} while(bucket = bucket->next);
	if(emptiest_bucket) {
		pool->current_bucket = emptiest_bucket;
	}
}

void dpool_free_bucket(DynamicMemoryPool* pool, i32 index)
{
	DynamicPoolBucket* bucket = pool->bucket_list;
	DynamicPoolBucket* last = NULL;
	
	do {
		if(bucket->index == index) {
			if(last == NULL) {
				pool->bucket_list = bucket->next;
			} else {
				last->next = bucket->next;
			}
			AllocatorFree(pool->alloc, bucket);
			dpool_refresh(pool);
			break;
		}

		last = bucket;
	} while(bucket = bucket->next);
	pool->bucket_count--; 
}

void dpool_free_all_buckets(DynamicMemoryPool* pool)
{
	DynamicPoolBucket* bucket = pool->bucket_list;
	DynamicPoolBucket* next;// = NULL;
	do {
		next = bucket->next;
		AllocatorFree(pool->alloc, bucket);
	} while(bucket = next);
	pool->bucket_count = 0;
}


DynamicPoolHandle* _dpool_new_handle(DynamicMemoryPool* pool)
{
	DynamicPoolBucket* bucket = pool->current_bucket;
	if(bucket->handle_count < pool->handle_capacity) {
		return bucket->handles + bucket->handle_count++;
	} else {
		//search for unlinked handle
		for(isize i = 0; i < pool->handle_capacity; ++i) {
			DynamicPoolHandle* handle = bucket->handles + i;
			if(handle->count == 0 && handle->next == NULL && handle->id == -1) {
				return handle;
			}
		}
		log_error("Error: pool [%s] bucket %d ran out of handles to store free memory", pool->name, bucket->index);
	}
	return NULL;
}

void _dpool_delete_handle(DynamicMemoryPool* pool, DynamicPoolHandle* handle)
{
	handle->count = 0; 
	handle->next = NULL;
	handle->id = -1;
}


void* dpool_retrieve(DynamicMemoryPool* pool)
{
	DynamicPoolBucket* bucket = pool->current_bucket;
	void* data;
	if(bucket->free_list->count > 0) {
		data = GetDynamicMemoryPoolData(pool, bucket->free_list->id);
		i32* meta = GetDynamicMemoryPoolMetadata(pool, bucket->free_list->id);
		meta[0] = bucket->index; //bucket
		meta[1] = bucket->free_list->id; // id
		bucket->free_list->count--;
		bucket->free_list->id++;
		bucket->count++;
	} else {
		if(bucket->count >= pool->bucket_capacity) {
			DynamicPoolBucket* newbucket = _dpool_new_bucket(pool, pool->bucket_count++);
			newbucket->next = pool->bucket_list;
			pool->bucket_list = newbucket;
			pool->current_bucket = newbucket;
			return dpool_retrieve(pool);
		}
		//allocate a new bucket, hook it into the list.
	}

	while(bucket->free_list->count == 0) {
		DynamicPoolHandle* next = bucket->free_list->next;
		if(next == NULL) {
			DynamicPoolBucket* newbucket = _dpool_new_bucket(pool, pool->bucket_count++);
			newbucket->next = pool->current_bucket;
			pool->bucket_list = newbucket;
			pool->current_bucket = newbucket;
			break;
		}
		_dpool_delete_handle(pool, bucket->free_list);
		bucket->free_list = next;
	}

	return data;
}

void dpool_release(DynamicMemoryPool* pool, void* element)
{
	DynamicPoolBucket* bucket = pool->current_bucket;
	i32* meta = (void*)((u8*)element - DynamicMemoryPoolMetadataOffset);
	i32 id = meta[1];
	i32 bucket_id = meta[0];

	if(bucket->index != bucket_id) {
		//uh oh
		//we better find that bucket!
		DynamicPoolBucket* bucket_head = pool->bucket_list;
		bucket = NULL;
		do {
			if(bucket_head->index == bucket_id) {
				bucket = bucket_head;
				break;
			}
		} while(bucket_head = bucket_head->next);
		if(bucket == NULL) {
			return;
		}
	}

	DynamicPoolHandle* head = bucket->free_list;
	DynamicPoolHandle* last = NULL;
	do {
		if(id > head->id) {
			if(head->next) {
				if(id < head->next->id) {
					break;
				}
			} else {
				break;	
			}
		}
		last = head;
	} while(head = head->next);

	if(!head) {
		wb_assert(last, "Something's wrong with the memory pool! %s", pool->name);
		//we rolled off the end of the list
		//this probably means there was only one thing in the list.

		head = last;
		if(id == head->id + head->count) {
			//extend head down to include id
			head->count++;
		} else if(id == head->id - 1) {
			//extend head up to include id
			head->id--;
			head->count++;
		} else {
			wb_assert(head == bucket->free_list, "Error: Expected DynamicMemoryPool[%s] head == bucket->free_list; possibly trying to release an already freed element?", pool->name);
			if(id < bucket->free_list->id) {
				DynamicPoolHandle* prev = _dpool_new_handle(pool);
				_dpool_handle_init(prev, id, 1, bucket->index, bucket->handle_count - 1);
				prev->next = bucket->free_list;
				bucket->free_list = prev;
			} else {
				DynamicPoolHandle* next = _dpool_new_handle(pool);
				_dpool_handle_init(next, id, 1, bucket->index, bucket->handle_count - 1);
				if(bucket->free_list->next == NULL) {
					bucket->free_list->next = next;
				} else {
					DynamicPoolHandle* oldnext = bucket->free_list->next;
					bucket->free_list->next = next;
					next->next = oldnext;
				}
			}
		}
	} else {
		if(id == head->id + head->count) {
			//extend head down to include id
			head->count++;
			if(head->id + head->count == head->next->id) {
				//it's possible these two now touch
				//so extend head through head->next, and decouple head->next
				DynamicPoolHandle* next = head->next;
				head->count += next->count;
				head->next = next->next;
				_dpool_delete_handle(pool, next);
			}

		} else if(id == head->next->id - 1) {
			//extend head->next up to include id
			DynamicPoolHandle* next = head->next;
			next->id--;
			next->count++;
			if(head->id + head->count == head->next->id - 1) {
				//it's possible these two now touch
				//so extend head through head->next, and decouple head->next
				head->count += next->count;
				head->next = next->next;
				_dpool_delete_handle(pool, next);
			}
		} else {
			//there's a gap between head and the new id and head->next
			//so make a new handle and hook it in between head and head->next
			DynamicPoolHandle* next = _dpool_new_handle(pool);
			_dpool_handle_init(next, id, 1, bucket->index, bucket->handle_count - 1);
			DynamicPoolHandle* oldnext = head->next;
			next->next = oldnext;
			head->next = next;
		}
	}

	bucket->count--;
}
#endif
*/
