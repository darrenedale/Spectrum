// t-state costs of opcodes prefixed with 0xdd 0xcb or 0xfd 0xcb (IX and IY register instructions). 32-bit unsigned
// values. conditional jumps contain the cost if jump is taken in 0xNNNN0000 and cost if jump is not taken in
// 0x0000NNNN. there are macros defined in z80.cpp to extract the values

23,        // Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__A

20,        // Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__B
20,        // Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__C
20,        // Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__D
20,        // Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__E
20,        // Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__H
20,        // Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__L
20,        // Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d
20,        // Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__A

20,        // Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__B
20,        // Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__C
20,        // Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__D
20,        // Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__E
20,        // Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__H
20,        // Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__L
20,        // Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d
20,        // Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__A

20,        // Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__B
20,        // Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__C
20,        // Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__D
20,        // Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__E
20,        // Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__H
20,        // Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__L
20,        // Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d
20,        // Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__A

20,        // Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__B
20,        // Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__C
20,        // Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__D
20,        // Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__E
20,        // Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__H
20,        // Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__L
20,        // Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d
20,        // Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__A

20,        // Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__B
20,        // Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__C
20,        // Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__D
20,        // Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__E
20,        // Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__H
20,        // Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__L
20,        // Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d
20,        // Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__A

20,        // Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__B
20,        // Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__C
20,        // Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__D
20,        // Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__E
20,        // Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__H
20,        // Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__L
20,        // Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d
20,        // Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__A

20,        // Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__B
20,        // Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__C
20,        // Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__D
20,        // Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__E
20,        // Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__H
20,        // Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__L
20,        // Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d
20,        // Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__A

20,        // Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__B
20,        // Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__C
20,        // Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__D
20,        // Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__E
20,        // Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__H
20,        // Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__L
20,        // Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d
20,        // Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__A

23,        // Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__B
23,        // Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__C
23,        // Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__D
23,        // Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__E
23,        // Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__H
23,        // Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__L
23,        // Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d
23,        // Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__A
