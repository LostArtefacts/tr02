    .486
    .model flat

    extern _GouraudTable:byte
    extern _DepthQTable:byte
    extern _phd_winptr:dword
    extern _phd_winwidth:dword
    extern _phd_scrwidth:dword
    extern _xbuffer:dword

    .code

@flatA@12 proc public
    pusha
    sub edx, eax
    jle flatA_4
    mov esi, eax
    shl esi, 3
    add esi, offset _xbuffer+2
    imul eax, dword ptr _phd_scrwidth
    add eax, dword ptr _phd_winptr
    mov ebp, eax
    cld
    mov bh, bl
    mov ax, bx
    bswap eax
    mov ax, bx
flatA_1:
    movsx ecx, word ptr [esi]
    movsx ebx, word ptr [esi+4]
    sub ebx, ecx
    jle flatA_3
    lea edi, [ebp+ecx]
    neg ecx
    and ecx, 3
    cmp ecx, ebx
    jge flatA_2
    sub ebx, ecx
    rep stosb
    mov ecx, ebx
    sar ecx, 2
    rep stosd
    and ebx, 3
flatA_2:
    mov ecx, ebx
    rep stosb
flatA_3:
    add esi, 8
    add ebp, dword ptr _phd_scrwidth
    dec edx
    jne flatA_1
flatA_4:
    popa
    ret
@flatA@12 endp

    end
