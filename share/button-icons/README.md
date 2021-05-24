These are selected button icons from https://thoseawesomeguys.com/prompts/.

Files are renamed using e.g.:

```shell
for F in PS4*.png; do mv $F $(echo $F | sed s/PS4_//g | tr [A-Z] [a-z]); done
```
