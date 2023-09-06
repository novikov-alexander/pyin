# pyin
pYin back from    
https://code.soundsoftware.ac.uk/projects/pyin/files

# multithreading
This particular fork contains small changes which improve performance of the precise time mode on multi-core systems.

Measurement made on Intel Core I5-1035G4 with 4 cores
for the default regression test on bob_02.wav.
Given average for 10 runs:

| Vanilla | Multithreaded |
|---------|---------------|
| 15.663s |     9.946s    |


Speedup:
| Vanilla | Multithreaded |
|---------|---------------|
|   1x    |     1.575x    |

Effectiveness:
0.393

