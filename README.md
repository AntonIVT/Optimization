# Improving Hash Table Performance

## Abstract

In this article I describe some methods for optimizing hash table written in C. 

## Introduction
 
I've optimized a hash table that can be used like a dictionary. By a word, we can get the translation of that word,
the goal is to reduce this time. I've used both algorithmic and low-level optimization methods. 
I used hashing with separate chaining and for "chains" I used my own structure List. 

# ADD DICTIONARY INPUT.

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
![Load factor plot](https://github.com/AntonIVT/Optimization/blob/main/images/plot.jpg)  
The plot shows that the load factor affects the speed of work. But the less the load factor, the more memory you have to spend on empty buckets.
It is necessary to choose the optimal value for memory and speed. So I decided that a load factor of **0.65** is ideal for my hash table.  
Load factor selection it is an algorithmic optimization. You don't need to think about how the hash table works.

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
The total running time of the program is **58.256s**. And as you can see the hashing function is taking too long. So I decided to use **CRC32** for hashing strings. Fortunately there is intrinsic in SSE4.2 (Streaming SIMD Extensions, you could read about it [here](https://stackoverflow.blog/2020/07/08/improving-performance-with-simd-intrinsics-in-three-use-cases/)) **_mm_crc32_u64** that accumulates a CRC32 value for unsigned 64-bit integers.
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
As you can see, the hashing function is **18.9 times** faster. But also the ger function is **1.7 times** faster, because of the CRC32 hashes better and there are fewer collisions in the hash table.  
The total running time of the program is **33.292s**.

### String compare optimization

### Get function optimization


## Result
