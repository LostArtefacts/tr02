    .386
    .model flat

    extern _WinSndFreeAllSamples:proc
    extern _DSound:dword

    .code

_WinSndFinish proc public
    call _WinSndFreeAllSamples
    jmp near ptr release_sound
    nop
    nop
    nop
    nop
    nop
    nop
release_sound:
    mov eax, _DSound
    test eax, eax
    je short sound_done
    mov ecx, [eax]
    push eax
    call dword ptr [ecx+8]
    mov _DSound, 0
sound_done:
    ret
_WinSndFinish endp

    end
