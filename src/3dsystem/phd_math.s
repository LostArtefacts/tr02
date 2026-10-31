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

    end
