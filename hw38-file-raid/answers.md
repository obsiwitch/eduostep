Q1. Use the simulator to perform some basic RAID mapping tests. Run with different levels (0, 1, 4, 5) and see if you can figure out the mappings of a set of requests. For RAID-5, see if you can figure out the difference between left-symmetric and left-asymmetric layouts. Use some different random seeds to generate different problems than above.

```sh
$ ./raid.py -n 16 -W seq -L 0 -c
# D0 D1 D2 D3
#  0  1  2  3
#  4  5  6  7
#  8  9 10 11
# 12 13 14 15

$ ./raid.py -n 8 -w 100 -W seq -L 1 -c
# D0 D1 D2 D3
#  0  0  1  1
#  2  2  3  3
#  4  4  5  5
#  6  6  7  7

$ ./raid.py -n 12 -w 100 -W seq -L 4 -c
# D0 D1 D2 D3
#  0  1  2  P
#  3  4  5  P
#  6  7  8  P
#  9 10 11  P

$ ./raid.py -n 12 -w 100 -W seq -L 5 -5 LS -c
# D0 D1 D2 D3
#  0  1  2  P
#  4  5  P  3
#  8  P  6  7
#  P  9 10 11

$ ./raid.py -n 12 -w 100 -W seq -L 5 -5 LA -c
# D0 D1 D2 D3
#  0  1  2  P
#  3  4  P  5
#  6  P  7  8
#  P  9 10 11
```

Q2. Do the same as the first problem, but this time vary the chunk size with `-C`. How does chunk size change the mappings?

```sh
$ ./raid.py -C 8k -n 16 -W seq -L 0 -c
# D0 D1 D2 D3
#  0  2  4  6
#  1  3  5  7
#  8 10 12 14
#  9 11 13 15
```

Q3. Do the same as above, but use the `-r` flag to reverse the nature of each problem.

Q4. Now use the reverse flag but increase the size of each request with the `-S` flag. Try specifying sizes of 8k, 12k, and 16k, while varying the RAID level. What happens to the underlying I/O pattern when the size of the request increases? Make sure to try this with the sequential workload too (`-W` sequential); for what request sizes are RAID-4 and RAID-5 much more I/O efficient?

I'm assuming a workload of sequential writes (`-W seq -w 100`) for these questions.

```
| level →          |     L0     ||     L1      ||     L4 & L5      |
| size (sectors) ↓ | r  | w | t || r | w  | t  || r   | w   | t    |
|------------------|----|---|---||---|----|----||-----|-----|------|
| 4k (1)           | 0  | 1 | 1 || 0 | 2  | 2  || 2   | 2   | 4    |
| 8k (2)           | 0  | 2 | 2 || 0 | 4  | 4  || 1;4 | 3;4 | 4;8  |
| 12k (3)          | 0  | 3 | 3 || 0 | 6  | 6  || 0   | 4   | 4    |
| 16k (4)          | 0  | 4 | 4 || 0 | 8  | 8  || 2   | 6   | 8    |
| 20k (5)          | 0  | 5 | 5 || 0 | 10 | 10 || 1;4 | 7;8 | 8;12 |
| 24k (6)          | 0  | 6 | 6 || 0 | 12 | 12 || 0   | 8   | 8    |
```

* `L0_total_ops = request_size / block_size`
* `L1_total_ops = ncopies * request_size / block_size`
* RAID-4 and RAID-5 are more efficient when `(request_size / block_size) % (ndisks - 1) == 0`

Q5. Use the timing mode of the simulator (`-t`) to estimate the performance of 100 random reads to the RAID, while varying the RAID levels, using 4 disks.

```sh
$ ./raid.py -n 100 -L 0 -t -c
# disk:0  busy: 100.00  I/Os:    28 (sequential:0 nearly:1 random:27)
# disk:1  busy:  93.91  I/Os:    29 (sequential:0 nearly:6 random:23)
# disk:2  busy:  87.92  I/Os:    24 (sequential:0 nearly:0 random:24)
# disk:3  busy:  65.94  I/Os:    19 (sequential:0 nearly:1 random:18)
# STAT totalTime 275.69999999999993

$ ./raid.py -n 100 -L 1 -t -c
# disk:0  busy: 100.00  I/Os:    28 (sequential:0 nearly:1 random:27)
# disk:1  busy:  86.98  I/Os:    24 (sequential:0 nearly:0 random:24)
# disk:2  busy:  97.52  I/Os:    29 (sequential:0 nearly:3 random:26)
# disk:3  busy:  65.23  I/Os:    19 (sequential:0 nearly:1 random:18)
# STAT totalTime 278.7


$ ./raid.py -n 100 -L 4 -t -c
# disk:0  busy:  78.48  I/Os:    30 (sequential:0 nearly:0 random:30)
# disk:1  busy: 100.00  I/Os:    40 (sequential:0 nearly:3 random:37)
# disk:2  busy:  76.46  I/Os:    30 (sequential:0 nearly:2 random:28)
# disk:3  busy:   0.00  I/Os:     0 (sequential:0 nearly:0 random:0)
# STAT totalTime 386.1000000000002

$ ./raid.py -n 100 -L 5 -t -c
# disk:0  busy: 100.00  I/Os:    28 (sequential:0 nearly:1 random:27)
# disk:1  busy:  95.84  I/Os:    29 (sequential:0 nearly:5 random:24)
# disk:2  busy:  87.60  I/Os:    24 (sequential:0 nearly:0 random:24)
# disk:3  busy:  65.70  I/Os:    19 (sequential:0 nearly:1 random:18)
# STAT totalTime 276.7
```

Q6. Do the same as above, but increase the number of disks. How does the performance of each RAID level scale as the number of disks increases?

```sh
for level in 0 1 4 5; do
for ndisks in {2..32..2}; do
    echo -n "$level $ndisks "
    ./raid.py -D "$ndisks" -n 100 -L "$level" -t -c | grep total
done; done | ./plot.py > plot-q6.png
```

![plot](plot-q6.png)

Q7. Do the same as above, but use all writes (`-w 100`) instead of reads. How does the performance of each RAID level scale now? Can you do a rough estimate of the time it will take to complete the workload of 100 random writes?

![plot](plot-q7.png)

Q8. Run the timing mode one last time, but this time with a sequential workload (`-W sequential`). How does the performance vary with RAID level, and when doing reads versus writes? How about when varying the size of each request? What size should you write to a RAID when using RAID-4 or RAID-5?

![plot seq read](plot-q8r.png)

![plot seq write](plot-q8w.png)

* Efficient request size for RAID-4 and RAID-5: see q4.
