/*
 * main.c
 */

#include <MSST_PWM.h>
#include "MSST_GlobalFunctions.h"
#include "F28x_Project.h"
#include "Syncopation_SCI.h"

#include "Syncopation_Data.h"
#include "MSST_PWM.h"

void deadloop();
void CpuTimerInit();
void CpuTimerIsr();

#define CPU_INT_MSEC 20

void main(void) {
	InitSysCtrl();

	EALLOW;
	ClkCfgRegs.LOSPCP.bit.LSPCLKDIV = 0;
	ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 0;
	EDIS;

    MSSTGpioConfig();
    GpioDataRegs.GPACLEAR.bit.GPIO18 = 1;  // Not prepared yet.
    CPU_LED_BIT = 0;
    DINT;
    InitPieCtrl();
    InterruptInit();

    SCI_Config();
    AdcInit();
    PwmInit();

    Pwm_EN();

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;  // Enable the PIE block
    IER = M_INT1 | M_INT9;

    EINT;

	deadloop();
}


Uint16 log_send_count = 0;

extern float Iac;
extern float Vdc;
extern float Idc;
extern Uint16 Prd;
extern Uint16 Duty;

extern Uint16 V_DC;
extern Uint16 I_DC_1;
extern Uint16 I_DC_2;
extern Uint16 I_AC_1;
extern Uint16 I_AC_2;
extern Uint16 LIGHT;

extern Uint16 TEMP_1;
extern Uint16 TEMP_2;
extern Uint16 TEMP_3;
extern Uint16 TEMP_4;

#pragma CODE_SECTION(deadloop, ".TI.ramfunc");
void deadloop()
{
    while(1)
    {
//        if(log_state == 2)
//        {
//            DataLog_SendSample(log_index);
//            log_send_count++;
//            log_index++;
//            if(log_index>=log_limit)
//                log_index = 0;
//            if(log_send_count >= log_limit)
//            {
//                log_send_count = 0;
//                log_index = 0;
//                log_state = 0;
//            }
//        }

        SCI_UpdatePacketFloat(0, Idc);
        SCI_UpdatePacketFloat(1, Vdc);
        SCI_UpdatePacketFloat(2, Iac);

        SCI_UpdatePacketInt16(0,V_DC);
        SCI_UpdatePacketInt16(1,I_DC_1);
        SCI_UpdatePacketInt16(2,I_DC_2);
        SCI_UpdatePacketInt16(3,I_AC_1);
        SCI_UpdatePacketInt16(4,I_AC_2);
//        SCI_UpdatePacketInt16(0,LIGHT);
//        SCI_UpdatePacketInt16(0,TEMP_1);
//        SCI_UpdatePacketInt16(0,TEMP_2);
//        SCI_UpdatePacketInt16(0,TEMP_3);
//        SCI_UpdatePacketInt16(0,TEMP_4);
        SCI_SendPacket();

        DELAY_US(4000);
    }
}

void CpuTimerInit()
{
    CpuTimer1Regs.TCR.all = 0x4010;
    CpuTimer1Regs.PRD.all = 200000 * CPU_INT_MSEC;
    EALLOW;
    PieVectTable.TIMER1_INT = &CpuTimerIsr;
    EDIS;

    DELAY_US(1);
//    CpuTimer1Regs.TCR.all = 0x4000;
}

#pragma CODE_SECTION(CpuTimerIsr, ".TI.ramfunc");
__interrupt void CpuTimerIsr()
{
    CPU_LED_TOGGLE = 1;

    CpuTimer1Regs.TCR.bit.TIF = 1;
}
