# Question

## 在添加serial中断代码的时候，系统在执行安装中断代码时出现了故障

问题描述：
执行到 `install_keyboard_interrupt()` 函数时，log_info() 停止了

void install_keyboard_interrupt(void)
{
    idt_set_gate(33, (uint32_t)isr33, 0x08, 0x8E);
    // log_info(interrupt_tag, "Keyboard interrupt handler installed at vector 0x21");
}

验证是否是因为日志长度的问题时，

void install_serial_interrupt(void)
{
    idt_set_gate(36, (uint32_t)isr36, 0x08, 0x8E);
    log_info(interrupt_tag, "Seria interrupt handler installed at vector 0x21");
}

报错：

=== DIVIDE BY ZERO EXCEPTION ===
Faulting Instruction: 0x29
Registers at fault:
  EAX: 0x635c79e1, EBX: 0x31327830, ECX: 0x125f8, EDX: 0x
System Halted

分析：确实跟长度有关，指定长度可能会导致出现崩溃
