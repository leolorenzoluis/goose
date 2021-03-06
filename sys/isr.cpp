#include "sys/isr.h"

#include "hal/keyboard.h"
#include "klib/debug.h"
#include "klib/macros.h"
#include "klib/panic.h"
#include "klib/types.h"
#include "sys/control_registers.h"
#include "sys/io.h"
#include "sys/idt.h"

using klib::Debug;

// Registers when the ISR was triggered. Used for (hopefully) diagnosing bugs.
typedef struct {
  // Pushed the segments last.
  uint32 gs;
  uint32 fs;
  uint32 es;
  uint32 ds;

  // Pushed by "pusha".
  uint32 edi;
  uint32 esi;
  uint32 ebp;
  uint32 esp;

  uint32 ebx;
  uint32 edx;
  uint32 ecx;
  uint32 eax;

  // "push byte #" and ecodes does this.
  uint32 int_no;
  uint32 err_code;

  // Pushed by the processor automatically.
  uint32 eip;
  uint32 cs;
  uint32 eflags;
  uint32 useresp;
  uint32 ss;
} regs;

extern "C" {

// The actual interrupt service routines that process traps and events.
//
// http://en.wikipedia.org/wiki/Interrupt_handler
// http://wiki.osdev.org/Interrupt_Service_Routines
// http://www.osdever.net/bkerndev/Docs/isrs.htm

// The first 32 interrupts are reserved for hardware exceptions. See
// kExceptionMessages below for descriptions.
extern void interrupt_handler_0();
extern void interrupt_handler_1();
extern void interrupt_handler_2();
extern void interrupt_handler_3();
extern void interrupt_handler_4();
extern void interrupt_handler_5();
extern void interrupt_handler_6();
extern void interrupt_handler_7();
extern void interrupt_handler_8();
extern void interrupt_handler_9();
extern void interrupt_handler_10();
extern void interrupt_handler_11();
extern void interrupt_handler_12();
extern void interrupt_handler_13();
extern void interrupt_handler_14();
extern void interrupt_handler_15();
extern void interrupt_handler_16();
extern void interrupt_handler_17();
extern void interrupt_handler_18();
extern void interrupt_handler_19();
extern void interrupt_handler_20();
extern void interrupt_handler_21();
extern void interrupt_handler_22();
extern void interrupt_handler_23();
extern void interrupt_handler_24();
extern void interrupt_handler_25();
extern void interrupt_handler_26();
extern void interrupt_handler_27();
extern void interrupt_handler_28();
extern void interrupt_handler_29();
extern void interrupt_handler_30();
extern void interrupt_handler_31();
// We use the next 16 IDT slots for handling IRQs.
extern void irq_handler_32();
extern void irq_handler_33();
extern void irq_handler_34();
extern void irq_handler_35();
extern void irq_handler_36();
extern void irq_handler_37();
extern void irq_handler_38();
extern void irq_handler_39();
extern void irq_handler_40();
extern void irq_handler_41();
extern void irq_handler_42();
extern void irq_handler_43();
extern void irq_handler_44();
extern void irq_handler_45();
extern void irq_handler_46();
extern void irq_handler_47();

}

// User friendly descriptions of CPU-defined interrupts.
// "Intel® 64 and IA-32 Architectures Software Developer’s Manual
// Volume 3A: System Programming Guide, Part 1"
const char *kInterruptDescriptions[] = {
    "Division By Zero",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved (15)",
    "x87 FPU Floating-Point Error",
    "Alignment Check Exception",
    "Machine Check Exception",
    "SIMD Floating-Point Exception",
    "Reserved (20)",
    "Reserved (21)",
    "Reserved (22)",
    "Reserved (23)",
    "Reserved (24)",
    "Reserved (25)",
    "Reserved (26)",
    "Reserved (27)",
    "Reserved (28)",
    "Reserved (29)",
    "Reserved (30)",
    "Reserved (31)",
};

namespace sys {

void InstallInterruptServiceRoutines() {
  // By default IRQs are mapped to IDT entries 8-15, but in protected mode
  // those indexes take on a different meaning. Remap IRQ handlers.
  outb(0x20, 0x11);
  outb(0xA0, 0x11);
  outb(0x21, 0x20);
  outb(0xA1, 0x28);
  outb(0x21, 0x04);
  outb(0xA1, 0x02);
  outb(0x21, 0x01);
  outb(0xA1, 0x01);
  outb(0x21, 0x0);
  outb(0xA1, 0x0);

  // Register all known ISRs. The first 32 are CPU-defined.
  InterruptDescriptorTableSetGate(0, (uint32) interrupt_handler_0, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(1, (uint32) interrupt_handler_1, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(2, (uint32) interrupt_handler_2, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(3, (uint32) interrupt_handler_3, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(4, (uint32) interrupt_handler_4, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(5, (uint32) interrupt_handler_5, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(6, (uint32) interrupt_handler_6, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(7, (uint32) interrupt_handler_7, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(8, (uint32) interrupt_handler_8, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(9, (uint32) interrupt_handler_9, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(10, (uint32) interrupt_handler_10, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(11, (uint32) interrupt_handler_11, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(12, (uint32) interrupt_handler_12, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(13, (uint32) interrupt_handler_13, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(14, (uint32) interrupt_handler_14, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(15, (uint32) interrupt_handler_15, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(16, (uint32) interrupt_handler_16, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(17, (uint32) interrupt_handler_17, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(18, (uint32) interrupt_handler_18, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(19, (uint32) interrupt_handler_19, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(20, (uint32) interrupt_handler_20, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(21, (uint32) interrupt_handler_21, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(22, (uint32) interrupt_handler_22, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(23, (uint32) interrupt_handler_23, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(24, (uint32) interrupt_handler_24, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(25, (uint32) interrupt_handler_25, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(26, (uint32) interrupt_handler_26, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(27, (uint32) interrupt_handler_27, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(28, (uint32) interrupt_handler_28, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(29, (uint32) interrupt_handler_29, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(30, (uint32) interrupt_handler_30, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(31, (uint32) interrupt_handler_31, 0x08, 0x8E);
  // The next 16 are used for handling IRQs
  InterruptDescriptorTableSetGate(32, (uint32) irq_handler_32, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(33, (uint32) irq_handler_33, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(34, (uint32) irq_handler_34, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(35, (uint32) irq_handler_35, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(36, (uint32) irq_handler_36, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(37, (uint32) irq_handler_37, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(38, (uint32) irq_handler_38, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(39, (uint32) irq_handler_39, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(30, (uint32) irq_handler_40, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(41, (uint32) irq_handler_41, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(42, (uint32) irq_handler_42, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(43, (uint32) irq_handler_43, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(44, (uint32) irq_handler_44, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(45, (uint32) irq_handler_45, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(46, (uint32) irq_handler_46, 0x08, 0x8E);
  InterruptDescriptorTableSetGate(47, (uint32) irq_handler_47, 0x08, 0x8E);

  // Set up timer cycle.
  // TODO(chris): Research modes, etc.
  // TODO(chris): Create some global variables...
  int hz = 1;
  int divisor = 1193180 / hz;
  outb(0x43, 0x36);
  outb(0x40, divisor & 0xFF);
  outb(0x40, divisor >> 8);

  // TODO(chris): Wrap this in a file "asm_ops.h" or similer.
  // Safe to handle interrupts.
  __asm__ __volatile__ ("sti");
}

}  // namespace sys

// All interrupts trigger this method. Note that all interrupts are disabled
// for the duration of this method. So when we start doing "real work" in
// these ISRs, save the state somewhere, and return immediately. Doing the
// long-running work async, and letting other interrupts file as normal.
extern "C" {
void interrupt_handler(regs* r) {
  // Processor interrupts.
  const char* description = "Unknown Interrupt";
  if (r->int_no < 32) {
    description = kInterruptDescriptions[r->int_no];
  }

  // DEBUGGING: Possible compiler bug? Something is amiss with our
  // variadic arg packs. Avoiding escaping by modifying a string:
  Debug::Log("");
  Debug::Log("-----------------");
  Debug::Log("interrupt_handler");
  char int_no_msg[] = "interrupt #00\0";
  int_no_msg[11] = '0' + (r->int_no / 10);
  int_no_msg[12] = '0' + (r->int_no % 10);
  Debug::Log(int_no_msg);
  Debug::Log("-----------------");

  Debug::Log("Received interrupt %s[%d] with code %d",
	     description, r->int_no, r->err_code);
  if (r->int_no == 14) {
    uint32 cr2 = get_cr2();
    Debug::Log("Was interrupt handler. CR2 %h", cr2);
  }

  klib::Panic("Unhandled interrupt.");
}

// TODO(chris): Put this elsewhere.
void handle_timer(regs* r) {
  if (r->int_no != 32) {
    Debug::Log("Error: Interrupt doesn't match expected.");
  }
  // TODO(chris): Thread scheduling or something?
}

// Handles IRQs. The mechanism is the same for interrupts, but we use a separate
// function to send the "End of Interrupt" command (0x20) to hardware.
void irq_handler(regs* r) {
  int irq_no = r->int_no - 32;

  // IRQ 8-15 need to send an End of Interrupt to slave controller at IDT 32.
  if (irq_no >= 8) {
    outb(0xA0, 0x20);
  }

  // TODO(chrsmith): Put this into a union with the other switch case variables.
  uint32 scancode;

  switch (irq_no) {
  case 0:
    handle_timer(r);
    break;
  case 1:
    // Keyboard
    scancode = inb(0x60);
    hal::Keyboard::SendScancode(scancode);
    break;
  default:
    Debug::Log("Unknown IRQ[%d]", irq_no);
  }

  // Send "End of Interrupt"
  outb(0x20, 0x20);
}

}  // extern "C"
