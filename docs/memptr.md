# MEMPTR
## esoteric register of the ZiLOG Z80 CPU.

### by Boo-boo
### (draft English translation by Vladimir Kladov)

As far as is known, after the instruction `BIT n,(HL)` executes, bits 3 and 5 of the flag register contain values that 
are not documented in the official documentation at all. Actually these bits are copied from bits 11 and 13 of the
internal register pair of Z80 CPU, which is used for 16-bit operations, and in most cases to handle addresses. This
is usual for processors having an 8-bit data bus working with 16-bit data.

It is not known why and how these bits of the internal buffer register are copied to the flags register though. At least
Sean Young in the _Undocumented Z80 Documented_ refers to the phenomenon
([http://www.myquest.nl/z80undocumented/](http://www.myquest.nl/z80undocumented/)) and a bit more info can be found in
the Z80 description of another "nocash" project
([http://www.work.de/nocash/zxdocs.htm](http://www.work.de/nocash/zxdocs.htm)) where this register pair is known as
`MEMPTR`. Unfortunately until now attempts to crack the algorithm ofr setting the value of `MEMPTR` by different
processor instructions on basis of knowing only two bits of those 16-bit register were not successful.

But miraculously as a result of many experiments (based on the hypothesis that index addressing instructions always
initialize `MEMPTR` the same way) and also after deep consideration of the results of these samples we have found that
the `CPI` instruction increments the `MEMPTR` by 1 whereas `CPD` instruction decrements it. Hence, decrementing `MEMPTR`
in the loop and monitoring borrow from the high bits having two known bits in the flag register, it is possible to
determine unambiguously the 14 low bits of `MEMPTR` and having these in our hands to say for sure the rules by which
`MEMPTR` is set after each instruction.

A list of instructions changing `MEMPTR` follows, together with the formula for the new `MEMPTR` value. _rp_ means
_register pair_ (16 bit register `BC`, `DE`, `HL` or `SP` - ?), and _INDEX_ means register pair `IX` or `IY`.
Instructions not listed below do not affect `MEMPTR` as far as is known. All the CPUs tested give the same results
except _KP1858BM1_ and _T34BM1_, whose differences are noted as _BM1_ in the text.

### `LD A,(addr)`
    MEMPTR = addr + 1

### `LD (addr),A`
    MEMPTR_low = (addr + 1) & #FF
    MEMPTR_hi = A
  
  Note for _BM1_:
    
    MEMPTR_low = (addr + 1) & #FF
    MEMPTR_hi = 0

### `LD A,(rp)`
  (Where rp is either BC or DE).

	MEMPTR = rp + 1

### `LD (rp),A`
  (Where rp is either BC or DE).

    MEMPTR_low = (rp + 1) & #FF
    MEMPTR_hi = A

  Note for _BM1_:
  
    MEMPTR_low = (rp + 1) & #FF
    MEMPTR_hi = 0

### `LD (addr), rp`
### `LD rp,(addr)`
	MEMPTR = addr + 1

### `EX (SP),rp`
	MEMPTR = rp value after the operation

### `ADD/ADC/SBC rp1,rp2`
	MEMPTR = rp1_before_operation + 1

### `RLD/RRD`
	MEMPTR = HL + 1

### `JR/DJNZ/RET/RETI/RST`
  (Jumping to addr)

	MEMPTR = addr

### `JP/CALL addr`
  (Except `JP rp`)

  (Even in case of conditional `call`/`jp`, regardless of whether jump is taken or not)

	MEMPTR = addr

### `IN A,(port)`
	MEMPTR = (A_before_operation << 8) + port + 1

### `IN A,(C)`
	MEMPTR = BC + 1

### `OUT (port),A`
	MEMPTR_low = (port + 1) & #FF
    MEMPTR_hi = A
  
  Note for _BM1_:
  
    MEMPTR_low = (port + 1) & #FF
    MEMPTR_hi = 0

### `OUT (C),A`
	MEMPTR = BC + 1

### `LDIR/LDDR`
  - When BC == 1:
    
    `MEMPTR` is not changed

  - When BC <> 1:
  
    MEMPTR = PC + 1

  where PC = instruction address

### `CPI`
	MEMPTR = MEMPTR + 1

### `CPD`
	MEMPTR = MEMPTR - 1

### `CPIR`
  When `BC=1` or `A=(HL)`: exactly as `CPI`
  In other cases `MEMPTR = PC + 1` on each step, where PC = instruction address.

  **Note** since at the last execution `BC=1` or `A=(HL)`, resulting `MEMPTR = PC + 1 + 1` (if there were not interrupts 
  during the execution) 

### `CPDR`
  When BC=1 or A=(HL): exactly as CPD
  In other cases `MEMPTR = PC + 1` on each step, where PC = instruction address.

  **Note** since at the last execution `BC=1` or `A=(HL)`, resulting `MEMPTR = PC + 1 - 1` (if there were not interrupts 
  during the execution)

### `INI`
	MEMPTR = BC_before_decrementing_B + 1

### `IND`
	MEMPTR = BC_before_decrementing_B - 1

### `INIR`
  Exactly as `INI` on each execution. I.e. resulting `MEMPTR = ((1 << 8) + C) + 1` 

### `INDR`
  Exactly as `IND` on each execution. I.e. resulting `MEMPTR = ((1 << 8) + C) - 1`

### `OUTI`
	MEMPTR = BC_after_decrementing_B + 1

### `OUTD`
	MEMPTR = BC_after_decrementing_B - 1

### `OTIR`
  Exactly as `OUTI` on each execution. I.e. resulting `MEMPTR = C + 1`

### `OTDR`
  Exactly as `OUTD` on each execution. I.e. resulting `MEMPTR = C - 1`

### Any instruction with (INDEX+d):
	MEMPTR = INDEX + d

### Interrupt call to addr:
  As usual `CALL`. I.e. `MEMPTR = addr`

What is the benefit of this secret knowledge? First of all, it is possible now to program Z80 emulators supporting _all_
the undocumented peculiarities of the CPU. Secondly, the fact that on some Z80 clones the `MEMPTR` register behaves a
bit differently provides another method of model checking. Seems useful enough!

**(c)2006, zx.pk.ru**

- Theoretical part: **boo_boo**, **Vladimir Kladov**
- Testing real Z80 chips: **Wlodek**, **CHRV**, **icebear**, **molodcov_alex**, **goodboy**

source:
[drhelius on GitHub](https://gist.githubusercontent.com/drhelius/8497817/raw/cdd2889d9ea980c5800e9a857f29037be1f36907/Z80%2520MEMPTR),
retrieved 26th Feb, 2021. Converted to MD and translations tidied up by D Edale
  