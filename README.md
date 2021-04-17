# Improving Hash Table Performance

## Abstract

In this article I describe some methods for optimizing hash table written in C. 

## Introduction
 
I've optimized a hash table that can be used like a dictionary. By a word, we can get the translation of that word,
the goal is to reduce this time. I've used both algorithmic and low-level optimization methods. 
I used hashing with separate chaining and for "chains" I used my own structure List.


### Analysis

Machine: 4xIntel® Core™ i5-6200U Processor  

For testing speed I've used a function that gets each word in the dictionary (160,000 words) 1000 times.
This is necessary so that the time spent on constructing the table and on other side functions is minimal.
It is also important not to call the same words several times in a row because people rarely translate the same words several times.
In this way we break the cache, but we get more realistic conditions. Here speed test code:  
```
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
Below is a plot of the dependence of the hash table operation time depending on the load factor:  
![Load factor plot](https://github.com/AntonIVT/Optimization/blob/main/images/plot.jpg)  
The plot shows that the load factor affects the speed of work. But the less the load factor, the more memory you have to spend on empty buckets.
It is necessary to choose the optimal value for memory and speed. So I decided that a load factor of 0.65 is ideal for my hash table.  
Load factor selection it is an algorithmic optimization. You don't need to think about how the hash table works.

### Hashing function optimization

### String compare optimization

### Get function optimization


## Result

Dictionary had 160000 words and program add all these words to the hash table at the start.  
For this hash table I used separate chaining by my own structure List. For testing speed I launched 1000 times get-function of every word in dictionary:  
![Speed test](https://github.com/AntonIVT/Optimization/blob/main/images/speed_test.png?raw=true)  
First profiler attempt:  
![First profiler attempt](https://github.com/AntonIVT/Optimization/blob/main/images/vtune_1.png?raw=true)  
As you can see a lot of time is spent on get function, hashing and strcmp function. So I tried to optimize them all.
For the beginning I optimized hashing function. That's how it looked before:  
![Slow hash](https://github.com/AntonIVT/Optimization/blob/main/images/hash_slow.png?raw=true)  
It's very slow becouse we hashing every byte apart. I've decided to use intrinsic _mm_crc32_u64. And it's become faster. But in exec file I found (with objdump)
that this intrinsic use system call crc32. So why don't we use this syscall directly? That's what I did:  
![Fast hash](https://github.com/AntonIVT/Optimization/blob/main/images/hash_asm.png?raw=true)  
And that's really works! Also get function has become faster too!:  
![Second profiler attempt](https://github.com/AntonIVT/Optimization/blob/main/images/vtune_2.png?raw=true)  
And here starts the hardest part of optimization. I've optimized get function. This is how it's looked before:  
![Get first view](https://github.com/AntonIVT/Optimization/blob/main/images/get_1.png?raw=true)  
I've decided that I can rewrite whole get function (with my own strcmp) on assembly.  
Why? Check how it looked on assembly:  
![Get assembly slow](https://github.com/AntonIVT/Optimization/blob/main/images/get_dump.png?raw=true)  
There's a looot of useless memory access moves.  
I rewrite this with almost only registers usage:  
![Get assembly fast](https://github.com/AntonIVT/Optimization/blob/main/images/get_fast.png?raw=true)  
And of course I called strcmp from assembly. My strcmp compares string by 8 bytes.  
For that dictionary must be aligned on a 8-byte boundary (Just zero extend to 8 byte).  
That's how it look like:  
![My strcmp](https://github.com/AntonIVT/Optimization/blob/main/images/strcmp_fast.png?raw=true)  
As a result My hash table optimized on 57%:  
![Third profiler attempt](https://github.com/AntonIVT/Optimization/blob/main/images/vtune_3.png?raw=true)
