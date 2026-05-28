#include "generic.h"
#include "partition.h"

typedef void (*app_entry_t)(void);

void boot2app(uint32_t address) {
    // address 是 app 槽的起始地址 (例如 0x08004000)，也就是 app 中断向量表的地址。
    // Cortex-M 向量表前 8 字节固定布局: [0]=初始 SP, [4]=Reset_Handler 入口。

    // 从向量表偏移 0 处读出 app 期望的栈顶地址。
    // volatile: 告诉编译器不要把这次读优化掉 —— 这是对硬件相关地址的直读。
    uint32_t sp = *(volatile uint32_t*)(address);

    // 从向量表偏移 4 处读出 app 的 Reset_Handler 地址 (上电后第一条要执行的指令)。
    uint32_t pc = *(volatile uint32_t*)(address + 4U);

    // 合法性检查: 如果槽里没烧过固件，读到的会是 0xFFFFFFFF (擦除态) 或随机值。
    // F103C8 的 SRAM 在 0x20000000，大小 20KB，合法 SP 必然以 0x2000xxxx 开头。
    // 这里用掩码做粗略判断，过滤掉明显非法的值，避免跳进去后立刻 HardFault。
    if ((sp & 0x2FFE0000U) != 0x20000000U) {
        return;
    }

    // 关全局中断 (设置 PRIMASK=1)。后面要切向量表、改 SP，过程中如果来个中断
    // 会跳到错误的地址执行，所以先把中断口子堵上。
    __disable_irq();

    // 把 HAL 初始化过的外设全部反初始化 (GPIO 复位、外设时钟关、SysTick 关等)，
    // 让 app 拿到一个接近"刚上电"的干净状态。否则 app 重新 init 时可能撞到
    // bootloader 留下的脏状态 (比如时钟已开但寄存器配置不一致)。
    HAL_DeInit();

    // 显式把 SysTick 三个寄存器清零 —— HAL_DeInit 不保证一定关 SysTick。
    SysTick->CTRL = 0;   // 关掉计数器和 SysTick 中断使能
    SysTick->LOAD = 0;   // 清重载值
    SysTick->VAL  = 0;   // 清当前计数值，顺便清 COUNTFLAG

    // 清 NVIC 里所有外部中断的"使能位"和"挂起位"。
    // ICER (Interrupt Clear Enable Register): 写 1 → 清对应中断的使能。
    // ICPR (Interrupt Clear Pending Register): 写 1 → 清对应中断的 pending 标志。
    // 写 0xFFFFFFFF 就是把这 32 个中断一次性全清。
    // 注释说 "F103 用前 2 组"，但 sizeof(NVIC->ICER)/sizeof(...) = 8 (CMSIS 标准定义)，
    // 循环跑 8 次也没关系——多写几个未使用位是无害的。
    for (uint32_t i = 0; i < sizeof(NVIC->ICER) / sizeof(NVIC->ICER[0]); i++) {
        NVIC->ICER[i] = 0xFFFFFFFFU;
        NVIC->ICPR[i] = 0xFFFFFFFFU;
    }

    // VTOR (Vector Table Offset Register): 告诉 CPU "向量表在哪里"。
    // 默认是 0x08000000 (指向 bootloader 的向量表)，现在改指向 app 的向量表。
    // 之后再来中断，CPU 就会去 app 的中断向量里找处理函数。
    SCB->VTOR = address;

    // 内存屏障: 保证 VTOR 的写入真的生效，再继续往下执行。
    __DSB();   // Data Sync Barrier: 等所有内存访问完成
    __ISB();   // Instruction Sync Barrier: 刷掉流水线里已预取的旧指令

    // 把主栈指针 MSP 设成 app 期望的栈顶。
    // 注意: 这一行之后，C 局部变量 (sp/pc/address 等) 在原栈上的位置就丢了，
    // 但它们的值已经被编译器存进寄存器里，所以紧接着的 pc 调用还能用。
    __set_MSP(sp);

    // 重新打开全局中断 (PRIMASK=0)。
    // 严格来说也可以不开，因为 app 的 Reset_Handler 启动后会自己控制中断；
    // 但开了对正常 app 没影响。
    __enable_irq();

    // 把 pc 强转成函数指针并调用 —— 这就是"跳进 app"的关键动作。
    // 等价于硬件复位后跳到 Reset_Handler。Cortex-M 用 Thumb 指令集，
    // 向量表里存的 PC 值最低位本来就是 1 (Thumb bit)，强转直接能用。
    ((app_entry_t)pc)();

    // 防御: 理论上 Reset_Handler 永远不会 return。
    // 万一真的返回了，死循环卡住，避免 CPU 跑飞。
    for (;;) { }
}
