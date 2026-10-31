    .486
    .model flat

    extern _phd_mxptr:dword

    .code

_phd_PushMatrix proc public
    push esi
    push edi
    push ecx
    cld
    mov esi, _phd_mxptr
    mov ecx, 12
    lea edi, [esi + ecx * 4]
    rep movsd
    mov _phd_mxptr, esi
    pop ecx
    pop edi
    pop esi
    ret
_phd_PushMatrix endp

_phd_PushUnitMatrix proc public
    add _phd_mxptr, 48
    push edi
    push eax
    mov edi, _phd_mxptr
    mov eax, 16384
    mov [edi], eax
    mov [edi + 20], eax
    mov [edi + 40], eax
    xor eax, eax
    mov [edi + 4], eax
    mov [edi + 8], eax
    mov [edi + 16], eax
    mov [edi + 24], eax
    mov [edi + 32], eax
    mov [edi + 36], eax
    pop eax
    pop edi
    ret
_phd_PushUnitMatrix endp

    end
