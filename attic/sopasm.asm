; Assembler support routines for Sopwith
; Reverse-engineered by Andrew Jenner
;
; Copyright (c) 1984-2000 David L Clark
; Copyright (c) 1999-2000 Andrew Jenner
;
; All rights reserved except as specified in the file license.txt.
; Distribution of this file without the license.txt file accompanying is
; prohibited.

PUBLIC _farmemmove,_writechar,_poscurs,_clearline,_init_timer,_farmemset
PUBLIC _restoreints,_setgmode,_getwordfromport,_cgafbar,_init_keyb

_farmemmove:
  PUSH BP
  MOV BP,SP
  PUSH DS
  PUSH ES
  PUSHF
  PUSH SI
  PUSH DI
  MOV SI,[BP+4]
  MOV BX,SI
  MOV CL,4
  SHR BX,CL
  ADD BX,[BP+6]
  MOV DI,[BP+8]
  MOV DX,DI
  MOV CL,4
  SHR DX,CL
  ADD DX,[BP+0a]
  MOV AX,0FH
  AND SI,AX
  AND DI,AX
  MOV CX,[BP+0c]
  MOV AX,1
  TEST AX,CX
  JNZ o8DD8
  TEST AX,SI
  JNZ o8DD8
  TEST AX,DI
  JNZ o8DD8
  XOR AX,AX
o8DD8:
  CMP BX,DX
  JB o8DE5
  JA o8DE2
  CMP SI,DI
  JB o8DE5
o8DE2:
  CLD
  JMP o8DF2
o8DE5:
  ADD SI,CX
  DEC SI
  ADD DI,CX
  DEC DI
  STD
  OR AX,AX
  JNZ o8DF2
  DEC SI
  DEC DI
o8DF2:
  MOV DS,BX
  MOV ES,DX
  OR AX,AX
  JZ o8DFE
  REP MOVSB ; Rep when cx >0 Mov [si] to es:[di]
  JMP o8E08
o8DFE:
  SHR CX,1
  CMP CX,2
  JNE o8E06
  CLI
o8E06:
  REP MOVSW ; Rep when cx >0 Mov [si] to es:[di]
o8E08:
  POP DI
  POP SI
  POPF
  POP ES
  POP DS
  POP BP
  RET

_farmemset:
  PUSH BP
  MOV BP,SP
  PUSH ES
  PUSH DI
  MOV DI,W[BP+4]
  MOV ES,W[BP+6]
  MOV CX,W[BP+8]
  MOV AL,B[BP+0a]
  MOV AH,AL
  MOV BX,CX
  SHR CX,1
  REP STOSW
  TEST BL,1
  JZ donememset
  STOSB
donememset:
  POP DI
  POP ES
  POP BP
  RET

_writechar:
  PUSH BP
  MOV BP,SP
  MOV AL,[BP+4]
  MOV BL,[_writecharcol]
  XOR BH,BH
  MOV AH,0e
  INT 010
  POP BP
  RET

_clearline:
  PUSH BP
  MOV AX,0A20
  MOV BX,0
  MOV CX,40
  INT 010
  POP BP
  RET

_poscurs:
  PUSH BP
  MOV BP,SP
  MOV DL,B[BP+4]
  MOV DH,B[BP+6]
  XOR BH,BH
  MOV AH,2
  INT 010
  POP BP
  RET

_setgmode:
  PUSH BP
  MOV BP,SP
  MOV AX,W[BP+4]
  INT 010
  POP BP
  RET

_init_timer:
  PUSH BP
  MOV BP,SP
  PUSH ES
  MOV AX,DS
  CS: MOV W[dssave],AX
  MOV AX,W[BP+4]
  CS: MOV W[timerroutine],AX
  XOR AX,AX
  MOV ES,AX
  CLI
  ES: MOV AX,W[020]
  CS: MOV W[int8save],AX
  ES: MOV AX,W[022]
  CS: MOV w[int8save+2],AX
  MOV AX,offset newint8
  ES: MOV W[020],AX
  MOV AX,CS
  ES: MOV W[022],AX
  STI
  POP ES
  POP BP
  RET

dssave:
  DW 0
timerroutine:
  DW 0
int8save:
  DW 0,0

newint8:
  PUSH AX
  PUSH BX
  PUSH CX
  PUSH DX
  PUSH SI
  PUSH DI
  PUSH BP
  PUSH DS
  PUSH ES
  CS: MOV AX,W[dssave]
  MOV DS,AX
  CS: CALL W[timerroutine]
  POP ES
  POP DS
  POP BP
  POP DI
  POP SI
  POP DX
  POP CX
  POP BX
  POP AX
  CS: JMP D[int8save]

_init_keyb:
  PUSH BP
  MOV BP,SP
  PUSH ES
  MOV AX,DS
  CS: MOV W[dssave9],AX
  MOV AX,W[BP+4]
  CS: MOV W[keybroutine],AX
  XOR AX,AX
  MOV ES,AX
  CLI
  ES: MOV AX,W[024]
  CS: MOV W[int9save],AX
  ES: MOV AX,W[026]
  CS: MOV w[int9save+2],AX
  MOV AX,offset newint9
  ES: MOV W[024],AX
  MOV AX,CS
  ES: MOV W[026],AX
  STI
  MOV AX,1
  CS: MOV W[keyhandlerinstalled],AX
  POP ES
  POP BP
  RET

keyhandlerinstalled:
  DW 0
dssave9:
  DW 0
keybroutine:
  DW 0
int9save:
  DW 0,0

newint9:
  PUSH AX
  PUSH BX
  PUSH CX
  PUSH DX
  PUSH SI
  PUSH DI
  PUSH BP
  PUSH DS
  PUSH ES
  CS: MOV AX,W[dssave9]
  MOV DS,AX
  CS: CALL W[keybroutine]
  POP ES
  POP DS
  POP BP
  POP DI
  POP SI
  POP DX
  POP CX
  POP BX
  POP AX
  CS: JMP D[int9save]

_restoreints:
  PUSH ES
  XOR AX,AX
  MOV ES,AX
  CLI
  CS: MOV AX,W[int8save]
  ES: MOV W[020],AX
  CS: MOV AX,W[int8save+2]
  ES: MOV W[022],AX
  MOV AX,W[keyhandlerinstalled]
  OR AX,AX
  JZ nokeybinstalled
  CS: MOV AX,W[int9save]
  ES: MOV W[024],AX
  CS: MOV AX,W[int9save+2]
  ES: MOV W[026],AX
  XOR AX,AX
  MOV W[keyhandlerinstalled],AX
nokeybinstalled:
  STI
  POP ES
  RET

_getwordfromport:
  PUSH BP
  MOV BP,SP
  MOV DX,W[BP+4]
  IN AL,DX
  MOV AH,AL
  IN AL,DX
  XCHG AL,AH
  POP BP
  RET

_cgafbar:
  PUSH BP
  MOV BP,SP
  PUSH SI
  PUSH DI
  MOV AX,W[_scrseg]
  MOV ES,AX
  MOV DI,W[BP+6]
  AND DI,1
  MOV AX,W[_interlacediff]
  MUL DI
  MOV DI,AX
  MOV AX,W[BP+6]
  SAR AX,1
  MOV DX,80
  MUL DX
  ADD DI,AX
  ADD DI,W[_scroff]
  MOV AX,W[BP+4]
  SHR AX,1
  SHR AX,1
  ADD DI,AX
  MOV AL,B[BP+0c]
  MOV DX,055
  MUL DX
  MOV BX,W[BP+8]
  SHR BX,1
  SHR BX,1
  MOV DX,W[_interlacediff]
  MOV SI,W[BP+0a]
ylooptop:
  MOV CX,BX
  REP STOSB
  SUB DI,BX
  ADD DI,DX
  SUB DX,80
  NEG DX
  DEC SI
  JNZ ylooptop
  POP DI
  POP SI
  POP BP
  RET
