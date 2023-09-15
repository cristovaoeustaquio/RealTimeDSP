/**
 * main.c
 */

#include "F28x_Project.h"


__interrupt void cpu_timer0_isr (void);

void main(void)
{

    // Step 1. Initialize System Control:
    // PLL, WatchDog, enable Peripheral Clocks
    InitSysCtrl();

    // Step 2. Initialize GPIO:
    InitGpio();
    EALLOW;
    GpioCtrlRegs.GPADIR.bit.GPIO31 = 1;
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1; // output
    GpioCtrlRegs.GPBDIR.bit.GPIO59 = 0; // input
    GpioCtrlRegs.GPBPUD.bit.GPIO59 = 0; // Habilita Pull-up
    GpioCtrlRegs.GPBINV.bit.GPIO59 = 0; // Não inverter
//    GpioCtrlRegs.GPBCTRL.bit.QUALPRD3 = 0x00; // Qualification sampling
//    GpioCtrlRegs.GPBCSEL4.bit.GPIO59 = 3;
    EDIS;

    // Step 3. Clear all interrupts and initialize PIE vector table:
    // Disable CPU interrupts
    DINT;

    InitPieCtrl();
    IER=0;
    IFR=0;
    InitPieVectTable();

    CpuTimer0Regs.PRD.all = 200000000*0.5; //Freq do CPU * Period da interrupção (em sec)

    CpuTimer0Regs.TCR.bit.TSS = 1;  // 1 = Stop timer, 0 = Start/Restart Timer
    CpuTimer0Regs.TCR.bit.TRB = 1;  // 1 = reload timer
    CpuTimer0Regs.TCR.bit.SOFT = 0;
    CpuTimer0Regs.TCR.bit.FREE = 0; // Timer Free Run Disabled
    CpuTimer0Regs.TCR.bit.TIE = 1;  // 0 = Disable/ 1 = Enable Timer Interrupt

    CpuTimer0Regs.TCR.bit.TSS = 0;  // 1 = Stop timer, 0 = Start/Restart Timer

    EALLOW;
    PieVectTable.TIMER0_INT = &cpu_timer0_isr;
    EDIS;


    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

    IER |= M_INT1;

    EINT;
    ERTM;

    while(1)
    {

    }
}

__interrupt void cpu_timer0_isr (void)
{
    static int InterruptCount=0;

    InterruptCount++;

    GpioDataRegs.GPATOGGLE.bit.GPIO31=1;

    if(GpioDataRegs.GPBDAT.bit.GPIO59 == 1)
    {GpioDataRegs.GPBSET.bit.GPIO34=1;}

    if(GpioDataRegs.GPBDAT.bit.GPIO59 == 0)
    {GpioDataRegs.GPBCLEAR.bit.GPIO34=1;}

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;     //libera interrupcao

}
