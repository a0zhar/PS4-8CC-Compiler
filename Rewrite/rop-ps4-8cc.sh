#!/bin/bash

self="$0"

if [ "x${self:0:1}" != x/ ]; then
    self="$PWD/$self"
fi

self="$(dirname "$self")"
temp="./temp"
rm -rf "$temp"
mkdir -p "$temp"

failure () {
    local error_message="$1"

    # Print the error message
    echo "rop-ps4-8cc Error: $error_message"

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
        cat > "$temp/pp.c" << EOF
unsigned short __builtin_bswap16(unsigned short);
unsigned int __builtin_bswap32(unsigned int);
unsigned long long __builtin_bswap64(unsigned long long);
EOF
        cpp -P -D__PS4__ -DPRINTF_DISABLE_SUPPORT_FLOAT '-D__asm__(...)=' '-D__restrict=' '-D__extension__=' '-D__builtin_va_list=int' '-D__inline=' '-D__attribute__(x)=' '-D__asm(...)=' '-D__builtin_offsetof(a, b)=(((char*)&((a*)0)->b)-(char*)0)' -isystem "$self/../include" -isystem "$self/../.." -isystem "$self/../../freebsd-headers" -nostdinc "$1" >> "$temp/pp.c" || failure
        
        "$self/../8cc" "$temp/pp.c" -S -o "$temp/pp.s" || failure
        cat "$temp/pp.s" >> "$temp/linked.s" || failure
        echo >> "$temp/linked.s" || failure
    fi
    shift
done

cat >> "$temp/linked.rop" << EOF
\$\$var main_ret = malloc(8);
\$\$var printf_buf = malloc(65536);
\$\$var __swbuf_addr = 0; // STUB
pop rax
dp rdi_bak
mov [rax], rdi
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
pop rsp
dp stack_bottom
_ps4_printf_buffer:
\$\$var printf_buf_offset = SP_OFFSET;
\$printf_buf
_ps4_printf_fd:
dq -1
stack:
db bytes(65536)
stack_bottom:
mov rax, rcx
pop rsi
\$main_ret
mov [rsi], rax
pop rdi
rdi_bak:
_pivot_back_addr:
dq 0
\$pivot_addr
EOF

python3 "$self/nativecalls.c" < "$temp/linked.s" | python3 "$self/s2rop.c" >> "$temp/linked.rop" || failure
cat "$temp/custom.rop" >> "$temp/linked.rop"

cat >> "$temp/linked.rop" << EOF
\$\$pivot(ropchain);
\$\$var main_ret = read_ptr_at(main_ret);
\$\$var printf_buf_end = read_ptr_at(ropchain+printf_buf_offset);
\$\$var printf_ans = read_mem_as_string(printf_buf, printf_buf_end-printf_buf);
\$\$var _ = malloc_nogc.pop();
\$\$var _ = malloc_nogc.pop();
\$\$var _ = malloc_nogc.pop();
EOF

cat "$temp/linked.rop" > "$out_o"
