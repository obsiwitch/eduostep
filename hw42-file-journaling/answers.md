Q1. First, run `fsck.py -D`; this flag turns off any corruption, and thus you can use it to generate a random file system, and see if you can determine which files and directories are in there. So, go ahead and do that! Use the` -p` flag to see if you were right. Try this for a few different randomly-generated file systems by setting the seed (`-s`) to different values, like 1, 2, and 3.

```
$ ./fsck.py -D
inode bitmap 1000100010000101
inodes       [d a:0 r:4] [] [] [] [d a:12 r:2] [] [] [] [d a:6 r:2] [] [] [] [] [f a:-1 r:2] [] [f a:-1 r:1]
data bitmap  1000001000001000
data         [(.,0) (..,0) (g,8) (w,4) (m,13) (z,13)] [] [] [] [] [] [(.,8) (..,0) (s,15)] [] [] [] [] [] [(.,4) (..,0)] [] [] []

/
├── g/
│   └── s
├── w/
├── m
└── z
```

Q2. Now, let’s introduce a corruption. Run `fsck.py -S 1` to start. Can you see what inconsistency is introduced? How would you fix it in a real file system repair tool? Use `-c` to check if you were right.

```
$ ./fsck.py -S 1
inode bitmap 1000100010000001
inodes       [d a:0 r:4] [] [] [] [d a:12 r:2] [] [] [] [d a:6 r:2] [] [] [] [] [f a:-1 r:2] [] [f a:-1 r:1]
data bitmap  1000001000001000
data         [(.,0) (..,0) (g,8) (w,4) (m,13) (z,13)] [] [] [] [] [] [(.,8) (..,0) (s,15)] [] [] [] [] [] [(.,4) (..,0)] [] [] []

There is an inconsistency between inode bitmap bit 13 and inode 13, the inode
is allocated but the inode bitmap indicates the inode should be unallocated.

inode bitmap fix: `1000100010000101`
```


Q3. Change the seed to `-S 3` or `-S 19`; which inconsistency do you see? Use `-c` to check your answer. What is different in these two cases?

```
$ ./fsck.py -S 3
inode bitmap 1000100010000101
inodes       [d a:0 r:4] [] [] [] [d a:12 r:2] [] [] [] [d a:6 r:2] [] [] [] [] [f a:-1 r:2] [] [f a:-1 r:2]
data bitmap  1000001000001000
data         [(.,0) (..,0) (g,8) (w,4) (m,13) (z,13)] [] [] [] [] [] [(.,8) (..,0) (s,15)] [] [] [] [] [] [(.,4) (..,0)] [] [] []

inodes[15] has a refcnt=2 when in reality it is referenced only once
in the data blocks ((s,15)).

inodes fix: [d a:0 r:4] [] [] [] [d a:12 r:2] [] [] [] [d a:6 r:2] [] [] [] [] [f a:-1 r:2] [] [f a:-1 r:1]
```

```
$ ./fsck.py -S 19
inode bitmap 1000100010000101
inodes       [d a:0 r:4] [] [] [] [d a:12 r:2] [] [] [] [d a:6 r:1] [] [] [] [] [f a:-1 r:2] [] [f a:-1 r:1]
data bitmap  1000001000001000
data         [(.,0) (..,0) (g,8) (w,4) (m,13) (z,13)] [] [] [] [] [] [(.,8) (..,0) (s,15)] [] [] [] [] [] [(.,4) (..,0)] [] [] []


inodes[8] has an incorrect refcnt of 1 instead of 2. All directories
are at least referenced twice: by themselves and by their parent.

inodes fix: [d a:0 r:4] [] [] [] [d a:12 r:2] [] [] [] [d a:6 r:2] [] [] [] [] [f a:-1 r:2] [] [f a:-1 r:1]
```

In the first case the reference count of a file increased when it shouldn't have (corruption? failed hard link?). In the second case the reference count of a directory decreased below 2 when it shouldn't have (corruption?).

Q4. Change the seed to `-S 5`; which inconsistency do you see? How hard would it be to fix this problem in an automatic way? Use `-c` to check your answer. Then, introduce a similar inconsistency with `-S 38`; is this harder/possible to detect? Finally, use `-S 642`; is this inconsistency detectable? If so, how would you fix the file system?

* `-S 5`: entry `(s,15)` of `data[6]` changed to `(y,15)`. Impossible to notice without `-c`. Without a log of operations it's impossible to detect the change or fix this automatically. With this simple FS, we don't have a way to distinguish a normal rename from a filename corruption.
* `-S 38`: entry `(..,0)` of `data[12]` changed to `(b,0)`. Since the directory lacks a `..` entry we don't know its parent anymore, we need to find that out first by iterating over every directories and check if the inode of our corrupted directory is referenced in an entry. Assuming no other problem has happened in the FS and hard links to directories are disallowed, one unique entry will be found. Once we have the parent we can add the missing `..` entry. Then the `fsck` could notice entry `(b,0)` is weird since it doesn't have a special name (`.` or `..`) and it references a directory higher in the hierarchy. Thus it would make this entry a hard link to a directory, which shouldn't be allowed by the FS, and it should be deleted. **Note**: I broke the fix into 2 steps because I intuited it could be risky to directly make assumptions about `(b,0)` being the missing and corrupted `(..,0)` entry in a real case scenario.
* `-S 642`: entry `(g,8)` of `data[0]` changed to `(w,8)`. The problem is detectable in this case because we have 2 entries (`(w,4)` and `(w,8)`) with the same name to 2 different directories. Without a log we can't retrieve the old directory name, but we can rename one of the two entries to differentiate them (e.g. by appending a number at the end).

Q5. Change the seed to `-S 6` or `-S 13`; which inconsistency do you see? Use `-c` to check your answer. What is the difference across these two cases? What should the repair tool do when encountering such a situation?

Q6. Change the seed to `-S 9`; which inconsistency do you see? Use `-c` to check your answer. Which piece of information should a check-and-repair tool trust in this case?

Q7. Change the seed to `-S 15`; which inconsistency do you see? Use `-c` to check your answer. What can a repair tool do in this case? If no repair is possible, how much data is lost?

Q8. Change the seed to `-S 10`; which inconsistency do you see? Use `-c` to check your answer. Is there redundancy in the file system structure here that can help a repair?

Q9. Change the seed to `-S 16` and `-S 20`; which inconsistency do you see? Use `-c` to check your answer. How should the repair tool fix the problem?