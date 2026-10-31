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

    end
