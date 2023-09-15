/**
 * main.c
 */

#include "F28x_Project.h"
#include "math.h"

#define Tamanho 256
Uint16 Vetor0[Tamanho];


__interrupt void cpu_timer0_isr (void);

void ConfigureTimer(void);

void ConfigureDAC(void);

void ConfigureADC(void);

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
    
    ConfigureTimer();
    ConfigureDAC();
    ConfigureADC();

    // Step 3. Clear all interrupts and initialize PIE vector table:
    // Disable CPU interrupts
    DINT;

    InitPieCtrl();
    IER=0;
    IFR=0;
    InitPieVectTable();


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
    static int InterruptCount = 0, resultsIndex = 0;
    static float tempo = 0, delt = 0.05; //delt é o periodo de amostragem

    InterruptCount++;

    GpioDataRegs.GPATOGGLE.bit.GPIO31=1;

    if(GpioDataRegs.GPBDAT.bit.GPIO59 == 1)
    {GpioDataRegs.GPBSET.bit.GPIO34=1;}

    if(GpioDataRegs.GPBDAT.bit.GPIO59 == 0)
    {GpioDataRegs.GPBCLEAR.bit.GPIO34=1;}

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;     //libera interrupcao
    
    DacbRegs.DACVALS.all = (Uint16)(2048 + 200*sin(2*3.141592*0.1*tempo)); //valor digital que sera convertido para analogico
    
    Vetor0[resultsIndex++] = AdcaResultRegs.ADCRESULT0;
    
    if(Tamanho <= resultsIndex)
    {
        resultsIndex = 0;
    }
    
    
    tempo += delt;

}

void ConfigureTimer(void)
{
    CpuTimer0Regs.PRD.all = 200000000*0.05; //Freq do CPU * Period da interrupção (em sec)

    CpuTimer0Regs.TCR.bit.TSS = 1;  // 1 = Stop timer, 0 = Start/Restart Timer
    CpuTimer0Regs.TCR.bit.TRB = 1;  // 1 = reload timer
    CpuTimer0Regs.TCR.bit.SOFT = 0;
    CpuTimer0Regs.TCR.bit.FREE = 0; // Timer Free Run Disabled
    CpuTimer0Regs.TCR.bit.TIE = 1;  // 0 = Disable/ 1 = Enable Timer Interrupt

    CpuTimer0Regs.TCR.bit.TSS = 0;  // 1 = Stop timer, 0 = Start/Restart Timer
}

void ConfigureDAC(void)
{
    EALLOW;
    DacbRegs.DACCTL.bit.DACREFSEL = 1;       //use DAC references
    DasbRegs.DACOUTEN.bit.DACOUTEN = 1;     //enable DAC
    EDIS;
}

void ConfigureADC(void)
{
    EALLOW;
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6;      //set ADCCLK divider to /4
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE); //precisa adicionar adc.c common souce
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;      //Power Up the ADC
    
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;       //ADCINA0
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 19;      //sample duration of 20 SYSCLK cycles (20 pulsos de clock como duração)
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 1;     //timer0
    EDIS;
}
