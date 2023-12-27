# list of all memory leaks currently

Note: I'm not putting this here because I don't want to fix them, I'm putting it here because I can't find the leaks. They give me where they are created with malloc/realloc but I can't find where I don't free the memory

Maybe I'll add creation points/initial data for each of these and see them travel through the program.

```
==526==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 64 byte(s) in 1 object(s) allocated from:
#1 0x5633ea473255 in sakuraX_makeNode source/parser.c:7

Direct leak of 10 byte(s) in 2 object(s) allocated from:
#1 0x5633ea477d02 in s_str_n source/sstr.c:18

Direct leak of 5 byte(s) in 1 object(s) allocated from:
#1 0x5633ea477d56 in s_str_copy source/sstr.c:26

Indirect leak of 512 byte(s) in 8 object(s) allocated from:
#1 0x5633ea473255 in sakuraX_makeNode source/parser.c:7

Indirect leak of 96 byte(s) in 4 object(s) allocated from:
#1 0x5633ea473d5e in sakuraY_analyze source/parser.c:158

Indirect leak of 72 byte(s) in 3 object(s) allocated from:
#1 0x5633ea473cac in sakuraY_analyze source/parser.c:193

Indirect leak of 24 byte(s) in 1 object(s) allocated from:
#1 0x5633ea473e8a in sakuraY_analyze source/parser.c:167

Indirect leak of 8 byte(s) in 1 object(s) allocated from:
#1 0x5633ea475b38 in sakuraX_parseBlocks source/parser.c:518

Indirect leak of 8 byte(s) in 1 object(s) allocated from:
#1 0x5633ea475659 in sakuraX_parseFactor source/parser.c:377

SUMMARY: AddressSanitizer: 799 byte(s) leaked in 22 allocation(s).
```
