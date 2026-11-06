    .386
    .model flat

    extern _T_IsExtendedNonCyrillicChar:proc
    extern _T_IsNonCyrillicChar:proc
    extern _T_GetGlyphIndex:proc
    extern _T_GetGlyphSpacing:proc
    extern _GetTextScaleH:proc
    extern _GetTextScaleV:proc

LETTER_SPACING equ 16
WORD_SPACING equ 18
SCALE_H equ 46
SCALE_V equ 50
STRING_POINTER equ 54

    .code

_T_GetTextWidth proc public
    push esi
    push edi
    mov edi, dword ptr [esp+0ch]
    mov eax, dword ptr [edi+SCALE_H]
    push eax
    call _GetTextScaleH
    mov ecx, dword ptr [edi+SCALE_V]
    add esp, 04h
    mov esi, eax
    push ecx
    call _GetTextScaleV
    mov edx, dword ptr [edi+STRING_POINTER]
    add esp, 04h
    xor eax, eax
    mov cl, byte ptr [edx]
    test cl, cl
    je near ptr width_done
    push ebx
width_letter:
    and ecx, 0ffh
    inc edx
    call _T_IsExtendedNonCyrillicChar
    nop
    jb near ptr width_next
    cmp ecx, 0ah
    jbe short width_controls
    cmp ecx, 020h
    jb near ptr width_next
width_controls:
    cmp ecx, 00h
    je near ptr width_next
    cmp ecx, 00h
    je short width_control_end
    cmp ecx, 00h
width_control_end:
    je near ptr width_next
    nop
    nop
    cmp ecx, 020h
    jne short width_pickup
    movsx ecx, word ptr [edi+WORD_SPACING]
    cmp esi, 010000h
    je short width_add
    jmp short width_scale
width_pickup:
    cmp ecx, 07fh
    jb short width_glyph
    call _T_IsNonCyrillicChar
    jae short width_glyph
    cmp esi, 010000h
    jne short width_scaled_pickup
    add eax, 010h
    jmp short width_next
width_scaled_pickup:
    mov ecx, esi
    shl ecx, 04h
    jmp short width_shift
width_glyph:
    cmp ecx, 0bh
    jae short width_small_glyph
    add ecx, 051h
    jmp short width_digit
width_small_glyph:
    cmp ecx, 010h
    jae short width_remap_glyph
    add ecx, 05bh
    jmp short width_digit
width_remap_glyph:
    call _T_GetGlyphIndex
    nop
    nop
width_digit:
    cmp ecx, 030h
    jb short width_spacing
    cmp ecx, 039h
    ja short width_spacing
    lea ecx, [esi+esi*2]
    shl ecx, 02h
    jmp short width_shift
width_spacing:
    call _T_GetGlyphSpacing
    nop
    nop
    movsx ebx, word ptr [edi+LETTER_SPACING]
    cmp esi, 010000h
    jne short width_scaled_spacing
    add ebx, eax
    lea eax, [ebx+ecx*1]
    jmp short width_next
width_scaled_spacing:
    add ecx, ebx
width_scale:
    imul ecx, esi
width_shift:
    shr ecx, 010h
width_add:
    add eax, ecx
width_next:
    mov cl, byte ptr [edx]
    test cl, cl
    jne near ptr width_letter
    pop ebx
width_done:
    mov dx, word ptr [edi+LETTER_SPACING]
    pop edi
    sub eax, edx
    pop esi
    and eax, 0fffeh
    ret
_T_GetTextWidth endp

    end
