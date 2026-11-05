    .486
    .model flat

    .code

@phd_atan@8 proc public
    push ebx
    push ecx
    push edx
    mov eax, ecx
    mov ebx, eax
    xor ecx, ecx
    or eax, edx
    je atan_done
    test ebx, ebx
    jns atan_xpos
    add ecx, 4
    neg ebx
atan_xpos:
    test edx, edx
    jns atan_ypos
    add ecx, 2
    neg edx
atan_ypos:
    xor eax, eax
    cmp edx, ebx
    jle atan_div
    inc ecx
    xchg ebx, edx
atan_div:
    shrd eax, edx, 21
    shr edx, 21
    idiv ebx
    mov ax, phdtantab[eax * 2]
    add eax, phdtan2[ecx * 4]
    jns atan_done
    neg eax
atan_done:
    pop edx
    pop ecx
    pop ebx
    ret
@phd_atan@8 endp

@phd_cos@4 proc public
    add ecx, 16384
@phd_cos@4 endp

@phd_sin@4 proc public
    mov eax, ecx
    and eax, 65535
    cmp eax, 32768
    jl phd_sin_half_turn
    sub eax, 32768
    call phd_sin_half_turn
    neg eax
    ret
@phd_sin@4 endp

phd_sin_half_turn proc public
    cmp eax, 16384
    jle sin_quarter
    neg eax
    add eax, 32768
sin_quarter:
    shr eax, 4
    mov ax, phdsintab[eax * 2]
    ret
phd_sin_half_turn endp

@phd_sqrt@4 proc public
    push ebx
    push ecx
    push edx
    mov ebx, 40000000h
    xor eax, eax
sqrt_loop:
    mov edx, eax
    add eax, ebx
    shr edx, 1
    cmp eax, ecx
    ja sqrt_skip
    sub ecx, eax
    mov eax, ebx
    or eax, edx
    shr ebx, 2
    jne sqrt_loop
    jmp sqrt_done
sqrt_skip:
    mov eax, edx
    shr ebx, 2
    jne sqrt_loop
sqrt_done:
    pop edx
    pop ecx
    pop ebx
    ret
@phd_sqrt@4 endp

    .data

    public phdtan2
phdtan2 label dword
    dd 000000000h, 0ffffc000h, 0ffff0001h, 00000c000h, 0ffff8000h, 000004000h, 000008000h, 0ffff4000h

    public phdtantab
phdtantab label word
    dw 00000h, 00005h, 0000ah, 0000fh, 00014h, 00019h, 0001fh, 00024h, 00029h, 0002eh, 00033h, 00038h, 0003dh, 00042h, 00047h, 0004ch
    dw 00051h, 00057h, 0005ch, 00061h, 00066h, 0006bh, 00070h, 00075h, 0007ah, 0007fh, 00084h, 0008ah, 0008fh, 00094h, 00099h, 0009eh
    dw 000a3h, 000a8h, 000adh, 000b2h, 000b7h, 000bch, 000c2h, 000c7h, 000cch, 000d1h, 000d6h, 000dbh, 000e0h, 000e5h, 000eah, 000efh
    dw 000f4h, 000fah, 000ffh, 00104h, 00109h, 0010eh, 00113h, 00118h, 0011dh, 00122h, 00127h, 0012ch, 00131h, 00137h, 0013ch, 00141h
    dw 00146h, 0014bh, 00150h, 00155h, 0015ah, 0015fh, 00164h, 00169h, 0016fh, 00174h, 00179h, 0017eh, 00183h, 00188h, 0018dh, 00192h
    dw 00197h, 0019ch, 001a1h, 001a6h, 001ach, 001b1h, 001b6h, 001bbh, 001c0h, 001c5h, 001cah, 001cfh, 001d4h, 001d9h, 001deh, 001e3h
    dw 001e9h, 001eeh, 001f3h, 001f8h, 001fdh, 00202h, 00207h, 0020ch, 00211h, 00216h, 0021bh, 00220h, 00226h, 0022bh, 00230h, 00235h
    dw 0023ah, 0023fh, 00244h, 00249h, 0024eh, 00253h, 00258h, 0025dh, 00262h, 00268h, 0026dh, 00272h, 00277h, 0027ch, 00281h, 00286h
    dw 0028bh, 00290h, 00295h, 0029ah, 0029fh, 002a4h, 002a9h, 002afh, 002b4h, 002b9h, 002beh, 002c3h, 002c8h, 002cdh, 002d2h, 002d7h
    dw 002dch, 002e1h, 002e6h, 002ebh, 002f0h, 002f6h, 002fbh, 00300h, 00305h, 0030ah, 0030fh, 00314h, 00319h, 0031eh, 00323h, 00328h
    dw 0032dh, 00332h, 00337h, 0033ch, 00341h, 00347h, 0034ch, 00351h, 00356h, 0035bh, 00360h, 00365h, 0036ah, 0036fh, 00374h, 00379h
    dw 0037eh, 00383h, 00388h, 0038dh, 00392h, 00397h, 0039ch, 003a2h, 003a7h, 003ach, 003b1h, 003b6h, 003bbh, 003c0h, 003c5h, 003cah
    dw 003cfh, 003d4h, 003d9h, 003deh, 003e3h, 003e8h, 003edh, 003f2h, 003f7h, 003fch, 00401h, 00407h, 0040ch, 00411h, 00416h, 0041bh
    dw 00420h, 00425h, 0042ah, 0042fh, 00434h, 00439h, 0043eh, 00443h, 00448h, 0044dh, 00452h, 00457h, 0045ch, 00461h, 00466h, 0046bh
    dw 00470h, 00475h, 0047ah, 0047fh, 00484h, 00489h, 0048eh, 00494h, 00499h, 0049eh, 004a3h, 004a8h, 004adh, 004b2h, 004b7h, 004bch
    dw 004c1h, 004c6h, 004cbh, 004d0h, 004d5h, 004dah, 004dfh, 004e4h, 004e9h, 004eeh, 004f3h, 004f8h, 004fdh, 00502h, 00507h, 0050ch
    dw 00511h, 00516h, 0051bh, 00520h, 00525h, 0052ah, 0052fh, 00534h, 00539h, 0053eh, 00543h, 00548h, 0054dh, 00552h, 00557h, 0055ch
    dw 00561h, 00566h, 0056bh, 00570h, 00575h, 0057ah, 0057fh, 00584h, 00589h, 0058eh, 00593h, 00598h, 0059dh, 005a2h, 005a7h, 005ach
    dw 005b1h, 005b6h, 005bbh, 005c0h, 005c5h, 005cah, 005cfh, 005d4h, 005d9h, 005deh, 005e3h, 005e8h, 005edh, 005f2h, 005f7h, 005fch
    dw 00601h, 00606h, 0060bh, 00610h, 00615h, 0061ah, 0061fh, 00624h, 00629h, 0062eh, 00633h, 00638h, 0063dh, 00642h, 00647h, 0064ch
    dw 00651h, 00656h, 0065bh, 00660h, 00665h, 0066ah, 0066eh, 00673h, 00678h, 0067dh, 00682h, 00687h, 0068ch, 00691h, 00696h, 0069bh
    dw 006a0h, 006a5h, 006aah, 006afh, 006b4h, 006b9h, 006beh, 006c3h, 006c8h, 006cdh, 006d2h, 006d7h, 006dch, 006e1h, 006e5h, 006eah
    dw 006efh, 006f4h, 006f9h, 006feh, 00703h, 00708h, 0070dh, 00712h, 00717h, 0071ch, 00721h, 00726h, 0072bh, 00730h, 00735h, 00739h
    dw 0073eh, 00743h, 00748h, 0074dh, 00752h, 00757h, 0075ch, 00761h, 00766h, 0076bh, 00770h, 00775h, 0077ah, 0077eh, 00783h, 00788h
    dw 0078dh, 00792h, 00797h, 0079ch, 007a1h, 007a6h, 007abh, 007b0h, 007b5h, 007b9h, 007beh, 007c3h, 007c8h, 007cdh, 007d2h, 007d7h
    dw 007dch, 007e1h, 007e6h, 007ebh, 007efh, 007f4h, 007f9h, 007feh, 00803h, 00808h, 0080dh, 00812h, 00817h, 0081ch, 00820h, 00825h
    dw 0082ah, 0082fh, 00834h, 00839h, 0083eh, 00843h, 00848h, 0084ch, 00851h, 00856h, 0085bh, 00860h, 00865h, 0086ah, 0086fh, 00873h
    dw 00878h, 0087dh, 00882h, 00887h, 0088ch, 00891h, 00896h, 0089ah, 0089fh, 008a4h, 008a9h, 008aeh, 008b3h, 008b8h, 008bdh, 008c1h
    dw 008c6h, 008cbh, 008d0h, 008d5h, 008dah, 008dfh, 008e3h, 008e8h, 008edh, 008f2h, 008f7h, 008fch, 00901h, 00905h, 0090ah, 0090fh
    dw 00914h, 00919h, 0091eh, 00922h, 00927h, 0092ch, 00931h, 00936h, 0093bh, 0093fh, 00944h, 00949h, 0094eh, 00953h, 00958h, 0095ch
    dw 00961h, 00966h, 0096bh, 00970h, 00975h, 00979h, 0097eh, 00983h, 00988h, 0098dh, 00992h, 00996h, 0099bh, 009a0h, 009a5h, 009aah
    dw 009aeh, 009b3h, 009b8h, 009bdh, 009c2h, 009c6h, 009cbh, 009d0h, 009d5h, 009dah, 009deh, 009e3h, 009e8h, 009edh, 009f2h, 009f6h
    dw 009fbh, 00a00h, 00a05h, 00a0ah, 00a0eh, 00a13h, 00a18h, 00a1dh, 00a22h, 00a26h, 00a2bh, 00a30h, 00a35h, 00a39h, 00a3eh, 00a43h
    dw 00a48h, 00a4dh, 00a51h, 00a56h, 00a5bh, 00a60h, 00a64h, 00a69h, 00a6eh, 00a73h, 00a77h, 00a7ch, 00a81h, 00a86h, 00a8bh, 00a8fh
    dw 00a94h, 00a99h, 00a9eh, 00aa2h, 00aa7h, 00aach, 00ab1h, 00ab5h, 00abah, 00abfh, 00ac4h, 00ac8h, 00acdh, 00ad2h, 00ad7h, 00adbh
    dw 00ae0h, 00ae5h, 00ae9h, 00aeeh, 00af3h, 00af8h, 00afch, 00b01h, 00b06h, 00b0bh, 00b0fh, 00b14h, 00b19h, 00b1eh, 00b22h, 00b27h
    dw 00b2ch, 00b30h, 00b35h, 00b3ah, 00b3fh, 00b43h, 00b48h, 00b4dh, 00b51h, 00b56h, 00b5bh, 00b60h, 00b64h, 00b69h, 00b6eh, 00b72h
    dw 00b77h, 00b7ch, 00b80h, 00b85h, 00b8ah, 00b8fh, 00b93h, 00b98h, 00b9dh, 00ba1h, 00ba6h, 00babh, 00bafh, 00bb4h, 00bb9h, 00bbdh
    dw 00bc2h, 00bc7h, 00bcbh, 00bd0h, 00bd5h, 00bd9h, 00bdeh, 00be3h, 00be7h, 00bech, 00bf1h, 00bf5h, 00bfah, 00bffh, 00c03h, 00c08h
    dw 00c0dh, 00c11h, 00c16h, 00c1bh, 00c1fh, 00c24h, 00c29h, 00c2dh, 00c32h, 00c37h, 00c3bh, 00c40h, 00c45h, 00c49h, 00c4eh, 00c53h
    dw 00c57h, 00c5ch, 00c60h, 00c65h, 00c6ah, 00c6eh, 00c73h, 00c78h, 00c7ch, 00c81h, 00c86h, 00c8ah, 00c8fh, 00c93h, 00c98h, 00c9dh
    dw 00ca1h, 00ca6h, 00cabh, 00cafh, 00cb4h, 00cb8h, 00cbdh, 00cc2h, 00cc6h, 00ccbh, 00ccfh, 00cd4h, 00cd9h, 00cddh, 00ce2h, 00ce6h
    dw 00cebh, 00cf0h, 00cf4h, 00cf9h, 00cfdh, 00d02h, 00d07h, 00d0bh, 00d10h, 00d14h, 00d19h, 00d1eh, 00d22h, 00d27h, 00d2bh, 00d30h
    dw 00d34h, 00d39h, 00d3eh, 00d42h, 00d47h, 00d4bh, 00d50h, 00d54h, 00d59h, 00d5eh, 00d62h, 00d67h, 00d6bh, 00d70h, 00d74h, 00d79h
    dw 00d7dh, 00d82h, 00d87h, 00d8bh, 00d90h, 00d94h, 00d99h, 00d9dh, 00da2h, 00da6h, 00dabh, 00dafh, 00db4h, 00db9h, 00dbdh, 00dc2h
    dw 00dc6h, 00dcbh, 00dcfh, 00dd4h, 00dd8h, 00dddh, 00de1h, 00de6h, 00deah, 00defh, 00df3h, 00df8h, 00dfch, 00e01h, 00e05h, 00e0ah
    dw 00e0fh, 00e13h, 00e18h, 00e1ch, 00e21h, 00e25h, 00e2ah, 00e2eh, 00e33h, 00e37h, 00e3ch, 00e40h, 00e45h, 00e49h, 00e4eh, 00e52h
    dw 00e56h, 00e5bh, 00e5fh, 00e64h, 00e68h, 00e6dh, 00e71h, 00e76h, 00e7ah, 00e7fh, 00e83h, 00e88h, 00e8ch, 00e91h, 00e95h, 00e9ah
    dw 00e9eh, 00ea3h, 00ea7h, 00each, 00eb0h, 00eb4h, 00eb9h, 00ebdh, 00ec2h, 00ec6h, 00ecbh, 00ecfh, 00ed4h, 00ed8h, 00edch, 00ee1h
    dw 00ee5h, 00eeah, 00eeeh, 00ef3h, 00ef7h, 00efch, 00f00h, 00f04h, 00f09h, 00f0dh, 00f12h, 00f16h, 00f1bh, 00f1fh, 00f23h, 00f28h
    dw 00f2ch, 00f31h, 00f35h, 00f3ah, 00f3eh, 00f42h, 00f47h, 00f4bh, 00f50h, 00f54h, 00f58h, 00f5dh, 00f61h, 00f66h, 00f6ah, 00f6eh
    dw 00f73h, 00f77h, 00f7ch, 00f80h, 00f84h, 00f89h, 00f8dh, 00f91h, 00f96h, 00f9ah, 00f9fh, 00fa3h, 00fa7h, 00fach, 00fb0h, 00fb5h
    dw 00fb9h, 00fbdh, 00fc2h, 00fc6h, 00fcah, 00fcfh, 00fd3h, 00fd7h, 00fdch, 00fe0h, 00fe5h, 00fe9h, 00fedh, 00ff2h, 00ff6h, 00ffah
    dw 00fffh, 01003h, 01007h, 0100ch, 01010h, 01014h, 01019h, 0101dh, 01021h, 01026h, 0102ah, 0102eh, 01033h, 01037h, 0103bh, 01040h
    dw 01044h, 01048h, 0104dh, 01051h, 01055h, 0105ah, 0105eh, 01062h, 01067h, 0106bh, 0106fh, 01073h, 01078h, 0107ch, 01080h, 01085h
    dw 01089h, 0108dh, 01092h, 01096h, 0109ah, 0109eh, 010a3h, 010a7h, 010abh, 010b0h, 010b4h, 010b8h, 010bch, 010c1h, 010c5h, 010c9h
    dw 010ceh, 010d2h, 010d6h, 010dah, 010dfh, 010e3h, 010e7h, 010ebh, 010f0h, 010f4h, 010f8h, 010fdh, 01101h, 01105h, 01109h, 0110eh
    dw 01112h, 01116h, 0111ah, 0111fh, 01123h, 01127h, 0112bh, 01130h, 01134h, 01138h, 0113ch, 01140h, 01145h, 01149h, 0114dh, 01151h
    dw 01156h, 0115ah, 0115eh, 01162h, 01166h, 0116bh, 0116fh, 01173h, 01177h, 0117ch, 01180h, 01184h, 01188h, 0118ch, 01191h, 01195h
    dw 01199h, 0119dh, 011a1h, 011a6h, 011aah, 011aeh, 011b2h, 011b6h, 011bbh, 011bfh, 011c3h, 011c7h, 011cbh, 011cfh, 011d4h, 011d8h
    dw 011dch, 011e0h, 011e4h, 011e9h, 011edh, 011f1h, 011f5h, 011f9h, 011fdh, 01202h, 01206h, 0120ah, 0120eh, 01212h, 01216h, 0121ah
    dw 0121fh, 01223h, 01227h, 0122bh, 0122fh, 01233h, 01237h, 0123ch, 01240h, 01244h, 01248h, 0124ch, 01250h, 01254h, 01259h, 0125dh
    dw 01261h, 01265h, 01269h, 0126dh, 01271h, 01275h, 0127ah, 0127eh, 01282h, 01286h, 0128ah, 0128eh, 01292h, 01296h, 0129ah, 0129fh
    dw 012a3h, 012a7h, 012abh, 012afh, 012b3h, 012b7h, 012bbh, 012bfh, 012c3h, 012c7h, 012cch, 012d0h, 012d4h, 012d8h, 012dch, 012e0h
    dw 012e4h, 012e8h, 012ech, 012f0h, 012f4h, 012f8h, 012fch, 01301h, 01305h, 01309h, 0130dh, 01311h, 01315h, 01319h, 0131dh, 01321h
    dw 01325h, 01329h, 0132dh, 01331h, 01335h, 01339h, 0133dh, 01341h, 01345h, 01349h, 0134dh, 01351h, 01355h, 0135ah, 0135eh, 01362h
    dw 01366h, 0136ah, 0136eh, 01372h, 01376h, 0137ah, 0137eh, 01382h, 01386h, 0138ah, 0138eh, 01392h, 01396h, 0139ah, 0139eh, 013a2h
    dw 013a6h, 013aah, 013aeh, 013b2h, 013b6h, 013bah, 013beh, 013c2h, 013c6h, 013cah, 013ceh, 013d2h, 013d6h, 013dah, 013deh, 013e2h
    dw 013e6h, 013e9h, 013edh, 013f1h, 013f5h, 013f9h, 013fdh, 01401h, 01405h, 01409h, 0140dh, 01411h, 01415h, 01419h, 0141dh, 01421h
    dw 01425h, 01429h, 0142dh, 01431h, 01435h, 01439h, 0143dh, 01440h, 01444h, 01448h, 0144ch, 01450h, 01454h, 01458h, 0145ch, 01460h
    dw 01464h, 01468h, 0146ch, 01470h, 01473h, 01477h, 0147bh, 0147fh, 01483h, 01487h, 0148bh, 0148fh, 01493h, 01497h, 0149bh, 0149eh
    dw 014a2h, 014a6h, 014aah, 014aeh, 014b2h, 014b6h, 014bah, 014beh, 014c1h, 014c5h, 014c9h, 014cdh, 014d1h, 014d5h, 014d9h, 014ddh
    dw 014e0h, 014e4h, 014e8h, 014ech, 014f0h, 014f4h, 014f8h, 014fbh, 014ffh, 01503h, 01507h, 0150bh, 0150fh, 01513h, 01516h, 0151ah
    dw 0151eh, 01522h, 01526h, 0152ah, 0152dh, 01531h, 01535h, 01539h, 0153dh, 01541h, 01544h, 01548h, 0154ch, 01550h, 01554h, 01558h
    dw 0155bh, 0155fh, 01563h, 01567h, 0156bh, 0156eh, 01572h, 01576h, 0157ah, 0157eh, 01581h, 01585h, 01589h, 0158dh, 01591h, 01594h
    dw 01598h, 0159ch, 015a0h, 015a4h, 015a7h, 015abh, 015afh, 015b3h, 015b7h, 015bah, 015beh, 015c2h, 015c6h, 015c9h, 015cdh, 015d1h
    dw 015d5h, 015d8h, 015dch, 015e0h, 015e4h, 015e8h, 015ebh, 015efh, 015f3h, 015f7h, 015fah, 015feh, 01602h, 01606h, 01609h, 0160dh
    dw 01611h, 01614h, 01618h, 0161ch, 01620h, 01623h, 01627h, 0162bh, 0162fh, 01632h, 01636h, 0163ah, 0163eh, 01641h, 01645h, 01649h
    dw 0164ch, 01650h, 01654h, 01658h, 0165bh, 0165fh, 01663h, 01666h, 0166ah, 0166eh, 01671h, 01675h, 01679h, 0167dh, 01680h, 01684h
    dw 01688h, 0168bh, 0168fh, 01693h, 01696h, 0169ah, 0169eh, 016a1h, 016a5h, 016a9h, 016ach, 016b0h, 016b4h, 016b7h, 016bbh, 016bfh
    dw 016c2h, 016c6h, 016cah, 016cdh, 016d1h, 016d5h, 016d8h, 016dch, 016e0h, 016e3h, 016e7h, 016ebh, 016eeh, 016f2h, 016f6h, 016f9h
    dw 016fdh, 01700h, 01704h, 01708h, 0170bh, 0170fh, 01713h, 01716h, 0171ah, 0171dh, 01721h, 01725h, 01728h, 0172ch, 01730h, 01733h
    dw 01737h, 0173ah, 0173eh, 01742h, 01745h, 01749h, 0174ch, 01750h, 01754h, 01757h, 0175bh, 0175eh, 01762h, 01766h, 01769h, 0176dh
    dw 01770h, 01774h, 01778h, 0177bh, 0177fh, 01782h, 01786h, 01789h, 0178dh, 01791h, 01794h, 01798h, 0179bh, 0179fh, 017a2h, 017a6h
    dw 017aah, 017adh, 017b1h, 017b4h, 017b8h, 017bbh, 017bfh, 017c2h, 017c6h, 017c9h, 017cdh, 017d1h, 017d4h, 017d8h, 017dbh, 017dfh
    dw 017e2h, 017e6h, 017e9h, 017edh, 017f0h, 017f4h, 017f7h, 017fbh, 017feh, 01802h, 01806h, 01809h, 0180dh, 01810h, 01814h, 01817h
    dw 0181bh, 0181eh, 01822h, 01825h, 01829h, 0182ch, 01830h, 01833h, 01837h, 0183ah, 0183eh, 01841h, 01845h, 01848h, 0184ch, 0184fh
    dw 01853h, 01856h, 0185ah, 0185dh, 01860h, 01864h, 01867h, 0186bh, 0186eh, 01872h, 01875h, 01879h, 0187ch, 01880h, 01883h, 01887h
    dw 0188ah, 0188eh, 01891h, 01894h, 01898h, 0189bh, 0189fh, 018a2h, 018a6h, 018a9h, 018adh, 018b0h, 018b3h, 018b7h, 018bah, 018beh
    dw 018c1h, 018c5h, 018c8h, 018cch, 018cfh, 018d2h, 018d6h, 018d9h, 018ddh, 018e0h, 018e3h, 018e7h, 018eah, 018eeh, 018f1h, 018f5h
    dw 018f8h, 018fbh, 018ffh, 01902h, 01906h, 01909h, 0190ch, 01910h, 01913h, 01917h, 0191ah, 0191dh, 01921h, 01924h, 01928h, 0192bh
    dw 0192eh, 01932h, 01935h, 01938h, 0193ch, 0193fh, 01943h, 01946h, 01949h, 0194dh, 01950h, 01953h, 01957h, 0195ah, 0195dh, 01961h
    dw 01964h, 01968h, 0196bh, 0196eh, 01972h, 01975h, 01978h, 0197ch, 0197fh, 01982h, 01986h, 01989h, 0198ch, 01990h, 01993h, 01996h
    dw 0199ah, 0199dh, 019a0h, 019a4h, 019a7h, 019aah, 019aeh, 019b1h, 019b4h, 019b8h, 019bbh, 019beh, 019c2h, 019c5h, 019c8h, 019cch
    dw 019cfh, 019d2h, 019d5h, 019d9h, 019dch, 019dfh, 019e3h, 019e6h, 019e9h, 019edh, 019f0h, 019f3h, 019f6h, 019fah, 019fdh, 01a00h
    dw 01a04h, 01a07h, 01a0ah, 01a0dh, 01a11h, 01a14h, 01a17h, 01a1bh, 01a1eh, 01a21h, 01a24h, 01a28h, 01a2bh, 01a2eh, 01a31h, 01a35h
    dw 01a38h, 01a3bh, 01a3eh, 01a42h, 01a45h, 01a48h, 01a4bh, 01a4fh, 01a52h, 01a55h, 01a58h, 01a5ch, 01a5fh, 01a62h, 01a65h, 01a69h
    dw 01a6ch, 01a6fh, 01a72h, 01a76h, 01a79h, 01a7ch, 01a7fh, 01a83h, 01a86h, 01a89h, 01a8ch, 01a8fh, 01a93h, 01a96h, 01a99h, 01a9ch
    dw 01a9fh, 01aa3h, 01aa6h, 01aa9h, 01aach, 01ab0h, 01ab3h, 01ab6h, 01ab9h, 01abch, 01ac0h, 01ac3h, 01ac6h, 01ac9h, 01acch, 01acfh
    dw 01ad3h, 01ad6h, 01ad9h, 01adch, 01adfh, 01ae3h, 01ae6h, 01ae9h, 01aech, 01aefh, 01af2h, 01af6h, 01af9h, 01afch, 01affh, 01b02h
    dw 01b05h, 01b09h, 01b0ch, 01b0fh, 01b12h, 01b15h, 01b18h, 01b1ch, 01b1fh, 01b22h, 01b25h, 01b28h, 01b2bh, 01b2eh, 01b32h, 01b35h
    dw 01b38h, 01b3bh, 01b3eh, 01b41h, 01b44h, 01b48h, 01b4bh, 01b4eh, 01b51h, 01b54h, 01b57h, 01b5ah, 01b5dh, 01b61h, 01b64h, 01b67h
    dw 01b6ah, 01b6dh, 01b70h, 01b73h, 01b76h, 01b79h, 01b7dh, 01b80h, 01b83h, 01b86h, 01b89h, 01b8ch, 01b8fh, 01b92h, 01b95h, 01b98h
    dw 01b9ch, 01b9fh, 01ba2h, 01ba5h, 01ba8h, 01babh, 01baeh, 01bb1h, 01bb4h, 01bb7h, 01bbah, 01bbdh, 01bc1h, 01bc4h, 01bc7h, 01bcah
    dw 01bcdh, 01bd0h, 01bd3h, 01bd6h, 01bd9h, 01bdch, 01bdfh, 01be2h, 01be5h, 01be8h, 01bebh, 01beeh, 01bf2h, 01bf5h, 01bf8h, 01bfbh
    dw 01bfeh, 01c01h, 01c04h, 01c07h, 01c0ah, 01c0dh, 01c10h, 01c13h, 01c16h, 01c19h, 01c1ch, 01c1fh, 01c22h, 01c25h, 01c28h, 01c2bh
    dw 01c2eh, 01c31h, 01c34h, 01c37h, 01c3ah, 01c3dh, 01c40h, 01c43h, 01c46h, 01c49h, 01c4ch, 01c4fh, 01c52h, 01c55h, 01c58h, 01c5bh
    dw 01c5eh, 01c61h, 01c64h, 01c67h, 01c6ah, 01c6dh, 01c70h, 01c73h, 01c76h, 01c79h, 01c7ch, 01c7fh, 01c82h, 01c85h, 01c88h, 01c8bh
    dw 01c8eh, 01c91h, 01c94h, 01c97h, 01c9ah, 01c9dh, 01ca0h, 01ca3h, 01ca6h, 01ca9h, 01cach, 01cafh, 01cb2h, 01cb5h, 01cb8h, 01cbbh
    dw 01cbeh, 01cc1h, 01cc3h, 01cc6h, 01cc9h, 01ccch, 01ccfh, 01cd2h, 01cd5h, 01cd8h, 01cdbh, 01cdeh, 01ce1h, 01ce4h, 01ce7h, 01ceah
    dw 01cedh, 01cf0h, 01cf3h, 01cf5h, 01cf8h, 01cfbh, 01cfeh, 01d01h, 01d04h, 01d07h, 01d0ah, 01d0dh, 01d10h, 01d13h, 01d16h, 01d18h
    dw 01d1bh, 01d1eh, 01d21h, 01d24h, 01d27h, 01d2ah, 01d2dh, 01d30h, 01d33h, 01d35h, 01d38h, 01d3bh, 01d3eh, 01d41h, 01d44h, 01d47h
    dw 01d4ah, 01d4dh, 01d4fh, 01d52h, 01d55h, 01d58h, 01d5bh, 01d5eh, 01d61h, 01d64h, 01d66h, 01d69h, 01d6ch, 01d6fh, 01d72h, 01d75h
    dw 01d78h, 01d7bh, 01d7dh, 01d80h, 01d83h, 01d86h, 01d89h, 01d8ch, 01d8eh, 01d91h, 01d94h, 01d97h, 01d9ah, 01d9dh, 01da0h, 01da2h
    dw 01da5h, 01da8h, 01dabh, 01daeh, 01db1h, 01db3h, 01db6h, 01db9h, 01dbch, 01dbfh, 01dc2h, 01dc4h, 01dc7h, 01dcah, 01dcdh, 01dd0h
    dw 01dd3h, 01dd5h, 01dd8h, 01ddbh, 01ddeh, 01de1h, 01de3h, 01de6h, 01de9h, 01dech, 01defh, 01df1h, 01df4h, 01df7h, 01dfah, 01dfdh
    dw 01dffh, 01e02h, 01e05h, 01e08h, 01e0bh, 01e0dh, 01e10h, 01e13h, 01e16h, 01e19h, 01e1bh, 01e1eh, 01e21h, 01e24h, 01e26h, 01e29h
    dw 01e2ch, 01e2fh, 01e32h, 01e34h, 01e37h, 01e3ah, 01e3dh, 01e3fh, 01e42h, 01e45h, 01e48h, 01e4ah, 01e4dh, 01e50h, 01e53h, 01e55h
    dw 01e58h, 01e5bh, 01e5eh, 01e60h, 01e63h, 01e66h, 01e69h, 01e6bh, 01e6eh, 01e71h, 01e74h, 01e76h, 01e79h, 01e7ch, 01e7fh, 01e81h
    dw 01e84h, 01e87h, 01e8ah, 01e8ch, 01e8fh, 01e92h, 01e94h, 01e97h, 01e9ah, 01e9dh, 01e9fh, 01ea2h, 01ea5h, 01ea8h, 01eaah, 01eadh
    dw 01eb0h, 01eb2h, 01eb5h, 01eb8h, 01ebah, 01ebdh, 01ec0h, 01ec3h, 01ec5h, 01ec8h, 01ecbh, 01ecdh, 01ed0h, 01ed3h, 01ed5h, 01ed8h
    dw 01edbh, 01edeh, 01ee0h, 01ee3h, 01ee6h, 01ee8h, 01eebh, 01eeeh, 01ef0h, 01ef3h, 01ef6h, 01ef8h, 01efbh, 01efeh, 01f00h, 01f03h
    dw 01f06h, 01f08h, 01f0bh, 01f0eh, 01f10h, 01f13h, 01f16h, 01f18h, 01f1bh, 01f1eh, 01f20h, 01f23h, 01f26h, 01f28h, 01f2bh, 01f2eh
    dw 01f30h, 01f33h, 01f36h, 01f38h, 01f3bh, 01f3dh, 01f40h, 01f43h, 01f45h, 01f48h, 01f4bh, 01f4dh, 01f50h, 01f53h, 01f55h, 01f58h
    dw 01f5ah, 01f5dh, 01f60h, 01f62h, 01f65h, 01f68h, 01f6ah, 01f6dh, 01f6fh, 01f72h, 01f75h, 01f77h, 01f7ah, 01f7ch, 01f7fh, 01f82h
    dw 01f84h, 01f87h, 01f8ah, 01f8ch, 01f8fh, 01f91h, 01f94h, 01f97h, 01f99h, 01f9ch, 01f9eh, 01fa1h, 01fa4h, 01fa6h, 01fa9h, 01fabh
    dw 01faeh, 01fb0h, 01fb3h, 01fb6h, 01fb8h, 01fbbh, 01fbdh, 01fc0h, 01fc3h, 01fc5h, 01fc8h, 01fcah, 01fcdh, 01fcfh, 01fd2h, 01fd5h
    dw 01fd7h, 01fdah, 01fdch, 01fdfh, 01fe1h, 01fe4h, 01fe6h, 01fe9h, 01fech, 01feeh, 01ff1h, 01ff3h, 01ff6h, 01ff8h, 01ffbh, 01ffdh
    dw 02000h, 02000h

    public phdsintab
phdsintab label word
    dw 00000h, 00019h, 00032h, 0004bh, 00065h, 0007eh, 00097h, 000b0h, 000c9h, 000e2h, 000fbh, 00114h, 0012eh, 00147h, 00160h, 00179h
    dw 00192h, 001abh, 001c4h, 001ddh, 001f7h, 00210h, 00229h, 00242h, 0025bh, 00274h, 0028dh, 002a6h, 002c0h, 002d9h, 002f2h, 0030bh
    dw 00324h, 0033dh, 00356h, 0036fh, 00388h, 003a1h, 003bbh, 003d4h, 003edh, 00406h, 0041fh, 00438h, 00451h, 0046ah, 00483h, 0049ch
    dw 004b5h, 004ceh, 004e7h, 00500h, 0051ah, 00533h, 0054ch, 00565h, 0057eh, 00597h, 005b0h, 005c9h, 005e2h, 005fbh, 00614h, 0062dh
    dw 00646h, 0065fh, 00678h, 00691h, 006aah, 006c3h, 006dch, 006f5h, 0070eh, 00727h, 00740h, 00759h, 00772h, 0078bh, 007a4h, 007bdh
    dw 007d6h, 007efh, 00807h, 00820h, 00839h, 00852h, 0086bh, 00884h, 0089dh, 008b6h, 008cfh, 008e8h, 00901h, 00919h, 00932h, 0094bh
    dw 00964h, 0097dh, 00996h, 009afh, 009c7h, 009e0h, 009f9h, 00a12h, 00a2bh, 00a44h, 00a5ch, 00a75h, 00a8eh, 00aa7h, 00ac0h, 00ad8h
    dw 00af1h, 00b0ah, 00b23h, 00b3bh, 00b54h, 00b6dh, 00b85h, 00b9eh, 00bb7h, 00bd0h, 00be8h, 00c01h, 00c1ah, 00c32h, 00c4bh, 00c64h
    dw 00c7ch, 00c95h, 00caeh, 00cc6h, 00cdfh, 00cf8h, 00d10h, 00d29h, 00d41h, 00d5ah, 00d72h, 00d8bh, 00da4h, 00dbch, 00dd5h, 00dedh
    dw 00e06h, 00e1eh, 00e37h, 00e4fh, 00e68h, 00e80h, 00e99h, 00eb1h, 00ecah, 00ee2h, 00efbh, 00f13h, 00f2bh, 00f44h, 00f5ch, 00f75h
    dw 00f8dh, 00fa5h, 00fbeh, 00fd6h, 00feeh, 01007h, 0101fh, 01037h, 01050h, 01068h, 01080h, 01099h, 010b1h, 010c9h, 010e1h, 010fah
    dw 01112h, 0112ah, 01142h, 0115ah, 01173h, 0118bh, 011a3h, 011bbh, 011d3h, 011ebh, 01204h, 0121ch, 01234h, 0124ch, 01264h, 0127ch
    dw 01294h, 012ach, 012c4h, 012dch, 012f4h, 0130ch, 01324h, 0133ch, 01354h, 0136ch, 01384h, 0139ch, 013b4h, 013cch, 013e4h, 013fbh
    dw 01413h, 0142bh, 01443h, 0145bh, 01473h, 0148bh, 014a2h, 014bah, 014d2h, 014eah, 01501h, 01519h, 01531h, 01549h, 01560h, 01578h
    dw 01590h, 015a7h, 015bfh, 015d7h, 015eeh, 01606h, 0161dh, 01635h, 0164ch, 01664h, 0167ch, 01693h, 016abh, 016c2h, 016dah, 016f1h
    dw 01709h, 01720h, 01737h, 0174fh, 01766h, 0177eh, 01795h, 017ach, 017c4h, 017dbh, 017f2h, 0180ah, 01821h, 01838h, 0184fh, 01867h
    dw 0187eh, 01895h, 018ach, 018c3h, 018dbh, 018f2h, 01909h, 01920h, 01937h, 0194eh, 01965h, 0197ch, 01993h, 019aah, 019c1h, 019d8h
    dw 019efh, 01a06h, 01a1dh, 01a34h, 01a4bh, 01a62h, 01a79h, 01a90h, 01aa7h, 01abeh, 01ad4h, 01aebh, 01b02h, 01b19h, 01b30h, 01b46h
    dw 01b5dh, 01b74h, 01b8ah, 01ba1h, 01bb8h, 01bceh, 01be5h, 01bfch, 01c12h, 01c29h, 01c3fh, 01c56h, 01c6ch, 01c83h, 01c99h, 01cb0h
    dw 01cc6h, 01cddh, 01cf3h, 01d0ah, 01d20h, 01d36h, 01d4dh, 01d63h, 01d79h, 01d90h, 01da6h, 01dbch, 01dd3h, 01de9h, 01dffh, 01e15h
    dw 01e2bh, 01e42h, 01e58h, 01e6eh, 01e84h, 01e9ah, 01eb0h, 01ec6h, 01edch, 01ef2h, 01f08h, 01f1eh, 01f34h, 01f4ah, 01f60h, 01f76h
    dw 01f8ch, 01fa2h, 01fb7h, 01fcdh, 01fe3h, 01ff9h, 0200fh, 02024h, 0203ah, 02050h, 02065h, 0207bh, 02091h, 020a6h, 020bch, 020d1h
    dw 020e7h, 020fdh, 02112h, 02128h, 0213dh, 02153h, 02168h, 0217dh, 02193h, 021a8h, 021beh, 021d3h, 021e8h, 021feh, 02213h, 02228h
    dw 0223dh, 02253h, 02268h, 0227dh, 02292h, 022a7h, 022bch, 022d2h, 022e7h, 022fch, 02311h, 02326h, 0233bh, 02350h, 02365h, 0237ah
    dw 0238eh, 023a3h, 023b8h, 023cdh, 023e2h, 023f7h, 0240bh, 02420h, 02435h, 0244ah, 0245eh, 02473h, 02488h, 0249ch, 024b1h, 024c5h
    dw 024dah, 024efh, 02503h, 02518h, 0252ch, 02541h, 02555h, 02569h, 0257eh, 02592h, 025a6h, 025bbh, 025cfh, 025e3h, 025f8h, 0260ch
    dw 02620h, 02634h, 02648h, 0265ch, 02671h, 02685h, 02699h, 026adh, 026c1h, 026d5h, 026e9h, 026fdh, 02711h, 02724h, 02738h, 0274ch
    dw 02760h, 02774h, 02788h, 0279bh, 027afh, 027c3h, 027d6h, 027eah, 027feh, 02811h, 02825h, 02838h, 0284ch, 02860h, 02873h, 02886h
    dw 0289ah, 028adh, 028c1h, 028d4h, 028e7h, 028fbh, 0290eh, 02921h, 02935h, 02948h, 0295bh, 0296eh, 02981h, 02994h, 029a7h, 029bbh
    dw 029ceh, 029e1h, 029f4h, 02a07h, 02a1ah, 02a2ch, 02a3fh, 02a52h, 02a65h, 02a78h, 02a8bh, 02a9dh, 02ab0h, 02ac3h, 02ad6h, 02ae8h
    dw 02afbh, 02b0dh, 02b20h, 02b33h, 02b45h, 02b58h, 02b6ah, 02b7dh, 02b8fh, 02ba1h, 02bb4h, 02bc6h, 02bd8h, 02bebh, 02bfdh, 02c0fh
    dw 02c21h, 02c34h, 02c46h, 02c58h, 02c6ah, 02c7ch, 02c8eh, 02ca0h, 02cb2h, 02cc4h, 02cd6h, 02ce8h, 02cfah, 02d0ch, 02d1eh, 02d2fh
    dw 02d41h, 02d53h, 02d65h, 02d76h, 02d88h, 02d9ah, 02dabh, 02dbdh, 02dcfh, 02de0h, 02df2h, 02e03h, 02e15h, 02e26h, 02e37h, 02e49h
    dw 02e5ah, 02e6bh, 02e7dh, 02e8eh, 02e9fh, 02eb0h, 02ec2h, 02ed3h, 02ee4h, 02ef5h, 02f06h, 02f17h, 02f28h, 02f39h, 02f4ah, 02f5bh
    dw 02f6ch, 02f7dh, 02f8dh, 02f9eh, 02fafh, 02fc0h, 02fd0h, 02fe1h, 02ff2h, 03002h, 03013h, 03024h, 03034h, 03045h, 03055h, 03066h
    dw 03076h, 03087h, 03097h, 030a7h, 030b8h, 030c8h, 030d8h, 030e8h, 030f9h, 03109h, 03119h, 03129h, 03139h, 03149h, 03159h, 03169h
    dw 03179h, 03189h, 03199h, 031a9h, 031b9h, 031c8h, 031d8h, 031e8h, 031f8h, 03207h, 03217h, 03227h, 03236h, 03246h, 03255h, 03265h
    dw 03274h, 03284h, 03293h, 032a3h, 032b2h, 032c1h, 032d0h, 032e0h, 032efh, 032feh, 0330dh, 0331dh, 0332ch, 0333bh, 0334ah, 03359h
    dw 03368h, 03377h, 03386h, 03395h, 033a3h, 033b2h, 033c1h, 033d0h, 033dfh, 033edh, 033fch, 0340bh, 03419h, 03428h, 03436h, 03445h
    dw 03453h, 03462h, 03470h, 0347fh, 0348dh, 0349bh, 034aah, 034b8h, 034c6h, 034d4h, 034e2h, 034f1h, 034ffh, 0350dh, 0351bh, 03529h
    dw 03537h, 03545h, 03553h, 03561h, 0356eh, 0357ch, 0358ah, 03598h, 035a5h, 035b3h, 035c1h, 035ceh, 035dch, 035eah, 035f7h, 03605h
    dw 03612h, 03620h, 0362dh, 0363ah, 03648h, 03655h, 03662h, 0366fh, 0367dh, 0368ah, 03697h, 036a4h, 036b1h, 036beh, 036cbh, 036d8h
    dw 036e5h, 036f2h, 036ffh, 0370ch, 03718h, 03725h, 03732h, 0373fh, 0374bh, 03758h, 03765h, 03771h, 0377eh, 0378ah, 03797h, 037a3h
    dw 037b0h, 037bch, 037c8h, 037d5h, 037e1h, 037edh, 037f9h, 03805h, 03812h, 0381eh, 0382ah, 03836h, 03842h, 0384eh, 0385ah, 03866h
    dw 03871h, 0387dh, 03889h, 03895h, 038a1h, 038ach, 038b8h, 038c3h, 038cfh, 038dbh, 038e6h, 038f2h, 038fdh, 03909h, 03914h, 0391fh
    dw 0392bh, 03936h, 03941h, 0394ch, 03958h, 03963h, 0396eh, 03979h, 03984h, 0398fh, 0399ah, 039a5h, 039b0h, 039bbh, 039c5h, 039d0h
    dw 039dbh, 039e6h, 039f0h, 039fbh, 03a06h, 03a10h, 03a1bh, 03a25h, 03a30h, 03a3ah, 03a45h, 03a4fh, 03a59h, 03a64h, 03a6eh, 03a78h
    dw 03a82h, 03a8dh, 03a97h, 03aa1h, 03aabh, 03ab5h, 03abfh, 03ac9h, 03ad3h, 03addh, 03ae6h, 03af0h, 03afah, 03b04h, 03b0eh, 03b17h
    dw 03b21h, 03b2ah, 03b34h, 03b3eh, 03b47h, 03b50h, 03b5ah, 03b63h, 03b6dh, 03b76h, 03b7fh, 03b88h, 03b92h, 03b9bh, 03ba4h, 03badh
    dw 03bb6h, 03bbfh, 03bc8h, 03bd1h, 03bdah, 03be3h, 03bech, 03bf5h, 03bfdh, 03c06h, 03c0fh, 03c17h, 03c20h, 03c29h, 03c31h, 03c3ah
    dw 03c42h, 03c4bh, 03c53h, 03c5bh, 03c64h, 03c6ch, 03c74h, 03c7dh, 03c85h, 03c8dh, 03c95h, 03c9dh, 03ca5h, 03cadh, 03cb5h, 03cbdh
    dw 03cc5h, 03ccdh, 03cd5h, 03cddh, 03ce4h, 03cech, 03cf4h, 03cfbh, 03d03h, 03d0bh, 03d12h, 03d1ah, 03d21h, 03d28h, 03d30h, 03d37h
    dw 03d3fh, 03d46h, 03d4dh, 03d54h, 03d5bh, 03d63h, 03d6ah, 03d71h, 03d78h, 03d7fh, 03d86h, 03d8dh, 03d93h, 03d9ah, 03da1h, 03da8h
    dw 03dafh, 03db5h, 03dbch, 03dc2h, 03dc9h, 03dd0h, 03dd6h, 03dddh, 03de3h, 03de9h, 03df0h, 03df6h, 03dfch, 03e03h, 03e09h, 03e0fh
    dw 03e15h, 03e1bh, 03e21h, 03e27h, 03e2dh, 03e33h, 03e39h, 03e3fh, 03e45h, 03e4ah, 03e50h, 03e56h, 03e5ch, 03e61h, 03e67h, 03e6ch
    dw 03e72h, 03e77h, 03e7dh, 03e82h, 03e88h, 03e8dh, 03e92h, 03e98h, 03e9dh, 03ea2h, 03ea7h, 03each, 03eb1h, 03eb6h, 03ebbh, 03ec0h
    dw 03ec5h, 03ecah, 03ecfh, 03ed4h, 03ed8h, 03eddh, 03ee2h, 03ee7h, 03eebh, 03ef0h, 03ef4h, 03ef9h, 03efdh, 03f02h, 03f06h, 03f0ah
    dw 03f0fh, 03f13h, 03f17h, 03f1ch, 03f20h, 03f24h, 03f28h, 03f2ch, 03f30h, 03f34h, 03f38h, 03f3ch, 03f40h, 03f43h, 03f47h, 03f4bh
    dw 03f4fh, 03f52h, 03f56h, 03f5ah, 03f5dh, 03f61h, 03f64h, 03f68h, 03f6bh, 03f6eh, 03f72h, 03f75h, 03f78h, 03f7bh, 03f7fh, 03f82h
    dw 03f85h, 03f88h, 03f8bh, 03f8eh, 03f91h, 03f94h, 03f97h, 03f99h, 03f9ch, 03f9fh, 03fa2h, 03fa4h, 03fa7h, 03faah, 03fach, 03fafh
    dw 03fb1h, 03fb4h, 03fb6h, 03fb8h, 03fbbh, 03fbdh, 03fbfh, 03fc1h, 03fc4h, 03fc6h, 03fc8h, 03fcah, 03fcch, 03fceh, 03fd0h, 03fd2h
    dw 03fd4h, 03fd5h, 03fd7h, 03fd9h, 03fdbh, 03fdch, 03fdeh, 03fe0h, 03fe1h, 03fe3h, 03fe4h, 03fe6h, 03fe7h, 03fe8h, 03feah, 03febh
    dw 03fech, 03fedh, 03fefh, 03ff0h, 03ff1h, 03ff2h, 03ff3h, 03ff4h, 03ff5h, 03ff6h, 03ff7h, 03ff7h, 03ff8h, 03ff9h, 03ffah, 03ffah
    dw 03ffbh, 03ffch, 03ffch, 03ffdh, 03ffdh, 03ffeh, 03ffeh, 03ffeh, 03fffh, 03fffh, 03fffh, 04000h, 04000h, 04000h, 04000h, 04000h
    dw 04000h, 04000h

    end
