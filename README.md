# Improving Hash Table Performance

## Abstract

In this article I describe some methods for optimizing hash table written in C. 

## Introduction
 
I've optimized a hash table that can be used like a dictionary. By a word, we can get the translation of that word,
the goal is to reduce this time. I've used both algorithmic and low-level optimization methods. 
I used hashing with separate chaining and for "chains" I used my own structure List. 

### Analysis

**Machine: 4xIntel® Core™ i5-6200U Processor**  
**Profiler: Intel® VTune™**  
**Disassembler: objdump**

For testing speed I've used a function that gets each word in the dictionary (160,000 words) 1000 times.
When starting the test, only the profiler was launched (IDE is also disabled) to **minimize the influence of other programs on the processor**.
This is necessary so that the time spent on constructing the table and on other side functions is minimal.
It is also important not to call the same words several times in a row because people rarely translate the same words several times.
In this way we break the cache, but we get more realistic conditions. Here speed test code:  
```C
void SpeedTest(HashTable *hash_table, DoubleWord *translates, size_t words_count)
{
    for (int j = 0; j < 1000; j++)
    for (size_t i = 0; i < words_count; i++)
            HashTable_get(hash_table, translates[i].primary_word);
}
```
DoubleWord it's a structure that have only two fields: primary_word(const char*) and translated_word(const char*).
And of course before speed test I tested hash table for correctness.

## Optimization

### Load factor optimization

In a hash table with separate chaining the load factor is the maximum ratio of the numbers of elements in the table to the number of bucket (Bucket it's one chain). Of course it should be less than 1 and in general the lower this number, the less collisions, and hence the faster work time (A collision is an equality of data keys, that is, two or more objects are in the same bucket).
Below is a **plot of the dependence of the hash table operation time depending on the load factor**:  
![Load factor](https://github.com/AntonIVT/Optimization/blob/main/images/plot_load_factor.jpg)  
The plot shows that the load factor affects the speed of work. But the less the load factor, the more memory you have to spend on empty buckets.
It is necessary to choose the optimal value for memory and speed. So I decided that a load factor of **0.7** is ideal for my hash table.  
Load factor selection it is an algorithmic optimization. You don't need to think about how the hash table works.

### Get function optimization

This is how *get* function looked before:
```C++
ValueType* HashTable_get(HashTable *ths, KeyType key)
{
    unsigned long long new_hash = HashingFunction(key);

    My_list<HashTableEl> *curr_bucket = &(ths->buckets[new_hash % ths->capacity]);
    size_t curr_size = curr_bucket->size;

    list_iterator iter = {};
    iter = curr_bucket->begin();

    for (size_t i = 0; i < curr_size; i++, curr_bucket->iter_increase(iter))
    {
        if (!strcmp(key, (*curr_bucket)[iter].key))
            return &((*curr_bucket)[iter].value);
    }

    return NULL;
}
```
This is very slow function because of cycle and iteration on list. And this is how this function has looked disassembled:
```Assembly
...
    mov    QWORD PTR [rbp-0x8],rax
    xor    eax,eax
    mov    rax,QWORD PTR [rbp-0x40]
    mov    rdi,rax
    call   1229 <_Z16hashing_functionPKc>
    mov    QWORD PTR [rbp-0x20],rax
    mov    rax,QWORD PTR [rbp-0x38]
    mov    rsi,QWORD PTR [rax+0x10]
    mov    rax,QWORD PTR [rbp-0x38]
    mov    rcx,QWORD PTR [rax]
    mov    rax,QWORD PTR [rbp-0x20]
    mov    edx,0x0
    div    rcx
    mov    rax,rdx
    shl    rax,0x6
    add    rax,rsi
    mov    QWORD PTR [rbp-0x18],rax
...
```
As you can see there's a lot of memory access (to get structures fields). And also I decided to break an abstraction. To reduce the running time, I access memory directly (to get a sheet item). That is, I am not using iterators. And having figured out the structures, I rewrote this function in assembly (There's a lot of important things in C calling conventions and about it you can read [here](https://en.wikipedia.org/wiki/X86_calling_conventions#System_V_AMD64_ABI)). And this is how it looks now:
```Assembly
HashTable_get:
    push rbx

    mov r8, rdi ; r8 = hash_table ptr
    mov r9, rsi ; r9 = char ptr

    mov rdi, r9
    call HashingFunction ; rax = hash

    mov rcx, [r8]        ; rcx = capacity
    xor rdx, rdx

    div rcx 
    mov rax, rdx
    shl rax, 0x6
    add rax, QWORD [r8 + 0x10] ; rax = curr bucket
    
    mov r10, rax               ; r10 = curr bucket
    mov r11, [r10 + 0x18]      ; r11 = curr size
    
    xor rcx, rcx               ; rcx = counter
...
```
# RESULTS

### Hashing function optimization

String hashing it's a very popular problem, because it's very important to minimize count of collisions in hash tables and it's also very useful in data encryption. You could read about string hashing [here](http://www.quretec.com/u/vilo/edu/2003-04/DM_seminar_2003_II/ver1/P12/articles/ramakrishna97performance.pdf). There are many hash functions for strings. 
You can see their comparison [here](https://www.strchr.com/hash_functions). So for the first version of the hash table I choosed simple hash function:
```C
unsigned long long HashingFunction(const char* key)
{   
    unsigned long long hash = 5381;

    while (*key)
    {
        hash = ((hash << 5) + hash) + *key;
        key++;
    }
    return hash;
}
```
And you can see here profiler results for this version of the hash table:  
![First version profiler results](https://github.com/AntonIVT/Optimization/blob/main/images/VtunePureVersion.png)  
The total running time of the program is **58.256s**. And as you can see the *hashing function* is taking too long. So I decided to use **CRC32** for hashing strings. Fortunately there is intrinsic in SSE4.2 (Streaming SIMD Extensions, you could read about it [here](https://stackoverflow.blog/2020/07/08/improving-performance-with-simd-intrinsics-in-three-use-cases/)) **_mm_crc32_u64** that accumulates a CRC32 value for unsigned 64-bit integers.
But not all strings are divisible by 8. So let's change the data format for the input dictionary. Every string **must** be divisible by 8. Just add the required number of zeros in the end. Example: *"Hello\0\0\0"*. So next I've written a hash function with intrinsic. But I noticed that CRC32 was implemented in hardware when I looked at the disassembled function:
```Assembly
    ...
    mov    rax,QWORD PTR [rbp-0x10]
    mov    rdx,QWORD PTR [rbp-0x8]
    crc32  rax,rdx
    nop
    mov    QWORD PTR [rbp-0x18],rax
    ...
```
So why don't we use it directly? I've written assembly hashing function:
```Assembly
HashingFunction:
    xor rax, rax

hashing_loop:

    crc32 rax, QWORD [rdi]

    add rdi, 8
    cmp BYTE [rdi], 0
    jne hashing_loop
    
    ret
```
And now we can check test with this optimization:
![Second version profiler results](https://github.com/AntonIVT/Optimization/blob/main/images/VtuneHashingVersion.png)
As you can see, the *hashing function* is **18.9 times** faster. But also the *get* function is **1.7 times** faster, because of the CRC32 hashes better and there are fewer collisions in the hash table.  
The total running time of the program is **33.292s**.

### String compare optimization

As you can see in the previous benchmark, the "heaviest" functions are still *get* and *strcmp*. I've decided to improve *strcmp* (because *get* is quite simple and hard optimizing). I've written *mstrcmp* (my strcmp) in assembly:
```Assembly
mstrcmp:

cmp_loop:

    mov rax, [rsi]
    mov rbx, [rdi]
    
    add rsi, 8
    add rdi, 8

    sub rax, rbx
    cmp rax, 0
    jne return_cmp

    mov al, BYTE [rsi]
    mov bl, BYTE [rdi]

    cmp al, 0
    jne rsi_not_zero
    movzx rax, bl
    ret

rsi_not_zero:
    cmp bl, 0
    jne cmp_loop
    movzx rax, al
    ret

return_cmp:
    ret
```
But *__strcmp_avx2* uses avx instructions and it's also inlining. So while *mstrcmp* is a little faster than the standard, *get* function becomes much slower. 
And I've had to rewrite *get* in assembly.

### Final optimization

# ALIGNMENT

## Results

So final optimization is **2.27** times on -O0. And with -O2 (The best stable optimization in C compilers) pure version works for **38.084s** and optimized function works for **24.052s**. And that's OK because in my optimized version almost all the most "haviest" function are written in assemly.

### Research task

In fact, the best way it's to write a hash table with open addressing. 
And if you interested in optimization and low-level programming you can try to do a comparison of hash table with open addressing and hash table with separate chaining. And of course both of them should be optimized :)

