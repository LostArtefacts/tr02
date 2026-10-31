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

@transA@12 proc public
    pusha
    sub esp, 8
    sub edx, eax
    jle transA_8
    mov dword ptr [esp+4], edx
    lea esi, _DepthQTable
    cmp ebx, 32
    ja transA_8
    shl ebx, 8
    add esi, ebx
    mov ebp, eax
    shl ebp, 3
    add ebp, offset _xbuffer+2
    imul eax, dword ptr _phd_scrwidth
    add eax, dword ptr _phd_winptr
    mov dword ptr [esp], eax
    xor eax, eax
transA_1:
    movsx edi, word ptr [ebp+0]
    test edi, edi
    jns transA_2
    xor edi, edi
transA_2:
    movsx ecx, word ptr [ebp+4]
    cmp ecx, dword ptr _phd_winwidth
    jle transA_3
    mov ecx, dword ptr _phd_winwidth
transA_3:
    sub ecx, edi
    jle transA_7
    add edi, dword ptr [esp]
    test edi, 1
    je transA_4
    mov al, byte ptr [edi]
    mov dl, byte ptr [esi+eax]
    mov byte ptr [edi], dl
    dec ecx
    inc edi
transA_4:
    sub ecx, 16
    js transA_5
    mov ebx, dword ptr [edi]
    mov al, bl
    mov dl, byte ptr [esi+eax]
    mov al, bh
    mov dh, byte ptr [esi+eax]
    bswap ebx
    mov word ptr [edi], dx
    mov al, bh
    mov dl, byte ptr [esi+eax]
    mov al, bl
    mov dh, byte ptr [esi+eax]
    mov word ptr [edi+2], dx
    add edi, 4
    mov ebx, dword ptr [edi]
    mov al, bl
    mov dl, byte ptr [esi+eax]
    mov al, bh
    mov dh, byte ptr [esi+eax]
    bswap ebx
    mov word ptr [edi], dx
    mov al, bh
    mov dl, byte ptr [esi+eax]
    mov al, bl
    mov dh, byte ptr [esi+eax]
    mov word ptr [edi+2], dx
    add edi, 4
    mov ebx, dword ptr [edi]
    mov al, bl
    mov dl, byte ptr [esi+eax]
    mov al, bh
    mov dh, byte ptr [esi+eax]
    bswap ebx
    mov word ptr [edi], dx
    mov al, bh
    mov dl, byte ptr [esi+eax]
    mov al, bl
    mov dh, byte ptr [esi+eax]
    mov word ptr [edi+2], dx
    add edi, 4
    mov ebx, dword ptr [edi]
    mov al, bl
    mov dl, byte ptr [esi+eax]
    mov al, bh
    mov dh, byte ptr [esi+eax]
    bswap ebx
    mov word ptr [edi], dx
    mov al, bh
    mov dl, byte ptr [esi+eax]
    mov al, bl
    mov dh, byte ptr [esi+eax]
    mov word ptr [edi+2], dx
    add edi, 4
    jmp transA_4
transA_5:
    add ecx, 16
    jle transA_7
transA_6:
    mov al, byte ptr [edi]
    mov dl, byte ptr [esi+eax]
    mov byte ptr [edi], dl
    inc edi
    dec ecx
    jne transA_6
transA_7:
    mov ebx, dword ptr _phd_scrwidth
    add ebp, 8
    add dword ptr [esp], ebx
    dec dword ptr [esp+4]
    jne transA_1
transA_8:
    add esp, 8
    popa
    ret
@transA@12 endp

    end
