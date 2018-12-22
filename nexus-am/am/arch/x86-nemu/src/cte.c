#include <am.h>
#include <x86.h>
#include <stdio.h>

static _Context* (*user_handler)(_Event, _Context*) = NULL;

void vectrap();
void vecnull();

_Context* irq_handle(_Context *tf) {
  _Context *next = tf;
  printf("irq_handle:\n");
  printf("eax: 0x%x, ecx: 0x%x, edx: 0x%x, ebx: 0x%x, esp: 0x%x, ebp: 0x%x, esi: 0x%x, edi: 0x%x, eflags: 0x%x\n", 
  tf->eax, tf->ecx, tf->edx, tf->ebx, tf->esp, tf->ebp, tf->esi, tf->edi, tf->eflags);
  if (user_handler) {
    _Event ev = {0};
    switch (tf->irq) {
      default: ev.event = _EVENT_ERROR; break;
    }

    next = user_handler(ev, tf);
    if (next == NULL) {
      next = tf;
    }
  }

  return next;
}

static GateDesc idt[NR_IRQ];

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  // initialize IDT
  for (unsigned int i = 0; i < NR_IRQ; i ++) {
    idt[i] = GATE(STS_TG32, KSEL(SEG_KCODE), vecnull, DPL_KERN);
  }

  // -------------------- system call --------------------------
  idt[0x81] = GATE(STS_TG32, KSEL(SEG_KCODE), vectrap, DPL_KERN);

  set_idt(idt, sizeof(idt));

  // register event handler
  user_handler = handler;

  return 0;
}

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
  return NULL;
}

void _yield() {
  asm volatile("int $0x81");
}

int _intr_read() {
  return 0;
}

void _intr_write(int enable) {
}
