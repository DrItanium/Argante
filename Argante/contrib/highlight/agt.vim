" Vim syn file
" Language:	Argante RSIS assembler
" Maintainer:	Jaroslaw J. Pyszny <arghil@bigfoot.com>
" Last Change:	2000 Dec 08

" Remove any old syn stuff hanging around
syn clear

"this language is oblivious to case.
syn case ignore

"compilator directive
syn match agtDire	"!DOMAINS"
syn match agtDire	"!PRIORITY"
syn match agtDire	"!IPCREG"
syn match agtDire	"!INITDOMAIN"
syn match agtDire	"!SIGNATURE"
syn match agtDire	"!INITUID"

"comments
syn match agtComm	"#.*"

"label
syn match agtLabl	":[a-zA-Z_][a-zA-Z0-9_]*"
syn match agtSysLb	"$[a-zA-Z_][a-zA-Z0-9_]*"

"segments
syn match agtSegs	".CODE"
syn match agtSegs	".DATA"
syn match agtSegs	".END"

"types
syn match agtDec	"[0-9]\+"
syn match agtHex	"0[xX][0-9a-fA-F]\+"
syn match agtFlt    "[+-]\=[0-9]\+\.[0-9]*"
syn match agtStr	"\".*\""

"registers
syn match agtRegs	"[usf][0-9]"
syn match agtRegs	"[usf]1[0-9]"

"operands
syn keyword agtOper	NOP    JMP
syn keyword agtOper	IFEQ   IFNEQ   IFABO   IFBEL  CALL
syn keyword agtOper	RET    HALT    SYSCALL 
syn keyword agtOper	MOV    ADD     SUB     MUL    DIV
syn keyword agtOper	MOVB
syn keyword agtOper	MOD    XOR     REV     AND    OR
syn keyword agtOper	CWAIT  TWAIT
syn keyword agtOper	ALLOC  REALLOC DEALLOC
syn keyword agtOper	CMPCTN CPCNT   ONFAIL  NOFAIL LOOP RAISE

if !exists("did_agt_syntax_inits")
	let did_agt_syntax_inits = 1

	hi link agtSegs	 Comment
	hi link agtDire	 Preproc
	hi link agtComm  Comment
	hi link agtLabl  Tag
	hi link agtSysLB Tag
	hi link agtHex	 Number
	hi link agtDec	 Number
	hi link agtStr	 String
	hi link agtFlt	 Float
	hi link agtRegs	 Type
	hi link agtOper  Operator
endif

let b:current_syntax = "agt"
