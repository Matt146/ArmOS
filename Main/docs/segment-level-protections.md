# Credits

Article by: Matt146 (https://github.com/Matt146)

# Overview

The x86 architecture provides many protection mechanisms through at both the segment level and page level. Although much of what is discussed in this article will apply to protected mode protection mechanisms, some of it still applies in 64-bit mode. However, thankfully, much of the complexity has been removed, as long mode page-level protections are much more convinient to use and segmentation is largely ignored.

# Requirements

In order to understand this article, one must have a solid understanding of the following:
- Segmentation
- Segment selectors
- Segment registers
- GDT
- LDT
- TSS

# Limit Checks

In protected mode, all memory accesses in a segment are checked against that segment's effective limit. The effective limit of a segment is defined as the limit of the segment - 1. Since every memory access is compared to this value, if you attempt to access memory past this limit for all types of segments except for expand-down segments, you'll get a #GP (General Protection Exception) or a #SS (Stack Exception).

In addition to checking segment limits, the processor also checks descriptor table limits. (By descriptor-table limits, I mean the limit value in the GDTR or LDTR registers). This is in order to prevent programs from selecting segment descriptors outside of their respective descritpor tables.

A segment's limit is can be interpreted by the processor in a couple of different ways, depending on which flags have been set in the GDT entry that defines that segment.

## The G Flag
- If the G flag is clear in the GDT entry that defines that segment, the limit is interpreted as-is in bytes. For example, a segment with a limit of 0xFFFFFH will have a limit of 0xFFFFFH bytes.
- If the G flag is set in the GDT entry that defines that segment, the limit is scaled by a factor of 4K. For example, a segment with a limit of 0xFFFFFH that has the G flag set will have a true limit of 0xFFFFFH * 0x1000, which equals 0xFFFFFFFFH.

## Expand-Down Segments:
- When the E flag is set in a GDT entry that defines a segment, that segment is considered "expand-down." In expand-down segments, the range of valid offsets is from (effective limit + 1) to 0xFFFFH if the D/B flag is clear and (effective limit + 1) to 0xFFFFFFFFH if the D/B flag is set.

## Limit Checking in 64-bit mode:
In 64-bit mode, the processor does not perform any runtime limit checking on code and data segments, but still checks descriptor table limits. Instead of performing limit checks in 64-bit mode, the processor checks that all virtual addresses are in canonical form.

# Type Checking
On top of limit checks, the process examines type information at various times while operating on segment selectors and segment descriptors. For example, when loading the CS register with a selector, the processor automatically checks that the descriptor that it points to is a valid code segment with the correct permissions. This check must pass before loading in the selector into the segment register. The Intel manual specifies a list of important scenarios where the processor employs type checking for further reference.


# Null Selector Checks
Null selectors are extremely useful in protected mode and compatibility mode, as any attempt to access data using a segment register that contains a null selector automatically causes a #GP to be generated. Therefore, null selectors can be used for segments that are unused in a program. However, loading a null selector into the CS or SS segment registers generates a #GP, as those can never contain null selectors. All other data segment registers (DS, FS, ES, and GS) can be loaded with a null selector, so long as they are not accessed.

In 64-bit mode, the CS register cannot contain a null selector ever and the SS register cannot contain a null selector in ring 3. The rest of the data segment registers can contain null selectors, as segmentation is largely ignored in 64-bit mode.

# Privilege Levels
The x86 architecture provides four protection rings a given program can be in at any point in time. These include ring 0, 1, 2, and 3 (from most privileged to least privileged). Most operating systems only use ring 0 for kernel code and ring 3 for code intended to run in user mode, largely ignoring rings 1 and 2.

- **Current Privilege Level (CPL):** This is the privilege level of the currently-executing program or task. It is stored in the first two bits of the CS register.
- **Descriptor Privilege Level (DPL):** This is the privilege level stored inside a segment or gate descriptor. When a segment selector is loaded into a segment register, the DPL of the segment descriptor is compared to the CPL and the RPL of the segment selector being loaded in. This protection check must pass for the selector to be loaded into the segment register.
- **Requestor Privilege Level (RPL):** This is an "override" to the CPL assigned to segment selectors that is stored in the first two bits of a selector. The processor usually compares the CPL with the RPL and then compares that result with the DPL. If the CPL is numerically greater than the RPL, the CPL takes precedence. However, if the RPL is numerically greater than the CPL, the RPL takes precedence and is used as the effective privilege when compared to the DPL.

When it comes to RPL and CPL, you can think of them as working together to establish an "effective privilege," in which the higher number of the two is taken as the effective privilege and then compared to the value of the DPL. Throughout the article, the notation `MAX(RPL, CPL)` will be used to represent this.

## Privilege Checks When Accessing Data Segments:
When loading in selectors into data segment registers (DS, ES, FS, and GS), the DPL of the target segment being loaded in must be greater than or equal to the `MAX(CPL, RPL of the selector).`
- *DPL of the target segment >= MAX(CPL, RPL of the segment selector being loaded in*

## Privilege Checks When Accessing Stack Segments:
When loading in selectors into the SS register, the DPL of the target segment must be equal to the CPL, which must also be equal to the RPL of the selector being loaded in.
- *DPL of the target segment = CPL = RPL of the segment selector being loaded in*

## Privilege Checks During Control Transfers:
In **near control transfers** (ie: `jmp some_addr` or `call some_addr`), the CS register remains unchanged and only the instruction pointer is set to the value of `some_addr.` The only check that occurs is a limit check to ensure the instruction pointer remains inside the current code segment.

In **far control transfers** (ie: `jmp some_cs:some_addr` or `call some_cs:some_addr`), the value of the CS register changes, and thus a privilege check must occur. When performing a far control transfer to a nonconforming code segment (the C flag in the GDT entry in which that code segment descriptor is defined is clear), the DPL must be equal to the `MAX(CPL, RPL of some_cs)`. When performing a far control transfer to a conforming code segment, (the C flag in the GDT entry in which taht code segment is defined is set), the DPL can be <= `MAX(CPL, RPL of some_cs).` However, even in a far control transfer to a conforming code segment, the CPL is not changed!

The only way to change CPL is through syscalls, which can be implemented in the following ways:
- Call Gates
- Sysenter/sysexit instructions: Available in protected mode and in long mode in Intel processors. (However, use these in protected mode only for compatibility purposes)
- Syscall/sysret instructions: Available in 64-bit mode of all processors.

# Call Gate Overview
Call gates can be used in both legacy and long mode, but are rarely nowadays in favor of sysenter/sysexit and syscall/sysret. They are defined as entries in the GDT. These entires include an offset, a CS selector (offset into the GDT, which specifies a target code segment), a present flag (must be set, otherwise a #NP (not present) exception will be generated if accessed), some type information, a DPL, and a 5-bit param count, which is ignored in long mode. 

Call gates can be accessed through either far CALL's or far JMP's, which you can do through `call some_cs:offset`. The offset value is ignored, but is required for it to be a far CALL or JMP. The `some_cs` part of the far pointer specifies the offset in the GDT in which the call gate is found. From this, a series of protection checks occur (which will be discussed later). If these checks pass, the entry point for the called procedure is calculated by the call gate descriptor offset value + the target code segment base descriptor value, and execution begins there.

## Call Gate Protection Checks:
Call gate protection checks depend on whether a far CALL or far JMP accessed the call gate.

- If far CALL:
	- 1. DPL of call gate descriptor >= MAX(CPL, RPL of `some_cs`)
	- 2. DPL of target code segment descriptor <= CPL
		- Doesn't matter if the target code segment descriptor is conforming or nonconforming. HOWEVER, the only way to access lower your CPL is through doing a far CALL to a nonconforming code segment.

- If far JMP:
	- 1. DPL of call gate descriptor >= MAX(CPL, RPL of `some_cs`)
	- 2. If target code segment = conforming, DPL of target code segment descriptor <= CPL
	- 2. If target code segment = nonconforming, DPL of target code segment descriptor = CPL


**The confusing part about this is that the only way to numerically lower CPL through a call gate is to access a call gate through a far CALL to a nonconforming code segment.** If a far CALL occurs through a call gate to a nonconforming target code segment and the checks pass, the CPL is lowered to the DPL of the destination code segment and a stack switch occurs.

If a call or jump is made to a more privileged conforming destination code segment, the CPL is not changed and no stack switch occurs. The rest just functions as a regular far control transfer, in which the CS register is set to the selector value inside the call gate descriptor and the instructin pointer is set to the offset inside of the call gate descriptor. It's basically a far jump/call with extra steps :^).

Therefore, a stack switch only occurs if there is a change in CPL.

## Call Gate Stack Switching:
When a procedure call through a call gate resutls in a change in privilege level, (remember: far call to a nonconforming code segment through a call gate), the processor does the following:
1. Use DPL of the destination code segment as an offset into the TSS
2. The CPL is set to the DPL of the destination code segment
3. Retrive the values of the new SS and new ESP from the TSS and perform checks to see if they are valid. If these checks do not pass, a #TS is generated (invalid TSS exception)
4. Load the new SS and ESP values from the TSS into the SS and ESP registers and push the old SS and ESP values onto the new stack
5. Automatically copy the number of parameters specified in the call gate descriptor's 5-bit "param count" and push them onto the new stack from the calling procedure's stack
	- The size of these parameters is either 4 bytes for a 32-bit call gate descriptor or 2 bytes for a 16-bit call gate descriptor.
6. Push the return (old) CS:EIP onto the new stack
7. CS is loaded in with the value from the selector value in the call gate descriptor, while EIP is loaded in with the offset value from the call gate descriptor
8. Called procedure now begins executing

## Call Gate Stack Switching in 64-bit Mode:
In 64-bit mode, the call gate protection check mechanisms remain unchanged from legacy mode. However, the stack switch mechanism is slightly different.
1. The DPL of the destiantion code segment is used as an offset into the TSS
2. The CPL is set to the DPL of the destination code segment
3. Only RSP is loaded from the TSS and SS is forced to NULL
4. The old SS and RSP are pushed onto the new stack
5. There is no parameter copying cringe. However, software can access the old SS and RSP values and manually copy parameters from the stack.
6. Return (old) CS:RIP values pushed onto the new stack and CS is loaded in with the new value from the call gate descriptor's selector value and RIP is loaded with the call gate descriptor's offset value
7. The called procedure now begins executing

# Return Control Transfers:
In **near returns,** the value of CS does not change, so the processor just pops off the return instruction pointer from the stack and sets that to the instruction pointer value
	- Just a normal `ret` in NASM.

In **far returns to the same privilege,** the value of CS changes. The way the processor ensures that it is a far return to the same privilege is by comparing the RPL of the return code segment to the CPL. If the CPL = RPL, then it is a far return to the same privilege and the old CS:RIP or CS:EIP (depending on operating mode) gets popped off from the stack and is loaded back in.

In **far returns to different privilege levels** (returning from a call gate), the value of CS changes and the CPL changes as well. The value of CPL only increases numerically. It cannot decrease an an inter-privilege far return. Since we are changing privilege levels, a stack switch occurs on these kinds of far returns.
1. The return (old) CS:RIP or CS:EIP value is popped off of the stack and restored
2. If the return instruction has an immediate operand, it pops off that amount of bytes after popping off the return CS:RIP or CS:EIP. This can be used in protected mode far returns, in which the call gate descriptor that was used used the optional automatic parameter copy mechanism.
3. The return program SS:RSP or SS:ESP is popped off of the stack and loaded into the SS:RSP or SS:ESP registers. However, in 64-bit mode, a null selector is popped into SS if the target mode is 64-bit mode and the target CPL < 3.
