## Debug with gdb
```
gcc -o main main.c -g && gdb main -ex "break main.c:131" -ex "run"
```
