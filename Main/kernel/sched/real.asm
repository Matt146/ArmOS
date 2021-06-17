global sched_core_binary_begin
global sched_core_binary_end
sched_core_binary_begin:

incbin 'kernel/sched/boot.bin'

sched_core_binary_end: