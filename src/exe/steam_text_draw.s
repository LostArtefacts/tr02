    .386
    .model flat

    extern _Func4558e1:proc
    extern _Func455905:proc
    extern _Func455922:proc
    extern _GetRenderHeight:proc
    extern _GetRenderWidth:proc
    extern _GetTextScaleH:proc
    extern _GetTextScaleV:proc
    extern _S_DrawPickup:proc
    extern _S_DrawScreenFBox:proc
    extern _S_DrawScreenSprite2d:proc
    extern _T_GetTextWidth:proc
    extern _camera:byte
    extern _draw_border:proc
    extern _objects:byte

TEXT_FLAGS equ 4
BACKGROUND_FLAGS equ 6
X_POSITION equ 10
Y_POSITION equ 12
Z_POSITION equ 14
LETTER_SPACING equ 16
WORD_SPACING equ 18
FLASH_RATE equ 20
FLASH_COUNT equ 22
BACKGROUND_COLOUR equ 24
BACKGROUND_GOURAUD equ 26
BACKGROUND_WIDTH equ 36
BACKGROUND_HEIGHT equ 38
BACKGROUND_X equ 40
BACKGROUND_Y equ 42
BACKGROUND_Z equ 44
SCALE_H equ 46
SCALE_V equ 50
STRING_POINTER equ 54

    .code

_T_DrawThisText proc public
    sub esp, 024h
    push ebx
    mov ebx, dword ptr [esp+02ch]
    push ebp
    push esi
    mov eax, dword ptr [ebx+SCALE_H]
    push edi
    xor edi, edi
    push eax
    mov DWORD PTR [esp+024h], edi
    mov DWORD PTR [esp+01ch], edi
    call _GetTextScaleH
    mov ecx, dword ptr [ebx+SCALE_V]
    add esp, 04h
    mov DWORD PTR [esp+010h], eax
    push ecx
    call _GetTextScaleV
    lea edx, [eax+eax*4]
    mov DWORD PTR [esp+030h], eax
    add esp, 04h
    lea esi, [eax+edx*2]
    mov al, byte ptr [ebx]
    shr esi, 010h
    test al, 02h
    je short draw_position
    mov ax, word ptr [_camera+30h]
    mov cx, word ptr [ebx+FLASH_RATE]
    sub WORD PTR [ebx+FLASH_COUNT], ax
    mov ax, word ptr [ebx+FLASH_COUNT]
    movsx edx, cx
    movsx ebp, ax
    neg edx
    cmp ebp, edx
    jg short draw_flash_hidden
    mov WORD PTR [ebx+FLASH_COUNT], cx
    jmp short draw_position
draw_flash_hidden:
    cmp ax, di
    jl near ptr draw_done
draw_position:
    movsx ecx, word ptr [ebx+Y_POSITION]
    movsx edx, word ptr [ebx+Z_POSITION]
    mov eax, dword ptr [ebx+STRING_POINTER]
    push ebx
    movsx edi, word ptr [ebx+X_POSITION]
    mov DWORD PTR [esp+020h], eax
    mov DWORD PTR [esp+03ch], ecx
    mov DWORD PTR [esp+028h], edx
    call _T_GetTextWidth
    mov ebp, eax
    mov eax, dword ptr [ebx]
    add esp, 04h
    mov DWORD PTR [esp+028h], ebp
    test al, 010h
    je short draw_right_align
    call _GetRenderWidth
    sub eax, ebp
    cdq
    sub eax, edx
    sar eax, 1
    jmp short draw_add_x
draw_right_align:
    test al, 080h
    je short draw_vertical_align
    call _GetRenderWidth
    sub eax, ebp
draw_add_x:
    add edi, eax
draw_vertical_align:
    mov eax, dword ptr [ebx]
    test al, 020h
    je short draw_bottom_align
    call _GetRenderHeight
    cdq
    sub eax, edx
    sar eax, 1
    jmp short draw_add_y
draw_bottom_align:
    test ah, 01h
    je short draw_background_origin
    call _GetRenderHeight
draw_add_y:
    add DWORD PTR [esp+038h], eax
draw_background_origin:
    mov eax, dword ptr [esp+010h]
    movsx ebp, word ptr [ebx+BACKGROUND_X]
    movsx edx, word ptr [ebx+BACKGROUND_Y]
    lea ecx, [eax+eax*1]
    mov eax, dword ptr [esp+02ch]
    shr ecx, 010h
    sub ebp, ecx
    lea ecx, [eax*4+00h]
    mov eax, dword ptr [esp+01ch]
    add ebp, edi
    shr ecx, 010h
    mov al, byte ptr [eax]
    sub edx, ecx
    mov ecx, dword ptr [esp+038h]
    mov DWORD PTR [esp+030h], ebp
    add edx, ecx
    sub edx, esi
    test al, al
    mov DWORD PTR [esp+014h], edx
    je near ptr draw_background
draw_letter:
    mov ecx, dword ptr [esp+01ch]
    and eax, 0ffh
    mov esi, eax
    inc ecx
    cmp esi, 012h
    mov DWORD PTR [esp+01ch], ecx
    jbe short draw_space
    cmp esi, 020h
    jb near ptr draw_next
draw_space:
    cmp esi, 020h
    jne short draw_pickup
    movsx ecx, word ptr [ebx+WORD_SPACING]
    imul ecx, dword ptr [esp+010h]
    jmp near ptr draw_advance_x
draw_pickup:
    cmp esi, 07fh
    jb short draw_glyph
    cmp esi, 081h
    ja short draw_extended_glyph
    mov ecx, dword ptr [esp+038h]
    lea edx, [esi+esi*2]
    shl edx, 04h
    push 00h
    push 01000h
    mov ax, word ptr [edx+_objects+(190-127)*48+2]
    lea edx, [edi+0ah]
    push eax
    push 01be8h
    push ecx
    push edx
    call _S_DrawPickup
    mov eax, dword ptr [esp+028h]
    add esp, 018h
    shl eax, 04h
    shr eax, 010h
    add edi, eax
draw_skip:
    jmp near ptr draw_next
draw_extended_glyph:
    push ecx
    mov ecx, esi
    call _Func4558e1
    pop ecx
    jb short draw_skip
draw_glyph:
    cmp esi, 0bh
    jae short draw_small_glyph
    lea ebp, [esi+051h]
    jmp short draw_digit_left
draw_small_glyph:
    cmp esi, 012h
    lea ebp, [esi+05bh]
    jbe short draw_digit_left
    push ecx
    mov ecx, esi
    call _Func455905
    mov ebp, ecx
    pop ecx
draw_digit_left:
    cmp esi, 030h
    jb short draw_visible
    cmp esi, 039h
    ja short draw_visible
    mov ecx, ebp
    call _Func455922
    mov eax, 0ch
    sub eax, ecx
    cdq
    sub eax, edx
    sar eax, 1
    imul eax, dword ptr [esp+010h]
    shr eax, 010h
    add edi, eax
draw_visible:
    test edi, edi
    jle short draw_digit_right
    call _GetRenderWidth
    cmp edi, eax
    jge short draw_digit_right
    mov eax, dword ptr [esp+038h]
    test eax, eax
    jle short draw_digit_right
    call _GetRenderHeight
    cmp DWORD PTR [esp+038h], eax
    jge short draw_digit_right
    mov dx, word ptr [ebx+TEXT_FLAGS]
    mov ecx, dword ptr [esp+02ch]
    mov ax, bp
    push edx
    add ax, word ptr [_objects+255*48+2]
    mov edx, dword ptr [esp+014h]
    push 01000h
    push eax
    mov eax, dword ptr [esp+030h]
    push ecx
    mov ecx, dword ptr [esp+048h]
    push edx
    push eax
    push ecx
    push edi
    call _S_DrawScreenSprite2d
    add esp, 020h
draw_digit_right:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    cmp esi, 030h
    jb short draw_letter_spacing
    cmp esi, 039h
    ja short draw_letter_spacing
    mov ecx, ebp
    call _Func455922
    mov edx, ecx
    mov eax, 0ch
    mov ecx, eax
    sub eax, edx
    cdq
    sub eax, edx
    sar eax, 1
    sub ecx, eax
    imul ecx, dword ptr [esp+010h]
    jmp short draw_advance_x
draw_letter_spacing:
    mov eax, dword ptr [esp+010h]
    cmp eax, 010000h
    jne short draw_scaled_spacing
    movsx eax, word ptr [ebx+LETTER_SPACING]
    push ecx
    mov ecx, ebp
    call _Func455922
    mov edx, ecx
    pop ecx
    add eax, edi
    lea edi, [eax+edx*1]
    jmp short draw_next
draw_scaled_spacing:
    mov ecx, ebp
    call _Func455922
    movsx edx, word ptr [ebx+LETTER_SPACING]
    add ecx, edx
    imul ecx, eax
draw_advance_x:
    shr ecx, 010h
    add edi, ecx
draw_next:
    mov eax, dword ptr [esp+01ch]
    mov al, byte ptr [eax]
    test al, al
    jne near ptr draw_letter
    mov ebp, dword ptr [esp+030h]
    mov edx, dword ptr [esp+014h]
draw_background:
    mov eax, dword ptr [ebx]
    mov esi, eax
    and esi, 0200h
    jne short draw_background_size
    test ah, 04h
    je short draw_fill
draw_background_size:
    mov cx, word ptr [ebx+BACKGROUND_WIDTH]
    test cx, cx
    je short draw_background_height
    mov eax, dword ptr [esp+028h]
    cdq
    sub eax, edx
    sar eax, 1
    add ebp, eax
    movsx eax, cx
    mov DWORD PTR [esp+028h], eax
    cdq
    sub eax, edx
    mov edx, dword ptr [esp+014h]
    sar eax, 1
    neg eax
    add ebp, eax
draw_background_height:
    mov ecx, dword ptr [esp+028h]
    mov ax, word ptr [ebx+BACKGROUND_HEIGHT]
    add ecx, 04h
    test ax, ax
    mov DWORD PTR [esp+020h], ecx
    je short draw_scaled_height
    movsx eax, ax
    jmp short draw_save_height
draw_scaled_height:
    mov eax, dword ptr [esp+02ch]
    shl eax, 04h
    shr eax, 010h
draw_save_height:
    mov DWORD PTR [esp+018h], eax
draw_fill:
    test esi, esi
    je near ptr draw_outline_size
    mov ecx, dword ptr [ebx+BACKGROUND_GOURAUD]
    test ecx, ecx
    je short draw_flat_fill
    mov edi, dword ptr [esp+020h]
    mov esi, dword ptr [esp+018h]
    mov eax, edi
    cdq
    sub eax, edx
    sar eax, 1
    mov DWORD PTR [esp+038h], eax
    mov eax, esi
    cdq
    sub eax, edx
    mov dx, word ptr [ebx+BACKGROUND_FLAGS]
    sar eax, 1
    push edx
    mov edx, dword ptr [esp+028h]
    mov DWORD PTR [esp+03ch], eax
    push ecx
    movsx eax, word ptr [ebx+BACKGROUND_COLOUR]
    movsx ecx, word ptr [ebx+BACKGROUND_Z]
    push eax
    push esi
    lea eax, [ecx+edx*1+02h]
    mov ecx, dword ptr [esp+024h]
    push edi
    push eax
    push ecx
    push ebp
    call _S_DrawScreenFBox
    mov edx, dword ptr [esp+034h]
    add esp, 020h
    jmp short draw_outline
draw_flat_fill:
    mov ax, word ptr [ebx+BACKGROUND_FLAGS]
    mov esi, dword ptr [esp+018h]
    movsx ecx, word ptr [ebx+BACKGROUND_COLOUR]
    mov edi, dword ptr [esp+020h]
    push eax
    movsx eax, word ptr [ebx+BACKGROUND_Z]
    push 00h
    push ecx
    mov ecx, dword ptr [esp+030h]
    push esi
    push edi
    lea eax, [eax+ecx*1+02h]
    push eax
    push edx
    push ebp
    call _S_DrawScreenFBox
    mov edx, dword ptr [esp+034h]
    add esp, 020h
    jmp short draw_outline
draw_outline_size:
    mov edi, dword ptr [esp+020h]
    mov esi, dword ptr [esp+018h]
draw_outline:
    mov eax, dword ptr [ebx]
    test ah, 04h
    je short draw_done
    mov ecx, dword ptr [esp+024h]
    push esi
    push edi
    push ecx
    push edx
    push ebp
    call _draw_border
    add esp, 014h
draw_done:
    pop edi
    pop esi
    pop ebp
    pop ebx
    add esp, 024h
    ret
_T_DrawThisText endp

    end
