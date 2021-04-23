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
Below is a **plot of the dependence of the hash table operation time depending on the size/capacity**:  

![Plot](https://github.com/AntonIVT/Optimization/blob/main/images/main_plot.jpg)  
<p align="left"> <i> Figure 1 </i> </p>  

The plot shows that the load factor affects the speed of work. But the less the load factor, the more memory you have to spend on empty buckets.
It is necessary to choose the optimal value for memory and speed. So I decided that a load factor of **0.7** is ideal for my hash table.  
Load factor selection it is an algorithmic optimization. You don't need to think about how the hash table works.  
First benchmark with total time **54.160s**:

![Vtune1](https://github.com/AntonIVT/Optimization/blob/main/images/Vtune1.png)  
<p align="center"> <i> Figure 2 </i> </p>   

### Get function optimization

As you can see on the *Figure 2* the most significant function is *get*.
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
    push r9
    push r8
    call HashingFunction ; rax = hash
    pop r8
    pop r9

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

And here you can see here profiler results for this version of the hash table with upgraded function *get*: 

![Vtune2](https://github.com/AntonIVT/Optimization/blob/main/images/Vtune2.png)
<p align="center"> <i> Figure 3 </i> </p>  

And total time is **50.042s**. As you can see on *Figure 3* hashing function is very slow and in the next step I improved it.

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
And as you can see on the previous test *hashing function* is taking too long. So I decided to use **CRC32** for hashing strings. Fortunately there is intrinsic in SSE4.2 (Streaming SIMD Extensions, you could read about it [here](https://stackoverflow.blog/2020/07/08/improving-performance-with-simd-intrinsics-in-three-use-cases/)) **_mm_crc32_u64** that accumulates a CRC32 value for unsigned 64-bit integers.There also exists intrinsic **_mm_crc32_u8** that accumulates a CRC32 with only 8-bit integers, but with that hashing function total time is greater than was before: **52.128s**:

![Vtune3](https://github.com/AntonIVT/Optimization/blob/main/images/Vtune3.png)
<p align="center"> <i> Figure 4 </i> </p>  

Not surprisingly, both functions are good enough to hash strings:  

![Hashing compare](https://github.com/AntonIVT/Optimization/blob/main/images/hashing_cmp.jpg) 
<p align="left"> <i> Figure 5 </i> </p>  

And for speed I decided to used _mm_crc32_u64.  
But not all strings are divisible by 8. So let's change the data format for the input dictionary. Every string **must** be divisible by 8. Just add the required number of zeros in the end. Example: *"Hello\0\0\0"*. So next I've written a hash function with intrinsic. And profiler result you can see here:

![Vtune4](https://github.com/AntonIVT/Optimization/blob/main/images/Vtune4.png)  
<p align="center"> <i> Figure 6 </i> </p>  

The total time is **30.248s**.

### String compare optimization

As you can see on the *Figure 6*, the "heaviest" functions are *get* and *strcmp*. I've written *mstrcmp* (my strcmp) in assembly:
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
And as you can see I compare string by 8 bytes, because on previous step I've changed dictionary format. So that's why it's a little faster than standart *strcmp*:

![Vtune5](https://github.com/AntonIVT/Optimization/blob/main/images/Vtune5.png)
<p align="center"> <i> Figure 7 </i> </p>  

And total time with *mstrcm* is **29.694s**.

### CRC32 optimization

In the disassembler I noticed that CRC32 was implemented in hardware when I looked at the disassembled function:
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

![Vtune6](https://github.com/AntonIVT/Optimization/blob/main/images/Vtune6.png)
<p align="center"> <i> Figure 8 </i> </p>  

The total running time of the program is **23.993s**.

### Final optimization

I realized that you can very quickly compare strings and calculate the hash if you make the key length the same for all words.
And I did it like before: I changed hash table keys with zero extand to 32 bits. For string compare I just compared bytes with 256-bit **YMM registers**:
```Assembly
    ...
    vlddqu ymm0, [rsi]
    vlddqu ymm1, [rdi]
    vpcmpeqq ymm0, ymm0, ymm1
    vpmovmskb eax, ymm0
    cmp eax, -1
    ...
```
And for hashing i just do four crc32 hashing:
```Assembly
    ...
    xor rax, rax
    crc32 rax, QWORD [r9]
    crc32 rax, QWORD [r9 + 8]
    crc32 rax, QWORD [r9 + 16]
    crc32 rax, QWORD [r9 + 24]
    ...
```
Also I can use these functions in get function by inlining because they are very small. And there will be less time spent on calling the function.
And here benchmark of the final version:

![Vtune7](https://github.com/AntonIVT/Optimization/blob/main/images/Vtune7.png)
<p align="center"> <i> Figure 9 </i> </p>  

That's very fast and total time is **19.737s**

## Results

So final optimization is **2.74** times on -O0. And with -O2 (The best stable optimization in C compilers) pure version works for **36.317s** and optimized function works for **18.814s**. And that's OK because in my optimized version almost all the most "haviest" function are written in assemly.

### Research task

In fact, the best way it's to write a hash table with open addressing. 
And if you interested in optimization and low-level programming you can try to do a comparison of hash table with open addressing and hash table with separate chaining. And of course both of them should be optimized :)

