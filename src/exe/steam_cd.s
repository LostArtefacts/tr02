    .386
    .model flat

    extern _CD_LoadAudioIndex:proc
    extern _GetRealTrack:proc
    extern _atol:proc
    extern _CD_CommandBuffer:dword
    extern _CD_AudioIndex:dword
    extern _CD_LoopTrack:dword
    extern _CD_LoopCounter:dword
    extern _Option_Music_Volume:word
    extern _cdtrack:word
    extern __imp__mciSendStringA@16:dword
    extern __imp__wsprintfA:dword

    .code
_CD_Init proc public
    sub esp, 20h
    push esi
    push edi
    push ecx
    push ebx
    call _CD_LoadAudioIndex
    test al, al
    je short init_done
    mov esi, __imp__mciSendStringA@16
    xor ebx, ebx
    push ebx
    push ebx
    push ebx
    push offset cd_open
    mov esi, __imp__mciSendStringA@16
    call esi
    test eax, eax
    jne short init_done
    push ebx
    push ebx
    push ebx
    push offset cd_time_format
    call esi
init_done:
    mov al, 1
    pop ebx
    pop ecx
    pop edi
    pop esi
    add esp, 20h
    ret
_CD_Init endp
cd_open db "open audio\cdaudio.mp3  type mpegvideo alias T2",0
cd_time_format db "set T2 time format ms",0
cd_close db "close T2",0

    rept 1
    nop
    endm
_CD_Cleanup proc public
    pushad
    mov esi, __imp__mciSendStringA@16
    xor ebx, ebx
    push ebx
    push ebx
    push ebx
    push offset cd_stop
    call esi
    push ebx
    push ebx
    push ebx
    push offset cd_close
    call esi
    popad
    ret
_CD_Cleanup endp
cd_position db "status T2 position",0
cd_stop db "stop T2",0

    rept 6
    nop
    endm
_S_CDLoop proc public
    pushad
    mov eax, _CD_LoopTrack
    test eax, eax
    je short loop_done
    mov eax, _CD_LoopCounter
    inc eax
    cmp eax, 150
    mov _CD_LoopCounter, eax
    jl short loop_done
    mov _CD_LoopCounter, 0
    push 0
    push 50
    mov edi, _CD_CommandBuffer
    push edi
    push offset cd_mode
    call dword ptr [__imp__mciSendStringA@16]
    test eax, eax
    jne short loop_done
    cmp dword ptr ss:[edi], 706f7473h
    jne short loop_done
    mov esi, _CD_LoopTrack
    lea edi, [esi+9]
    push edi
    push esi
    push offset cd_play
    mov edi, _CD_CommandBuffer
    push edi
    call dword ptr [__imp__wsprintfA]
    add esp, 16
    xor ecx, ecx
    push ecx
    push ecx
    push ecx
    push edi
    call dword ptr [__imp__mciSendStringA@16]
loop_done:
    popad
    ret
_S_CDLoop endp
cd_play db "play T2 from %s to %s",0
    nop
    nop
    nop
    nop
cd_mode db "status T2 mode",0

    rept 0
    nop
    endm
_S_CDPlay proc public
    pushad
    cmp _Option_Music_Volume, 0
    je short play_done
    mov eax, [esp+24h]
    mov _cdtrack, ax
    movsx eax, ax
    push eax
    call _GetRealTrack
    add esp, 4
    dec eax
    lea eax, [eax+eax*8]
    shl eax, 1
    mov esi, _CD_AudioIndex
    add esi, eax
    mov ebx, esi
    add esi, 9
    push esi
    push ebx
    push offset cd_play
    mov edi, _CD_CommandBuffer
    push edi
    call dword ptr [__imp__wsprintfA]
    add esp, 16
    xor ecx, ecx
    push ecx
    push ecx
    push ecx
    push edi
    call dword ptr [__imp__mciSendStringA@16]
    cmp dword ptr [esp+28h], 0
    je short play_done
    mov _CD_LoopTrack, ebx
    mov _CD_LoopCounter, 120
play_done:
    popad
    ret
_S_CDPlay endp

    rept 16
    nop
    endm
_S_CDStop proc public
    push ecx
    cmp _cdtrack, 0
    jle short stop_done
    xor ecx, ecx
    push ecx
    push ecx
    push ecx
    push offset cd_stop
    call dword ptr [__imp__mciSendStringA@16]
    mov _CD_LoopTrack, 0
    mov _cdtrack, 0
stop_done:
    pop ecx
    ret
_S_CDStop endp

    rept 16
    nop
    endm
_StartSyncedAudio proc public
    push esi
    push edi
    push ebx
    push ecx
    push edx
    mov eax, [esp+18h]
    push eax
    mov _cdtrack, ax
    call _GetRealTrack
    add esp, 4
    mov ebx, eax
    xor ecx, ecx
    push ecx
    push ecx
    push ecx
    push offset cd_time_format
    call dword ptr [__imp__mciSendStringA@16]
    test eax, eax
    jne short sync_failed
    dec ebx
    lea ebx, [ebx+ebx*8]
    shl ebx, 1
    mov esi, _CD_AudioIndex
    add esi, ebx
    mov ebx, esi
    add esi, 9
    push esi
    push ebx
    push offset cd_play
    mov edi, _CD_CommandBuffer
    push edi
    call dword ptr [__imp__wsprintfA]
    add esp, 16
    xor ecx, ecx
    push ecx
    push ecx
    push ecx
    push edi
    call dword ptr [__imp__mciSendStringA@16]
    neg eax
    sbb eax, eax
    inc eax
    jmp short sync_done
sync_failed:
    xor eax, eax
sync_done:
    pop edx
    pop ecx
    pop ebx
    pop edi
    pop esi
    ret
_StartSyncedAudio endp
cd_volume db "setaudio T2 volume to %u",0

    rept 4
    nop
    endm
_S_CDGetLoc proc public
    push 0
    push 50
    mov edi, _CD_CommandBuffer
    push edi
    push offset cd_position
    call dword ptr [__imp__mciSendStringA@16]
    test eax, eax
    jne short location_failed
    push edi
    call _atol
    add esp, 4
    mov esi, eax
    mov eax, dword ptr _cdtrack
    movzx eax, ax
    push eax
    call _GetRealTrack
    add esp, 4
    dec eax
    lea eax, [eax+eax*8]
    shl eax, 1
    mov edx, _CD_AudioIndex
    lea eax, [edx+eax]
    push eax
    call _atol
    add esp, 4
    sub esi, eax
    lea eax, [esi+esi*2]
    mov edx, 0cccccccdh
    mul edx
    shr edx, 5
    mov eax, edx
    ret
location_failed:
    xor eax, eax
    ret
_S_CDGetLoc endp

    rept 13
    nop
    endm

_S_CDVolume proc public
    pushad
volume_argument:
    db 08bh, 07ch, (3 shl 6) or (4 shl 3) or 4, 24h
    test edi, edi
    je short volume_format
    sub edi, 5
    shl edi, 2
volume_format:
    push edi
    push offset cd_volume
    mov eax, _CD_CommandBuffer
    push eax
    call dword ptr [__imp__wsprintfA]
    add esp, 12
    xor eax, eax
    push eax
    push eax
    push eax
    mov eax, _CD_CommandBuffer
    push eax
    call dword ptr [__imp__mciSendStringA@16]
    popad
    ret
_S_CDVolume endp

    end
