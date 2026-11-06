    .386
    .model flat

    extern _T_CyrillicASCIIMap:byte
    extern _T_CyrillicSpacing:byte
    extern _T_remapASCII:byte
    extern _T_textSpacing:byte

    .code

_T_IsExtendedNonCyrillicChar proc public
    cmp ecx, 81h
    ja short _T_IsNonCyrillicChar
    jmp short glyph_remapped
_T_IsExtendedNonCyrillicChar endp

_T_IsNonCyrillicChar proc public
    cmp ecx, 0a8h
    je short glyph_remapped
    cmp ecx, 0b8h
    je short glyph_remapped
    cmp ecx, 0c0h
    jb short glyph_standard
    cmp ecx, 0ffh
    ja short glyph_standard
glyph_remapped::
    clc
    ret
glyph_standard:
    stc
    ret
_T_IsNonCyrillicChar endp

_T_GetGlyphIndex proc public
    call _T_IsNonCyrillicChar
    jc short remap_standard
    sub ecx, 0a8h
    movzx ecx, byte ptr [_T_CyrillicASCIIMap+ecx]
    ret
remap_standard:
    movzx ecx, byte ptr [_T_remapASCII+ecx-32]
    ret
_T_GetGlyphIndex endp

_T_GetGlyphSpacing proc public
    cmp ecx, 6eh
    jb short spacing_standard
    sub ecx, 6eh
    movzx ecx, byte ptr [_T_CyrillicSpacing+ecx]
    ret
spacing_standard:
    movzx ecx, byte ptr [_T_textSpacing+ecx]
    ret
_T_GetGlyphSpacing endp

    end
