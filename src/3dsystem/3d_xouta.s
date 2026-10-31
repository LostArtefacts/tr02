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

@gourA@12 proc public
    push ecx
    push esi
    push edi
    push ebp
    sub esp, 16
    sub edx, eax
    jle gourA_7
    mov dword ptr [esp+4], edx
    mov ecx, eax
    shl ecx, 4
    add ecx, offset _xbuffer
    mov dword ptr [esp+8], ecx
    imul eax, dword ptr _phd_scrwidth
    add eax, dword ptr _phd_winptr
    mov dword ptr [esp+12], eax
    movzx ebx, bx
    shl ebx, 5
    add ebx, offset _GouraudTable
    mov dword ptr [esp], ebx
gourA_1:
    mov eax, dword ptr [esp+8]
    movsx ebx, word ptr [eax+10]
    movsx edi, word ptr [eax+2]
    cmp edi, ebx
    jge gourA_6
    mov ecx, dword ptr [eax+4]
    mov ebp, ebx
    mov edx, dword ptr [eax+12]
    sub edx, ecx
    sub ebx, edi
    mov eax, edx
    sar edx, 31
    idiv ebx
    test edi, edi
    jns gourA_2
    imul edi, eax
    sub ecx, edi
    xor edi, edi
gourA_2:
    sub ebp, edi
    add edi, dword ptr [esp+12]
    mov edx, eax
    mov esi, ecx
    sar eax, 16
    sar ecx, 16
    shl edx, 16
    shl esi, 16
    add ecx, dword ptr [esp]
    test edi, 1
    je gourA_3
    mov bl, byte ptr [ecx]
    add esi, edx
    mov byte ptr [edi], bl
    adc ecx, eax
    dec ebp
    inc edi
gourA_3:
    sub ebp, 16
    js gourA_4
    mov bl, byte ptr [ecx]
    add esi, edx
    adc ecx, eax
    mov bh, byte ptr [ecx]
    add esi, edx
    mov word ptr [edi], bx
    adc ecx, eax
    mov bl, byte ptr [ecx]
    add esi, edx
    adc ecx, eax
    mov bh, byte ptr [ecx]
    add esi, edx
    mov word ptr [edi+2], bx
    adc ecx, eax
    mov bl, byte ptr [ecx]
    add esi, edx
    adc ecx, eax
    mov bh, byte ptr [ecx]
    add esi, edx
    mov word ptr [edi+4], bx
    adc ecx, eax
    mov bl, byte ptr [ecx]
    add esi, edx
    adc ecx, eax
    mov bh, byte ptr [ecx]
    add esi, edx
    mov word ptr [edi+6], bx
    adc ecx, eax
    mov bl, byte ptr [ecx]
    add esi, edx
    adc ecx, eax
    mov bh, byte ptr [ecx]
    add esi, edx
    mov word ptr [edi+8], bx
    adc ecx, eax
    mov bl, byte ptr [ecx]
    add esi, edx
    adc ecx, eax
    mov bh, byte ptr [ecx]
    add esi, edx
    mov word ptr [edi+10], bx
    adc ecx, eax
    mov bl, byte ptr [ecx]
    add esi, edx
    adc ecx, eax
    mov bh, byte ptr [ecx]
    add esi, edx
    mov word ptr [edi+12], bx
    adc ecx, eax
    mov bl, byte ptr [ecx]
    add esi, edx
    adc ecx, eax
    mov bh, byte ptr [ecx]
    add esi, edx
    mov word ptr [edi+14], bx
    adc ecx, eax
    add edi, 16
    jmp gourA_3
gourA_4:
    add ebp, 16
    jle gourA_6
gourA_5:
    mov bl, byte ptr [ecx]
    add esi, edx
    mov byte ptr [edi], bl
    adc ecx, eax
    inc edi
    dec ebp
    jne gourA_5
gourA_6:
    mov eax, _phd_scrwidth
    add dword ptr [esp+8], 16
    add dword ptr [esp+12], eax
    dec dword ptr [esp+4]
    jne gourA_1
gourA_7:
    add esp, 16
    pop ebp
    pop edi
    pop esi
    pop ecx
    ret
@gourA@12 endp

@gtmapA@12 proc public
    pusha
    sub esp, 44
    sub edx, eax
    jle gtmapA_6
    mov dword ptr [esp+32], edx
    mov ecx, eax
    shl ecx, 5
    add ecx, offset _xbuffer
    mov dword ptr [esp+28], ecx
    imul eax, dword ptr _phd_scrwidth
    add eax, dword ptr _phd_winptr
    mov dword ptr [esp+40], eax
    mov dword ptr [esp+36], ebx
gtmapA_1:
    mov esi, dword ptr [esp+28]
    mov edi, dword ptr [esi]
    mov ebp, dword ptr [esi+16]
    sar edi, 16
    sar ebp, 16
    cmp ebp, edi
    jle gtmapA_5
    mov ebx, ebp
    sub ebx, edi
    mov ecx, dword ptr [esi+4]
    mov eax, dword ptr [esi+20]
    sub eax, ecx
    cdq
    idiv ebx
    mov dword ptr [esp+20], ecx
    mov dword ptr [esp+8], eax
    mov ecx, dword ptr [esi+8]
    mov eax, dword ptr [esi+24]
    sub eax, ecx
    cdq
    idiv ebx
    mov dword ptr [esp+12], ecx
    mov dword ptr [esp], eax
    mov ecx, dword ptr [esi+12]
    mov eax, dword ptr [esi+28]
    sub eax, ecx
    cdq
    idiv ebx
    mov dword ptr [esp+16], ecx
    mov dword ptr [esp+4], eax
    sub ebp, edi
    add edi, dword ptr [esp+40]
    mov esi, dword ptr [esp+36]
    mov dword ptr [esp+24], ebp
    xor eax, eax
    xor ebx, ebx
    mov ecx, dword ptr [esp+12]
    mov edx, dword ptr [esp+16]
    mov ebp, dword ptr [esp+20]
    shl ecx, 16
    shl edx, 16
    shl ebp, 16
    mov cl, byte ptr [esp+2]
    mov ch, byte ptr [esp+6]
    mov dh, byte ptr [esp+10]
    shl dword ptr [esp], 16
    shl dword ptr [esp+4], 16
    shl dword ptr [esp+8], 16
    mov al, byte ptr [esp+14]
    mov ah, byte ptr [esp+18]
    mov bh, byte ptr [esp+22]
gtmapA_2:
    sub dword ptr [esp+24], 16
    js gtmapA_3
    mov bl, byte ptr [esi+eax]
    mov dl, byte ptr _DepthQTable[ebx]
    add ecx, dword ptr [esp]
    mov byte ptr [edi], dl
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    mov dl, byte ptr _DepthQTable[ebx]
    add ecx, dword ptr [esp]
    mov byte ptr [edi+1], dl
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    mov dl, byte ptr _DepthQTable[ebx]
    add ecx, dword ptr [esp]
    mov byte ptr [edi+2], dl
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    mov dl, byte ptr _DepthQTable[ebx]
    add ecx, dword ptr [esp]
    mov byte ptr [edi+3], dl
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    mov dl, byte ptr _DepthQTable[ebx]
    add ecx, dword ptr [esp]
    mov byte ptr [edi+4], dl
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    mov dl, byte ptr _DepthQTable[ebx]
    add ecx, dword ptr [esp]
    mov byte ptr [edi+5], dl
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    mov dl, byte ptr _DepthQTable[ebx]
    add ecx, dword ptr [esp]
    mov byte ptr [edi+6], dl
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    mov dl, byte ptr _DepthQTable[ebx]
    add ecx, dword ptr [esp]
    mov byte ptr [edi+7], dl
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    mov dl, byte ptr _DepthQTable[ebx]
    add ecx, dword ptr [esp]
    mov byte ptr [edi+8], dl
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    mov dl, byte ptr _DepthQTable[ebx]
    add ecx, dword ptr [esp]
    mov byte ptr [edi+9], dl
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    mov dl, byte ptr _DepthQTable[ebx]
    add ecx, dword ptr [esp]
    mov byte ptr [edi+10], dl
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    mov dl, byte ptr _DepthQTable[ebx]
    add ecx, dword ptr [esp]
    mov byte ptr [edi+11], dl
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    mov dl, byte ptr _DepthQTable[ebx]
    add ecx, dword ptr [esp]
    mov byte ptr [edi+12], dl
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    mov dl, byte ptr _DepthQTable[ebx]
    add ecx, dword ptr [esp]
    mov byte ptr [edi+13], dl
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    mov dl, byte ptr _DepthQTable[ebx]
    add ecx, dword ptr [esp]
    mov byte ptr [edi+14], dl
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    mov dl, byte ptr _DepthQTable[ebx]
    add ecx, dword ptr [esp]
    mov byte ptr [edi+15], dl
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    add edi, 16
    jmp gtmapA_2
gtmapA_3:
    add dword ptr [esp+24], 16
    jle gtmapA_5
gtmapA_4:
    mov bl, byte ptr [esi+eax]
    mov dl, byte ptr _DepthQTable[ebx]
    add ecx, dword ptr [esp]
    mov byte ptr [edi], dl
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    inc edi
    dec dword ptr [esp+24]
    jne gtmapA_4
gtmapA_5:
    mov eax, _phd_scrwidth
    add dword ptr [esp+28], 32
    add dword ptr [esp+40], eax
    dec dword ptr [esp+32]
    jne gtmapA_1
gtmapA_6:
    add esp, 44
    popa
    ret
@gtmapA@12 endp

@wgtmapA@12 proc public
    pusha
    sub esp, 44
    sub edx, eax
    jle wgtmapA_23
    mov dword ptr [esp+32], edx
    mov ecx, eax
    shl ecx, 5
    add ecx, offset _xbuffer
    mov dword ptr [esp+28], ecx
    imul eax, dword ptr _phd_scrwidth
    add eax, dword ptr _phd_winptr
    mov dword ptr [esp+40], eax
    mov dword ptr [esp+36], ebx
wgtmapA_1:
    mov esi, dword ptr [esp+28]
    mov edi, dword ptr [esi]
    mov ebp, dword ptr [esi+16]
    sar edi, 16
    sar ebp, 16
    cmp ebp, edi
    jle wgtmapA_22
    mov ebx, ebp
    sub ebx, edi
    mov ecx, dword ptr [esi+4]
    mov eax, dword ptr [esi+20]
    sub eax, ecx
    cdq
    idiv ebx
    mov dword ptr [esp+20], ecx
    mov dword ptr [esp+8], eax
    mov ecx, dword ptr [esi+8]
    mov eax, dword ptr [esi+24]
    sub eax, ecx
    cdq
    idiv ebx
    mov dword ptr [esp+12], ecx
    mov dword ptr [esp], eax
    mov ecx, dword ptr [esi+12]
    mov eax, dword ptr [esi+28]
    sub eax, ecx
    cdq
    idiv ebx
    mov dword ptr [esp+16], ecx
    mov dword ptr [esp+4], eax
    sub ebp, edi
    add edi, dword ptr [esp+40]
    mov esi, dword ptr [esp+36]
    mov dword ptr [esp+24], ebp
    xor eax, eax
    xor ebx, ebx
    mov ecx, dword ptr [esp+12]
    mov edx, dword ptr [esp+16]
    mov ebp, dword ptr [esp+20]
    shl ecx, 16
    shl edx, 16
    shl ebp, 16
    mov cl, byte ptr [esp+2]
    mov ch, byte ptr [esp+6]
    mov dh, byte ptr [esp+10]
    shl dword ptr [esp], 16
    shl dword ptr [esp+4], 16
    shl dword ptr [esp+8], 16
    mov al, byte ptr [esp+14]
    mov ah, byte ptr [esp+18]
    mov bh, byte ptr [esp+22]
wgtmapA_2:
    sub dword ptr [esp+24], 16
    js wgtmapA_19
    mov bl, byte ptr [esi+eax]
    or bl, bl
    je wgtmapA_3
    mov dl, byte ptr _DepthQTable[ebx]
    mov byte ptr [edi], dl
wgtmapA_3:
    add ecx, dword ptr [esp]
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    or bl, bl
    je wgtmapA_4
    mov dl, byte ptr _DepthQTable[ebx]
    mov byte ptr [edi+1], dl
wgtmapA_4:
    add ecx, dword ptr [esp]
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    or bl, bl
    je wgtmapA_5
    mov dl, byte ptr _DepthQTable[ebx]
    mov byte ptr [edi+2], dl
wgtmapA_5:
    add ecx, dword ptr [esp]
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    or bl, bl
    je wgtmapA_6
    mov dl, byte ptr _DepthQTable[ebx]
    mov byte ptr [edi+3], dl
wgtmapA_6:
    add ecx, dword ptr [esp]
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    or bl, bl
    je wgtmapA_7
    mov dl, byte ptr _DepthQTable[ebx]
    mov byte ptr [edi+4], dl
wgtmapA_7:
    add ecx, dword ptr [esp]
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    or bl, bl
    je wgtmapA_8
    mov dl, byte ptr _DepthQTable[ebx]
    mov byte ptr [edi+5], dl
wgtmapA_8:
    add ecx, dword ptr [esp]
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    or bl, bl
    je wgtmapA_9
    mov dl, byte ptr _DepthQTable[ebx]
    mov byte ptr [edi+6], dl
wgtmapA_9:
    add ecx, dword ptr [esp]
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    or bl, bl
    je wgtmapA_10
    mov dl, byte ptr _DepthQTable[ebx]
    mov byte ptr [edi+7], dl
wgtmapA_10:
    add ecx, dword ptr [esp]
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    or bl, bl
    je wgtmapA_11
    mov dl, byte ptr _DepthQTable[ebx]
    mov byte ptr [edi+8], dl
wgtmapA_11:
    add ecx, dword ptr [esp]
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    or bl, bl
    je wgtmapA_12
    mov dl, byte ptr _DepthQTable[ebx]
    mov byte ptr [edi+9], dl
wgtmapA_12:
    add ecx, dword ptr [esp]
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    or bl, bl
    je wgtmapA_13
    mov dl, byte ptr _DepthQTable[ebx]
    mov byte ptr [edi+10], dl
wgtmapA_13:
    add ecx, dword ptr [esp]
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    or bl, bl
    je wgtmapA_14
    mov dl, byte ptr _DepthQTable[ebx]
    mov byte ptr [edi+11], dl
wgtmapA_14:
    add ecx, dword ptr [esp]
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    or bl, bl
    je wgtmapA_15
    mov dl, byte ptr _DepthQTable[ebx]
    mov byte ptr [edi+12], dl
wgtmapA_15:
    add ecx, dword ptr [esp]
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    or bl, bl
    je wgtmapA_16
    mov dl, byte ptr _DepthQTable[ebx]
    mov byte ptr [edi+13], dl
wgtmapA_16:
    add ecx, dword ptr [esp]
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    or bl, bl
    je wgtmapA_17
    mov dl, byte ptr _DepthQTable[ebx]
    mov byte ptr [edi+14], dl
wgtmapA_17:
    add ecx, dword ptr [esp]
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    mov bl, byte ptr [esi+eax]
    or bl, bl
    je wgtmapA_18
    mov dl, byte ptr _DepthQTable[ebx]
    mov byte ptr [edi+15], dl
wgtmapA_18:
    add ecx, dword ptr [esp]
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    add edi, 16
    jmp wgtmapA_2
wgtmapA_19:
    add dword ptr [esp+24], 16
    jle wgtmapA_22
wgtmapA_20:
    mov bl, byte ptr [esi+eax]
    or bl, bl
    je wgtmapA_21
    mov dl, byte ptr _DepthQTable[ebx]
    mov byte ptr [edi], dl
wgtmapA_21:
    add ecx, dword ptr [esp]
    adc al, cl
    add edx, dword ptr [esp+4]
    adc ah, ch
    add ebp, dword ptr [esp+8]
    adc bh, dh
    inc edi
    dec dword ptr [esp+24]
    jne wgtmapA_20
wgtmapA_22:
    mov eax, _phd_scrwidth
    add dword ptr [esp+28], 32
    add dword ptr [esp+40], eax
    dec dword ptr [esp+32]
    jne wgtmapA_1
wgtmapA_23:
    add esp, 44
    popa
    ret
@wgtmapA@12 endp

    end
