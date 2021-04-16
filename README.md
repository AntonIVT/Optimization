# Optimization
This is optimization project. I've tried to optimize hash table with assembly.  
Hash table was used as a dictionary. Dictionary had 160000 words and program add all these words to the hash table at the start.  
For this hash table I used separate chaining by my own structure List. For testing speed I launched 1000 times get-function of every word in dictionary:  
![Speed text](https://github.com/AntonIVT/Optimization/blob/main/images/speed_test.png?raw=true)  
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
I've decided that I can rewrite all get function (with my own strcmp) on assembly.  
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
