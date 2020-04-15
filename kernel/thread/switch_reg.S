.globl _SwitchReg

# void switch_reg(uintptr_t *cur_esp, uintptr_t *new_esp);

_SwitchReg:
  # 'eax' store 'cur_esp'
  movl 4(%esp), %eax
  # 'edx' store 'new_esp'
  movl 8(%esp), %edx

  # save current callee registers
  pushl %ebx
  pushl %esi
  pushl %edi
  pushl %ebp

  # store current 'esp'
  movl %esp, (%eax)
  # restore new 'esp'
  movl (%edx), %esp

  # load new callee registers
  popl %ebp
  popl %edi
  popl %esi
  popl %ebx

  # Kernel threads in its first time slice return directly to its task, so we
  # have to enable interrupts here.
  sti

  ret