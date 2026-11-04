    .386
    .model flat

    extern _Var44b226:byte
    extern _Var44b27e:byte
    extern _T_remapASCII:byte
    extern _T_textSpacing:byte

    .code

_Func4558d7 proc public
    cmp ecx, 81h
    ja short _Func4558e1
    jmp short glyph_remapped
_Func4558d7 endp

_Func4558e1 proc public
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
_Func4558e1 endp

_Func455905 proc public
    call _Func4558e1
    jc short remap_standard
    sub ecx, 0a8h
    movzx ecx, byte ptr [_Var44b226+ecx]
    ret
remap_standard:
    movzx ecx, byte ptr [_T_remapASCII+ecx-32]
    ret
_Func455905 endp

_Func455922 proc public
    cmp ecx, 6eh
    jb short spacing_standard
    sub ecx, 6eh
    movzx ecx, byte ptr [_Var44b27e+ecx]
    ret
spacing_standard:
    movzx ecx, byte ptr [_T_textSpacing+ecx]
    ret
_Func455922 endp

    end
