#!/bin/bash

self="$0"

if [ "x${self:0:1}" != x/ ]; then
  self="$PWD/$self"
fi

self="$(dirname "$self")"
temp="./temp"
rm -rf "$temp"
mkdir -p "$temp"

failure() {
    local error_message="$1"

    # Print the error message
    echo "rop-yasm-8cc Error: $error_message"

    # Exit with error code 1
    exit 1
}

out_o="$1"
shift

"$self/../8cc" "$self/../crt/crt_rop.c" -S -o "$temp/pp.s" || failure
cat "$temp/pp.s" > "$temp/linked.s" || failure
touch "$temp/custom.rop"

while [ "x$1" != x ]; do
  ii="$1"
  ll="${#ii}"
  ll="$((ll-4))"
  if [ "${ii:ll}" == ".rop" ]; then
    cat "$ii" >> "$temp/custom.rop"
  else
    cat > "$temp/pp.c" <<EOF
unsigned short __builtin_bswap16(unsigned short);
unsigned int __builtin_bswap32(unsigned int);
unsigned long long __builtin_bswap64(unsigned long long);
EOF
    cpp -DPRINTF_DISABLE_SUPPORT_FLOAT '-D__asm__(...)=' '-D__restrict=' '-D__extension__=' '-D__builtin_va_list=int' '-D__inline=' '-D__attribute__(x)=' -isystem "$self/../include" -isystem "$self/../.." "$1" >> "$temp/pp.c" || failure
    "$self/../8cc" "$temp/pp.c" -S -o "$temp/pp.s" || failure
    cat "$temp/pp.s" >> "$temp/linked.s" || failure
    echo >> "$temp/linked.s" || failure
  fi
  shift
done

cat >> "$temp/linked.rop" <<EOF
pop rdi
dp stack_bottom
pop rsi
dq 8
sub rdi, rsi ; mov rdx, rdi
pop rax
dp ret_addr
mov [rdi], rax
pop rsp
dp _main
ret_addr:
mov rdi, rcx
pop rsp
dp stack_bottom
stack:
\$times 65536 db 0
stack_bottom:
dp exit
EOF

python3 "$self/nativecalls.c" < "$temp/linked.s" | python3 "$self/s2rop.c" >> "$temp/linked.rop" || failure
cat "$temp/custom.rop" >> "$temp/linked.rop"
python3 "$self/rop2asm.c" < "$temp/linked.rop" > "$temp/linked.asm"
yasm -f elf64 "$temp/linked.asm" -o "$out_o" || failure


