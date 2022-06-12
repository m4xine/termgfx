for i in examples/*.c; do
  [ -f "$i" ] || break
  gcc -o"${i%.c}.o" $i -Iinclude -std=c99
done