    .486
    .model flat

    extern phdtan2:dword
    extern phdtantab:word
    extern phdsintab:word

    .code

@phd_atan@8 proc public
    push ebx
    push ecx
    push edx
    mov eax, ecx
    mov ebx, eax
    xor ecx, ecx
    or eax, edx
    je atan_done
    test ebx, ebx
    jns atan_xpos
    add ecx, 4
    neg ebx
atan_xpos:
    test edx, edx
    jns atan_ypos
    add ecx, 2
    neg edx
atan_ypos:
    xor eax, eax
    cmp edx, ebx
    jle atan_div
    inc ecx
    xchg ebx, edx
atan_div:
    shrd eax, edx, 21
    shr edx, 21
    idiv ebx
    mov ax, phdtantab[eax * 2]
    add eax, phdtan2[ecx * 4]
    jns atan_done
    neg eax
atan_done:
    pop edx
    pop ecx
    pop ebx
    ret
@phd_atan@8 endp

@phd_cos@4 proc public
    add ecx, 16384
@phd_cos@4 endp

@phd_sin@4 proc public
    mov eax, ecx
    and eax, 65535
    cmp eax, 32768
    jl Func457f09
    sub eax, 32768
    call Func457f09
    neg eax
    ret
@phd_sin@4 endp

Func457f09 proc
    cmp eax, 16384
    jle sin_quarter
    neg eax
    add eax, 32768
sin_quarter:
    shr eax, 4
    mov ax, phdsintab[eax * 2]
    ret
Func457f09 endp

@phd_sqrt@4 proc public
    push ebx
    push ecx
    push edx
    mov ebx, 40000000h
    xor eax, eax
sqrt_loop:
    mov edx, eax
    add eax, ebx
    shr edx, 1
    cmp eax, ecx
    ja sqrt_skip
    sub ecx, eax
    mov eax, ebx
    or eax, edx
    shr ebx, 2
    jne sqrt_loop
    jmp sqrt_done
sqrt_skip:
    mov eax, edx
    shr ebx, 2
    jne sqrt_loop
sqrt_done:
    pop edx
    pop ecx
    pop ebx
    ret
@phd_sqrt@4 endp

    end
