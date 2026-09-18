/*----------------------------------------------------------------------------*
 * Copyright Statement:                                                       *
 *                                                                            *
 *   This software/firmware and related documentation ("MediaTek Software")   *
 * are protected under international and related jurisdictions'copyright laws *
 * as unpublished works. The information contained herein is confidential and *
 * proprietary to MediaTek Inc. Without the prior written permission of       *
 * MediaTek Inc., any reproduction, modification, use or disclosure of        *
 * MediaTek Software, and information contained herein, in whole or in part,  *
 * shall be strictly prohibited.                                              *
 * MediaTek Inc. Copyright (C) 2010. All rights reserved.                     *
 *                                                                            *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND     *
 * AGREES TO THE FOLLOWING:                                                   *
 *                                                                            *
 *   1)Any and all intellectual property rights (including without            *
 * limitation, patent, copyright, and trade secrets) in and to this           *
 * Software/firmware and related documentation ("MediaTek Software") shall    *
 * remain the exclusive property of MediaTek Inc. Any and all intellectual    *
 * property rights (including without limitation, patent, copyright, and      *
 * trade secrets) in and to any modifications and derivatives to MediaTek     *
 * Software, whoever made, shall also remain the exclusive property of        *
 * MediaTek Inc.  Nothing herein shall be construed as any transfer of any    *
 * title to any intellectual property right in MediaTek Software to Receiver. *
 *                                                                            *
 *   2)This MediaTek Software Receiver received from MediaTek Inc. and/or its *
 * representatives is provided to Receiver on an "AS IS" basis only.          *
 * MediaTek Inc. expressly disclaims all warranties, expressed or implied,    *
 * including but not limited to any implied warranties of merchantability,    *
 * non-infringement and fitness for a particular purpose and any warranties   *
 * arising out of course of performance, course of dealing or usage of trade. *
 * MediaTek Inc. does not provide any warranty whatsoever with respect to the *
 * software of any third party which may be used by, incorporated in, or      *
 * supplied with the MediaTek Software, and Receiver agrees to look only to   *
 * such third parties for any warranty claim relating thereto.  Receiver      *
 * expressly acknowledges that it is Receiver's sole responsibility to obtain *
 * from any third party all proper licenses contained in or delivered with    *
 * MediaTek Software.  MediaTek is not responsible for any MediaTek Software  *
 * releases made to Receiver's specifications or to conform to a particular   *
 * standard or open forum.                                                    *
 *                                                                            *
 *   3)Receiver further acknowledge that Receiver may, either presently       *
 * and/or in the future, instruct MediaTek Inc. to assist it in the           *
 * development and the implementation, in accordance with Receiver's designs, *
 * of certain softwares relating to Receiver's product(s) (the "Services").   *
 * Except as may be otherwise agreed to in writing, no warranties of any      *
 * kind, whether express or implied, are given by MediaTek Inc. with respect  *
 * to the Services provided, and the Services are provided on an "AS IS"      *
 * basis. Receiver further acknowledges that the Services may contain errors  *
 * that testing is important and it is solely responsible for fully testing   *
 * the Services and/or derivatives thereof before they are used, sublicensed  *
 * or distributed. Should there be any third party action brought against     *
 * MediaTek Inc. arising out of or relating to the Services, Receiver agree   *
 * to fully indemnify and hold MediaTek Inc. harmless.  If the parties        *
 * mutually agree to enter into or continue a business relationship or other  *
 * arrangement, the terms and conditions set forth herein shall remain        *
 * effective and, unless explicitly stated otherwise, shall prevail in the    *
 * event of a conflict in the terms in any agreements entered into between    *
 * the parties.                                                               *
 *                                                                            *
 *   4)Receiver's sole and exclusive remedy and MediaTek Inc.'s entire and    *
 * cumulative liability with respect to MediaTek Software released hereunder  *
 * will be, at MediaTek Inc.'s sole discretion, to replace or revise the      *
 * MediaTek Software at issue.                                                *
 *                                                                            *
 *   5)The transaction contemplated hereunder shall be construed in           *
 * accordance with the laws of Singapore, excluding its conflict of laws      *
 * principles.  Any disputes, controversies or claims arising thereof and     *
 * related thereto shall be settled via arbitration in Singapore, under the   *
 * then current rules of the International Chamber of Commerce (ICC).  The    *
 * arbitration shall be conducted in English. The awards of the arbitration   *
 * shall be final and binding upon both parties and shall be entered and      *
 * enforceable in any court of competent jurisdiction.                        *
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 *
 * $Author: jc.wu $
 * $Date: 2012/6/5 $
 * $RCSfile: pi_calibration_api.c,v $
 * $Revision: #5 $
 *
 *---------------------------------------------------------------------------*/

/** @file pi_calibration_api.c
 *  Basic DRAMC calibration API implementation
 */

//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------

#include "dramc_common.h"
#include "x_hal_io.h"
#include "dramc_pi_api.h"
//#include "dramc.h"

#if 1//REG_ACCESS_PORTING_DGB
U8 RegLogEnable=0;
#endif

#define MAX_CA_PI_DELAY         63
#define MAX_CS_PI_DELAY         63
#define MAX_CLK_PI_DELAY        31


#define PASS_RANGE_NA   0x7fff

#define SIMULATION_WRITE_LEVELING  1
#define SIMULATION_GATING 1
#define SIMULATION_DATLAT 1
#define SIMULATION_SW_IMPED 1
#define SIMULATION_RX_PERBIT    1
#define SIMULATION_TX_PERBIT    1  // Please enable with write leveling

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
static U8 fgwrlevel_done = 0;

U8 u1MR2Value=0x1a;

PASS_WIN_DATA_T DlyPerBit[32];
static S32 CATrain_ClkDelay = 0; //cc add initialization value

//cc change [RANK] dimension. Although we only LEVEL one RANK (TX DQS delay is common for both RANKs)
static S32 wrlevel_dqs_final_delay[RANK_MAX][DQS_NUMBER];  
static U16 u2rx_window_sum;

U32 gDramcSwImpedanceResule[4];

#ifdef DRAM_CALIB_LOG
U32 gDRAM_CALIB_LOG_pointer=0;
#endif
#if defined(DRAM_CALIB_LOG) || defined(DRAM_ETT)
SAVE_TO_SRAM_FORMAT_CHANNEL_T gDRAM_CALIB_LOG;
#endif

U32 u4DRAMdebugLOgEnable = 0x01;
#ifdef TARGET_BUILD_VARIANT_ENG
U32 u4DRAMdebugLOgEnable2 = 1;
#else
U32 u4DRAMdebugLOgEnable2 = 0;
#endif

static void dle_factor_handler(DRAMC_CTX_T *p, U8 curr_val);

static U16 Round_Operation(U16 A, U16 B)
{
	U16 temp;

	if (B == 0)
	{
		return 0xffff;
	}

	temp = A/B;

	if ((A-temp*B) >= ((temp+1)*B-A))
	{
		return (temp+1);
	}
	else
	{
		return temp;
	}
}

void vSetRankNumber(DRAMC_CTX_T *p)
{
    #if DUAL_RANK_ENABLE
    p->support_rank_num =RANK_DUAL;
    #else
    p->support_rank_num =RANK_SINGLE;
    #endif
}

void vSetRank(DRAMC_CTX_T *p, U8 ucRank)
{
    p->rank = ucRank;
}

U8 u1GetRank(DRAMC_CTX_T *p)
{
    return p->rank;
}

void vSetCalibrationResult(DRAMC_CTX_T *p, U8 ucCalType, U8 ucResult)
{
    p->aru4CalExecuteFlag[p->rank] |= (1<<ucCalType); // ececution done
    if (ucResult == DRAM_OK)  // Calibration OK
    {
        p->aru4CalResultFlag[p->rank] &= (~(1<<ucCalType));
    }
    else  //Calibration fail
    {
        p->aru4CalResultFlag[p->rank] |= (1<<ucCalType);
    }
}

void vGetCalibrationResult_All(DRAMC_CTX_T *p, U8 u1Rank, U32 *u4CalExecute, U32 *u4CalResult)
{
    *u4CalExecute = p->aru4CalExecuteFlag[u1Rank];
    *u4CalResult = p->aru4CalResultFlag[u1Rank];
}

#if 0  //no use now, disable for saving code size.
void vGetCalibrationResult(DRAMC_CTX_T *p, U8 ucCalType, U8 *ucCalExecute, U8 *ucCalResult)
{
    U32 ucCalResult_All, ucCalExecute_All;

    ucCalExecute_All = p->aru4CalExecuteFlag[p->rank];
    ucCalResult_All = p->aru4CalResultFlag[p->rank];

    *ucCalExecute = (U8)((ucCalExecute_All >>ucCalType) & 0x1);
    *ucCalResult =  (U8)((ucCalResult_All >>ucCalType) & 0x1);
}
#endif

#if COMPILE_THIS_PART
const char *szCalibStatusName[DRAM_CALIBRATION_MAX]=
{
	"ZQ Calibration",
	"SW Impedance",
	"CA Training",
	"Write leveling",
	"RX DQS gating",
	"RX DATLAT",
	"RX DQ/DQS(RDDQC)",
	"RX DQ/DQS(Engine)",
	"TX DQ/DQS",
};

void vPrintCalibrationResult(DRAMC_CTX_T *p)
{
    U8  ucRankIdx, ucCalIdx;
    U32 ucCalResult_All, ucCalExecute_All;
    U8 ucCalResult, ucCalExecute;

    for(ucRankIdx=0; ucRankIdx<RANK_MAX; ucRankIdx++)
    {
        ucCalExecute_All = p->aru4CalExecuteFlag[ucRankIdx];
        ucCalResult_All = p->aru4CalResultFlag[ucRankIdx];
        mcSHOW_DBG_MSG(("[vPrintCalibrationResult] Rank= %d, (ucCalExecute_All 0x%x, ucCalResult_All 0x%x)\n", ucRankIdx, ucCalExecute_All, ucCalResult_All));

        for(ucCalIdx =0; ucCalIdx<DRAM_CALIBRATION_MAX; ucCalIdx++)
        {
            ucCalExecute = (U8)((ucCalExecute_All >>ucCalIdx) & 0x1);
            ucCalResult =  (U8)((ucCalResult_All >>ucCalIdx) & 0x1);
            mcSHOW_DBG_MSG(("    %s : Execute= %d, Result= %d (0: Ok, 1:Fail)\n", szCalibStatusName[ucCalIdx], ucCalExecute, ucCalResult));
        }

    }
}
#endif

static const char * str_ddrtype[] = {
	"RSV",
	"DDR1",
	"LPDDR2",
	"LPDDR3",
	"DDR2",
	"DDR3",
	"DDR4",
};

void vPrintCalibrationBasicInfo(DRAMC_CTX_T *p)
{
    mcSHOW_DBG_MSG(("===============================================================================\n"));
    mcSHOW_DBG_MSG(("Dram Type= %s, Freqency= %d, rank %d, odt_onoff %d, pin_mux %d\n",
                           str_ddrtype[p->dram_type], p->frequency, p->rank, 
                           p->odt_onoff, p->pinmux));
    mcSHOW_DBG_MSG(("===============================================================================\n"));
}


#if 0//cc mark
// for LP3 to control all PHY of single channel
void vIO32WriteFldAlign_Phy_All(U32 reg32, U32 val32, U32 fld)
{
    if(reg32<Channel_A_PHY_BASE_VIRTUAL)
    {
        mcSHOW_DBG_MSG(("\n[vIO32WriteFldAlign_Phy_All] wrong address %d\n", reg32));
        return;
    }

    reg32 &= 0xffff;

    vIO32WriteFldAlign(reg32+Channel_A_PHY_BASE_VIRTUAL, val32, fld);
    vIO32WriteFldAlign(reg32+Channel_B_PHY_BASE_VIRTUAL, val32, fld);

}
#endif

void vApplyConfigAfterCalibration(DRAMC_CTX_T *p)
{
    //DA mode
    vIO32WriteFldAlign(DDRPHY_B0_DQ6, 0x0, B0_DQ6_RG_RX_ARDQ_BIAS_PS_B0);
    vIO32WriteFldAlign(DDRPHY_B1_DQ6, 0x0, B1_DQ6_RG_RX_ARDQ_BIAS_PS_B1);
    vIO32WriteFldAlign(DDRPHY_CA_CMD6, 0x0, CA_CMD6_RG_RX_ARCMD_BIAS_PS);

    vIO32WriteFldAlign(DDRPHY_B0_DQ6, 0x1, B0_DQ6_RG_TX_ARDQ_OE_EXT_DIS_B0);
    vIO32WriteFldAlign(DDRPHY_B1_DQ6, 0x1, B1_DQ6_RG_TX_ARDQ_OE_EXT_DIS_B1);
    vIO32WriteFldAlign(DDRPHY_CA_CMD6, 0x1, CA_CMD6_RG_TX_ARCMD_OE_EXT_DIS);

    //IMPCAL Settings
    vIO32WriteFldMulti(DRAMC_REG_IMPCAL, P_Fld(0, IMPCAL_IMPCAL_IMPPDP) | P_Fld(0, IMPCAL_IMPCAL_IMPPDN) |\
    							P_Fld(0, IMPCAL_IMPCAL_CALI_EN));    //RG_RIMP_BIAS_EN and RG_RIMP_VREF_EN move to IMPPDP and IMPPDN

    //Prevent M_CK OFF because of hardware auto-sync
    vIO32WriteFldAlign(DDRPHY_MISC_CG_CTRL0, 0, Fld(4,0,AC_MSKB0));

    //DFS- fix Gating Tracking settings
    vIO32WriteFldAlign(DDRPHY_MISC_CTRL0, 0, MISC_CTRL0_R_STBENCMP_DIV4CK_EN);
    vIO32WriteFldAlign(DDRPHY_MISC_CTRL1, 0, MISC_CTRL1_R_DMSTBENCMP_RK_OPT);

    vIO32WriteFldAlign(DRAMC_REG_SPCMDCTRL, 0x1, SPCMDCTRL_REFRDIS);    //MR4 Disable
    vIO32WriteFldMulti(DRAMC_REG_DQSOSCR, P_Fld(0x1, DQSOSCR_DQSOSCRDIS)|P_Fld(0x1, DQSOSCR_DQSOSCENDIS));  //MR18, MR19 Disable
    vIO32WriteFldMulti(DRAMC_REG_DUMMY_RD, P_Fld(0x0, DUMMY_RD_DUMMY_RD_EN)
                                            | P_Fld(0x0, DUMMY_RD_SREF_DMYRD_EN)
                                            | P_Fld(0x0, DUMMY_RD_DQSG_DMYRD_EN)
                                            | P_Fld(0x0, DUMMY_RD_DMY_RD_DBG));

#if APPLY_POWER_INIT_SEQUENCE
    //CKE dynamic
    vIO32WriteFldMulti(DRAMC_REG_CKECTRL, P_Fld(0, CKECTRL_CKEFIXON) | P_Fld(0, CKECTRL_CKE1FIXON)
                                            | P_Fld(0, CKECTRL_CKEFIXOFF) | P_Fld(0, CKECTRL_CKE1FIXOFF));
    //// Enable  HW MIOCK control to make CLK dynamic
    vIO32WriteFldAlign(DRAMC_REG_DRAMC_PD_CTRL, 0, DRAMC_PD_CTRL_MIOCKCTRLOFF);
#endif

    //reset pad workaround
    vIO32WriteFldAlign(DDRPHY_CA_CMD8, 1, CA_CMD8_RG_TX_RRESETB_PULL_UP);
    mcDELAY_MS(2);
    vIO32WriteFldAlign(DDRPHY_MISC_CTRL1, 0, MISC_CTRL1_R_DMDA_RRESETB_E);

}

void vApplyConfigBeforeCalibration(DRAMC_CTX_T *p)
{
    //Clk free run
    EnableDramcPhyDCM(p, 0);

    //---- ZQ CS init --------
    vIO32WriteFldAlign(DRAMC_REG_SHU_SCINTV, 0x1f, SHU_SCINTV_TZQLAT); //ZQ Calibration Time, unit: 38.46ns, tZQCAL min is 1 us. need to set larger than 0x1b
    vIO32WriteFldAlign(DRAMC_REG_SHU_CONF3, 0x1ff, SHU_CONF3_ZQCSCNT); //Every refresh number to issue ZQCS commands, only for DDR3/LPDDR2/LPDDR3/LPDDR4
    vIO32WriteFldAlign(DRAMC_REG_DRAMCTRL, 0, DRAMCTRL_ZQCALL);  // HW send ZQ command for both rank, disable it due to some dram only have 1 ZQ pin for two rank.

    //ZQCSDUAL=0, ZQCSMASK=0
    vIO32WriteFldMulti(DRAMC_REG_ZQCS, P_Fld(0, ZQCS_ZQCSDUAL)| P_Fld(0x0, ZQCS_ZQCSMASK));


    // ---- End of ZQ CS init -----
    vIO32WriteFldAlign(DRAMC_REG_SHU1_WODT, p->DBI_W_onoff, SHU1_WODT_DBIWR);
    vIO32WriteFldAlign(DDRPHY_SHU1_B0_DQ7, p->DBI_R_onoff, SHU1_B0_DQ7_R_DMDQMDBI_SHU_B0);
    vIO32WriteFldAlign(DDRPHY_SHU1_B1_DQ7, p->DBI_R_onoff, SHU1_B1_DQ7_R_DMDQMDBI_SHU_B1);

    //disable MR4 read, REFRDIS=1
    vIO32WriteFldAlign(DRAMC_REG_SPCMDCTRL, 1, SPCMDCTRL_REFRDIS);

    // Disable ZQ
    vIO32WriteFldAlign(DRAMC_REG_SPCMDCTRL, 0, SPCMDCTRL_ZQCSDISB);   //ZQCSDISB=0
    vIO32WriteFldAlign(DRAMC_REG_SPCMDCTRL, 0, SPCMDCTRL_ZQCALDISB);   //ZQCALDISB=0

    // Disable HW gating tracking first, 0x1c0[31], need to disable both UI and PI tracking or the gating delay reg won't be valid.
    DramcHWGatingOnOff(p, 0);

    // Disable gating debug
    vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 0, EYESCAN_STB_GERRSTOP);

    // ARPI_DQ SW mode mux, TX DQ use 1: PHY Reg 0: DRAMC Reg
    vIO32WriteFldAlign(DDRPHY_MISC_CTRL1, 1, MISC_CTRL1_R_DMARPIDQ_SW);

    // Set to all-bank refresh
    vIO32WriteFldAlign(DRAMC_REG_REFCTRL0,  0, REFCTRL0_PBREFEN);

    //Disable HW Gating
    DramcHWGatingOnOff(p, 0);
}


void vApplyConfigBeforeCalibration_PC2(DRAMC_CTX_T *p)
{
	//Clk free run
	EnableDramcPhyDCM(p, 0);

#if 0
	//---- ZQ CS init --------
	vIO32WriteFldAlign(DRAMC_REG_SHU_SCINTV, 0x1f, SHU_SCINTV_TZQLAT); //ZQ Calibration Time, unit: 38.46ns, tZQCAL min is 1 us. need to set larger than 0x1b
	vIO32WriteFldAlign(DRAMC_REG_SHU_CONF3, 0x1ff, SHU_CONF3_ZQCSCNT); //Every refresh number to issue ZQCS commands, only for DDR3/LPDDR2/LPDDR3/LPDDR4
	vIO32WriteFldAlign(DRAMC_REG_DRAMCTRL, 0, DRAMCTRL_ZQCALL);  // HW send ZQ command for both rank, disable it due to some dram only have 1 ZQ pin for two rank.

	//ZQCSDUAL=0, ZQCSMASK=0
	vIO32WriteFldMulti(DRAMC_REG_ZQCS, P_Fld(0, ZQCS_ZQCSDUAL)| P_Fld(0x0, ZQCS_ZQCSMASK));
	// ---- End of ZQ CS init -----
#endif

	vIO32WriteFldAlign(DRAMC_REG_SHU1_WODT, p->DBI_W_onoff, SHU1_WODT_DBIWR);
	vIO32WriteFldAlign(DDRPHY_SHU1_B0_DQ7, p->DBI_R_onoff, SHU1_B0_DQ7_R_DMDQMDBI_SHU_B0);
	vIO32WriteFldAlign(DDRPHY_SHU1_B1_DQ7, p->DBI_R_onoff, SHU1_B1_DQ7_R_DMDQMDBI_SHU_B1);

	//disable MR4 read, REFRDIS=1
	vIO32WriteFldAlign(DRAMC_REG_SPCMDCTRL, 1, SPCMDCTRL_REFRDIS);

#if 1
	// Disable ZQ
	vIO32WriteFldAlign(DRAMC_REG_SPCMDCTRL, 0, SPCMDCTRL_ZQCSDISB);   //ZQCSDISB=0
	vIO32WriteFldAlign(DRAMC_REG_SPCMDCTRL, 0, SPCMDCTRL_ZQCALDISB);   //ZQCALDISB=0
#endif

	// Disable HW gating tracking first, 0x1c0[31], need to disable both UI and PI tracking or the gating delay reg won't be valid.
	DramcHWGatingOnOff(p, 0);

	// Disable gating debug
	vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 0, EYESCAN_STB_GERRSTOP);

	// ARPI_DQ SW mode mux, TX DQ use 1: PHY Reg 0: DRAMC Reg
	vIO32WriteFldAlign(DDRPHY_MISC_CTRL1, 1, MISC_CTRL1_R_DMARPIDQ_SW);

	// Set to all-bank refresh
	//byzj vIO32WriteFldAlign(DRAMC_REG_REFCTRL0,  0, REFCTRL0_PBREFEN);

	//Disable HW Gating
	DramcHWGatingOnOff(p, 0);
}

//Reset PHY to prevent glitch when change DQS gating delay or RX DQS input delay
// [Lynx] Everest :  All DramC and All Phy have to reset together.
void DramPhyReset(DRAMC_CTX_T *p)
{
    U32 backupReg0x64, backupReg0xC8, backupReg0xD0;

    backupReg0x64 = u4IO32Read4B(DRAMC_REG_SPCMDCTRL);
    backupReg0xC8 = u4IO32Read4B(DRAMC_REG_DQSOSCR);
    backupReg0xD0 = u4IO32Read4B(DRAMC_REG_DUMMY_RD);

    //Disable MR4 MR18/MR19, TxHWTracking, Dummy RD before reset
    vIO32WriteFldAlign(DRAMC_REG_SPCMDCTRL, 0x1, SPCMDCTRL_REFRDIS);    //MR4 Disable
    vIO32WriteFldMulti(DRAMC_REG_DQSOSCR, P_Fld(0x1, DQSOSCR_DQSOSCRDIS)|P_Fld(0x1, DQSOSCR_DQSOSCENDIS));  //MR18, MR19 Disable
    vIO32WriteFldMulti(DRAMC_REG_DUMMY_RD, P_Fld(0x0, DUMMY_RD_DUMMY_RD_EN)
                                            | P_Fld(0x0, DUMMY_RD_SREF_DMYRD_EN)
                                            | P_Fld(0x0, DUMMY_RD_DQSG_DMYRD_EN)
                                            | P_Fld(0x0, DUMMY_RD_DMY_RD_DBG));
    mcDELAY_US(4);

    //Everest change reset order : reset DQS before DQ, move PHY reset to final.
    {
        // Everest change : must reset all dramC and PHY together.
        //cc mark since it will cause DQS counter not update vIO32WriteFldAlign(DRAMC_REG_DDRCONF0, 1, DDRCONF0_DMSW_RST);
        vIO32WriteFldAlign(DRAMC_REG_DDRCONF0, 1, DDRCONF0_RDATRST);// read data counter reset
        vIO32WriteFldAlign(DDRPHY_MISC_CTRL1, 1, MISC_CTRL1_R_DMPHYRST);

        //RG_ARCMD_RESETB & RG_ARDQ_RESETB_B0/1 only reset once at init, Justin Chan.
        vIO32WriteFldMulti(DDRPHY_B0_DQ3, P_Fld(0, B0_DQ3_RG_RX_ARDQS0_STBEN_RESETB) |P_Fld(0, B0_DQ3_RG_RX_ARDQ_STBEN_RESETB_B0));
        vIO32WriteFldMulti(DDRPHY_B1_DQ3, P_Fld(0, B1_DQ3_RG_RX_ARDQS1_STBEN_RESETB) |P_Fld(0, B1_DQ3_RG_RX_ARDQ_STBEN_RESETB_B1));
        mcDELAY_US(1);//delay 10ns
        vIO32WriteFldMulti(DDRPHY_B1_DQ3, P_Fld(1, B1_DQ3_RG_RX_ARDQS1_STBEN_RESETB) |P_Fld(1, B1_DQ3_RG_RX_ARDQ_STBEN_RESETB_B1));
        vIO32WriteFldMulti(DDRPHY_B0_DQ3, P_Fld(1, B0_DQ3_RG_RX_ARDQS0_STBEN_RESETB) |P_Fld(1, B0_DQ3_RG_RX_ARDQ_STBEN_RESETB_B0));

        vIO32WriteFldAlign(DDRPHY_MISC_CTRL1, 0, MISC_CTRL1_R_DMPHYRST);
        vIO32WriteFldAlign(DRAMC_REG_DDRCONF0, 0, DDRCONF0_RDATRST);// read data counter reset
        //cc mark since it will cause DQS counter not update vIO32WriteFldAlign(DRAMC_REG_DDRCONF0, 0, DDRCONF0_DMSW_RST);
    }

    //Restore backup regs
    vIO32Write4B(DRAMC_REG_SPCMDCTRL, backupReg0x64);
    vIO32Write4B(DRAMC_REG_DQSOSCR, backupReg0xC8);
    vIO32Write4B(DRAMC_REG_DUMMY_RD, backupReg0xD0);
}


#if 0// cc mark
void DramEyeStbenReset(DRAMC_CTX_T *p)
{
    vIO32WriteFldAlign_Phy_All(DDRPHY_B0_DQ5, 0, B0_DQ5_RG_RX_ARDQ_EYE_STBEN_RESETB_B0);
    vIO32WriteFldAlign_Phy_All(DDRPHY_B1_DQ5, 0, B1_DQ5_RG_RX_ARDQ_EYE_STBEN_RESETB_B1);
    vIO32WriteFldAlign_Phy_All(DDRPHY_B1_DQ5, 0, CA_CMD5_RG_RX_ARCMD_EYE_STBEN_RESETB); //only in LP3 due to DQ pinmux to CA

    mcDELAY_US(1);//delay 10ns

    vIO32WriteFldAlign_Phy_All(DDRPHY_B0_DQ5, 1, B0_DQ5_RG_RX_ARDQ_EYE_STBEN_RESETB_B0);
    vIO32WriteFldAlign_Phy_All(DDRPHY_B1_DQ5, 1, B1_DQ5_RG_RX_ARDQ_EYE_STBEN_RESETB_B1);
    vIO32WriteFldAlign_Phy_All(DDRPHY_B1_DQ5, 1, CA_CMD5_RG_RX_ARCMD_EYE_STBEN_RESETB);//only in LP3 due to DQ pinmux to CA
}
#endif

DRAM_STATUS_T DramcRankSwap(DRAMC_CTX_T *p, U8 u1Rank)
{
    U8 u1Multi;

    if (p->support_rank_num > 1)
        u1Multi = 1;
    else
        u1Multi = 0;

    mcSHOW_DBG_MSG2(("[DramcRankSwap] Rank number %d, (u1Multi %d), Rank %d\n", p->support_rank_num, u1Multi, u1Rank));

    //Set to non-zero for multi-rank
    vIO32WriteFldAlign(DRAMC_REG_RKCFG, (u1Rank<<3) | u1Multi, RKCFG_RKMODE);

    if (u1Rank == 0)
    {
        vIO32WriteFldAlign(DRAMC_REG_RKCFG, 0, RKCFG_TXRANKFIX);
    }
    else
    {
        vIO32WriteFldAlign(DRAMC_REG_RKCFG, 1, RKCFG_TXRANKFIX);
    }
    vIO32WriteFldAlign(DRAMC_REG_RKCFG, u1Rank, RKCFG_TXRANK); //use other rank's setting

    return DRAM_OK;
}

//cc add for jitter meter from Azalea
#if ENABLE_MIOCK_JMETER
#define duty_cal	1
#define PI_sweep	0
#define duty_sweep	1
#if duty_cal
typedef struct _Edge_JM_DATA_T
{
    U8 large_trans_tap;
    U8 small_trans_tap;
    U32 large_trans_cnt;
	U32 small_trans_cnt;
} Edge_JM_DATA_T;
#endif

u16 u2gdelay_cell_ps = 0;
u8 ucg_num_dlycell_perT = 0;
#if COMPILE_THIS_PART
DRAM_STATUS_T DramcMiockJmeter(DRAMC_CTX_T *p)
{
	U8 u1ByteIdx;
	U32 u4RevBX[DQS_NUMBER];
	U8 ucsearch_state, ucdqs_dly, fgcurrent_value, fginitial_value, ucstart_period, ucend_period=0;
	U32 u4sample_cnt, u4ones_cnt[DQS_NUMBER], u4MPDIV_IN_SEL;
	U16 u2real_freq, u2real_period;
	U32 u4prv_register_B0_DQ6, u4prv_register_B1_DQ6;
	U32 u4prv_register_B0_DQ5, u4prv_register_B1_DQ5;
	U32 u4prv_register_SHU1_R0B0_DQ6, u4prv_register_SHU1_R0B1_DQ6;
	U32 u4prv_register_B0_DQ3, u4prv_register_B1_DQ3;
	U32 u4prv_register_CA_CMD4, u4prv_register_CA_CMD6;
	U32 u4prv_register_SHU1_B0_DQ7, u4prv_register_SHU1_B1_DQ7;
	U32 u4prv_register_MISC_CTRL1;
	U32 u4prv_register_EYESCAN, u4prv_register_STBCAL1;
	U32 u4prv_register_PD_CTL;

	U8 u1ShuLevel;
	U32 u4PLL5_ADDR;
	U32 u4PLL8_ADDR;
	U32 u4CA_CMD6;
	U32 u4SDM_PCW;
	U32 u4PREDIV;
	U32 u4POSDIV;
	U32 u4EXTFBDIV;
	U32 u4CKDIV4;
	U32 u4VCOFreq;
	U32 u4DataRate;

	u2gdelay_cell_ps=0;

	// error handling
	if (!p)
	{
		mcSHOW_ERR_MSG(("context NULL\n"));
		return DRAM_FAIL;
	}

	mcSHOW_DBG_MSG(("[DramcMiockJmeter] ====Begin====\n"));
	#if 1//dbg_print
	prom_puts("[DramcMiockJmeter] ====Begin====\n");
	#endif

	//backup register value
	u4prv_register_EYESCAN = u4IO32Read4B(DRAMC_REG_EYESCAN);
	u4prv_register_STBCAL1 = u4IO32Read4B(DRAMC_REG_STBCAL1);

	u4prv_register_B0_DQ6 = u4IO32Read4B(DDRPHY_B0_DQ6);
	u4prv_register_B1_DQ6 = u4IO32Read4B(DDRPHY_B1_DQ6);
	u4prv_register_B0_DQ5 = u4IO32Read4B(DDRPHY_B0_DQ5);
	u4prv_register_B1_DQ5 = u4IO32Read4B(DDRPHY_B1_DQ5);
	u4prv_register_B0_DQ3 = u4IO32Read4B(DDRPHY_B0_DQ3);
	u4prv_register_B1_DQ3 = u4IO32Read4B(DDRPHY_B1_DQ3);
	u4prv_register_SHU1_B0_DQ7 = u4IO32Read4B(DDRPHY_SHU1_B0_DQ7);
	u4prv_register_SHU1_B1_DQ7 = u4IO32Read4B(DDRPHY_SHU1_B1_DQ7);
#if 0
	u4prv_register_B0_DQ4 = u4IO32Read4B(DDRPHY_B0_DQ4);
	u4prv_register_B1_DQ4 = u4IO32Read4B(DDRPHY_B1_DQ4);
#endif
	u4prv_register_SHU1_R0B0_DQ6 = u4IO32Read4B(DDRPHY_SHU1_R0_B0_DQ6);
	u4prv_register_SHU1_R0B1_DQ6 = u4IO32Read4B(DDRPHY_SHU1_R0_B1_DQ6);
	u4prv_register_MISC_CTRL1 = u4IO32Read4B(DDRPHY_MISC_CTRL1);
	u4prv_register_PD_CTL = u4IO32Read4B(DRAMC_REG_DRAMC_PD_CTRL);

	//MCK4X CG
	vIO32WriteFldAlign(DDRPHY_MISC_CTRL1, 0, MISC_CTRL1_R_DMDQSIENCG_EN);

	// Bypass DQS glitch-free mode
	// RG_RX_*RDQ_EYE_DLY_DQS_BYPASS_B**
	vIO32WriteFldAlign(DDRPHY_B0_DQ6, 1, B0_DQ6_RG_RX_ARDQ_EYE_DLY_DQS_BYPASS_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ6, 1, B1_DQ6_RG_RX_ARDQ_EYE_DLY_DQS_BYPASS_B1);
	vIO32WriteFldAlign(DRAMC_REG_DRAMC_PD_CTRL, 0x0, DRAMC_PD_CTRL_DCMEN);

	//Enable DQ eye scan
	//RG_??_RX_EYE_SCAN_EN
	//RG_??_RX_VREF_EN
	//RG_??_RX_SMT_EN
	#ifdef WHITNEY_USE
	vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 1, EYESCAN_RG_RX_EYE_SCAN_EN);
	#else
	//TODO, double confirm,zj
	vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 1, EYESCAN_RG_EX_EYE_SCAN_EN);
	#endif
	vIO32WriteFldAlign(DDRPHY_B0_DQ5, 1, B0_DQ5_RG_RX_ARDQ_EYE_EN_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ5, 1, B1_DQ5_RG_RX_ARDQ_EYE_EN_B1);
	vIO32WriteFldAlign(DDRPHY_B0_DQ5, 1, B0_DQ5_RG_RX_ARDQ_VREF_EN_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ5, 1, B1_DQ5_RG_RX_ARDQ_VREF_EN_B1);
	//zj mark vIO32WriteFldAlign(DDRPHY_B0_DQ3), 1, B0_DQ3_RG_RX_ARDQ_SMT_EN_B0);
	//zj mark vIO32WriteFldAlign(DDRPHY_B1_DQ3), 1, B1_DQ3_RG_RX_ARDQ_SMT_EN_B1);
	vIO32WriteFldAlign(DDRPHY_B0_DQ3, 0, B0_DQ3_RG_RX_ARDQ_SMT_EN_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ3, 0, B1_DQ3_RG_RX_ARDQ_SMT_EN_B1);

	vIO32WriteFldAlign(DDRPHY_B0_DQ8, 0x1, B0_DQ8_RG_T2RLPBK_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ8, 0x1, B1_DQ8_RG_T2RLPBK_B1);

#if 0
	//JM_SEL: MCK4X_CLK
	vIO32WriteFldAlign(DDRPHY_B0_DQ6, 0x1, B0_DQ6_RG_RX_ARDQ_JM_SEL_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ6, 0x1, B1_DQ6_RG_RX_ARDQ_JM_SEL_B1);
#else
	//JM_SEL: MCK4X_CLK
	vIO32WriteFldAlign(DDRPHY_B0_DQ6, 0x0, B0_DQ6_RG_RX_ARDQ_JM_SEL_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ6, 0x0, B1_DQ6_RG_RX_ARDQ_JM_SEL_B1);
#endif

	//Enable MIOCK jitter meter mode ( RG_RX_MIOCK_JIT_EN=1)
	vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 1, EYESCAN_RG_RX_MIOCK_JIT_EN);

	//Disable DQ eye scan (b'1), for counter clear
	#ifdef WHITNEY_USE
	vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 0, EYESCAN_RG_RX_EYE_SCAN_EN);
	#else
	vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 0, EYESCAN_RG_EX_EYE_SCAN_EN);
	#endif
	vIO32WriteFldAlign(DRAMC_REG_STBCAL1, 0, STBCAL1_DQSERRCNT_DIS);

	ucsearch_state = 0;
	for (ucdqs_dly=0; ucdqs_dly<128; ucdqs_dly++)
	{

		//Set DQS delay (RG_??_RX_DQS_EYE_DLY)
	#if 0
		vIO32WriteFldAlign(DDRPHY_B0_DQ4, ucdqs_dly, B0_DQ4_RG_RX_ARDQS_EYE_R_DLY_B0);
		vIO32WriteFldAlign(DDRPHY_B0_DQ4, ucdqs_dly, B0_DQ4_RG_RX_ARDQS_EYE_F_DLY_B0);
		vIO32WriteFldAlign(DDRPHY_B1_DQ4, ucdqs_dly, B1_DQ4_RG_RX_ARDQS_EYE_R_DLY_B1);
		vIO32WriteFldAlign(DDRPHY_B1_DQ4, ucdqs_dly, B1_DQ4_RG_RX_ARDQS_EYE_F_DLY_B1);
	#endif		
		vIO32WriteFldAlign(DDRPHY_SHU1_R0_B0_DQ6, ucdqs_dly, SHU1_R0_B0_DQ6_RK0_RX_ARDQS0_F_DLY_B0);
		vIO32WriteFldAlign(DDRPHY_SHU1_R0_B0_DQ6, ucdqs_dly, SHU1_R0_B0_DQ6_RK0_RX_ARDQS0_R_DLY_B0);
		vIO32WriteFldAlign(DDRPHY_SHU1_R0_B1_DQ6, ucdqs_dly, SHU1_R0_B1_DQ6_RK0_RX_ARDQS0_F_DLY_B1);
		vIO32WriteFldAlign(DDRPHY_SHU1_R0_B1_DQ6, ucdqs_dly, SHU1_R0_B1_DQ6_RK0_RX_ARDQS0_R_DLY_B1);

		DramPhyReset(p);

		//Reset eye scan counters (reg_sw_rst): 1 to 0
		vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 1, EYESCAN_REG_SW_RST);
		vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 0, EYESCAN_REG_SW_RST);

		//Enable DQ eye scan (b'1)
		#ifdef WHITNEY_USE
		vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 1, EYESCAN_RG_RX_EYE_SCAN_EN);
		#else
		vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 1, EYESCAN_RG_EX_EYE_SCAN_EN);
		#endif

		#if FOR_DV_SIMULATION
			mcDELAY_US(1);
		#else
		// 2ns/sample, here we delay 1ms about 500 samples
		mcDELAY_US(1000);
		#endif

		//Disable DQ eye scan (b'1), for counter latch
		#ifdef WHITNEY_USE
		vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 0, EYESCAN_RG_RX_EYE_SCAN_EN);
		#else
		vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 0, EYESCAN_RG_EX_EYE_SCAN_EN);
		#endif

		//Read the counter values from registers (toggle_cnt*, dqs_err_cnt*);
		u4sample_cnt = u4IO32ReadFldAlign(DRAMC_REG_TOGGLE_CNT, TOGGLE_CNT_TOGGLE_CNT);
		u4ones_cnt[0] = u4IO32ReadFldAlign(DRAMC_REG_DQS0_ERR_CNT, DQS0_ERR_CNT_DQS0_ERR_CNT);
		u4ones_cnt[1] = u4IO32ReadFldAlign(DRAMC_REG_DQS1_ERR_CNT, DQS1_ERR_CNT_DQS1_ERR_CNT);
		u4ones_cnt[2] = u4IO32ReadFldAlign(DRAMC_REG_DQS2_ERR_CNT, DQS2_ERR_CNT_DQS2_ERR_CNT);
		u4ones_cnt[3] = u4IO32ReadFldAlign(DRAMC_REG_DQS3_ERR_CNT, DQS3_ERR_CNT_DQS3_ERR_CNT);
		#ifdef ETT_PRINT_FORMAT
		mcSHOW_DBG_MSG(("%d : %d, %d, %d, %d, %d\n", ucdqs_dly, u4sample_cnt, u4ones_cnt[0],u4ones_cnt[1],u4ones_cnt[2],u4ones_cnt[3]));
		#else
		mcSHOW_DBG_MSG(("%3d : %8d, %8d, %8d, %8d, %8d\n", ucdqs_dly, u4sample_cnt, u4ones_cnt[0], u4ones_cnt[1], u4ones_cnt[2], u4ones_cnt[3]));
		#if dbg_print
		prom_print_dec(ucdqs_dly);
		prom_puts(": ");
		prom_print_dec(u4sample_cnt);
		prom_puts(", ");
		prom_print_dec(u4ones_cnt[0]);
		prom_puts(", ");
		prom_print_dec(u4ones_cnt[1]);
		prom_puts(", ");
		prom_print_dec(u4ones_cnt[2]);
		prom_puts(", ");
		prom_print_dec(u4ones_cnt[3]);
		prom_puts("\n");
		#endif
		#endif

		/*
		//Disable DQ eye scan (RG_RX_EYE_SCAN_EN=0, RG_RX_*RDQ_VREF_EN_B*=0, RG_RX_*RDQ_EYE_VREF_EN_B*=0, RG_RX_*RDQ_SMT_EN_B*=0)
		vIO32WriteFldAlign(DRAMC_REG_STBCAL_F), 0, STBCAL_F_RG_EX_EYE_SCAN_EN);
		vIO32WriteFldAlign(DDRPHY_EYE2), 0, EYE2_RG_RX_ARDQ_VREF_EN_B0);
		vIO32WriteFldAlign(DDRPHY_EYEB1_2), 0, EYEB1_2_RG_RX_ARDQ_VREF_EN_B1);
		vIO32WriteFldAlign(DDRPHY_EYE2), 0, EYE2_RG_RX_ARDQ_EYE_VREF_EN_B0);
		vIO32WriteFldAlign(DDRPHY_EYEB1_2), 0, EYEB1_2_RG_RX_ARDQ_EYE_VREF_EN_B1);
		vIO32WriteFldAlign(DDRPHY_TXDQ3), 0, TXDQ3_RG_RX_ARDQ_SMT_EN_B0);
		vIO32WriteFldAlign(DDRPHY_RXDQ13),0, RXDQ13_RG_RX_ARDQ_SMT_EN_B1);
		*/

		//change to boolean value
		if (u4ones_cnt[0] < (u4sample_cnt/2))
		{
			fgcurrent_value = 0;
		}
		else
		{
			fgcurrent_value = 1;
		}

		#if 1//more than 1T data
		{
			if (ucsearch_state==0)
			{
				//record initial value at the beginning
				fginitial_value = fgcurrent_value;
				ucsearch_state = 1;
			}
			else if (ucsearch_state==1)
			{
				// check if change value
				if (fgcurrent_value != fginitial_value)
				{
					// start of the period
					fginitial_value = fgcurrent_value;
					ucstart_period = ucdqs_dly;
					ucsearch_state = 2;
				}
			}
			else if (ucsearch_state==2)
			{
				// check if change value
				if (fgcurrent_value != fginitial_value)
				{
					fginitial_value = fgcurrent_value;
					ucsearch_state = 3;
				}
			}
			else if (ucsearch_state==3)
			{
				// check if change value
				if (fgcurrent_value != fginitial_value)
				{
					// end of the period, break the loop
					ucend_period = ucdqs_dly;
					ucsearch_state = 4;
					break;
				}
			}
			else
			{
				//nothing
			}
		}
		#else //only 0.5T data
		{
			if (ucsearch_state==0)
			{
				//record initial value at the beginning
				fginitial_value = fgcurrent_value;
				ucsearch_state = 1;
			}
			else if (ucsearch_state==1)
			{
				// check if change value
				if (fgcurrent_value != fginitial_value)
				{
					// start of the period
					fginitial_value = fgcurrent_value;
					ucstart_period = ucdqs_dly;
					ucsearch_state = 2;
				}
			}
			else if (ucsearch_state==2)
			{
				// check if change value
				if (fgcurrent_value != fginitial_value)
				{
					// end of the period, break the loop
					ucend_period = ucdqs_dly;
					ucsearch_state = 4;
					break;
				}
			}
		}
		#endif
	}

	//restore to orignal value
	vIO32Write4B(DRAMC_REG_EYESCAN, u4prv_register_EYESCAN);
	vIO32Write4B(DRAMC_REG_STBCAL1, u4prv_register_STBCAL1);

	vIO32Write4B(DDRPHY_B0_DQ6, u4prv_register_B0_DQ6);
	vIO32Write4B(DDRPHY_B1_DQ6, u4prv_register_B1_DQ6);
	vIO32Write4B(DDRPHY_B0_DQ5, u4prv_register_B0_DQ5);
	vIO32Write4B(DDRPHY_B1_DQ5, u4prv_register_B1_DQ5);
#if 0
	vIO32Write4B(DDRPHY_B0_DQ4, u4prv_register_B0_DQ4);
	vIO32Write4B(DDRPHY_B1_DQ4, u4prv_register_B1_DQ4);
#endif
	vIO32Write4B(DDRPHY_SHU1_R0_B0_DQ6, u4prv_register_SHU1_R0B0_DQ6);
	vIO32Write4B(DDRPHY_SHU1_R0_B1_DQ6, u4prv_register_SHU1_R0B1_DQ6);

	vIO32Write4B(DDRPHY_B0_DQ3, u4prv_register_B0_DQ3);
	vIO32Write4B(DDRPHY_B1_DQ3, u4prv_register_B1_DQ3);
	vIO32Write4B(DDRPHY_SHU1_B0_DQ7, u4prv_register_SHU1_B0_DQ7);
	vIO32Write4B(DDRPHY_SHU1_B1_DQ7, u4prv_register_SHU1_B1_DQ7);
	vIO32Write4B(DDRPHY_MISC_CTRL1, u4prv_register_MISC_CTRL1);
	vIO32Write4B(DRAMC_REG_DRAMC_PD_CTRL, u4prv_register_PD_CTL);

	if(ucsearch_state!=4)
	{
		mcSHOW_DBG_MSG(("\n\tMIOCK jitter meter\n"));
		mcSHOW_DBG_MSG(("\tLess than 0.5T data. Cannot calculate delay cell time\n\n"));
		#if 1//dbg_print
		prom_puts("\nMIOCK jitter meter\nLess than 0.5T data. Cannot calculate delay cell time\n\n");
		#endif
		return DRAM_FAIL;
	}

	//Calculate 1 delay cell = ? ps
	// 1T = ? delay cell
	ucg_num_dlycell_perT = (ucend_period - ucstart_period);
	// 1T = ? ps

#ifdef WHITNEY_USE
	u1ShuLevel = u4IO32ReadFldAlign(DRAMC_REG_SHUSTATUS, SHUSTATUS_SHUFFLE_LEVEL);
#else
	u1ShuLevel = 0;
#endif

#if 1 //cc mark
	//zj add begin, azalea only uses CLRPLL.
	u4SDM_PCW = u4IO32ReadFldAlign(DDRPHY_SHU1_PLL7, SHU1_PLL7_RG_RCLRPLL_SDM_PCW);
	u4PREDIV = u4IO32ReadFldAlign(DDRPHY_SHU1_PLL10, SHU1_PLL10_RG_RCLRPLL_PREDIV);
	u4POSDIV = u4IO32ReadFldAlign(DDRPHY_SHU1_PLL10, SHU1_PLL10_RG_RCLRPLL_POSDIV);
	u4EXTFBDIV = u4IO32ReadFldAlign(DDRPHY_SHU1_PLL11, SHU1_PLL11_RG_CLRPLL_FBDIV_SEL);
	//u4VCOFreq = (((u4SDM_PCW*1000 >> 9)*(26 >> u4PREDIV)) >> u4POSDIV << u4EXTFBDIV)/1000;

	u4VCOFreq = (((u4SDM_PCW >> 24) * (25 >> u4PREDIV)) >> u4POSDIV << u4EXTFBDIV);
	u2real_freq = u4VCOFreq>>1;
	//zj add end
	
	u2real_period = (U16) (1000000/u2real_freq);//cc: ps
	//calculate delay cell time

	u2gdelay_cell_ps = u2real_period*100 / ucg_num_dlycell_perT;

	mcSHOW_DBG_MSG(("\n\tMIOCK jitter meter\n\n"
		"1T = (%d-%d)*2 = %d dly cells\n"
		"Clock freq = %d MHz, period = %d ps, 1 dly cell = %d/100 ps\n",
		ucend_period, ucstart_period, ucg_num_dlycell_perT,
		u2real_freq, u2real_period, u2gdelay_cell_ps));
	#if 1//dbg_print
	prom_puts("\nMIOCK jitter meter\n\n1T = (");
	prom_print_dec(ucend_period);
	prom_puts("-");
	prom_print_dec(ucstart_period);
	prom_puts(") = ");
	prom_print_dec(ucg_num_dlycell_perT);
	prom_puts(" dly cells\nClock freq = ");
	prom_print_dec(u2real_freq);
	prom_puts(" MHz, period = ");
	prom_print_dec(u2real_period);
	prom_puts(" ps, 1 dly cell = ");
	prom_print_dec(u2gdelay_cell_ps);
	prom_puts("/100 ps\n");
	#endif

	mcSHOW_DBG_MSG(("[DramcMiockJmeter] ====Done====\n"));
	prom_puts("[DramcMiockJmeter] ====Done====\n");
#endif	
	return DRAM_OK;

// log example
/* dly: sample_cnt   DQS0_cnt  DQS1_cnt
    0 : 10962054,        0,        0
    1 : 10958229,        0,        0
    2 : 10961109,        0,        0
    3 : 10946916,        0,        0
    4 : 10955421,        0,        0
    5 : 10967274,        0,        0
    6 : 10893582,        0,        0
    7 : 10974762,        0,        0
    8 : 10990278,        0,        0
    9 : 10972026,        0,        0
   10 :  7421004,        0,        0
   11 : 10943883,        0,        0
   12 : 10984275,        0,        0
   13 : 10955268,        0,        0
   14 : 10960326,        0,        0
   15 : 10952451,        0,        0
   16 : 10956906,        0,        0
   17 : 10960803,        0,        0
   18 : 10944108,        0,        0
   19 : 10959939,        0,        0
   20 : 10959246,        0,        0
   21 : 11002212,        0,        0
   22 : 10919700,        0,        0
   23 : 10977489,        0,        0
   24 : 11009853,        0,        0
   25 : 10991133,        0,        0
   26 : 10990431,        0,        0
   27 : 10970703,    11161,        0
   28 : 10970775,   257118,        0
   29 : 10934442,  9450467,        0
   30 : 10970622, 10968475,        0
   31 : 10968831, 10968831,        0
   32 : 10956123, 10956123,        0
   33 : 10950273, 10950273,        0
   34 : 10975770, 10975770,        0
   35 : 10983024, 10983024,        0
   36 : 10981701, 10981701,        0
   37 : 10936782, 10936782,        0
   38 : 10889523, 10889523,        0
   39 : 10985913, 10985913,    55562
   40 : 10970235, 10970235,   272294
   41 : 10996056, 10996056,  9322868
   42 : 10972350, 10972350, 10969738
   43 : 10963917, 10963917, 10963917
   44 : 10967895, 10967895, 10967895
   45 : 10961739, 10961739, 10961739
   46 : 10937097, 10937097, 10937097
   47 : 10937952, 10937952, 10937952
   48 : 10926018, 10926018, 10926018
   49 : 10943793, 10943793, 10943793
   50 : 10954638, 10954638, 10954638
   51 : 10968048, 10968048, 10968048
   52 : 10944036, 10944036, 10944036
   53 : 11012112, 11012112, 11012112
   54 : 10969137, 10969137, 10969137
   55 : 10968516, 10968516, 10968516
   56 : 10952532, 10952532, 10952532
   57 : 10985832, 10985832, 10985832
   58 : 11002527, 11002527, 11002527
   59 : 10950660, 10873571, 10950660
   60 : 10949022, 10781797, 10949022
   61 : 10974366, 10700617, 10974366
   62 : 10972422,  1331974, 10972422
   63 : 10926567,        0, 10926567
   64 : 10961658,        0, 10961658
   65 : 10978893,        0, 10978893
   66 : 10962828,        0, 10962828
   67 : 10957599,        0, 10957599
   68 : 10969227,        0, 10969227
   69 : 10960722,        0, 10960722
   70 : 10970937,        0, 10963180
   71 : 10962054,        0, 10711639
   72 : 10954719,        0, 10612707
   73 : 10958778,        0,   479589
   74 : 10973898,        0,        0
   75 : 11004156,        0,        0
   76 : 10944261,        0,        0
   77 : 10955340,        0,        0
   78 : 10998153,        0,        0
   79 : 10998774,        0,        0
   80 : 10953234,        0,        0
   81 : 10960020,        0,        0
   82 : 10923831,        0,        0
   83 : 10951362,        0,        0
   84 : 10965249,        0,        0
   85 : 10949103,        0,        0
   86 : 10948707,        0,        0
   87 : 10941147,        0,        0
   88 : 10966572,        0,        0
   89 : 10971333,        0,        0
   90 : 10943721,        0,        0
   91 : 10949337,        0,        0
   92 : 10965942,        0,        0
   93 : 10970397,        0,        0
   94 : 10956429,        0,        0
   95 : 10939896,        0,        0
   96 : 10967112,        0,        0
   97 : 10951911,        0,        0
   98 : 10953702,        0,        0
   99 : 10971090,        0,        0
  100 : 10939590,        0,        0
  101 : 10993392,        0,        0
  102 : 10975932,        0,        0
  103 : 10949499,    40748,        0
  104 : 10962522,   258638,        0
  105 : 10951524,   275292,        0
  106 : 10982475,   417642,        0
  107 : 10966887, 10564347,        0
  ===============================================================================
      MIOCK jitter meter - channel=0
  ===============================================================================
  1T = (107-29) = 78 delay cells
  Clock frequency = 936 MHz, Clock period = 1068 ps, 1 delay cell = 13 ps
*/
}
#endif
DRAM_STATUS_T DramcMiockJmeter_CLK(DRAMC_CTX_T *p)
{
	U8 u1ByteIdx;
	U32 u4RevBX[DQS_NUMBER];
	U8 ucsearch_state, ucdqs_dly, fgcurrent_value, fginitial_value, ucstart_period, ucend_period=0;
	U32 u4sample_cnt, u4ones_cnt, u4MPDIV_IN_SEL;
	U16 u2real_freq, u2real_period;
	U32 u4prv_register_B0_DQ6, u4prv_register_B1_DQ6;
	U32 u4prv_register_B0_DQ5, u4prv_register_B1_DQ5;
	U32 u4prv_register_SHU1_R0B0_DQ6, u4prv_register_SHU1_R0B1_DQ6;
	U32 u4prv_register_B0_DQ3, u4prv_register_B1_DQ3;
	U32 u4prv_register_CA_CMD4, u4prv_register_CA_CMD6;
	U32 u4prv_register_SHU1_B0_DQ7, u4prv_register_SHU1_B1_DQ7;
	U32 u4prv_register_MISC_CTRL1;
	U32 u4prv_register_EYESCAN, u4prv_register_STBCAL1;
	U32 u4prv_register_PD_CTL;
	U32 u4prv_register_MISC_CG_CTRL4;
	U32 u4prv_register_CA_MISC;
	U32 u4prv_register_CA_CMD3;
	#if 1//PI_sweep
	U32 u4prv_register_CA_CMD9;
	U8	ucPI_dly;
	#endif
	#if duty_sweep
	U32 u4prv_register_CA_CMD10;
	U16	u2duty_opt, u2duty_diff, u2duty_diff_opt = 0xffff;
	U8	ucDuty_dly, ucMCK4X_DLY_EN, ucDuty_dly_opt, ucMCK4X_DLY_EN_opt;
	#endif
	U8 u1ShuLevel;
	U32 u4PLL5_ADDR;
	U32 u4PLL8_ADDR;
	U32 u4CA_CMD6;
	U32 u4SDM_PCW;
	U32 u4PREDIV;
	U32 u4POSDIV;
	U32 u4EXTFBDIV;
	U32 u4CKDIV4;
	U32 u4VCOFreq;
	U32 u4DataRate;
	#if duty_cal
	U8 ucFirstStep_cnt, uc2ndStep_cnt;
	U16 u2duty, mid_ratio;
	U8 ucFlag_LowStepFirst, ii;
	Edge_JM_DATA_T	edge[3];
	U32 trans_cnt_tmp;
	U32 edge_pos[3];
	#endif
	
	u2gdelay_cell_ps=0;

	// error handling
	if (!p)
	{
		mcSHOW_ERR_MSG(("context NULL\n"));
		return DRAM_FAIL;
	}

	mcSHOW_DBG_MSG(("[DramcMiockJmeter_CLK] ====Begin====\n"));
	prom_puts("[DramcMiockJmeter_CLK] ====Begin====\n");
	//backup register value
	u4prv_register_EYESCAN = u4IO32Read4B(DRAMC_REG_EYESCAN);
	u4prv_register_STBCAL1 = u4IO32Read4B(DRAMC_REG_STBCAL1);

	u4prv_register_B0_DQ6 = u4IO32Read4B(DDRPHY_B0_DQ6);
	u4prv_register_B1_DQ6 = u4IO32Read4B(DDRPHY_B1_DQ6);
	u4prv_register_B0_DQ5 = u4IO32Read4B(DDRPHY_B0_DQ5);
	u4prv_register_B1_DQ5 = u4IO32Read4B(DDRPHY_B1_DQ5);
	u4prv_register_B0_DQ3 = u4IO32Read4B(DDRPHY_B0_DQ3);
	u4prv_register_B1_DQ3 = u4IO32Read4B(DDRPHY_B1_DQ3);
	u4prv_register_SHU1_B0_DQ7 = u4IO32Read4B(DDRPHY_SHU1_B0_DQ7);
	u4prv_register_SHU1_B1_DQ7 = u4IO32Read4B(DDRPHY_SHU1_B1_DQ7);
#if 0
	u4prv_register_B0_DQ4 = u4IO32Read4B(DDRPHY_B0_DQ4);
	u4prv_register_B1_DQ4 = u4IO32Read4B(DDRPHY_B1_DQ4);
#endif
	u4prv_register_SHU1_R0B0_DQ6 = u4IO32Read4B(DDRPHY_SHU1_R0_B0_DQ6);
	u4prv_register_SHU1_R0B1_DQ6 = u4IO32Read4B(DDRPHY_SHU1_R0_B1_DQ6);
	u4prv_register_MISC_CTRL1 = u4IO32Read4B(DDRPHY_MISC_CTRL1);
	u4prv_register_PD_CTL = u4IO32Read4B(DRAMC_REG_DRAMC_PD_CTRL);
	u4prv_register_MISC_CG_CTRL4 = u4IO32Read4B(DDRPHY_MISC_CG_CTRL4);
	u4prv_register_CA_MISC = u4IO32Read4B(DDRPHY_CA_MISC);
	u4prv_register_CA_CMD3 = u4IO32Read4B(DDRPHY_CA_CMD3);
	#if PI_sweep
	u4prv_register_CA_CMD9 = u4IO32Read4B(DDRPHY_SHU1_R0_CA_CMD9);
	#endif
	#if duty_sweep
	u4prv_register_CA_CMD10 = u4IO32Read4B(DDRPHY_SHU1_R0_CA_CMD10);
	#endif
	
	//MCK4X CG
	vIO32WriteFldAlign(DDRPHY_MISC_CTRL1, 0, MISC_CTRL1_R_DMDQSIENCG_EN);
	vIO32WriteFldAlign(DDRPHY_MISC_CG_CTRL4, 0x0, Fld(1, 22, AC_MSKB2));
	
	// Bypass DQS glitch-free mode
	// RG_RX_*RDQ_EYE_DLY_DQS_BYPASS_B**
	//...vIO32WriteFldAlign(DDRPHY_B0_DQ6, 1, B0_DQ6_RG_RX_ARDQ_EYE_DLY_DQS_BYPASS_B0);
	//...vIO32WriteFldAlign(DDRPHY_B1_DQ6, 1, B1_DQ6_RG_RX_ARDQ_EYE_DLY_DQS_BYPASS_B1);
	vIO32WriteFldAlign(DRAMC_REG_DRAMC_PD_CTRL, 0x0, DRAMC_PD_CTRL_DCMEN);

	//Enable DQ eye scan
	//RG_??_RX_EYE_SCAN_EN
	//RG_??_RX_VREF_EN
	//RG_??_RX_SMT_EN
	#ifdef WHITNEY_USE
	vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 1, EYESCAN_RG_RX_EYE_SCAN_EN);
	#else
	//TODO, double confirm,zj
	vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 1, EYESCAN_RG_EX_EYE_SCAN_EN);
	#endif
	//new setting
	vIO32WriteFldMulti(DDRPHY_CA_MISC, P_Fld(1, CA_MISC_RG_ARPI_SMT_XLATCH_FORCE_CLK) | P_Fld(1, CA_MISC_RG_ARPI_PSMUX_XLATCH_FORCE_CLK) | 
			P_Fld(1, CA_MISC_RG_ARPI_BUFGP_XLATCH_FORCE_CLK) | P_Fld(1, CA_MISC_RG_ARPI_8PHASE_XLATCH_FORCE_CA));
	
	vIO32WriteFldAlign(DDRPHY_CA_MISC, 0x1, CA_MISC_RG_TX_ARCLK_JM_EN);
	vIO32WriteFldAlign(DDRPHY_CA_CMD3, 0x1, CA_CMD3_RG_RX_ARCMD_SMT_EN);

	//JM_SEL: MCK4X_CLK
	#if duty_sweep
	//0:LBK_CLK, 2:LBK_CLKB
	vIO32WriteFldAlign(DDRPHY_CA_MISC, 0x2, CA_MISC_RG_TX_ARCLK_JM_SEL);
	#else
	vIO32WriteFldAlign(DDRPHY_CA_MISC, 0x0, CA_MISC_RG_TX_ARCLK_JM_SEL);
	#endif
	
	//Enable MIOCK jitter meter mode ( RG_RX_MIOCK_JIT_EN=1)
	vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 1, EYESCAN_RG_RX_MIOCK_JIT_EN);

	//Disable DQ eye scan (b'1), for counter clear
	#ifdef WHITNEY_USE
	vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 0, EYESCAN_RG_RX_EYE_SCAN_EN);
	#else
	vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 0, EYESCAN_RG_EX_EYE_SCAN_EN);
	#endif
	vIO32WriteFldAlign(DRAMC_REG_STBCAL1, 0, STBCAL1_DQSERRCNT_DIS);
	#if 1
	ucPI_dly = 0;
	prom_puts("\nTX PI = ");
	prom_print_dec(ucPI_dly);
	prom_puts("\n");
	vIO32WriteFldAlign(DDRPHY_SHU1_R0_CA_CMD9, ucPI_dly, SHU1_R0_CA_CMD9_RG_RK0_ARPI_CLK);
	#endif
#if PI_sweep
	for(ucPI_dly=0; ucPI_dly<64; ucPI_dly++)
#elif duty_sweep
	for(ucDuty_dly=0; ucDuty_dly<16; ucDuty_dly++)
#endif
	{	
		#if PI_sweep
		prom_puts("\nTX PI = ");
		prom_print_dec(ucPI_dly);
		prom_puts("\n");
		vIO32WriteFldAlign(DDRPHY_SHU1_R0_CA_CMD9, ucPI_dly, SHU1_R0_CA_CMD9_RG_RK0_ARPI_CLK);
		#endif
		#if duty_sweep
		if(ucDuty_dly<8)
		{
			ucMCK4X_DLY_EN = 1;
		}
		else
		{
			ucMCK4X_DLY_EN = 0;
		}
		prom_puts("\n[duty calibration]\nMCK4X_DLY_EN = ");
		prom_print_dec(ucMCK4X_DLY_EN);
		prom_puts("\nMCK4XB_DLY_EN = ");
		prom_print_dec((~ucMCK4X_DLY_EN)&0x1);
		prom_puts("\nARCLK_DLY & ARCLKB_DLY = ");
		//prom_puts("\nARCLK_DLY = ");
		prom_print_dec(((ucDuty_dly%8)*ucMCK4X_DLY_EN));
		prom_puts("\nnARCLK_DLYB & ARCLKB_DLYB = ");
		//prom_puts("\nARCLK_DLYB = ");
		prom_print_dec(((ucDuty_dly%8)*((~ucMCK4X_DLY_EN)&0x1)));
		prom_puts("\n");
		vIO32WriteFldAlign(DDRPHY_CA_MISC, ucMCK4X_DLY_EN, CA_MISC_RG_TX_ARCLK_MCK4X_DLY_EN);
		vIO32WriteFldAlign(DDRPHY_CA_MISC, ((~ucMCK4X_DLY_EN)&0x1), CA_MISC_RG_TX_ARCLK_MCK4XB_DLY_EN);
		vIO32WriteFldAlign(DDRPHY_SHU1_R0_CA_CMD10, ((ucDuty_dly%8)*ucMCK4X_DLY_EN), SHU1_R0_CA_CMD10_RK0_TX_ARCLK_DLY);
		vIO32WriteFldAlign(DDRPHY_SHU1_R0_CA_CMD10, ((ucDuty_dly%8)*((~ucMCK4X_DLY_EN)&0x1)), SHU1_R0_CA_CMD10_RK0_TX_ARCLK_DLYB);
		vIO32WriteFldAlign(DDRPHY_SHU1_R0_CA_CMD10, ((ucDuty_dly%8)*ucMCK4X_DLY_EN), SHU1_R0_CA_CMD10_RK0_TX_ARCLKB_DLY);
		vIO32WriteFldAlign(DDRPHY_SHU1_R0_CA_CMD10, ((ucDuty_dly%8)*((~ucMCK4X_DLY_EN)&0x1)), SHU1_R0_CA_CMD10_RK0_TX_ARCLKB_DLYB);
		#endif
		#if duty_cal
		ucFirstStep_cnt = 0;
		uc2ndStep_cnt = 0;
		ucFlag_LowStepFirst=0;
		trans_cnt_tmp = 0;
		for(ii=0;ii<3;ii++)
		{
			edge[ii].large_trans_tap = 0;
			edge[ii].large_trans_cnt = 0;
			edge[ii].small_trans_tap = 0;
			edge[ii].small_trans_cnt = 0;
		}
		ii=0;
		#endif
		ucsearch_state = 0;
		for (ucdqs_dly=0; ucdqs_dly<128; ucdqs_dly++)
		{
			vIO32WriteFldAlign(DDRPHY_SHU1_R0_CA_CMD8, ucdqs_dly, \
				SHU1_R0_CA_CMD8_RG_RK0_RX_ARCLK_DLY);
		
			DramPhyReset(p);

			//Reset eye scan counters (reg_sw_rst): 1 to 0
			vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 1, EYESCAN_REG_SW_RST);
			vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 0, EYESCAN_REG_SW_RST);

			//Enable DQ eye scan (b'1)
			#ifdef WHITNEY_USE
			vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 1, EYESCAN_RG_RX_EYE_SCAN_EN);
			#else
			vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 1, EYESCAN_RG_EX_EYE_SCAN_EN);
			#endif

			#if FOR_DV_SIMULATION
				mcDELAY_US(1);
			#else
			// 2ns/sample, here we delay 1ms about 500 samples
			mcDELAY_US(1000);
			#endif

			//Disable DQ eye scan (b'1), for counter latch
			#ifdef WHITNEY_USE
			vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 0, EYESCAN_RG_RX_EYE_SCAN_EN);
			#else
			vIO32WriteFldAlign(DRAMC_REG_EYESCAN, 0, EYESCAN_RG_EX_EYE_SCAN_EN);
			#endif

			//Read the counter values from registers (toggle_cnt*, dqs_err_cnt*);
			u4sample_cnt = u4IO32ReadFldAlign(DRAMC_REG_TOGGLE_CNT, TOGGLE_CNT_TOGGLE_CNT);
			u4ones_cnt = u4IO32ReadFldAlign(DRAMC_REG_DQS2_ERR_CNT, DQS2_ERR_CNT_DQS2_ERR_CNT);
			//u4ones_cnt[1] = u4IO32ReadFldAlign(DRAMC_REG_DQS1_ERR_CNT, DQS1_ERR_CNT_DQS1_ERR_CNT);
			//u4ones_cnt[2] = u4IO32ReadFldAlign(DRAMC_REG_DQS2_ERR_CNT, DQS2_ERR_CNT_DQS2_ERR_CNT);
			//u4ones_cnt[3] = u4IO32ReadFldAlign(DRAMC_REG_DQS3_ERR_CNT, DQS3_ERR_CNT_DQS3_ERR_CNT);
			#ifdef ETT_PRINT_FORMAT
			mcSHOW_DBG_MSG(("%d : %d, %d, %d, %d, %d\n", ucdqs_dly, u4sample_cnt, u4ones_cnt[0],u4ones_cnt[1],u4ones_cnt[2],u4ones_cnt[3]));
			#else
			mcSHOW_DBG_MSG(("%3d : %8d, %8d, %8d, %8d, %8d\n", ucdqs_dly, u4sample_cnt, u4ones_cnt[0], u4ones_cnt[1], u4ones_cnt[2], u4ones_cnt[3]));
	#if dbg_print
			prom_print_dec(ucdqs_dly);
			prom_puts(": ");
			prom_print_dec(u4sample_cnt);
			prom_puts(", ");
			prom_print_dec(u4ones_cnt);
			prom_puts("\n");
			#endif
			#endif
			#if duty_cal
			//if(ucsearch_state>0)
			{
				if(ucdqs_dly==0&&(u4ones_cnt < (u4sample_cnt/2)))
				ucFlag_LowStepFirst=1;
				#if 1
				if(ii>2)
					break;
				if(ucFlag_LowStepFirst)
				{
					if(ii==0||ii==2)
					{
						if(u4ones_cnt>(u4sample_cnt/2))
						{
							edge[ii].large_trans_cnt=u4ones_cnt;
							edge[ii].large_trans_tap=ucdqs_dly;
							edge[ii].small_trans_cnt=trans_cnt_tmp;
							edge[ii].small_trans_tap=ucdqs_dly-1;
							ii++;
						}
						else
						{
							trans_cnt_tmp=u4ones_cnt;
						}
					}
					else
					{
						if(u4ones_cnt<(u4sample_cnt/2))
						{
							edge[ii].small_trans_cnt=u4ones_cnt;
							edge[ii].small_trans_tap=ucdqs_dly;
							edge[ii].large_trans_cnt=trans_cnt_tmp;
							edge[ii].large_trans_tap=ucdqs_dly-1;
							ii++;
						}
						else
						{
							trans_cnt_tmp=u4ones_cnt;
						}
					}
				}
				else
				{
					if(ii==1)
					{
						if(u4ones_cnt>(u4sample_cnt/2))
						{
							edge[ii].large_trans_cnt=u4ones_cnt;
							edge[ii].large_trans_tap=ucdqs_dly;
							edge[ii].small_trans_cnt=trans_cnt_tmp;
							edge[ii].small_trans_tap=ucdqs_dly-1;
							ii++;
						}
						else
						{
							trans_cnt_tmp=u4ones_cnt;
						}
					}
					else
					{
						if(u4ones_cnt<(u4sample_cnt/2))
						{
							edge[ii].small_trans_cnt=u4ones_cnt;
							edge[ii].small_trans_tap=ucdqs_dly;
							edge[ii].large_trans_cnt=trans_cnt_tmp;
							edge[ii].large_trans_tap=ucdqs_dly-1;
							ii++;
						}
						else
						{
							trans_cnt_tmp=u4ones_cnt;
						}
					}
				}
				#endif
			}
			#endif
			/*
			//Disable DQ eye scan (RG_RX_EYE_SCAN_EN=0, RG_RX_*RDQ_VREF_EN_B*=0, RG_RX_*RDQ_EYE_VREF_EN_B*=0, RG_RX_*RDQ_SMT_EN_B*=0)
			vIO32WriteFldAlign(DRAMC_REG_STBCAL_F), 0, STBCAL_F_RG_EX_EYE_SCAN_EN);
			vIO32WriteFldAlign(DDRPHY_EYE2), 0, EYE2_RG_RX_ARDQ_VREF_EN_B0);
			vIO32WriteFldAlign(DDRPHY_EYEB1_2), 0, EYEB1_2_RG_RX_ARDQ_VREF_EN_B1);
			vIO32WriteFldAlign(DDRPHY_EYE2), 0, EYE2_RG_RX_ARDQ_EYE_VREF_EN_B0);
			vIO32WriteFldAlign(DDRPHY_EYEB1_2), 0, EYEB1_2_RG_RX_ARDQ_EYE_VREF_EN_B1);
			vIO32WriteFldAlign(DDRPHY_TXDQ3), 0, TXDQ3_RG_RX_ARDQ_SMT_EN_B0);
			vIO32WriteFldAlign(DDRPHY_RXDQ13),0, RXDQ13_RG_RX_ARDQ_SMT_EN_B1);
			*/

			//change to boolean value
			if (u4ones_cnt < (u4sample_cnt/2))
			{
				fgcurrent_value = 0;
			}
			else
			{
				fgcurrent_value = 1;
			}

			#if 1//more than 1T data
			{
				if (ucsearch_state==0)
				{
					//record initial value at the beginning
					fginitial_value = fgcurrent_value;
					ucsearch_state = 1;
				}
				else if (ucsearch_state==1)
				{
					// check if change value
					if (fgcurrent_value != fginitial_value)
					{
						// start of the period
						fginitial_value = fgcurrent_value;
						ucstart_period = ucdqs_dly;
						ucsearch_state = 2;
						#if duty_cal
						ucFirstStep_cnt = 1;
						#endif
					}
				}
				else if (ucsearch_state==2)
				{
					// check if change value
					if (fgcurrent_value != fginitial_value)
					{
						fginitial_value = fgcurrent_value;
						ucsearch_state = 3;
						#if duty_cal
						uc2ndStep_cnt = 1;
						#endif
					}
					else
					{
						ucFirstStep_cnt++;
					}
				}
				else if (ucsearch_state==3)
				{
					// check if change value
					if (fgcurrent_value != fginitial_value)
					{
						// end of the period, break the loop
						ucend_period = ucdqs_dly;
						ucsearch_state = 4;
						//break;
					}
					else
					{
						uc2ndStep_cnt++;
					}
				}
				else
				{
					//nothing
				}
			}
			#else //only 0.5T data
			{
				if (ucsearch_state==0)
				{
					//record initial value at the beginning
					fginitial_value = fgcurrent_value;
					ucsearch_state = 1;
				}
				else if (ucsearch_state==1)
				{
					// check if change value
					if (fgcurrent_value != fginitial_value)
					{
						// start of the period
						fginitial_value = fgcurrent_value;
						ucstart_period = ucdqs_dly;
						ucsearch_state = 2;
					}
				}
				else if (ucsearch_state==2)
				{
					// check if change value
					if (fgcurrent_value != fginitial_value)
					{
						// end of the period, break the loop
						ucend_period = ucdqs_dly;
						ucsearch_state = 4;
						break;
					}
				}
			}
			#endif
		}

		if(ucsearch_state!=4)
		{
			mcSHOW_DBG_MSG(("\n\tMIOCK jitter meter\n"));
			mcSHOW_DBG_MSG(("\tLess than 0.5T data. Cannot calculate delay cell time\n\n"));
			#if 1//dbg_print
			prom_puts("\nMIOCK jitter meter\nLess than 0.5T data. Cannot calculate delay cell time\n\n");
			#endif
			return DRAM_FAIL;
		}

		//Calculate 1 delay cell = ? ps
		// 1T = ? delay cell
		ucg_num_dlycell_perT = (ucend_period - ucstart_period);
		// 1T = ? ps

#ifdef WHITNEY_USE
		u1ShuLevel = u4IO32ReadFldAlign(DRAMC_REG_SHUSTATUS, SHUSTATUS_SHUFFLE_LEVEL);
#else
		u1ShuLevel = 0;
#endif

#if 1 //cc mark
		//zj add begin, azalea only uses CLRPLL.
		u4SDM_PCW = u4IO32ReadFldAlign(DDRPHY_SHU1_PLL7, SHU1_PLL7_RG_RCLRPLL_SDM_PCW);
		u4PREDIV = u4IO32ReadFldAlign(DDRPHY_SHU1_PLL10, SHU1_PLL10_RG_RCLRPLL_PREDIV);
		u4POSDIV = u4IO32ReadFldAlign(DDRPHY_SHU1_PLL10, SHU1_PLL10_RG_RCLRPLL_POSDIV);
		u4EXTFBDIV = u4IO32ReadFldAlign(DDRPHY_SHU1_PLL11, SHU1_PLL11_RG_CLRPLL_FBDIV_SEL);
		//u4VCOFreq = (((u4SDM_PCW*1000 >> 9)*(26 >> u4PREDIV)) >> u4POSDIV << u4EXTFBDIV)/1000;

		u4VCOFreq = (((u4SDM_PCW >> 24) * (25 >> u4PREDIV)) >> u4POSDIV << u4EXTFBDIV);
		u2real_freq = u4VCOFreq>>1;
		//zj add end
		
		u2real_period = (U16) (1000000/u2real_freq);//cc: ps
		//calculate delay cell time

		u2gdelay_cell_ps = u2real_period*100 / ucg_num_dlycell_perT;

		mcSHOW_DBG_MSG(("\n\tMIOCK jitter meter\n\n"
			"1T = (%d-%d)*2 = %d dly cells\n"
			"Clock freq = %d MHz, period = %d ps, 1 dly cell = %d/100 ps\n",
			ucend_period, ucstart_period, ucg_num_dlycell_perT,
			u2real_freq, u2real_period, u2gdelay_cell_ps));
		#if	duty_cal
		for(ii=0;ii<3;ii++)
		{
			mid_ratio = (U16)((231500-edge[ii].small_trans_cnt)*100/(edge[ii].large_trans_cnt-edge[ii].small_trans_cnt));
			if(edge[ii].small_trans_tap<edge[ii].large_trans_tap)
				edge_pos[ii]=mid_ratio+edge[ii].small_trans_tap*100;
			else
				edge_pos[ii]=edge[ii].small_trans_tap*100-mid_ratio;
		}	
		if(ucFlag_LowStepFirst)
			u2duty = (U16)((edge_pos[1]-edge_pos[0])*1000/(edge_pos[2]-edge_pos[0]));
		else
			u2duty = (U16)((edge_pos[2]-edge_pos[1])*1000/(edge_pos[2]-edge_pos[0]));
		#endif
		#if duty_sweep
		if(u2duty>500)
		{
			u2duty_diff = u2duty-500;
		}
		else
		{
			u2duty_diff = 500-u2duty;
		}
		if(u2duty_diff<u2duty_diff_opt)
		{
			u2duty_diff_opt = u2duty_diff;
			u2duty_opt = u2duty;
			ucMCK4X_DLY_EN_opt = ucMCK4X_DLY_EN;
			ucDuty_dly_opt = ucDuty_dly;
		}
		#endif
		#if 1//dbg_print
		prom_puts("MIOCK jitter meter\n\n1T = (");
		prom_print_dec(ucend_period);
		prom_puts("-");
		prom_print_dec(ucstart_period);
		prom_puts(") = ");
		prom_print_dec(ucg_num_dlycell_perT);
		prom_puts(" dly cells\nClock freq = ");
		prom_print_dec(u2real_freq);
		prom_puts(" MHz, period = ");
		prom_print_dec(u2real_period);
		prom_puts(" ps, 1 dly cell = ");
		prom_print_dec(u2gdelay_cell_ps);
		prom_puts("/100 ps\n");
		#endif
		#if duty_cal
		prom_puts("1st edge: ");
		if(ucFlag_LowStepFirst)
			prom_puts("Low to High\n");
		else
			prom_puts("High to Low\n");
		prom_puts("1st edge position: ");
		prom_print_dec(edge_pos[0]);
		prom_puts("/100, 2nd edge position: ");
		prom_print_dec(edge_pos[1]);
		prom_puts("/100, 3rd edge position: ");
		prom_print_dec(edge_pos[2]);
		prom_puts("/100\n");
		if(ucFlag_LowStepFirst)
		{
			prom_puts("high step width = 2nd - 1st = ");
			prom_print_dec((edge_pos[1]-edge_pos[0]));
			prom_puts("/100, low step width = 3rd - 2nd = ");
			prom_print_dec((edge_pos[2]-edge_pos[1]));
		}
		else
		{
			prom_puts("high step width = 3rd - 2nd = ");
			prom_print_dec((edge_pos[2]-edge_pos[1]));
			prom_puts(", low step width = 2nd - 1st = ");
			prom_print_dec((edge_pos[1]-edge_pos[0]));
		}
		prom_puts("/100\nduty = ");
		prom_print_dec(u2duty);
		prom_puts("/10 %\n");
		#if dbg_print
		for(ii=0;ii<3;ii++)
		{
			prom_print_dec(edge[ii].small_trans_tap);
			prom_puts(" : ");
			prom_print_dec(edge[ii].small_trans_cnt);
			prom_puts(", ");
			prom_print_dec(edge[ii].large_trans_tap);
			prom_puts(" : ");
			prom_print_dec(edge[ii].large_trans_cnt);
			prom_puts("\n");
		}
		#endif
		#endif
		#if 0
		while(1)
		{
			delay_a_while(3000000);
			if((ADDR_READ_REG(0x1fbf0204)&0x1)==0)
			{
				break;
			}
		}
		#endif
	}
	#if duty_sweep
	prom_puts("Optimal duty: ");
	prom_print_dec(u2duty_opt);
	prom_puts("\nOptimal setting: ");
	prom_puts("\nMCK4X_DLY_EN = ");
	prom_print_dec(ucMCK4X_DLY_EN_opt);
	prom_puts("\nMCK4XB_DLY_EN = ");
	prom_print_dec((~ucMCK4X_DLY_EN_opt)&0x1);
	prom_puts("\nARCLKB_DLY = ");
	//prom_puts("\nARCLK_DLY = ");
	prom_print_dec(((ucDuty_dly_opt%8)*ucMCK4X_DLY_EN_opt));
	prom_puts("\nARCLKB_DLYB = ");
	//prom_puts("\nARCLK_DLYB = ");
	prom_print_dec(((ucDuty_dly_opt%8)*((~ucMCK4X_DLY_EN_opt)&0x1)));
	prom_puts("\n");
	#endif
	//restore to orignal value
	vIO32Write4B(DRAMC_REG_EYESCAN, u4prv_register_EYESCAN);
	vIO32Write4B(DRAMC_REG_STBCAL1, u4prv_register_STBCAL1);

	vIO32Write4B(DDRPHY_B0_DQ6, u4prv_register_B0_DQ6);
	vIO32Write4B(DDRPHY_B1_DQ6, u4prv_register_B1_DQ6);
	vIO32Write4B(DDRPHY_B0_DQ5, u4prv_register_B0_DQ5);
	vIO32Write4B(DDRPHY_B1_DQ5, u4prv_register_B1_DQ5);
#if 0
	vIO32Write4B(DDRPHY_B0_DQ4, u4prv_register_B0_DQ4);
	vIO32Write4B(DDRPHY_B1_DQ4, u4prv_register_B1_DQ4);
#endif
	vIO32Write4B(DDRPHY_SHU1_R0_B0_DQ6, u4prv_register_SHU1_R0B0_DQ6);
	vIO32Write4B(DDRPHY_SHU1_R0_B1_DQ6, u4prv_register_SHU1_R0B1_DQ6);

	vIO32Write4B(DDRPHY_B0_DQ3, u4prv_register_B0_DQ3);
	vIO32Write4B(DDRPHY_B1_DQ3, u4prv_register_B1_DQ3);
	vIO32Write4B(DDRPHY_SHU1_B0_DQ7, u4prv_register_SHU1_B0_DQ7);
	vIO32Write4B(DDRPHY_SHU1_B1_DQ7, u4prv_register_SHU1_B1_DQ7);
	vIO32Write4B(DDRPHY_MISC_CTRL1, u4prv_register_MISC_CTRL1);
	vIO32Write4B(DRAMC_REG_DRAMC_PD_CTRL, u4prv_register_PD_CTL);
	vIO32Write4B(DDRPHY_MISC_CG_CTRL4, u4prv_register_MISC_CG_CTRL4);
	vIO32Write4B(DDRPHY_CA_MISC,u4prv_register_CA_MISC);
	vIO32Write4B(DDRPHY_CA_CMD3,u4prv_register_CA_CMD3);
	#if PI_sweep
	vIO32Write4B(DDRPHY_SHU1_R0_CA_CMD9,u4prv_register_CA_CMD9);
	#endif
	#if duty_sweep
	vIO32Write4B(DDRPHY_SHU1_R0_CA_CMD10,u4prv_register_CA_CMD10);
	vIO32WriteFldAlign(DDRPHY_CA_MISC, ucMCK4X_DLY_EN_opt, CA_MISC_RG_TX_ARCLK_MCK4X_DLY_EN);
	vIO32WriteFldAlign(DDRPHY_CA_MISC, ((~ucMCK4X_DLY_EN_opt)&0x1), CA_MISC_RG_TX_ARCLK_MCK4XB_DLY_EN);
	vIO32WriteFldAlign(DDRPHY_SHU1_R0_CA_CMD10, ((ucDuty_dly_opt%8)*ucMCK4X_DLY_EN_opt), SHU1_R0_CA_CMD10_RK0_TX_ARCLK_DLY);
	vIO32WriteFldAlign(DDRPHY_SHU1_R0_CA_CMD10, ((ucDuty_dly_opt%8)*((~ucMCK4X_DLY_EN_opt)&0x1)), SHU1_R0_CA_CMD10_RK0_TX_ARCLK_DLYB);
	#endif
	mcSHOW_DBG_MSG(("[DramcMiockJmeter] ====Done====\n"));
	prom_puts("[DramcMiockJmeter] ====Done====\n");
#endif
	return DRAM_OK;

// log example
/* dly: sample_cnt   DQS0_cnt  DQS1_cnt
    0 : 10962054,        0,        0
    1 : 10958229,        0,        0
    2 : 10961109,        0,        0
    3 : 10946916,        0,        0
    4 : 10955421,        0,        0
    5 : 10967274,        0,        0
    6 : 10893582,        0,        0
    7 : 10974762,        0,        0
    8 : 10990278,        0,        0
    9 : 10972026,        0,        0
   10 :  7421004,        0,        0
   11 : 10943883,        0,        0
   12 : 10984275,        0,        0
   13 : 10955268,        0,        0
   14 : 10960326,        0,        0
   15 : 10952451,        0,        0
   16 : 10956906,        0,        0
   17 : 10960803,        0,        0
   18 : 10944108,        0,        0
   19 : 10959939,        0,        0
   20 : 10959246,        0,        0
   21 : 11002212,        0,        0
   22 : 10919700,        0,        0
   23 : 10977489,        0,        0
   24 : 11009853,        0,        0
   25 : 10991133,        0,        0
   26 : 10990431,        0,        0
   27 : 10970703,    11161,        0
   28 : 10970775,   257118,        0
   29 : 10934442,  9450467,        0
   30 : 10970622, 10968475,        0
   31 : 10968831, 10968831,        0
   32 : 10956123, 10956123,        0
   33 : 10950273, 10950273,        0
   34 : 10975770, 10975770,        0
   35 : 10983024, 10983024,        0
   36 : 10981701, 10981701,        0
   37 : 10936782, 10936782,        0
   38 : 10889523, 10889523,        0
   39 : 10985913, 10985913,    55562
   40 : 10970235, 10970235,   272294
   41 : 10996056, 10996056,  9322868
   42 : 10972350, 10972350, 10969738
   43 : 10963917, 10963917, 10963917
   44 : 10967895, 10967895, 10967895
   45 : 10961739, 10961739, 10961739
   46 : 10937097, 10937097, 10937097
   47 : 10937952, 10937952, 10937952
   48 : 10926018, 10926018, 10926018
   49 : 10943793, 10943793, 10943793
   50 : 10954638, 10954638, 10954638
   51 : 10968048, 10968048, 10968048
   52 : 10944036, 10944036, 10944036
   53 : 11012112, 11012112, 11012112
   54 : 10969137, 10969137, 10969137
   55 : 10968516, 10968516, 10968516
   56 : 10952532, 10952532, 10952532
   57 : 10985832, 10985832, 10985832
   58 : 11002527, 11002527, 11002527
   59 : 10950660, 10873571, 10950660
   60 : 10949022, 10781797, 10949022
   61 : 10974366, 10700617, 10974366
   62 : 10972422,  1331974, 10972422
   63 : 10926567,        0, 10926567
   64 : 10961658,        0, 10961658
   65 : 10978893,        0, 10978893
   66 : 10962828,        0, 10962828
   67 : 10957599,        0, 10957599
   68 : 10969227,        0, 10969227
   69 : 10960722,        0, 10960722
   70 : 10970937,        0, 10963180
   71 : 10962054,        0, 10711639
   72 : 10954719,        0, 10612707
   73 : 10958778,        0,   479589
   74 : 10973898,        0,        0
   75 : 11004156,        0,        0
   76 : 10944261,        0,        0
   77 : 10955340,        0,        0
   78 : 10998153,        0,        0
   79 : 10998774,        0,        0
   80 : 10953234,        0,        0
   81 : 10960020,        0,        0
   82 : 10923831,        0,        0
   83 : 10951362,        0,        0
   84 : 10965249,        0,        0
   85 : 10949103,        0,        0
   86 : 10948707,        0,        0
   87 : 10941147,        0,        0
   88 : 10966572,        0,        0
   89 : 10971333,        0,        0
   90 : 10943721,        0,        0
   91 : 10949337,        0,        0
   92 : 10965942,        0,        0
   93 : 10970397,        0,        0
   94 : 10956429,        0,        0
   95 : 10939896,        0,        0
   96 : 10967112,        0,        0
   97 : 10951911,        0,        0
   98 : 10953702,        0,        0
   99 : 10971090,        0,        0
  100 : 10939590,        0,        0
  101 : 10993392,        0,        0
  102 : 10975932,        0,        0
  103 : 10949499,    40748,        0
  104 : 10962522,   258638,        0
  105 : 10951524,   275292,        0
  106 : 10982475,   417642,        0
  107 : 10966887, 10564347,        0
  ===============================================================================
      MIOCK jitter meter - channel=0
  ===============================================================================
  1T = (107-29) = 78 delay cells
  Clock frequency = 936 MHz, Clock period = 1068 ps, 1 delay cell = 13 ps
*/
}
#endif //ENABLE_MIOCK_JMETER

//-------------------------------------------------------------------------
/** DramcSwImpedanceCal
 *  start TX OCD impedance calibration.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  apply           (U8): 0 don't apply the register we set  1 apply the register we set ,default don't apply.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 */
//-------------------------------------------------------------------------
#if SIMULATION_SW_IMPED
void DramcSwImpedanceSaveRegister(DRAMC_CTX_T *p)
{
    U8 u1Offest_code_b01_p = 0;
    U8 u1Offest_code_b01_n = 0;
    U8 u1Offest_code_b23_p = 0;
    U8 u1Offest_code_b23_n = 0;
    U8 u1Offest_code_cmd_p = 0;
    U8 u1Offest_code_cmd_n = 0;

	 /* OCD */
     //DQ
     vIO32WriteFldAlign(DRAMC_REG_SHU1_DRVING2, (gDramcSwImpedanceResule[0]<<5) | gDramcSwImpedanceResule[1], SHU1_DRVING2_DQDRV1);

     //DQS
     vIO32WriteFldAlign(DRAMC_REG_SHU1_DRVING1, (gDramcSwImpedanceResule[0]<<5) | gDramcSwImpedanceResule[1], SHU1_DRVING1_DQSDRV1);

     //CMD
     vIO32WriteFldAlign(DRAMC_REG_SHU1_DRVING2, (gDramcSwImpedanceResule[0]<<5) | gDramcSwImpedanceResule[1], SHU1_DRVING2_CMDDRV1);

     //CLK
     vIO32WriteFldAlign(DRAMC_REG_SHU1_DRVING2, (gDramcSwImpedanceResule[0]<<5) | gDramcSwImpedanceResule[1], SHU1_DRVING2_CMDDRV2);

	 
	 /* ODT */
     //DQ
     vIO32WriteFldAlign(DRAMC_REG_SHU1_DRVING4, (gDramcSwImpedanceResule[2]<<5) | gDramcSwImpedanceResule[3], SHU1_DRVING4_DQODT1);

     //DQS
     vIO32WriteFldAlign(DRAMC_REG_SHU1_DRVING3, (gDramcSwImpedanceResule[2]<<5) | gDramcSwImpedanceResule[3], SHU1_DRVING3_DQSODT1);

     //CMD
     //cc mark since ODT is not applied to CMD. vIO32WriteFldAlign(DRAMC_REG_SHU1_DRVING4, (gDramcSwImpedanceResule[2]<<5) | gDramcSwImpedanceResule[3], SHU1_DRVING4_CMDODT1);

     //CLK
     //cc mark since ODT is not applied to CLK. vIO32WriteFldAlign(DRAMC_REG_SHU1_DRVING4, (gDramcSwImpedanceResule[2]<<5) | gDramcSwImpedanceResule[3], SHU1_DRVING4_CMDODT2);
}

DRAM_STATUS_T DramcSwImpedanceCal(DRAMC_CTX_T *p)
{
	U32 u4ImpxDrv, u4ImpCalResult;
	U32 u4DRVP_Result =0xff, u4DRVN_Result =0xff;
	U32 u4BaklReg_DDRPHY_MISC_IMP_CTRL0, u4BaklReg_DDRPHY_MISC_IMP_CTRL1, u4BaklReg_DRAMC_REG_IMPCAL;
	U8 u1ByteIdx;

	vIO32WriteFldAlign(DDRPHY_MISC_SPM_CTRL1, 0x0, MISC_SPM_CTRL1_PHY_SPM_CTL1);
	vIO32WriteFldAlign(DDRPHY_MISC_SPM_CTRL2, 0x0, MISC_SPM_CTRL2_PHY_SPM_CTL2);
	vIO32WriteFldAlign(DDRPHY_MISC_SPM_CTRL0, 0x0, MISC_SPM_CTRL0_PHY_SPM_CTL0);

	//Register backup
	//u4BaklReg_DDRPHY_MISC_IMP_CTRL0 = u4IO32Read4B((DDRPHY_MISC_IMP_CTRL0));
	//u4BaklReg_DDRPHY_MISC_IMP_CTRL1 = u4IO32Read4B((DDRPHY_MISC_IMP_CTRL1));
	//u4BaklReg_DRAMC_REG_IMPCAL = u4IO32Read4B((DRAMC_REG_IMPCAL));

	//Disable IMP HW Tracking
	vIO32WriteFldAlign(DRAMC_REG_IMPCAL, 0, IMPCAL_IMPCAL_HW);

	//RG_IMPCAL_VREF_SEL=6'h0f
	//RG_IMPCAL_LP3_EN=0, RG_IMPCAL_LP4_EN=1
	//RG_IMPCAL_ODT_EN=0

	vIO32WriteFldMulti(DDRPHY_MISC_IMP_CTRL1, P_Fld(0, MISC_IMP_CTRL1_RG_RIMP_PRE_EN));
	vIO32WriteFldMulti(DRAMC_REG_IMPCAL, P_Fld(0, IMPCAL_IMPCAL_CALI_ENN) | P_Fld(1, IMPCAL_IMPCAL_IMPPDP) | 
		P_Fld(1, IMPCAL_IMPCAL_IMPPDN));	//RG_RIMP_BIAS_EN and RG_RIMP_VREF_EN move to IMPPDP and IMPPDN, ODT_EN move to CALI_ENN

	#if 0 //cc remove 
	vIO32WriteFldMulti(DDRPHY_MISC_IMP_CTRL0, P_Fld(1, MISC_IMP_CTRL0_RG_RIMP_DDR3_SEL) | \
						P_Fld(0, MISC_IMP_CTRL0_RG_RIMP_DDR4_SEL) | \
						P_Fld(0x2e, MISC_IMP_CTRL0_RG_RIMP_VREF_SEL));
	#endif
	
	/* Add by cc */	
	vIO32WriteFldAlign(DDRPHY_MISC_IMP_CTRL0, 0x12, MISC_IMP_CTRL0_RG_RIMP_VREF_SEL);
	vIO32WriteFldMulti(DDRPHY_PLL3, P_Fld(0, PLL3_RG_RPHYPLL_TSTOD_EN) |
		P_Fld(0, PLL3_RG_RPHYPLL_TSTCK_EN));
	vIO32WriteFldMulti(DDRPHY_MISC_IMP_CTRL0, P_Fld(1, MISC_IMP_CTRL0_RG_RIMP_DDR3_SEL) |
		P_Fld(0, MISC_IMP_CTRL0_RG_RIMP_DDR4_SEL));
	/* cc add end */

	mcSHOW_DBG_MSG2(("0x%X=0x%X\n", DDRPHY_MISC_IMP_CTRL1, u4IO32Read4B(DDRPHY_MISC_IMP_CTRL1)));
	mcSHOW_DBG_MSG2(("0x%X=0x%X\n", DDRPHY_MISC_IMP_CTRL0, u4IO32Read4B(DDRPHY_MISC_IMP_CTRL0)));

	mcDELAY_US(1);

	// K pull up
	mcSHOW_DBG_MSG2(("======= K DRVP=====================\n"));
	#if	dbg_print
	prom_puts("======= K DRVP=====================\n");
	#endif

	//PUCMP_EN=1
	//ODT_EN=0
	vIO32WriteFldAlign(DRAMC_REG_IMPCAL, 1, IMPCAL_IMPCAL_CALI_EN);
	vIO32WriteFldAlign(DRAMC_REG_IMPCAL, 1, IMPCAL_IMPCAL_CALI_ENP);  //PUCMP_EN move to CALI_ENP
	vIO32WriteFldAlign(DRAMC_REG_IMPCAL, 0, IMPCAL_IMPCAL_CALI_ENN);  //ODT_EN move to CALI_ENN

	//DRVP=0
	//DRV05=1
	vIO32WriteFldMulti(DRAMC_REG_SHU_IMPCAL1, P_Fld(0, SHU_IMPCAL1_IMPDRVN) |
		P_Fld(0, SHU_IMPCAL1_IMPDRVP));
	vIO32WriteFldMulti(DDRPHY_MISC_IMP_CTRL1, P_Fld(0, MISC_IMP_CTRL1_RG_RIMP_REV));  //DRV05=1 cc change from 1->0 from DE golden

	//OCDP Flow
	//If RGS_TX_OCD_IMPCALOUTX=0
	//RG_IMPX_DRVP++;
	//Else save and keep RG_IMPX_DRVP value, and assign to DRVP
	for(u4ImpxDrv=0; u4ImpxDrv<21; u4ImpxDrv++)
	{
		/* cc add: 1~15, 27~31 */
		if (u4ImpxDrv == 16)
			u4ImpxDrv = 27;
		
		vIO32WriteFldAlign(DRAMC_REG_SHU_IMPCAL1, u4ImpxDrv, SHU_IMPCAL1_IMPDRVP);
		mcDELAY_US(1);
		u4ImpCalResult = u4IO32ReadFldAlign(DDRPHY_MISC_PHY_RGS1, MISC_PHY_RGS1_RGS_RIMPCALOUT);
		mcSHOW_DBG_MSG2(("1. OCD DRVP=%d CALOUT=%d\n", u4ImpxDrv, u4ImpCalResult));
		#if	dbg_print
		prom_puts("1. OCD DRVP=");
		prom_print_dec(u4ImpxDrv);
		prom_puts(" CALOUT=");
		prom_print_dec(u4ImpCalResult);
		prom_puts("\n");
		#endif
		if((u4ImpCalResult ==1) && (u4DRVP_Result == 0xff))//first found
		{
			u4DRVP_Result = u4ImpxDrv;
			mcSHOW_DBG_MSG2(("1. OCD DRVP calibration OK! DRVP=%d\n\n", u4DRVP_Result));
			#if	dbg_print
			prom_puts("1. OCD DRVP calibration OK! DRVP=");
			prom_print_dec(u4DRVP_Result);
			prom_puts("\n");
			#endif
			break;
		}
	}

	if (u4DRVP_Result == 0xff)
	{
		mcSHOW_DBG_MSG(("OCD DRVP calibration FAIL\n"));
		#if	dbg_print
		prom_puts("OCD DRVP calibration FAIL\n");
		#endif
	}

	//LP3: DRVN calibration
	mcSHOW_DBG_MSG2(("======= K DRVN=====================\n"));
	#if	dbg_print
	prom_puts("======= K DRVN=====================\n");
	#endif
	//PUCMP_EN=0
	vIO32WriteFldAlign(DRAMC_REG_IMPCAL, 0, IMPCAL_IMPCAL_CALI_ENP);  //PUCMP_EN move to CALI_ENP

	//DRVP=DRVP_FINAL
	//DRVN=0
	//DRV05=1
	vIO32WriteFldMulti(DRAMC_REG_SHU_IMPCAL1, P_Fld(u4DRVP_Result, SHU_IMPCAL1_IMPDRVP) | P_Fld(0, SHU_IMPCAL1_IMPDRVN));
	vIO32WriteFldMulti(DDRPHY_MISC_IMP_CTRL1, P_Fld(0, MISC_IMP_CTRL1_RG_RIMP_REV));  //DRV05=1 cc change from 1->0 from DE golden

	//If RGS_TX_OCD_IMPCALOUTX=1
	//RG_IMPX_DRVN++;
	//Else save RG_IMPX_DRVN value and assign to DRVN
	for(u4ImpxDrv=0; u4ImpxDrv<32; u4ImpxDrv++)
	{
		/* cc add: 1~15, 27~31 */
		if (u4ImpxDrv == 16)
			u4ImpxDrv = 27;
		
		vIO32WriteFldAlign(DRAMC_REG_SHU_IMPCAL1, u4ImpxDrv, SHU_IMPCAL1_IMPDRVN);
		mcDELAY_US(1);
		u4ImpCalResult = u4IO32ReadFldAlign((DDRPHY_MISC_PHY_RGS1), MISC_PHY_RGS1_RGS_RIMPCALOUT);
		mcSHOW_DBG_MSG2(("3. OCD DRVN=%d ,CALOUT=%d\n", u4ImpxDrv, u4ImpCalResult));
		#if	dbg_print
		prom_puts("3. OCD DRVN=");
		prom_print_dec(u4ImpxDrv);
		prom_puts(" CALOUT=");
		prom_print_dec(u4ImpCalResult);
		prom_puts("\n");
		#endif
		if((u4ImpCalResult ==0) &&(u4DRVN_Result == 0xff))//first found
		{
			u4DRVN_Result = u4ImpxDrv;
			mcSHOW_DBG_MSG2(("3. OCD DRVN calibration OK! ODTN=%d\n\n", u4DRVN_Result));
			#if	dbg_print
			prom_puts("3. OCD DRVN calibration OK! ODTN=");
			prom_print_dec(u4DRVN_Result);
			prom_puts("\n");
			#endif
			break;
		}
	}

	//Register Restore
	//vIO32Write4B((DRAMC_REG_IMPCAL), u4BaklReg_DRAMC_REG_IMPCAL);
	//vIO32Write4B((DDRPHY_MISC_IMP_CTRL0), u4BaklReg_DDRPHY_MISC_IMP_CTRL0);
	//vIO32Write4B((DDRPHY_MISC_IMP_CTRL1), u4BaklReg_DDRPHY_MISC_IMP_CTRL1);

	if(u4DRVN_Result==0xff || u4DRVP_Result==0xff)
	{
		//MT8167 default value??
		u4DRVP_Result = 0xb;
		u4DRVN_Result = 0xb;
	}

	/* add by cc.
	 * Calculate OCDP/OCDN/ODTP/ODTN 
	 */
	/* Review: Confirm Rext & Rodt value */
	gDramcSwImpedanceResule[0] = Round_Operation(fcR_EXT * ((U16)u4DRVP_Result), fcR_OCD); //OCDP
	gDramcSwImpedanceResule[1] = Round_Operation(fcR_EXT * ((U16)u4DRVN_Result), fcR_OCD); //OCDN
	gDramcSwImpedanceResule[2] = Round_Operation(fcR_EXT * ((U16)u4DRVP_Result), fcR_ODT); //ODTP
	gDramcSwImpedanceResule[3] = Round_Operation(fcR_EXT * ((U16)u4DRVN_Result), fcR_ODT); //ODTN

	mcSHOW_DBG_MSG(("Final Impdance Cal Result: OCDP %x, OCDN %x, ODTP %x, ODTN %x\n",
		gDramcSwImpedanceResule[0],
		gDramcSwImpedanceResule[1],
		gDramcSwImpedanceResule[2],
		gDramcSwImpedanceResule[3]));
	#if	dbg_print
	prom_puts("Final Impdance Cal Result: OCDP:0x");
	prom_print_hex(gDramcSwImpedanceResule[0],8);
	prom_puts(", OCDN:0x");
	prom_print_hex(gDramcSwImpedanceResule[1],8);
	prom_puts(", ODTP:0x");
	prom_print_hex(gDramcSwImpedanceResule[2],8);
	prom_puts(", ODTN:0x");
	prom_print_hex(gDramcSwImpedanceResule[3],8);
	prom_puts("\n");
	#endif
	
	DramcSwImpedanceSaveRegister(p);


	vSetCalibrationResult(p, DRAM_CALIBRATION_SW_IMPEDANCE, DRAM_OK);
	mcSHOW_DBG_MSG2(("[DramcSwImpedanceCal] Done \n\n"));
	#if	dbg_print
	prom_puts("[DramcSwImpedanceCal] Done");
	prom_puts("\n");
	#endif
#if defined(DRAM_CALIB_LOG) || defined(DRAM_ETT)
    gDRAM_CALIB_LOG.RANK[p->rank].SwImpedanceCal.DRVP = gDramcSwImpedanceResule[0];
    gDRAM_CALIB_LOG.RANK[p->rank].SwImpedanceCal.DRVN = gDramcSwImpedanceResule[1];
    gDRAM_CALIB_LOG.RANK[p->rank].SwImpedanceCal.ODTP = gDramcSwImpedanceResule[2];
    gDRAM_CALIB_LOG.RANK[p->rank].SwImpedanceCal.ODTN = gDramcSwImpedanceResule[3];
#endif
	//cc add. RIMP_EN, RIMP_BIAS_EN and RIMP_VREF_EN shall be set to 0x0 after IMP CAL.
	vIO32WriteFldMulti(DRAMC_REG_IMPCAL, P_Fld(0, IMPCAL_IMPCAL_CALI_EN) | P_Fld(0, IMPCAL_IMPCAL_IMPPDP) | 
		P_Fld(0, IMPCAL_IMPCAL_IMPPDN));	//RG_RIMP_BIAS_EN and RG_RIMP_VREF_EN move to IMPPDP and IMPPDN, ODT_EN move to CALI_ENN

	return DRAM_OK;
}
#endif //SIMULATION_SW_IMPED

U32 Another_Rank_CKE_low_backup_another_reg0x24;
U8  Another_Rank_CKE_low_backup_rank;
U8  Another_Rank_CKE_low_another_rank;
void Another_Rank_CKE_low(DRAMC_CTX_T *p, U8 on_off)
{
#if DUAL_RANK_ENABLE
    if (on_off == 0)
    {
        //another rank CKE low
        Another_Rank_CKE_low_backup_rank = u1GetRank(p);
        Another_Rank_CKE_low_another_rank = (Another_Rank_CKE_low_backup_rank == RANK_0) ? RANK_1 : RANK_0;
        vSetRank(p, Another_Rank_CKE_low_another_rank);
        Another_Rank_CKE_low_backup_another_reg0x24 = u4IO32Read4B(DRAMC_REG_CKECTRL);
        vIO32WriteFldAlign(DRAMC_REG_CKECTRL, 1, CKECTRL_CKEFIXOFF);
        vSetRank(p, Another_Rank_CKE_low_backup_rank);
    }
    else
    {   //restore original value
        vSetRank(p, Another_Rank_CKE_low_another_rank);
        vIO32Write4B(DRAMC_REG_CKECTRL, Another_Rank_CKE_low_backup_another_reg0x24);
        vSetRank(p, Another_Rank_CKE_low_backup_rank);
    }
#endif

}

void O1PathOnOff(DRAMC_CTX_T *p, U8 u1OnOff)
{
#if 0// cc modified
    if(u1OnOff)
    {
        vIO32WriteFldAlign_Phy_All(DDRPHY_B0_DQ5, 1, B0_DQ5_RG_RX_ARDQ_EYE_VREF_EN_B0);
        vIO32WriteFldAlign_Phy_All(DDRPHY_B1_DQ5, 1, B1_DQ5_RG_RX_ARDQ_EYE_VREF_EN_B1);
        vIO32WriteFldAlign_Phy_All(DDRPHY_CA_CMD5, 1, CA_CMD5_RG_RX_ARCMD_EYE_VREF_EN);
        vIO32WriteFldAlign_Phy_All(DDRPHY_B0_DQ3, 1, B0_DQ3_RG_RX_ARDQ_SMT_EN_B0);
        vIO32WriteFldAlign_Phy_All(DDRPHY_B1_DQ3, 1, B1_DQ3_RG_RX_ARDQ_SMT_EN_B1);
        vIO32WriteFldAlign_Phy_All(DDRPHY_CA_CMD3, 1, CA_CMD3_RG_RX_ARCMD_SMT_EN);
        mcDELAY_US(1);
    }
    else
    {
        vIO32WriteFldAlign_Phy_All(DDRPHY_B0_DQ5, 0, B0_DQ5_RG_RX_ARDQ_EYE_VREF_EN_B0);
        vIO32WriteFldAlign_Phy_All(DDRPHY_B1_DQ5, 0, B1_DQ5_RG_RX_ARDQ_EYE_VREF_EN_B1);
        vIO32WriteFldAlign_Phy_All(DDRPHY_CA_CMD5, 0, CA_CMD5_RG_RX_ARCMD_EYE_VREF_EN);
        vIO32WriteFldAlign_Phy_All(DDRPHY_B0_DQ3, 0, B0_DQ3_RG_RX_ARDQ_SMT_EN_B0);
        vIO32WriteFldAlign_Phy_All(DDRPHY_B1_DQ3, 0, B1_DQ3_RG_RX_ARDQ_SMT_EN_B1);
        vIO32WriteFldAlign_Phy_All(DDRPHY_CA_CMD3, 0, CA_CMD3_RG_RX_ARCMD_SMT_EN);
    }
#else
if(u1OnOff)
    {
        vIO32WriteFldAlign(DDRPHY_B0_DQ5, 1, B0_DQ5_RG_RX_ARDQ_EYE_VREF_EN_B0);
        vIO32WriteFldAlign(DDRPHY_B1_DQ5, 1, B1_DQ5_RG_RX_ARDQ_EYE_VREF_EN_B1);
        vIO32WriteFldAlign(DDRPHY_CA_CMD5, 1, CA_CMD5_RG_RX_ARCMD_EYE_VREF_EN);
        vIO32WriteFldAlign(DDRPHY_B0_DQ3, 1, B0_DQ3_RG_RX_ARDQ_SMT_EN_B0);
        vIO32WriteFldAlign(DDRPHY_B1_DQ3, 1, B1_DQ3_RG_RX_ARDQ_SMT_EN_B1);
        vIO32WriteFldAlign(DDRPHY_CA_CMD3, 1, CA_CMD3_RG_RX_ARCMD_SMT_EN);
        mcDELAY_US(1);
    }
    else
    {
        vIO32WriteFldAlign(DDRPHY_B0_DQ5, 0, B0_DQ5_RG_RX_ARDQ_EYE_VREF_EN_B0);
        vIO32WriteFldAlign(DDRPHY_B1_DQ5, 0, B1_DQ5_RG_RX_ARDQ_EYE_VREF_EN_B1);
        vIO32WriteFldAlign(DDRPHY_CA_CMD5, 0, CA_CMD5_RG_RX_ARCMD_EYE_VREF_EN);
        vIO32WriteFldAlign(DDRPHY_B0_DQ3, 0, B0_DQ3_RG_RX_ARDQ_SMT_EN_B0);
        vIO32WriteFldAlign(DDRPHY_B1_DQ3, 0, B1_DQ3_RG_RX_ARDQ_SMT_EN_B1);
        vIO32WriteFldAlign(DDRPHY_CA_CMD3, 0, CA_CMD3_RG_RX_ARCMD_SMT_EN);
    }
#endif

}

const U32 uiDDR_PHY_Mapping[PIN_MUX_TYPE_MAX][16] = {
    //uiPCDDR3_PHY_Mapping_KGD
    {
        4, 6, 7, 5, 13, 15, 12, 14,
		1, 3, 0, 2, 10,  8, 11, 9
    },
    //uiPCDDR3_PHY_Mapping_X16X1
    {
         6,  2,  4, 0,  3,  5,  7, 1,
		11, 13, 15, 9, 14, 12, 10, 8
    },
    //uiPCDDR4_PHY_Mapping_X16X1
    {
        0, 1, 2, 3, 4, 5, 6, 7, 8,
		9, 10, 11,  12,  13,  14, 15
    },   
};

#if COMPILE_THIS_PART
//for 4bitMux
unsigned char Bit_DQ_Mapping[16] =
{
    0, 1, 2, 3, 4, 5, 6, 7,
    8, 9, 10, 11, 12, 13, 14, 15
};
#endif

/* DQS & DQM mapping */
unsigned char PCDDR3_DQS_Mapping[2] = {0, 1};	//YMC change to avoid warning
unsigned char PCDDR4_DQS_Mapping[2] = {1, 0};	//YMC change to avoid warning

//-------------------------------------------------------------------------
/** DramcWriteLeveling
 *  start Write Leveling Calibration.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  apply           (U8): 0 don't apply the register we set  1 apply the register we set ,default don't apply.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 */
//-------------------------------------------------------------------------
#define WRITE_LEVELING_MOVD_DQS 1//UI

typedef struct _REG_TRANSFER
{
    U32 u4Addr;
    U32 u4Fld;
} REG_TRANSFER_T;


// NOT suitable for Gating delay
static DRAM_STATUS_T ExecuteMoveDramCDelay(DRAMC_CTX_T *p, REG_TRANSFER_T regs[], S8 iShiftUI)
{
    S32 s4HighLevelDelay, s4DelaySum;
    U32 u4Tmp0p5T, u4Tmp2T;
    U8 ucDataRateDivShift;
    DRAM_STATUS_T MoveResult;

    ucDataRateDivShift= 2;
	
	/* cc notes: 0p5T = 1UI. 2T = 4*0p5T = 4UI
	 * Here SW calculates total UIs that will be set.
	 */
    u4Tmp0p5T = u4IO32ReadFldAlign(regs[0].u4Addr, regs[0].u4Fld) & (~(1<<ucDataRateDivShift));
    u4Tmp2T = u4IO32ReadFldAlign(regs[1].u4Addr, regs[1].u4Fld);
    //mcSHOW_DBG_MSG(("\n[MoveDramC_Orz]  u4Tmp2T:%d,  u4Tmp0p5T: %d,\n",  u4Tmp2T, u4Tmp0p5T));

    s4HighLevelDelay = (u4Tmp2T <<ucDataRateDivShift) + u4Tmp0p5T;
    s4DelaySum = (s4HighLevelDelay + iShiftUI);
    //mcSHOW_DBG_MSG(("\n[MoveDramC_Orz]  s4HighLevelDealy(%d) +  iShiftUI(%d) = %d\n",  s4HighLevelDelay, iShiftUI, s4DelaySum));

    if(s4DelaySum < 0)
    {
        u4Tmp0p5T =0;
        u4Tmp2T=0;
        MoveResult =  DRAM_FAIL;
        //mcSHOW_ERR_MSG(("\n[MoveDramC_Orz]  s4HighLevelDealy(%d) +  iShiftUI(%d) is small than 0!!\n",  s4HighLevelDelay, iShiftUI));
    }
    else
    {
        u4Tmp2T = s4DelaySum >> ucDataRateDivShift;
        u4Tmp0p5T = s4DelaySum - (u4Tmp2T <<ucDataRateDivShift);
        MoveResult = DRAM_OK;
    }

    vIO32WriteFldAlign(regs[0].u4Addr, u4Tmp0p5T, regs[0].u4Fld);
    vIO32WriteFldAlign(regs[1].u4Addr, u4Tmp2T, regs[1].u4Fld);
    //mcSHOW_DBG_MSG(("\n[MoveDramC_Orz]  Final ==> u4Tmp2T:%d,  u4Tmp0p5T: %d,\n",  u4Tmp2T, u4Tmp0p5T));

    return MoveResult;
}

#if WRITE_LEVELING_MOVE_DQS_INSTEAD_OF_CLK
static void MoveDramC_TX_DQS(DRAMC_CTX_T *p, U8 u1ByteIdx, S8 iShiftUI)
{
    REG_TRANSFER_T TransferReg[2];

    //mcSHOW_DBG_MSG(("\n[MoveDramC_TX_DQS] Byte %d, iShiftUI %d\n", u1ByteIdx, iShiftUI));

    switch(u1ByteIdx)
    {
        case 0:
            // DQS0
            TransferReg[0].u4Addr = DRAMC_REG_SHU_SELPH_DQS1;
            TransferReg[0].u4Fld = SHU_SELPH_DQS1_DLY_DQS0;
            TransferReg[1].u4Addr = DRAMC_REG_SHU_SELPH_DQS0;
            TransferReg[1].u4Fld = SHU_SELPH_DQS0_TXDLY_DQS0;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
            break;

        case 1:
            // DQS1
            TransferReg[0].u4Addr = DRAMC_REG_SHU_SELPH_DQS1;
            TransferReg[0].u4Fld = SHU_SELPH_DQS1_DLY_DQS1;
            TransferReg[1].u4Addr = DRAMC_REG_SHU_SELPH_DQS0;
            TransferReg[1].u4Fld = SHU_SELPH_DQS0_TXDLY_DQS1;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
            break;
	#if COMPILE_THIS_PART
        case 2:
            // DQS2
            TransferReg[0].u4Addr = DRAMC_REG_SHU_SELPH_DQS1;
            TransferReg[0].u4Fld = SHU_SELPH_DQS1_DLY_DQS2;
            TransferReg[1].u4Addr = DRAMC_REG_SHU_SELPH_DQS0;
            TransferReg[1].u4Fld = SHU_SELPH_DQS0_TXDLY_DQS2;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
            break;

        case 3:
            // DQS3
            TransferReg[0].u4Addr = DRAMC_REG_SHU_SELPH_DQS1;
            TransferReg[0].u4Fld = SHU_SELPH_DQS1_DLY_DQS3;
            TransferReg[1].u4Addr = DRAMC_REG_SHU_SELPH_DQS0;
            TransferReg[1].u4Fld = SHU_SELPH_DQS0_TXDLY_DQS3;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
            break;
	#endif
            default:
                break;
    }
}

static void MoveDramC_TX_DQS_OEN(DRAMC_CTX_T *p, U8 u1ByteIdx, S8 iShiftUI)
{
    REG_TRANSFER_T TransferReg[2];

    //mcSHOW_DBG_MSG(("\n[MoveDramC_TX_DQS_OEN] Byte %d, iShiftUI %d\n", u1ByteIdx, iShiftUI));

    switch(u1ByteIdx)
    {
        case 0:
            // DQS_OEN_0
            TransferReg[0].u4Addr = DRAMC_REG_SHU_SELPH_DQS1;
            TransferReg[0].u4Fld =SHU_SELPH_DQS1_DLY_OEN_DQS0;
            TransferReg[1].u4Addr = DRAMC_REG_SHU_SELPH_DQS0;
            TransferReg[1].u4Fld =SHU_SELPH_DQS0_TXDLY_OEN_DQS0;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
            break;

        case 1:
            // DQS_OEN_1
            TransferReg[0].u4Addr = DRAMC_REG_SHU_SELPH_DQS1;
            TransferReg[0].u4Fld =SHU_SELPH_DQS1_DLY_OEN_DQS1;
            TransferReg[1].u4Addr = DRAMC_REG_SHU_SELPH_DQS0;
            TransferReg[1].u4Fld =SHU_SELPH_DQS0_TXDLY_OEN_DQS1;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
            break;

	#if COMPILE_THIS_PART //cc notes: 16bit width, only DQS0 & DQS1 make sense.
        case 2:
            // DQS_OEN_2
            TransferReg[0].u4Addr = DRAMC_REG_SHU_SELPH_DQS1;
            TransferReg[0].u4Fld =SHU_SELPH_DQS1_DLY_OEN_DQS2;
            TransferReg[1].u4Addr = DRAMC_REG_SHU_SELPH_DQS0;
            TransferReg[1].u4Fld =SHU_SELPH_DQS0_TXDLY_OEN_DQS2;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
            break;

        case 3:
            // DQS_OEN_3
            TransferReg[0].u4Addr = DRAMC_REG_SHU_SELPH_DQS1;
            TransferReg[0].u4Fld =SHU_SELPH_DQS1_DLY_OEN_DQS3;
            TransferReg[1].u4Addr = DRAMC_REG_SHU_SELPH_DQS0;
            TransferReg[1].u4Fld =SHU_SELPH_DQS0_TXDLY_OEN_DQS3;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
            break;
	#endif
        default:
            break;
    }
}
#endif


static void MoveDramC_TX_DQ(DRAMC_CTX_T *p, U8 u1ByteIdx, S8 iShiftUI)
{
    REG_TRANSFER_T TransferReg[2];

    //mcSHOW_DBG_MSG(("\n[MoveDramC_TX_DQ] Byte %d, iShiftUI %d\n", u1ByteIdx, iShiftUI));

    switch(u1ByteIdx)
    {
        case 0:
            // DQM0
            TransferReg[0].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ3;
            TransferReg[0].u4Fld =SHURK0_SELPH_DQ3_DLY_DQM0;
            TransferReg[1].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ1;
            TransferReg[1].u4Fld =SHURK0_SELPH_DQ1_TXDLY_DQM0;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
            // DQM_OEN_0
            TransferReg[0].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ3;
            TransferReg[0].u4Fld =SHURK0_SELPH_DQ3_DLY_OEN_DQM0;
            TransferReg[1].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ1;
            TransferReg[1].u4Fld =SHURK0_SELPH_DQ1_TXDLY_OEN_DQM0;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
            // DQ0
            TransferReg[0].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ2;
            TransferReg[0].u4Fld =SHURK0_SELPH_DQ2_DLY_DQ0;
            TransferReg[1].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ0;
            TransferReg[1].u4Fld =SHURK0_SELPH_DQ0_TXDLY_DQ0;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
            // DQ_OEN_0
            TransferReg[0].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ2;
            TransferReg[0].u4Fld =SHURK0_SELPH_DQ2_DLY_OEN_DQ0;
            TransferReg[1].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ0;
            TransferReg[1].u4Fld =SHURK0_SELPH_DQ0_TXDLY_OEN_DQ0;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
        break;

        case 1:
            // DQM1
            TransferReg[0].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ3;
            TransferReg[0].u4Fld =SHURK0_SELPH_DQ3_DLY_DQM1;
            TransferReg[1].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ1;
            TransferReg[1].u4Fld =SHURK0_SELPH_DQ1_TXDLY_DQM1;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
            // DQM_OEN_1
            TransferReg[0].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ3;
            TransferReg[0].u4Fld =SHURK0_SELPH_DQ3_DLY_OEN_DQM1;
            TransferReg[1].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ1;
            TransferReg[1].u4Fld =SHURK0_SELPH_DQ1_TXDLY_OEN_DQM1;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
            // DQ1
            TransferReg[0].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ2;
            TransferReg[0].u4Fld =SHURK0_SELPH_DQ2_DLY_DQ1;
            TransferReg[1].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ0;
            TransferReg[1].u4Fld =SHURK0_SELPH_DQ0_TXDLY_DQ1;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
             // DQ_OEN_1
            TransferReg[0].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ2;
            TransferReg[0].u4Fld =SHURK0_SELPH_DQ2_DLY_OEN_DQ1;
            TransferReg[1].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ0;
            TransferReg[1].u4Fld =SHURK0_SELPH_DQ0_TXDLY_OEN_DQ1;
             ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
        break;

	#if COMPILE_THIS_PART
        case 2:
                // DQM2
            TransferReg[0].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ3;
            TransferReg[0].u4Fld =SHURK0_SELPH_DQ3_DLY_DQM2;
            TransferReg[1].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ1;
            TransferReg[1].u4Fld =SHURK0_SELPH_DQ1_TXDLY_DQM2;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
            // DQM_OEN_2
            TransferReg[0].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ3;
            TransferReg[0].u4Fld =SHURK0_SELPH_DQ3_DLY_OEN_DQM2;
            TransferReg[1].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ1;
            TransferReg[1].u4Fld =SHURK0_SELPH_DQ1_TXDLY_OEN_DQM2;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
            // DQ2
            TransferReg[0].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ2;
            TransferReg[0].u4Fld =SHURK0_SELPH_DQ2_DLY_DQ2;
            TransferReg[1].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ0;
            TransferReg[1].u4Fld =SHURK0_SELPH_DQ0_TXDLY_DQ2;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
            // DQ_OEN_2
            TransferReg[0].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ2;
            TransferReg[0].u4Fld =SHURK0_SELPH_DQ2_DLY_OEN_DQ2;
            TransferReg[1].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ0;
            TransferReg[1].u4Fld =SHURK0_SELPH_DQ0_TXDLY_OEN_DQ2;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
        break;

        case 3:
            // DQM3
            TransferReg[0].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ3;
            TransferReg[0].u4Fld =SHURK0_SELPH_DQ3_DLY_DQM3;
            TransferReg[1].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ1;
            TransferReg[1].u4Fld =SHURK0_SELPH_DQ1_TXDLY_DQM3;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
            // DQM_OEN_3
            TransferReg[0].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ3;
            TransferReg[0].u4Fld =SHURK0_SELPH_DQ3_DLY_OEN_DQM3;
            TransferReg[1].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ1;
            TransferReg[1].u4Fld =SHURK0_SELPH_DQ1_TXDLY_OEN_DQM3;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
            // DQ3
            TransferReg[0].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ2;
            TransferReg[0].u4Fld =SHURK0_SELPH_DQ2_DLY_DQ3;
            TransferReg[1].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ0;
            TransferReg[1].u4Fld =SHURK0_SELPH_DQ0_TXDLY_DQ3;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
            // DQ_OEN_3
            TransferReg[0].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ2;
            TransferReg[0].u4Fld =SHURK0_SELPH_DQ2_DLY_OEN_DQ3;
            TransferReg[1].u4Addr = DRAMC_REG_SHURK0_SELPH_DQ0;
            TransferReg[1].u4Fld =SHURK0_SELPH_DQ0_TXDLY_OEN_DQ3;
            ExecuteMoveDramCDelay(p, TransferReg, iShiftUI);
        break;
	#endif
		default:
			break;
    }
}

//for LPDDR3 DQ delay line used
static void Set_RX_DQ_DelayLine_Phy_Byte(DRAMC_CTX_T *p, U8 u1ByteIdx, S8 value[8])
{
    switch(u1ByteIdx)
    {
        case 0: //BYTE0
            //DQ0  & DQ1
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ2, P_Fld(value[0], SHU1_R0_B0_DQ2_RK0_RX_ARDQ0_F_DLY_B0) | P_Fld(value[0], SHU1_R0_B0_DQ2_RK0_RX_ARDQ0_R_DLY_B0) |
                                                      P_Fld(value[1], SHU1_R0_B0_DQ2_RK0_RX_ARDQ1_F_DLY_B0) | P_Fld(value[1], SHU1_R0_B0_DQ2_RK0_RX_ARDQ1_R_DLY_B0) );
            //DQ2  & DQ3
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ3, P_Fld(value[2], SHU1_R0_B0_DQ3_RK0_RX_ARDQ2_F_DLY_B0) | P_Fld(value[2], SHU1_R0_B0_DQ3_RK0_RX_ARDQ2_R_DLY_B0) |
                                                      P_Fld(value[3], SHU1_R0_B0_DQ3_RK0_RX_ARDQ3_F_DLY_B0) | P_Fld(value[3], SHU1_R0_B0_DQ3_RK0_RX_ARDQ3_R_DLY_B0) );
            //DQ4  & DQ5
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ4, P_Fld(value[4], SHU1_R0_B0_DQ4_RK0_RX_ARDQ4_F_DLY_B0) | P_Fld(value[4], SHU1_R0_B0_DQ4_RK0_RX_ARDQ4_R_DLY_B0) |
                                                      P_Fld(value[5], SHU1_R0_B0_DQ4_RK0_RX_ARDQ5_F_DLY_B0) | P_Fld(value[5], SHU1_R0_B0_DQ4_RK0_RX_ARDQ5_R_DLY_B0) );
            //DQ6  & DQ7
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ5, P_Fld(value[6], SHU1_R0_B0_DQ5_RK0_RX_ARDQ6_F_DLY_B0) | P_Fld(value[6], SHU1_R0_B0_DQ5_RK0_RX_ARDQ6_R_DLY_B0) |
                                                      P_Fld(value[7], SHU1_R0_B0_DQ5_RK0_RX_ARDQ7_F_DLY_B0) | P_Fld(value[7], SHU1_R0_B0_DQ5_RK0_RX_ARDQ7_R_DLY_B0) );
        break;
        case 1: //BYTE1
            //DQ0  & DQ1
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ2, P_Fld(value[0], SHU1_R0_B1_DQ2_RK0_RX_ARDQ0_F_DLY_B1) | P_Fld(value[0], SHU1_R0_B1_DQ2_RK0_RX_ARDQ0_R_DLY_B1) |
                                                      P_Fld(value[1], SHU1_R0_B1_DQ2_RK0_RX_ARDQ1_F_DLY_B1) | P_Fld(value[1], SHU1_R0_B1_DQ2_RK0_RX_ARDQ1_R_DLY_B1) );
            //DQ2  & DQ3
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ3, P_Fld(value[2], SHU1_R0_B1_DQ3_RK0_RX_ARDQ2_F_DLY_B1) | P_Fld(value[2], SHU1_R0_B1_DQ3_RK0_RX_ARDQ2_R_DLY_B1) |
                                                      P_Fld(value[3], SHU1_R0_B1_DQ3_RK0_RX_ARDQ3_F_DLY_B1) | P_Fld(value[3], SHU1_R0_B1_DQ3_RK0_RX_ARDQ3_R_DLY_B1) );
            //DQ4  & DQ5
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ4, P_Fld(value[4], SHU1_R0_B1_DQ4_RK0_RX_ARDQ4_F_DLY_B1) | P_Fld(value[4], SHU1_R0_B1_DQ4_RK0_RX_ARDQ4_R_DLY_B1) |
                                                      P_Fld(value[5], SHU1_R0_B1_DQ4_RK0_RX_ARDQ5_F_DLY_B1) | P_Fld(value[5], SHU1_R0_B1_DQ4_RK0_RX_ARDQ5_R_DLY_B1) );
            //DQ6  & DQ7
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ5, P_Fld(value[6], SHU1_R0_B1_DQ5_RK0_RX_ARDQ6_F_DLY_B1) | P_Fld(value[6], SHU1_R0_B1_DQ5_RK0_RX_ARDQ6_R_DLY_B1) |
                                                      P_Fld(value[7], SHU1_R0_B1_DQ5_RK0_RX_ARDQ7_F_DLY_B1) | P_Fld(value[7], SHU1_R0_B1_DQ5_RK0_RX_ARDQ7_R_DLY_B1) );
        break;
	#if COMPILE_THIS_PART //cc mark since 7580 only supports X16 configuration.
        case 2: //BYTE2
            //DQ0  & DQ1
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ2+(1<<POS_BANK_NUM), P_Fld(value[0], SHU1_R0_B0_DQ2_RK0_RX_ARDQ0_F_DLY_B0) | P_Fld(value[0], SHU1_R0_B0_DQ2_RK0_RX_ARDQ0_R_DLY_B0) |
                                                      P_Fld(value[1], SHU1_R0_B0_DQ2_RK0_RX_ARDQ1_F_DLY_B0) | P_Fld(value[1], SHU1_R0_B0_DQ2_RK0_RX_ARDQ1_R_DLY_B0) );
            //DQ2  & DQ3
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ3+(1<<POS_BANK_NUM), P_Fld(value[2], SHU1_R0_B0_DQ3_RK0_RX_ARDQ2_F_DLY_B0) | P_Fld(value[2], SHU1_R0_B0_DQ3_RK0_RX_ARDQ2_R_DLY_B0) |
                                                      P_Fld(value[3], SHU1_R0_B0_DQ3_RK0_RX_ARDQ3_F_DLY_B0) | P_Fld(value[3], SHU1_R0_B0_DQ3_RK0_RX_ARDQ3_R_DLY_B0) );
            //DQ4  & DQ5
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ4+(1<<POS_BANK_NUM), P_Fld(value[4], SHU1_R0_B0_DQ4_RK0_RX_ARDQ4_F_DLY_B0) | P_Fld(value[4], SHU1_R0_B0_DQ4_RK0_RX_ARDQ4_R_DLY_B0) |
                                                      P_Fld(value[5], SHU1_R0_B0_DQ4_RK0_RX_ARDQ5_F_DLY_B0) | P_Fld(value[5], SHU1_R0_B0_DQ4_RK0_RX_ARDQ5_R_DLY_B0) );
            //DQ6  & DQ7
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ5+(1<<POS_BANK_NUM), P_Fld(value[6], SHU1_R0_B0_DQ5_RK0_RX_ARDQ6_F_DLY_B0) | P_Fld(value[6], SHU1_R0_B0_DQ5_RK0_RX_ARDQ6_R_DLY_B0) |
                                                      P_Fld(value[7], SHU1_R0_B0_DQ5_RK0_RX_ARDQ7_F_DLY_B0) | P_Fld(value[7], SHU1_R0_B0_DQ5_RK0_RX_ARDQ7_R_DLY_B0) );
        break;
        case 3: //BYTE3
            //DQ0  & DQ1
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ2+(1<<POS_BANK_NUM), P_Fld(value[0], SHU1_R0_B1_DQ2_RK0_RX_ARDQ0_F_DLY_B1) | P_Fld(value[0], SHU1_R0_B1_DQ2_RK0_RX_ARDQ0_R_DLY_B1) |
                                                      P_Fld(value[1], SHU1_R0_B1_DQ2_RK0_RX_ARDQ1_F_DLY_B1) | P_Fld(value[1], SHU1_R0_B1_DQ2_RK0_RX_ARDQ1_R_DLY_B1) );
            //DQ2  & DQ3
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ3+(1<<POS_BANK_NUM), P_Fld(value[2], SHU1_R0_B1_DQ3_RK0_RX_ARDQ2_F_DLY_B1) | P_Fld(value[2], SHU1_R0_B1_DQ3_RK0_RX_ARDQ2_R_DLY_B1) |
                                                      P_Fld(value[3], SHU1_R0_B1_DQ3_RK0_RX_ARDQ3_F_DLY_B1) | P_Fld(value[3], SHU1_R0_B1_DQ3_RK0_RX_ARDQ3_R_DLY_B1) );
            //DQ4  & DQ5
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ4+(1<<POS_BANK_NUM), P_Fld(value[4], SHU1_R0_B1_DQ4_RK0_RX_ARDQ4_F_DLY_B1) | P_Fld(value[4], SHU1_R0_B1_DQ4_RK0_RX_ARDQ4_R_DLY_B1) |
                                                      P_Fld(value[5], SHU1_R0_B1_DQ4_RK0_RX_ARDQ5_F_DLY_B1) | P_Fld(value[5], SHU1_R0_B1_DQ4_RK0_RX_ARDQ5_R_DLY_B1) );
            //DQ6  & DQ7
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ5+(1<<POS_BANK_NUM), P_Fld(value[6], SHU1_R0_B1_DQ5_RK0_RX_ARDQ6_F_DLY_B1) | P_Fld(value[6], SHU1_R0_B1_DQ5_RK0_RX_ARDQ6_R_DLY_B1) |
                                                      P_Fld(value[7], SHU1_R0_B1_DQ5_RK0_RX_ARDQ7_F_DLY_B1) | P_Fld(value[7], SHU1_R0_B1_DQ5_RK0_RX_ARDQ7_R_DLY_B1) );
        break;
	#endif
		default:
			break;
    }
}


//for LPDDR3 DQM delay line used
static void Set_RX_DQM_DelayLine_Phy_Byte(DRAMC_CTX_T *p, U8 u1ByteIdx, S8 value)
{
    switch(u1ByteIdx)
    {
        case 0:
            //DQM0
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ6, P_Fld(value,SHU1_R0_B0_DQ6_RK0_RX_ARDQM0_R_DLY_B0)
            				| P_Fld(value, SHU1_R0_B0_DQ6_RK0_RX_ARDQM0_F_DLY_B0));
        break;
        case 1:
            //DQM1
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ6, P_Fld(value,SHU1_R0_B1_DQ6_RK0_RX_ARDQM0_R_DLY_B1)
            				| P_Fld(value, SHU1_R0_B1_DQ6_RK0_RX_ARDQM0_F_DLY_B1));
        break;

	#if COMPILE_THIS_PART
        case 2:
            //DQM2
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ6+(1<<POS_BANK_NUM), P_Fld(value,SHU1_R0_B0_DQ6_RK0_RX_ARDQM0_R_DLY_B0)
            				| P_Fld(value, SHU1_R0_B0_DQ6_RK0_RX_ARDQM0_F_DLY_B0));
        break;
        case 3:
            //DQM3
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ6+(1<<POS_BANK_NUM), P_Fld(value,SHU1_R0_B1_DQ6_RK0_RX_ARDQM0_R_DLY_B1)
            				| P_Fld(value, SHU1_R0_B1_DQ6_RK0_RX_ARDQM0_F_DLY_B1));
        break;
	#endif
		default:
			break;
    }
}


#if SIMULATION_WRITE_LEVELING
#if WRITE_LEVELING_MOVE_DQS_INSTEAD_OF_CLK
static void WriteLevelingMoveDQSInsteadOfCLK(DRAMC_CTX_T *p)
{
    U8 u1ByteIdx, ucbit_num;
    U8 backup_rank, ii;

    backup_rank = u1GetRank(p);

#if COMPILE_THIS_PART
    if (p->pinmux == PIN_MUX_TYPE_DDR3X4)
        ucbit_num = 4;
    else
#endif
        ucbit_num = DQS_BIT_NUMBER;
	
    for(u1ByteIdx =0 ; u1ByteIdx<(p->data_width/ucbit_num); u1ByteIdx++)
    {
    	/* cc notes: set delay of DQS, DQS_OEN, DQ = 
    	 * (Default value - WRITE_LEVELING_MOVD_DQS)UI
    	 */
        MoveDramC_TX_DQS(p, u1ByteIdx, -WRITE_LEVELING_MOVD_DQS);

        MoveDramC_TX_DQS_OEN(p, u1ByteIdx, -WRITE_LEVELING_MOVD_DQS);

        for(ii=RANK_0; ii<RANK_MAX; ii++)
        {
        	vSetRank(p, ii);
        	MoveDramC_TX_DQ(p, u1ByteIdx, -WRITE_LEVELING_MOVD_DQS);
        }
        
        vSetRank(p, backup_rank);
    }
}
#endif


static void vSetDramMRWriteLevelingOnOff(DRAMC_CTX_T *p, U8 u1OnOff)
{
    // MR2 OP[7] to enable/disable write leveling
    if(u1OnOff)
        u1MR2Value |= 0x80;  // OP[7] WR LEV =1
    else
        u1MR2Value &= 0x7f;  // OP[7] WR LEV =0

    if (p->dram_type == TYPE_PCDDR3)
    {
    #if COMPILE_PCDDR3
        DramcModeRegWrite_PC3(p, 1, (u1OnOff<<7) | DDR_PC3_MR1);//cc change set RTT_NOM to RZQ/4
    #endif
    }
}


DRAM_STATUS_T DramcWriteLeveling(DRAMC_CTX_T *p)
{
    // Note that below procedure is based on "ODT off"
    //U32 *uiLPDDR_PHY_Mapping;
    U32 u4value, u4value1=0, u4dq_o1=0, u4dq_o1_tmp[DQS_NUMBER];
    U32 u4Value2T, u4Value05T;
    U8 byte_i, ucsample_count;
    S32 ii, ClockDelayMax;
    U8 ucsample_status[DQS_NUMBER], ucdq_o1_perbyte[DQS_NUMBER], ucdq_o1_index[DQS_NUMBER];
    U32 u4prv_register_1dc, u4prv_register_044, u4prv_register_0e4, u4prv_register_13c, u4prv_register_008;
    U32 u4prv_register_04c, u4prv_register_064, u4prv_register_038, u4prv_register_0bc, u4prv_register_024;
	U32 u4pre_register_shu_wodt;
    DRAM_RANK_T backup_rank;

    S32 wrlevel_dq_delay[DQS_NUMBER] = {0,0};	//YMC add to avoid warning
    S32 wrlevel_dqs_delay[2] = {0,0};

    #if WRITE_LEVELING_MOVE_DQS_INSTEAD_OF_CLK
    S32 i4PIBegin, i4PIEnd;
    #endif

	// error handling
    if (!p)
    {
        mcSHOW_ERR_MSG(("context is NULL\n"));
        return DRAM_FAIL;
    }

    Another_Rank_CKE_low(p, 0);

    fgwrlevel_done = 0;
    backup_rank = u1GetRank(p);

    DramcRankSwap(p, p->rank);

    //uiLPDDR_PHY_Mapping = (U32 *)uiDDR_PHY_Mapping[(U8)(p->pinmux)];

    // DQ mapping
    ///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    /// Note : uiLPDDR_PHY_Mapping_POP, need to take care mapping in real chip, but not in test chip.
    /// Everest : there is bit swap inside single byte. PHY & DRAM is 1-1 byte mapping, no swap.
    for (byte_i=0; byte_i<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); byte_i++)		//YMC mod to avoid waring
    {
        ucdq_o1_index[byte_i] = byte_i*8;
    }

#if REG_ACCESS_PORTING_DGB
    RegLogEnable =1;
    mcSHOW_DBG_MSG(("\n[REG_ACCESS_PORTING_FUNC]   DramcWriteLeveling\n"));
#endif

    // backup mode settings
    u4prv_register_04c = u4IO32Read4B(DRAMC_REG_REFCTRL0);
    u4prv_register_064 = u4IO32Read4B(DRAMC_REG_SPCMDCTRL);
    u4prv_register_038 = u4IO32Read4B(DRAMC_REG_DRAMC_PD_CTRL);
    u4prv_register_0bc = u4IO32Read4B(DRAMC_REG_WRITE_LEV);
    //CKEONªº­È
    u4prv_register_024 = u4IO32Read4B(DRAMC_REG_CKECTRL);
	u4pre_register_shu_wodt = u4IO32Read4B(DRAMC_REG_SHU1_WODT);

	//cc add
	vIO32WriteFldAlign(DDRPHY_B0_DQ3, 0x1, B0_DQ3_RG_RX_ARDQ_SMT_EN_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ3, 0x1, B1_DQ3_RG_RX_ARDQ_SMT_EN_B1);
	//vIO32WriteFldAlign(DDRPHY_B01_MISC, 0x1, B01_MISC_RG_ARPI_SMT_EN_B01);
	//vIO32WriteFldAlign(DDRPHY_CA_MISC, 0x1, CA_MISC_RG_ARPI_SMT_EN_CA);
	vIO32WriteFldAlign(DDRPHY_CA_CMD3, 0x1, CA_CMD3_RG_RX_ARCMD_SMT_EN);

	vIO32WriteFldAlign(DDRPHY_B0_DQ6, 0x1, B0_DQ6_RG_RX_ARDQ_BIAS_PS_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ6, 0x1, B1_DQ6_RG_RX_ARDQ_BIAS_PS_B1);
	vIO32WriteFldAlign(DDRPHY_CA_CMD6, 0x1, CA_CMD6_RG_RX_ARCMD_BIAS_PS);
	//cc add end
	
    //write leveling mode initialization
    //disable auto refresh, REFCNT_FR_CLK = 0 (0x1dc[23:16]), ADVREFEN = 0 (0x44[30]), (CONF2_REFCNT =0)
    vIO32WriteFldAlign(DRAMC_REG_REFCTRL0, 1, REFCTRL0_REFDIS);      //REFDIS=1, disable auto refresh
    vIO32WriteFldAlign(DRAMC_REG_DRAMC_PD_CTRL, 1, DRAMC_PD_CTRL_MIOCKCTRLOFF);   //MIOCKCTRLOFF=1
    vIO32WriteFldAlign(DRAMC_REG_DRAMC_PD_CTRL, 0, DRAMC_PD_CTRL_PHYCLKDYNGEN);   //PHYCLKDYNGEN=0

    //Make CKE fixed at 1 (Don't enter power down, Put this before issuing MRS): CKEFIXON = 1
    vIO32WriteFldAlign(DRAMC_REG_CKECTRL, 1, CKECTRL_CKEFIXON); //CKEFIXDON must be operated under MIOCKCTRLOFF = 1


    //ODT, DQIEN fixed at 1; FIXODT = 1 (0xd8[23]), FIXDQIEN = 1111 (0xd8[15:12])
    vIO32WriteFldAlign(DRAMC_REG_SHU1_WODT, 1, SHU1_WODT_WODTFIXOFF);

    //PHY RX Setting for Write Leveling
    //Let IO toO1 path valid, Enable SMT_EN
    O1PathOnOff(p, 1);

    // enable DDR write leveling mode:  issue MR2[7] to enable write leveling (refer to DEFAULT MR2 value)
    vSetDramMRWriteLevelingOnOff(p, ENABLE);

    //wait tWLDQSEN (25 nCK / 25ns) after enabling write leveling mode (DDR3 / LPDDDR3)
    mcDELAY_US(1);

	//cc add. to be confirmed: BX[3:0] is the same as above 8167 code??
	vIO32WriteFldAlign(DRAMC_REG_WRITE_LEV, 0xA, WRITE_LEV_DQSBX_G);

    vIO32WriteFldAlign(DRAMC_REG_WRITE_LEV, 1, WRITE_LEV_WRITE_LEVEL_EN);
    vIO32WriteFldAlign(DRAMC_REG_WRITE_LEV, 1, WRITE_LEV_CBTMASKDQSOE);

    // select DQS
    if (p->data_width == DATA_WIDTH_16BIT)
    {
        u4value = 0x3;//select byte 0.1
    }
    else
    {
        u4value = 0xf;//select byte 0.1.2.3
    }

    vIO32WriteFldAlign(DRAMC_REG_WRITE_LEV, u4value, WRITE_LEV_DQS_SEL);

    // wait tWLMRD (40 nCL / 40 ns) before DQS pulse (DDR3 / LPDDR3)
    mcDELAY_US(1);

    //Proceed write leveling...
    //Initilize sw parameters
    ClockDelayMax = MAX_TX_DQSDLY_TAPS;
    for (ii=0; ii < (S32)(DQ_DATA_WIDTH/DQS_BIT_NUMBER); ii++)
    {
        ucsample_status[ii] = 0;
        wrlevel_dqs_final_delay[p->rank][ii] = 0;
    }

    //used for WL done status
    // each bit of sample_cnt represents one-byte WL status
    // 1: done or N/A. 0: NOK
    if ((p->data_width == DATA_WIDTH_16BIT))
    {
        ucsample_count = 0xfc;
    }
    else
    {

        ucsample_count = 0xf0;
    }

    mcSHOW_DBG_MSG(("\n[Write Leveling]\n"));
	#if	dbg_print
	prom_puts("\n[Write Leveling]\n");
	#endif
    mcSHOW_DBG_MSG2(("delay  byte0  byte1  byte2  byte3\n"));
    mcSHOW_DBG_MSG2(("-----------------------------\n"));

    #if WRITE_LEVELING_MOVE_DQS_INSTEAD_OF_CLK
    if(p->arfgWriteLevelingInitShif[p->rank] ==FALSE)
    {
        WriteLevelingMoveDQSInsteadOfCLK(p);
        //p->arfgWriteLevelingInitShif[p->rank] =TRUE;
        p->arfgWriteLevelingInitShif[p->rank] =TRUE;
        //cc mark p->arfgWriteLevelingInitShif[RANK_1] =TRUE;
        #if TX_PERBIT_INIT_FLOW_CONTROL
        // both 2 rank use one write leveling result, TX need to udpate.
        p->fgTXPerbifInit[RANK_0]= FALSE;
        //cc mark p->fgTXPerbifInit[RANK_1]= FALSE;
        #endif

        mcSHOW_DBG_MSG(("WriteLevelingMoveDQSInsteadOfCLK\n"));
		#if	dbg_print
		prom_puts("WriteLevelingMoveDQSInsteadOfCLK\n");
		#endif
    }
    #endif

    // Set DQS output delay to 0
    //MT8167 TX DQS
    vIO32WriteFldAlign(DDRPHY_SHU1_R0_B0_DQ7, 0, SHU1_R0_B0_DQ7_RK0_ARPI_PBYTE_B0);  //rank0, byte0, DQS delay
    vIO32WriteFldAlign(DDRPHY_SHU1_R0_B1_DQ7, 0, SHU1_R0_B1_DQ7_RK0_ARPI_PBYTE_B1);  //rank0, byte1, DQS delay
    //cc mark vIO32WriteFldAlign(DDRPHY_SHU1_R0_B0_DQ7+(1<<POS_BANK_NUM), 0, SHU1_R0_B0_DQ7_RK0_ARPI_PBYTE_B0);  //rank0, byte2, DQS delay
    //cc mark vIO32WriteFldAlign(DDRPHY_SHU1_R0_B1_DQ7+(1<<POS_BANK_NUM), 0, SHU1_R0_B1_DQ7_RK0_ARPI_PBYTE_B1);  //rank0, byte3, DQS delay

    #if WRITE_LEVELING_MOVE_DQS_INSTEAD_OF_CLK
    i4PIBegin = WRITE_LEVELING_MOVD_DQS*32 -MAX_CLK_PI_DELAY-1;
    i4PIEnd = i4PIBegin + 64;
    #endif


    #if  WRITE_LEVELING_MOVE_DQS_INSTEAD_OF_CLK
    for (ii=i4PIBegin; ii<i4PIEnd; ii++)
    #else
    for (ii=(-MAX_CLK_PI_DELAY); ii<=MAX_TX_DQSDLY_TAPS; ii++)
    #endif
    {
        if (ii <= 0)
        {
            // Adjust Clk output delay.
            vIO32WriteFldAlign(DDRPHY_SHU1_R0_CA_CMD9, -ii, SHU1_R0_CA_CMD9_RG_RK0_ARPI_CLK);
        }
        else
        {
            // Adjust DQS output delay.
            // PI (TX DQ/DQS adjust at the same time)
            //MT8167 TX DQS
            vIO32WriteFldAlign(DDRPHY_SHU1_R0_B0_DQ7, ii, SHU1_R0_B0_DQ7_RK0_ARPI_PBYTE_B0);  //rank0, byte0, DQS delay
            vIO32WriteFldAlign(DDRPHY_SHU1_R0_B1_DQ7, ii, SHU1_R0_B1_DQ7_RK0_ARPI_PBYTE_B1);  //rank0, byte1, DQS delay
            //cc mark vIO32WriteFldAlign(DDRPHY_SHU1_R0_B0_DQ7+(1<<POS_BANK_NUM), ii, SHU1_R0_B0_DQ7_RK0_ARPI_PBYTE_B0);  //rank0, byte2, DQS delay
            //cc mark vIO32WriteFldAlign(DDRPHY_SHU1_R0_B1_DQ7+(1<<POS_BANK_NUM), ii, SHU1_R0_B1_DQ7_RK0_ARPI_PBYTE_B1);  //rank0, byte3, DQS delay

        }
        //Trigger DQS pulse, R_DQS_WLEV: 0x13c[8] from 1 to 0
        vIO32WriteFldAlign(DRAMC_REG_WRITE_LEV, 1, WRITE_LEV_DQS_WLEV);
        vIO32WriteFldAlign(DRAMC_REG_WRITE_LEV, 0, WRITE_LEV_DQS_WLEV);


        //wait tWLO (7.5ns / 20ns) before output (DDR3 / LPDDR3)
        mcDELAY_US(1);

        //Read DQ_O1 from register

        // Get DQ value.
        u4dq_o1 = u4IO32Read4B(DDRPHY_MISC_DQO1) & 0xffff;
        //cc mark u4dq_o1 |= (u4IO32Read4B(DDRPHY_MISC_DQO1+(1<<POS_BANK_NUM)) & 0xffff0000);
        //mcSHOW_DBG_MSG2(("DQ_O1: 0x%x\n", u4dq_o1));

	#if COMPILE_THIS_PART
        if (p->en_4bitMux == ENABLE)
        {
            u4dq_o1_tmp[0] = (u4dq_o1 >> 4) & 0xf;
            u4dq_o1_tmp[1] = (u4dq_o1 >> 8) & 0xf;
            //cc mark u4dq_o1_tmp[2] = (u4dq_o1 >> 20) & 0xf;
            //cc mark u4dq_o1_tmp[3] = (u4dq_o1 >> 24) & 0xf;
            u4dq_o1 &= (~(0xf << 4) & ~(0xf << 8)  & ~(0xf << 20) & ~(0xf << 24));
            u4dq_o1 |= (u4dq_o1_tmp[0] << 8) | (u4dq_o1_tmp[1] << 4);//cc mark | (u4dq_o1_tmp[2] << 24) | (u4dq_o1_tmp[3] << 20);
        }
	#endif

        mcSHOW_DBG_MSG2(("%d    ", ii));
		#if	dbg_print
		prom_puts("delay:");
		prom_print_dec(ii);
		#endif

        mcSHOW_DBG_MSG2(("0x%x    ", u4dq_o1));

        for (byte_i = 0; byte_i < (DQ_DATA_WIDTH/DQS_BIT_NUMBER);  byte_i++)
        {
            ucdq_o1_perbyte[byte_i] = (U8)((u4dq_o1>>ucdq_o1_index[byte_i]) & 0xff);

            mcSHOW_DBG_MSG2(("%x   ", ucdq_o1_perbyte[byte_i]));
			#if	dbg_print
			prom_puts(", byte:");
			prom_print_dec(byte_i);
			prom_puts(", DQ value:0x");
			prom_print_hex(ucdq_o1_perbyte[byte_i],2);
			#endif

            if ((ucsample_status[byte_i]==0) && (ucdq_o1_perbyte[byte_i]==0))
            {
                ucsample_status[byte_i] = 1;
            }
            else if ((ucsample_status[byte_i]>=1) && (ucdq_o1_perbyte[byte_i] ==0))
            {
                ucsample_status[byte_i] = 1;
            }
            else if ((ucsample_status[byte_i]>=1) && (ucdq_o1_perbyte[byte_i] !=0))
            {
                ucsample_status[byte_i]++;
            }
            //mcSHOW_DBG_MSG(("(%x) ", ucsample_status[byte_i]));

            if((ucsample_count &(0x01 << byte_i))==0)// result not found of byte yet
            {
                #if  WRITE_LEVELING_MOVE_DQS_INSTEAD_OF_CLK
                if((ucsample_status[byte_i] ==8) || ((ii==i4PIEnd-1) && (ucsample_status[byte_i] >1)))
                #else
                if((ucsample_status[byte_i] ==8) || ((ii==MAX_TX_DQSDLY_TAPS)&& (ucsample_status[byte_i] >1)))
                #endif
                {
                    wrlevel_dqs_final_delay[p->rank][byte_i] = ii -ucsample_status[byte_i] +2;
                    ucsample_count |= (0x01 << byte_i);
                    //mcSHOW_DBG_MSG(("(record %d) ", wrlevel_dqs_final_delay[p->rank][byte_i]));
                }
            }
        }
        mcSHOW_DBG_MSG2(("\n"));
		#if	dbg_print
		prom_puts("\n");
		#endif
        if (ucsample_count == 0xff)
            break;  // all byte found, early break.
    }

    if (ucsample_count == 0xff)
    {
        // all bytes are done
        fgwrlevel_done= 1;
        vSetCalibrationResult(p, DRAM_CALIBRATION_WRITE_LEVEL, DRAM_OK);
    }
    else
    {
        vSetCalibrationResult(p, DRAM_CALIBRATION_WRITE_LEVEL, DRAM_FAIL);
    }

    mcSHOW_DBG_MSG2(("pass bytecount = 0x%x (0xff means all bytes pass) \n\n", ucsample_count));
	#if	dbg_print
	prom_puts("pass bytecount = 0x");
	prom_print_hex(ucsample_count,2);
	prom_puts(" (0xff means all bytes pass)\n");
	#endif

    for (byte_i = 0; byte_i < (DQ_DATA_WIDTH/DQS_BIT_NUMBER);  byte_i++)
    {
        if (ClockDelayMax > wrlevel_dqs_final_delay[p->rank][byte_i])
        {
            ClockDelayMax = wrlevel_dqs_final_delay[p->rank][byte_i];
        }
    }

    if (ClockDelayMax > 0)
    {
        ClockDelayMax = 0;
    }
    else
    {
        ClockDelayMax = -ClockDelayMax;
    }

    vPrintCalibrationBasicInfo(p);

    mcSHOW_DBG_MSG(("WL Clk delay = %d, CA CLK delay = %d\n", ClockDelayMax, CATrain_ClkDelay));
	#if	dbg_print
	prom_puts("WL Clk delay = ");
	prom_print_dec(ClockDelayMax);
	prom_puts(", CA CLK delay = ");
	prom_print_dec(CATrain_ClkDelay);
	prom_puts("\n");
	#endif
#if defined(DRAM_CALIB_LOG) || defined(DRAM_ETT)
    gDRAM_CALIB_LOG.RANK[p->rank].Write_Leveling.Clk_delay = ClockDelayMax;
    gDRAM_CALIB_LOG.RANK[p->rank].Write_Leveling.CA_delay = CATrain_ClkDelay;
#endif

    // Adjust Clk & CA if needed
    if (CATrain_ClkDelay < ClockDelayMax)
    {
        S32 Diff = ClockDelayMax - CATrain_ClkDelay;
        mcSHOW_DBG_MSG(("CA adjust %d taps... \n", Diff));
		#if	dbg_print
		prom_puts("CA adjust ");
		prom_print_dec(Diff);
		prom_puts(" taps... \n");
		#endif
        // Write shift value into CA output delay.

        u4value = u4IO32ReadFldAlign(DDRPHY_SHU1_R0_CA_CMD9, SHU1_R0_CA_CMD9_RG_RK0_ARPI_CMD);
        u4value += Diff;

        //default value in init() is 0x10 , vIO32Write4B(mcSET_DDRPHY_REG_ADDR_CHC(0x0458), 0x00100000);   // Partickl
        vIO32WriteFldAlign(DDRPHY_SHU1_R0_CA_CMD9, u4value, SHU1_R0_CA_CMD9_RG_RK0_ARPI_CMD);

        mcSHOW_DBG_MSG(("[DramcWriteLeveling] Update Macro0 CA PI Delay = %d, Macro1 CA PI Delay = %d\n", u4value, u4value1));
		#if	dbg_print
		prom_puts("[DramcWriteLeveling] Update Macro0 CA PI Delay = ");
		prom_print_dec(u4value);
		prom_puts("\n");
		#endif
        // Write shift value into CS output delay.

        vIO32WriteFldAlign(DDRPHY_SHU1_R0_CA_CMD9, ClockDelayMax, SHU1_R0_CA_CMD9_RG_RK0_ARPI_CS);
        mcSHOW_DBG_MSG(("[DramcWriteLeveling] Update CS Delay = %d\n", ClockDelayMax));
		#if	dbg_print
		prom_puts("[DramcWriteLeveling] Update CS Delay = ");
		prom_print_dec(ClockDelayMax);
		prom_puts("\n");
		#endif
    }
    else
    {
    	mcSHOW_DBG_MSG(("No need to update CA/CS delay because the CLK delay is small than CA training.\n"));
		#if	dbg_print
		prom_puts("No need to update CA/CS delay because the CLK delay is small than CA training.\n");
		#endif
    	ClockDelayMax = CATrain_ClkDelay;
    }

    //DramcEnterSelfRefresh(p, 1);  //enter self refresh mode when changing CLK
    // Write max center value into Clk output delay.
    vIO32WriteFldAlign(DDRPHY_SHU1_R0_CA_CMD9, ClockDelayMax, SHU1_R0_CA_CMD9_RG_RK0_ARPI_CLK);
    //DramcEnterSelfRefresh(p, 0);

    mcSHOW_DBG_MSG(("Final Clk output delay = %d\n", ClockDelayMax));
	#if	dbg_print
	prom_puts("Final Clk output delay = ");
	prom_print_dec(ClockDelayMax);
	prom_puts("\n");
	#endif
    //mcSHOW_DBG_MSG(("After adjustment...\n"));

    u4Value2T= u4IO32Read4B(DRAMC_REG_SHU_SELPH_DQS0);
    u4Value05T= u4IO32Read4B(DRAMC_REG_SHU_SELPH_DQS1);
    for (byte_i = 0; byte_i < (DQ_DATA_WIDTH/DQS_BIT_NUMBER);  byte_i++)
    {
        wrlevel_dqs_final_delay[p->rank][byte_i] += (ClockDelayMax);

        mcSHOW_DBG_MSG(("R%d FINAL: WriteLeveling DQS:(%d, %d) OEN:(%d, %d) ", p->rank, (u4Value2T >> (byte_i*4)) & 0x7, (u4Value05T >> (byte_i*4)) & 0x3, \
			(u4Value2T >> (byte_i*4+16)) & 0x7, (u4Value05T >> (byte_i*4+16)) & 0x3));
        mcSHOW_DBG_MSG(("DQS%d delay =  %d\n", byte_i, wrlevel_dqs_final_delay[p->rank][byte_i]));
		#if	dbg_print
		prom_puts("DQS");
		prom_print_dec(byte_i);
		prom_puts(" delay = ");
		prom_print_dec(wrlevel_dqs_final_delay[p->rank][byte_i]);
		prom_puts("\n");
		#endif
    }

    // write leveling done, mode settings recovery if necessary
    // recover mode registers : issue MR2[7] to disable write leveling (refer to DEFAULT MR2 value)

#if 0
	//cc add. Send a precharge to DRAM. Because model will report DRAM NOT IDLE errors.
	//Model will also report some errors, but WL can pass anyway. Need confirm??
	vIO32WriteFldAlign(DRAMC_REG_SPCMD, 0x1, SPCMD_PREAEN);
	mcDELAY_US(10);
	vIO32WriteFldAlign(DRAMC_REG_SPCMD, 0x0, SPCMD_PREAEN);
	//cc add end
#endif

    vSetDramMRWriteLevelingOnOff(p, DISABLE);

    // restore registers.
    vIO32Write4B(DRAMC_REG_CKECTRL, u4prv_register_024);    //restore CKEFIXON value before MIOCKCTRLOFF restore
    vIO32Write4B(DRAMC_REG_REFCTRL0, u4prv_register_04c);
    vIO32Write4B(DRAMC_REG_SPCMDCTRL, u4prv_register_064);
    vIO32Write4B(DRAMC_REG_DRAMC_PD_CTRL, u4prv_register_038);
    vIO32Write4B(DRAMC_REG_WRITE_LEV, u4prv_register_0bc);
    vIO32Write4B(DRAMC_REG_SHU1_WODT, u4pre_register_shu_wodt);

    //Disable DQ_O1, SELO1ASO=0 for power saving
    O1PathOnOff(p, 0);

    for(byte_i=0; byte_i<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); byte_i++)
    {
        if(wrlevel_dqs_final_delay[p->rank][byte_i] >= 0x40) //ARPI_PBYTE_B* is 6 bits, max 0x40
        {
            wrlevel_dqs_delay[byte_i] = wrlevel_dqs_final_delay[p->rank][byte_i] - 0x40;
            MoveDramC_TX_DQS(p, byte_i, 2);
            MoveDramC_TX_DQS_OEN(p, byte_i, 2);
        }
        else
        {
            wrlevel_dqs_delay[byte_i] = wrlevel_dqs_final_delay[p->rank][byte_i];
        }
    }

    /* p->rank = RANK_0, save to Reg Rank0 and Rank1, p->rank = RANK_1, save to Reg Rank1 */
    for(ii=p->rank; ii<RANK_MAX; ii++)
    {
        vSetRank(p,ii);

    // set to best values for  DQS

        //MT8167 TX DQS
        vIO32WriteFldAlign(DDRPHY_SHU1_R0_B0_DQ7, wrlevel_dqs_delay[0], SHU1_R0_B0_DQ7_RK0_ARPI_PBYTE_B0);  //rank0, byte0, DQS delay
        vIO32WriteFldAlign(DDRPHY_SHU1_R0_B1_DQ7, wrlevel_dqs_delay[1], SHU1_R0_B1_DQ7_RK0_ARPI_PBYTE_B1);  //rank0, byte1, DQS delay
        //cc mark vIO32WriteFldAlign(DDRPHY_SHU1_R0_B0_DQ7+(1<<POS_BANK_NUM), wrlevel_dqs_delay[2], SHU1_R0_B0_DQ7_RK0_ARPI_PBYTE_B0);	//rank0, byte2, DQS delay
        //cc mark vIO32WriteFldAlign(DDRPHY_SHU1_R0_B1_DQ7+(1<<POS_BANK_NUM), wrlevel_dqs_delay[3], SHU1_R0_B1_DQ7_RK0_ARPI_PBYTE_B1);	//rank0, byte3, DQS delay
    }
    vSetRank(p,backup_rank);

    #if 1//EVEREST_CHANGE_OF_PHY_PBYTE
    //Evereest new change, ARPI_DQ_RK0_ARPI_PBYTE_B* only move DQS, not including of DQM&DQ anymore.
    //Add move DQ, DQ= DQS+0x10, after cali.  take care diff. UI. with DQS
    for(byte_i=0; byte_i<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); byte_i++)
    {
        wrlevel_dq_delay[byte_i] = wrlevel_dqs_final_delay[p->rank][byte_i] + 0x10;
        if(wrlevel_dq_delay[byte_i] >= 0x40) //ARPI_DQ_B* is 6 bits, max 0x40
        {
            wrlevel_dq_delay[byte_i] -= 0x40;
            MoveDramC_TX_DQ(p, byte_i, 2);
        }
    }

    /* p->rank = RANK_0, save to Reg Rank0 and Rank1, p->rank = RANK_1, save to Reg Rank1 */
    for(ii=p->rank; ii<RANK_MAX; ii++)
    {
        vSetRank(p,ii);
        #if 0
        mcSHOW_DBG_MSG(("rank%d\n", p->rank));
        u4Value2T= u4IO32Read4B(DRAMC_REG_SHURK0_SELPH_DQ0);
        u4Value05T= u4IO32Read4B(DRAMC_REG_SHURK0_SELPH_DQ2);
        for (byte_i = 0; byte_i < (p->data_width/DQS_BIT_NUMBER);  byte_i++)
        {
        	mcSHOW_DBG_MSG(("DQ:(%d, %d) OEN:(%d, %d)",(u4Value2T >> (byte_i*4)) & 0x7, (u4Value05T >> (byte_i*4)) & 0x3, \
        		(u4Value2T >> (byte_i*4+16)) & 0x7, (u4Value05T >> (byte_i*4+16)) & 0x3));
        	mcSHOW_DBG_MSG(("DQ%d delay =  %d\n", byte_i, wrlevel_dq_delay[byte_i]));
        }
        u4Value2T= u4IO32Read4B(DRAMC_REG_SHURK0_SELPH_DQ1);
        u4Value05T= u4IO32Read4B(DRAMC_REG_SHURK0_SELPH_DQ3);
        for (byte_i = 0; byte_i < (p->data_width/DQS_BIT_NUMBER);  byte_i++)
        {
        	mcSHOW_DBG_MSG(("DQM:(%d, %d) OEN:(%d, %d)", (u4Value2T >> (byte_i*4)) & 0x7, (u4Value05T >> (byte_i*4)) & 0x3, \
        		(u4Value2T >> (byte_i*4+16)) & 0x7, (u4Value05T >> (byte_i*4+16)) & 0x3));
        	mcSHOW_DBG_MSG(("DQM%d delay =  %d\n", byte_i, wrlevel_dq_delay[byte_i]));
        }
        #endif
        // set to best values for  DQ/DQM/DQ_OEN/DQM_OEN
        //MT8167 TX DQ/DQM
        // set to best values for  DQM, DQ
        vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ7, P_Fld(wrlevel_dq_delay[0], SHU1_R0_B0_DQ7_RK0_ARPI_DQM_B0) |
					P_Fld(wrlevel_dq_delay[0], SHU1_R0_B0_DQ7_RK0_ARPI_DQ_B0));
        vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ7, P_Fld(wrlevel_dq_delay[1], SHU1_R0_B1_DQ7_RK0_ARPI_DQM_B1) |
					P_Fld(wrlevel_dq_delay[1], SHU1_R0_B1_DQ7_RK0_ARPI_DQ_B1));
        //cc mark vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ7+(1<<POS_BANK_NUM), P_Fld(wrlevel_dq_delay[2], SHU1_R0_B0_DQ7_RK0_ARPI_DQM_B0) |
		//			P_Fld(wrlevel_dq_delay[2], SHU1_R0_B0_DQ7_RK0_ARPI_DQ_B0));
        //cc mark vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ7+(1<<POS_BANK_NUM), P_Fld(wrlevel_dq_delay[3], SHU1_R0_B1_DQ7_RK0_ARPI_DQM_B1) |
		//			P_Fld(wrlevel_dq_delay[3], SHU1_R0_B1_DQ7_RK0_ARPI_DQ_B1));
    }
    vSetRank(p,backup_rank);
    #endif

	//cc add
	vIO32WriteFldAlign(DDRPHY_B0_DQ3, 0x0, B0_DQ3_RG_RX_ARDQ_SMT_EN_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ3, 0x0, B1_DQ3_RG_RX_ARDQ_SMT_EN_B1);
	//vIO32WriteFldAlign(DDRPHY_B01_MISC, 0x0, B01_MISC_RG_ARPI_SMT_EN_B01);
	//vIO32WriteFldAlign(DDRPHY_CA_MISC, 0x0, CA_MISC_RG_ARPI_SMT_EN_CA);
	vIO32WriteFldAlign(DDRPHY_CA_CMD3, 0x0, CA_CMD3_RG_RX_ARCMD_SMT_EN);

	vIO32WriteFldAlign(DDRPHY_B0_DQ6, 0x0, B0_DQ6_RG_RX_ARDQ_BIAS_PS_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ6, 0x0, B1_DQ6_RG_RX_ARDQ_BIAS_PS_B1);
	vIO32WriteFldAlign(DDRPHY_CA_CMD6, 0x0, CA_CMD6_RG_RX_ARCMD_BIAS_PS);
	//cc add end

    DramcRankSwap(p, RANK_0);

#if REG_ACCESS_PORTING_DGB
    RegLogEnable =0;
#endif
    mcSHOW_DBG_MSG(("[DramcWriteLeveling] ====Done====\n"));
	#if	dbg_print
	prom_puts("[DramcWriteLeveling] ====Done====\n");
	#endif
#if defined(DRAM_CALIB_LOG) || defined(DRAM_ETT)
    gDRAM_CALIB_LOG.RANK[p->rank].Write_Leveling.Final_Clk_delay =  ClockDelayMax;
    gDRAM_CALIB_LOG.RANK[p->rank].Write_Leveling.DQS0_delay = wrlevel_dqs_final_delay[p->rank][0];
    gDRAM_CALIB_LOG.RANK[p->rank].Write_Leveling.DQS1_delay = wrlevel_dqs_final_delay[p->rank][1];
    //cc mark gDRAM_CALIB_LOG.RANK[p->rank].Write_Leveling.DQS2_delay = wrlevel_dqs_final_delay[p->rank][2];
    //cc mark gDRAM_CALIB_LOG.RANK[p->rank].Write_Leveling.DQS3_delay = wrlevel_dqs_final_delay[p->rank][3];
#endif

    Another_Rank_CKE_low(p, 1);

    return DRAM_OK;
}

#if COMPILE_THIS_PART
DRAM_STATUS_T DramcWriteLeveling4Bit(DRAMC_CTX_T *p, U8 u1DlyLine)
{
    // Note that below procedure is based on "ODT off"
    //U32 *uiLPDDR_PHY_Mapping;
    U32 u4value, u4value1=0, u4dq_o1=0, u4dq_o1_tmp[DQS_NUMBER];
    U32 u4Value2T, u4Value05T;
    U8 byte_i, ucsample_count, ucbit_num;
    static U8 ucSkewPI, ucSkewBit;
    S32 ii, ClockDelayMax;
    U8 ucsample_status[DQS_NUMBER], ucdq_o1_perbyte[DQS_NUMBER], ucdq_o1_index[DQS_NUMBER];
    U32 u4prv_register_1dc, u4prv_register_044, u4prv_register_0e4, u4prv_register_13c, u4prv_register_008;
    U32 u4prv_register_04c, u4prv_register_064, u4prv_register_038, u4prv_register_0bc, u4prv_register_024;
    DRAM_RANK_T backup_rank;

    S32 wrlevel_dq_delay[DQS_NUMBER];
    S32 wrlevel_dqs_delay[4] = {0,0,0,0};

    #if WRITE_LEVELING_MOVE_DQS_INSTEAD_OF_CLK
    S32 i4PIBegin, i4PIEnd;
    #endif

    if (u1DlyLine && ucSkewPI == 0)
    {
        mcSHOW_DBG_MSG(("DDR3X4 DQS1/3 NO SKEW\n"));
        p->density = 0;
        return DRAM_OK;
    }       
        
    Another_Rank_CKE_low(p, 0);

    fgwrlevel_done = 0;
    backup_rank = u1GetRank(p);

    // error handling
    if (!p)
    {
        mcSHOW_ERR_MSG(("context is NULL\n"));
        return DRAM_FAIL;
    }

    DramcRankSwap(p, p->rank);

    if (p->pinmux == PIN_MUX_TYPE_DDR3X4)
        ucbit_num = 4;
    else
        ucbit_num = DQS_BIT_NUMBER;

    //uiLPDDR_PHY_Mapping = (U32 *)uiDDR_PHY_Mapping[(U8)(p->pinmux)];

    // DQ mapping
    ///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    /// Note : uiLPDDR_PHY_Mapping_POP, need to take care mapping in real chip, but not in test chip.
    /// Everest : there is bit swap inside single byte. PHY & DRAM is 1-1 byte mapping, no swap.
    for (byte_i=0; byte_i<(p->data_width/ucbit_num); byte_i++)
    {
        ucdq_o1_index[byte_i] = byte_i*ucbit_num;
    }

    if (p->pinmux == PIN_MUX_TYPE_DDR3X4)
    {
        //swap B12 & B13
        ucdq_o1_index[2] = 16;
        ucdq_o1_index[3] = 20;
    }

    //backup wrlevel_dqs_final_delay
    if (u1DlyLine)
    {
        memcpy(wrlevel_dqs_delay, wrlevel_dqs_final_delay, sizeof(wrlevel_dqs_delay));
    }

#if REG_ACCESS_PORTING_DGB
    RegLogEnable =1;
    mcSHOW_DBG_MSG(("\n[REG_ACCESS_PORTING_FUNC]   DramcWriteLeveling\n"));
#endif

    // backup mode settings
    u4prv_register_04c = u4IO32Read4B(DRAMC_REG_REFCTRL0);
    u4prv_register_064 = u4IO32Read4B(DRAMC_REG_SPCMDCTRL);
    u4prv_register_038 = u4IO32Read4B(DRAMC_REG_DRAMC_PD_CTRL);
    u4prv_register_0bc = u4IO32Read4B(DRAMC_REG_WRITE_LEV);
    //CKEONªº­È
    u4prv_register_024 = u4IO32Read4B(DRAMC_REG_CKECTRL);

	//cc add
	vIO32WriteFldAlign(DDRPHY_B0_DQ3, 0x1, B0_DQ3_RG_RX_ARDQ_SMT_EN_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ3, 0x1, B1_DQ3_RG_RX_ARDQ_SMT_EN_B1);
	vIO32WriteFldAlign(DDRPHY_B01_MISC, 0x1, B01_MISC_RG_ARPI_SMT_EN_B01);
	vIO32WriteFldAlign(DDRPHY_CA_MISC, 0x1, CA_MISC_RG_ARPI_SMT_EN_CA);
	vIO32WriteFldAlign(DDRPHY_CA_CMD3, 0x1, CA_CMD3_RG_RX_ARCMD_SMT_EN);

	vIO32WriteFldAlign(DDRPHY_B0_DQ6, 0x1, B0_DQ6_RG_RX_ARDQ_BIAS_PS_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ6, 0x1, B1_DQ6_RG_RX_ARDQ_BIAS_PS_B1);
	vIO32WriteFldAlign(DDRPHY_CA_CMD6, 0x1, CA_CMD6_RG_RX_ARCMD_BIAS_PS);
	//cc add end

    //write leveling mode initialization
    //disable auto refresh, REFCNT_FR_CLK = 0 (0x1dc[23:16]), ADVREFEN = 0 (0x44[30]), (CONF2_REFCNT =0)
    vIO32WriteFldAlign(DRAMC_REG_REFCTRL0, 1, REFCTRL0_REFDIS);      //REFDIS=1, disable auto refresh
    vIO32WriteFldAlign(DRAMC_REG_DRAMC_PD_CTRL, 1, DRAMC_PD_CTRL_MIOCKCTRLOFF);   //MIOCKCTRLOFF=1
    vIO32WriteFldAlign(DRAMC_REG_DRAMC_PD_CTRL, 0, DRAMC_PD_CTRL_PHYCLKDYNGEN);   //PHYCLKDYNGEN=0

    //Make CKE fixed at 1 (Don't enter power down, Put this before issuing MRS): CKEFIXON = 1
    vIO32WriteFldAlign(DRAMC_REG_CKECTRL, 1, CKECTRL_CKEFIXON); //CKEFIXDON must be operated under MIOCKCTRLOFF = 1


    //ODT, DQIEN fixed at 1; FIXODT = 1 (0xd8[23]), FIXDQIEN = 1111 (0xd8[15:12])
    vIO32WriteFldAlign(DRAMC_REG_SHU1_WODT, 1, SHU1_WODT_WODTFIXOFF);

    //PHY RX Setting for Write Leveling
    //Let IO toO1 path valid, Enable SMT_EN
    O1PathOnOff(p, 1);

    // enable DDR write leveling mode:  issue MR2[7] to enable write leveling (refer to DEFAULT MR2 value)
    vSetDramMRWriteLevelingOnOff(p, ENABLE);

    //wait tWLDQSEN (25 nCK / 25ns) after enabling write leveling mode (DDR3 / LPDDDR3)
    mcDELAY_US(1);

    //Set {R_DQS_B3_G R_DQS_B2_G R_DQS_B1_G R_DQS_B0_G}=1010: 0x13c[4:1] (this depends on sel_ph setting)
    //Enable Write leveling: 0x13c[0]
    //vIO32WriteFldMulti(DRAMC_REG_WRITE_LEVELING, P_Fld(0xa, WRITE_LEVELING_DQSBX_G)|P_Fld(1, WRITE_LEVELING_WRITE_LEVEL_EN));
#if (fcFOR_CHIP_ID == MT8167)
	vIO32WriteFldMulti(DRAMC_REG_WRITE_LEV, P_Fld(0x1, WRITE_LEV_DQS_B3_G)
                                                | P_Fld(0x0, WRITE_LEV_DQS_B2_G)
                                                | P_Fld(0x1, WRITE_LEV_DQS_B1_G)
                                                | P_Fld(0x0, WRITE_LEV_DQS_B0_G));
#elif (fcFOR_CHIP_ID == EN7580)	
//cc add. to be confirmed: BX[3:0] is the same as above 8167 code??
vIO32WriteFldAlign(DRAMC_REG_WRITE_LEV, 0xA, WRITE_LEV_DQSBX_G);
#endif
    vIO32WriteFldAlign(DRAMC_REG_WRITE_LEV, 1, WRITE_LEV_WRITE_LEVEL_EN);
    vIO32WriteFldAlign(DRAMC_REG_WRITE_LEV, 1, WRITE_LEV_CBTMASKDQSOE);

    // select DQS
    if (p->data_width == DATA_WIDTH_16BIT && p->pinmux != PIN_MUX_TYPE_DDR3X4)
    {
        u4value = 0x3;//select byte 0.1
    }
    else
    {
        u4value = 0xf;//select byte 0.1.2.3
    }

    vIO32WriteFldAlign(DRAMC_REG_WRITE_LEV, u4value, WRITE_LEV_DQS_SEL);

    // wait tWLMRD (40 nCL / 40 ns) before DQS pulse (DDR3 / LPDDR3)
    mcDELAY_US(1);

    //Proceed write leveling...
    //Initilize sw parameters
    ClockDelayMax = MAX_TX_DQSDLY_TAPS;
    for (ii=0; ii < (S32)(p->data_width/ucbit_num); ii++)
    {
        ucsample_status[ii] = 0;
        wrlevel_dqs_final_delay[ii] = 0;
    }

    //used for WL done status
    // each bit of sample_cnt represents one-byte WL status
    // 1: done or N/A. 0: NOK
    if (p->data_width == DATA_WIDTH_16BIT && p->pinmux != PIN_MUX_TYPE_DDR3X4)
    {
        ucsample_count = 0xfc;
    }
    else
    {
        ucsample_count = 0xf0;
    }

    mcSHOW_DBG_MSG(("\n[Write Leveling]\n"));
    mcSHOW_DBG_MSG2(("delay  byte0  byte1  byte2  byte3\n"));
    mcSHOW_DBG_MSG2(("-----------------------------\n"));

    if (!u1DlyLine)
    {
        #if WRITE_LEVELING_MOVE_DQS_INSTEAD_OF_CLK
        if(p->arfgWriteLevelingInitShif[p->rank] ==FALSE)
        {
            WriteLevelingMoveDQSInsteadOfCLK(p);
            //p->arfgWriteLevelingInitShif[p->rank] =TRUE;
            p->arfgWriteLevelingInitShif[RANK_0] =TRUE;
            //cc mark p->arfgWriteLevelingInitShif[RANK_1] =TRUE;
            #if TX_PERBIT_INIT_FLOW_CONTROL
            // both 2 rank use one write leveling result, TX need to udpate.
            p->fgTXPerbifInit[RANK_0]= FALSE;
            //cc mark p->fgTXPerbifInit[RANK_1]= FALSE;
            #endif

            mcSHOW_DBG_MSG(("WriteLevelingMoveDQSInsteadOfCLK\n"));
        }
        #endif

        // Set DQS output delay to 0
        vIO32WriteFldAlign(DDRPHY_SHU1_R0_B0_DQ7, 0, SHU1_R0_B0_DQ7_RK0_ARPI_PBYTE_B0);  //rank0, byte0, DQS delay
        vIO32WriteFldAlign(DDRPHY_SHU1_R0_B1_DQ7, 0, SHU1_R0_B1_DQ7_RK0_ARPI_PBYTE_B1);  //rank0, byte1, DQS delay
        //vIO32WriteFldAlign(DDRPHY_SHU1_R0_B0_DQ7+(1<<POS_BANK_NUM), 0, SHU1_R0_B0_DQ7_RK0_ARPI_PBYTE_B0);  //rank0, byte2, DQS delay
        //vIO32WriteFldAlign(DDRPHY_SHU1_R0_B1_DQ7+(1<<POS_BANK_NUM), 0, SHU1_R0_B1_DQ7_RK0_ARPI_PBYTE_B1);  //rank0, byte3, DQS delay

        #if WRITE_LEVELING_MOVE_DQS_INSTEAD_OF_CLK
        i4PIBegin = WRITE_LEVELING_MOVD_DQS*32 -MAX_CLK_PI_DELAY-1;
        i4PIEnd = i4PIBegin + 64;
        #endif
    }
    else
    {
        i4PIBegin = 0;
        i4PIEnd = 15;
    }

    #if  WRITE_LEVELING_MOVE_DQS_INSTEAD_OF_CLK
    for (ii=i4PIBegin; ii<i4PIEnd; ii++)
    #else
    for (ii=(-MAX_CLK_PI_DELAY); ii<=MAX_TX_DQSDLY_TAPS; ii++)
    #endif
    {
        if (!u1DlyLine)
        {
            if (ii <= 0)
            {
                // Adjust Clk output delay.
                vIO32WriteFldAlign(DDRPHY_SHU1_R0_CA_CMD9, -ii, SHU1_R0_CA_CMD9_RG_RK0_ARPI_CLK);
            }
            else
            {
                // Adjust DQS output delay.
                // PI (TX DQ/DQS adjust at the same time)
                //MT8167 TX DQS
                vIO32WriteFldAlign(DDRPHY_SHU1_R0_B0_DQ7, ii, SHU1_R0_B0_DQ7_RK0_ARPI_PBYTE_B0);  //rank0, byte0, DQS delay
                vIO32WriteFldAlign(DDRPHY_SHU1_R0_B1_DQ7, ii, SHU1_R0_B1_DQ7_RK0_ARPI_PBYTE_B1);  //rank0, byte1, DQS delay
                //vIO32WriteFldAlign(DDRPHY_SHU1_R0_B0_DQ7+(1<<POS_BANK_NUM), ii, SHU1_R0_B0_DQ7_RK0_ARPI_PBYTE_B0);  //rank0, byte2, DQS delay
                //vIO32WriteFldAlign(DDRPHY_SHU1_R0_B1_DQ7+(1<<POS_BANK_NUM), ii, SHU1_R0_B1_DQ7_RK0_ARPI_PBYTE_B1);  //rank0, byte3, DQS delay

            }
        }
        else
        {
            //Adjust DQS Delay Line
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ1, P_Fld(ii, SHU1_R0_B0_DQ1_RK0_TX_ARDQS0B_DLY_B0) | P_Fld(ii, SHU1_R0_B0_DQ1_RK0_TX_ARDQS0_DLY_B0) \
                                                        | P_Fld(ii, SHU1_R0_B0_DQ1_RK0_TX_ARDQS0B_DLYB_B0) | P_Fld(ii, SHU1_R0_B0_DQ1_RK0_TX_ARDQS0_DLYB_B0));
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ1, P_Fld(ii, SHU1_R0_B1_DQ1_RK0_TX_ARDQS0B_DLY_B1) | P_Fld(ii, SHU1_R0_B1_DQ1_RK0_TX_ARDQS0_DLY_B1) \
                                                        | P_Fld(ii, SHU1_R0_B1_DQ1_RK0_TX_ARDQS0B_DLYB_B1) | P_Fld(ii, SHU1_R0_B1_DQ1_RK0_TX_ARDQS0_DLYB_B1));  
        }
        //Trigger DQS pulse, R_DQS_WLEV: 0x13c[8] from 1 to 0
        vIO32WriteFldAlign(DRAMC_REG_WRITE_LEV, 1, WRITE_LEV_DQS_WLEV);
        vIO32WriteFldAlign(DRAMC_REG_WRITE_LEV, 0, WRITE_LEV_DQS_WLEV);


        //wait tWLO (7.5ns / 20ns) before output (DDR3 / LPDDR3)
        mcDELAY_US(1);

        //Read DQ_O1 from register

        // Get DQ value.
        u4dq_o1 = u4IO32Read4B(DDRPHY_MISC_DQO1) & 0xffff;
        //cc mark u4dq_o1 |= (u4IO32Read4B(DDRPHY_MISC_DQO1+(1<<POS_BANK_NUM)) & 0xffff0000);
        //mcSHOW_DBG_MSG2(("DQ_O1: 0x%x\n", u4dq_o1));

        if (p->en_4bitMux == ENABLE)
        {
            u4dq_o1_tmp[0] = (u4dq_o1 >> 4) & 0xf;
            u4dq_o1_tmp[1] = (u4dq_o1 >> 8) & 0xf;
            u4dq_o1_tmp[2] = (u4dq_o1 >> 20) & 0xf;
            u4dq_o1_tmp[3] = (u4dq_o1 >> 24) & 0xf;
            u4dq_o1 &= (~(0xf << 4) & ~(0xf << 8)  & ~(0xf << 20) & ~(0xf << 24));
            u4dq_o1 |= (u4dq_o1_tmp[0] << 8) | (u4dq_o1_tmp[1] << 4) | (u4dq_o1_tmp[2] << 24) | (u4dq_o1_tmp[3] << 20);
        }

        mcSHOW_DBG_MSG2(("%d    ", ii));

        mcSHOW_DBG_MSG2(("0x%x    ", u4dq_o1));

        for (byte_i = 0; byte_i < (p->data_width/ucbit_num);  byte_i++)
        {
            if (p->pinmux != PIN_MUX_TYPE_DDR3X4)
                ucdq_o1_perbyte[byte_i] = (U8)((u4dq_o1>>ucdq_o1_index[byte_i]) & 0xff);
            else
                ucdq_o1_perbyte[byte_i] = (U8)((u4dq_o1>>ucdq_o1_index[byte_i]) & 0x0f);
            mcSHOW_DBG_MSG2(("%x   ", ucdq_o1_perbyte[byte_i]));

            if ((ucsample_status[byte_i]==0) && ((ucdq_o1_perbyte[byte_i]==0) || (u1DlyLine && ucdq_o1_perbyte[byte_i]!=0)))
            {
                if (u1DlyLine && ucdq_o1_perbyte[byte_i]!=0)
                    ucsample_status[byte_i] = 2;
                else
                    ucsample_status[byte_i] = 1;
            }
            else if ((ucsample_status[byte_i]>=1) && (ucdq_o1_perbyte[byte_i] ==0))
            {
                ucsample_status[byte_i] = 1;
            }
            else if ((ucsample_status[byte_i]>=1) && (ucdq_o1_perbyte[byte_i] !=0))
            {
                ucsample_status[byte_i]++;
            }
            //mcSHOW_DBG_MSG(("(%x) ", ucsample_status[byte_i]));

            if((ucsample_count &(0x01 << byte_i))==0)// result not found of byte yet
            {
                #if  WRITE_LEVELING_MOVE_DQS_INSTEAD_OF_CLK
                if((ucsample_status[byte_i] ==8) || ((ii==i4PIEnd-1) && (ucsample_status[byte_i] >1)))
                #else
                if((ucsample_status[byte_i] ==8) || ((ii==MAX_TX_DQSDLY_TAPS)&& (ucsample_status[byte_i] >1)))
                #endif
                {
                    wrlevel_dqs_final_delay[byte_i] = ii -ucsample_status[byte_i] +2;
                    ucsample_count |= (0x01 << byte_i);
                    //mcSHOW_DBG_MSG(("(record %d) ", wrlevel_dqs_final_delay[byte_i]));
                }
            }
        }
        mcSHOW_DBG_MSG2(("\n"));

        if (ucsample_count == 0xff)
            break;  // all byte found, early break.
        else if (u1DlyLine && (ucsample_count>>ucSkewBit) & 1)
            break; //K Dly Line need only check one skew bit
    }

    if ((ucsample_count == 0xff) || (u1DlyLine && (ucsample_count>>ucSkewBit) & 1))
    {
        // all bytes are done
        fgwrlevel_done= 1;
        vSetCalibrationResult(p, DRAM_CALIBRATION_WRITE_LEVEL, DRAM_OK);
    }
    else
    {
        vSetCalibrationResult(p, DRAM_CALIBRATION_WRITE_LEVEL, DRAM_FAIL);
    }

    mcSHOW_DBG_MSG2(("pass bytecount = 0x%x (0xff means all bytes pass) \n\n", ucsample_count));

    if (!u1DlyLine)
    {
        if (p->pinmux == PIN_MUX_TYPE_DDR3X4)
        {
            //first PI K, dqs will be set the smallest, the larger PI bit needs to K Dly Line
            if (wrlevel_dqs_final_delay[1] > wrlevel_dqs_final_delay[3])
            {
                ucSkewPI = wrlevel_dqs_final_delay[1] - wrlevel_dqs_final_delay[3];
                ucSkewBit = 1; 
                wrlevel_dqs_final_delay[1] = wrlevel_dqs_final_delay[3];
            }
            else
            {
                ucSkewPI = wrlevel_dqs_final_delay[3] - wrlevel_dqs_final_delay[1];
                ucSkewBit = 3;
                wrlevel_dqs_final_delay[3] = wrlevel_dqs_final_delay[1];
            }

            mcSHOW_DBG_MSG(("DDR3X4 (ucSkewBit)DQS%d-DQS%d = (ucSkewPI)%d\n", ucSkewBit, 4-ucSkewBit, ucSkewPI));
        }

        for (byte_i = 0; byte_i < (p->data_width/ucbit_num);  byte_i++)
        {
            if (ClockDelayMax > wrlevel_dqs_final_delay[byte_i])
            {
                ClockDelayMax = wrlevel_dqs_final_delay[byte_i];
            }
        }

        if (ClockDelayMax > 0)
        {
            ClockDelayMax = 0;
        }
        else
        {
            ClockDelayMax = -ClockDelayMax;
        }

        vPrintCalibrationBasicInfo(p);

        mcSHOW_DBG_MSG(("WL Clk delay = %d, CA CLK delay = %d\n", ClockDelayMax, CATrain_ClkDelay));

#if defined(DRAM_CALIB_LOG) || defined(DRAM_ETT)
        gDRAM_CALIB_LOG.RANK[p->rank].Write_Leveling.Clk_delay = ClockDelayMax;
        gDRAM_CALIB_LOG.RANK[p->rank].Write_Leveling.CA_delay = CATrain_ClkDelay;
#endif

        // Adjust Clk & CA if needed
        if (CATrain_ClkDelay < ClockDelayMax)
        {
            S32 Diff = ClockDelayMax - CATrain_ClkDelay;
            mcSHOW_DBG_MSG(("CA adjust %d taps... \n", Diff));

            // Write shift value into CA output delay.

            u4value = u4IO32ReadFldAlign(DDRPHY_SHU1_R0_CA_CMD9, SHU1_R0_CA_CMD9_RG_RK0_ARPI_CMD);
            u4value += Diff;

            //default value in init() is 0x10 , vIO32Write4B(mcSET_DDRPHY_REG_ADDR_CHC(0x0458), 0x00100000);   // Partickl
            vIO32WriteFldAlign(DDRPHY_SHU1_R0_CA_CMD9, u4value, SHU1_R0_CA_CMD9_RG_RK0_ARPI_CMD);

            mcSHOW_DBG_MSG(("[DramcWriteLeveling] Update Macro0 CA PI Delay = %d, Macro1 CA PI Delay = %d\n", u4value, u4value1));

            // Write shift value into CS output delay.

            vIO32WriteFldAlign(DDRPHY_SHU1_R0_CA_CMD9, ClockDelayMax, SHU1_R0_CA_CMD9_RG_RK0_ARPI_CS);
            mcSHOW_DBG_MSG(("[DramcWriteLeveling] Update CS Delay = %d\n", ClockDelayMax));

        }
        else
        {
            mcSHOW_DBG_MSG(("No need to update CA/CS delay because the CLK delay is small than CA training.\n"));
            ClockDelayMax = CATrain_ClkDelay;
        }

        //DramcEnterSelfRefresh(p, 1);  //enter self refresh mode when changing CLK
        // Write max center value into Clk output delay.
        vIO32WriteFldAlign(DDRPHY_SHU1_R0_CA_CMD9, ClockDelayMax, SHU1_R0_CA_CMD9_RG_RK0_ARPI_CLK);
        //DramcEnterSelfRefresh(p, 0);

        mcSHOW_DBG_MSG(("Final Clk output delay = %d\n", ClockDelayMax));
        //mcSHOW_DBG_MSG(("After adjustment...\n"));

        u4Value2T= u4IO32Read4B(DRAMC_REG_SHU_SELPH_DQS0);
        u4Value05T= u4IO32Read4B(DRAMC_REG_SHU_SELPH_DQS1);
        for (byte_i = 0; byte_i < (p->data_width/ucbit_num);  byte_i++)
        {
            wrlevel_dqs_final_delay[byte_i] += (ClockDelayMax);

            mcSHOW_DBG_MSG(("R%d FINAL: WriteLeveling DQS:(%d, %d) OEN:(%d, %d) ", p->rank, (u4Value2T >> (byte_i*4)) & 0x7, (u4Value05T >> (byte_i*4)) & 0x3, \
                (u4Value2T >> (byte_i*4+16)) & 0x7, (u4Value05T >> (byte_i*4+16)) & 0x3));
            mcSHOW_DBG_MSG(("DQS%d delay =  %d\n", byte_i, wrlevel_dqs_final_delay[byte_i]));
        }
    }
    // write leveling done, mode settings recovery if necessary
    // recover mode registers : issue MR2[7] to disable write leveling (refer to DEFAULT MR2 value)

    vSetDramMRWriteLevelingOnOff(p, DISABLE);

    // restore registers.
    vIO32Write4B(DRAMC_REG_CKECTRL, u4prv_register_024);    //restore CKEFIXON value before MIOCKCTRLOFF restore
    vIO32Write4B(DRAMC_REG_REFCTRL0, u4prv_register_04c);
    vIO32Write4B(DRAMC_REG_SPCMDCTRL, u4prv_register_064);
    vIO32Write4B(DRAMC_REG_DRAMC_PD_CTRL, u4prv_register_038);
    vIO32Write4B(DRAMC_REG_WRITE_LEV, u4prv_register_0bc);

    //Disable DQ_O1, SELO1ASO=0 for power saving
    O1PathOnOff(p, 0);

    if (!u1DlyLine)
    {
        for(byte_i=0; byte_i<(p->data_width/ucbit_num); byte_i++)
        {
            if(wrlevel_dqs_final_delay [byte_i] >= 0x40) //ARPI_PBYTE_B* is 6 bits, max 0x40
            {
                wrlevel_dqs_delay[byte_i] = wrlevel_dqs_final_delay [byte_i] - 0x40;
                MoveDramC_TX_DQS(p, byte_i, 2);
                MoveDramC_TX_DQS_OEN(p, byte_i, 2);
            }
            else
            {
                wrlevel_dqs_delay[byte_i] = wrlevel_dqs_final_delay [byte_i];
            }
        }

        /* p->rank = RANK_0, save to Reg Rank0 and Rank1, p->rank = RANK_1, save to Reg Rank1 */
        for(ii=p->rank; ii<RANK_MAX; ii++)
        {
            vSetRank(p,ii);

            // set to best values for  DQS
            vIO32WriteFldAlign(DDRPHY_SHU1_R0_B0_DQ7, wrlevel_dqs_delay[0], SHU1_R0_B0_DQ7_RK0_ARPI_PBYTE_B0);  //rank0, byte0, DQS delay
            vIO32WriteFldAlign(DDRPHY_SHU1_R0_B1_DQ7, wrlevel_dqs_delay[1], SHU1_R0_B1_DQ7_RK0_ARPI_PBYTE_B1);  //rank0, byte1, DQS delay
           // vIO32WriteFldAlign(DDRPHY_SHU1_R0_B0_DQ7+(1<<POS_BANK_NUM), wrlevel_dqs_delay[2], SHU1_R0_B0_DQ7_RK0_ARPI_PBYTE_B0);	//rank0, byte2, DQS delay
            //vIO32WriteFldAlign(DDRPHY_SHU1_R0_B1_DQ7+(1<<POS_BANK_NUM), wrlevel_dqs_delay[3], SHU1_R0_B1_DQ7_RK0_ARPI_PBYTE_B1);	//rank0, byte3, DQS delay
        }
        vSetRank(p,backup_rank);

        #if 1//EVEREST_CHANGE_OF_PHY_PBYTE
        //Evereest new change, ARPI_DQ_RK0_ARPI_PBYTE_B* only move DQS, not including of DQM&DQ anymore.
        //Add move DQ, DQ= DQS+0x10, after cali.  take care diff. UI. with DQS
        for(byte_i=0; byte_i<(p->data_width/ucbit_num); byte_i++)
        {
            wrlevel_dq_delay[byte_i] = wrlevel_dqs_final_delay [byte_i] + 0x10;
            if(wrlevel_dq_delay[byte_i] >= 0x40) //ARPI_DQ_B* is 6 bits, max 0x40
            {
                wrlevel_dq_delay[byte_i] -= 0x40;
                MoveDramC_TX_DQ(p, byte_i, 2);
            }
        }

        /* p->rank = RANK_0, save to Reg Rank0 and Rank1, p->rank = RANK_1, save to Reg Rank1 */
        for(ii=p->rank; ii<RANK_MAX; ii++)
        {
            vSetRank(p,ii);
            #if 0
            mcSHOW_DBG_MSG(("rank%d\n", p->rank));
            u4Value2T= u4IO32Read4B(DRAMC_REG_SHURK0_SELPH_DQ0);
            u4Value05T= u4IO32Read4B(DRAMC_REG_SHURK0_SELPH_DQ2);
            for (byte_i = 0; byte_i < (p->data_width/DQS_BIT_NUMBER);  byte_i++)
            {
            	mcSHOW_DBG_MSG(("DQ:(%d, %d) OEN:(%d, %d)",(u4Value2T >> (byte_i*4)) & 0x7, (u4Value05T >> (byte_i*4)) & 0x3, \
            		(u4Value2T >> (byte_i*4+16)) & 0x7, (u4Value05T >> (byte_i*4+16)) & 0x3));
            	mcSHOW_DBG_MSG(("DQ%d delay =  %d\n", byte_i, wrlevel_dq_delay[byte_i]));
            }
            u4Value2T= u4IO32Read4B(DRAMC_REG_SHURK0_SELPH_DQ1);
            u4Value05T= u4IO32Read4B(DRAMC_REG_SHURK0_SELPH_DQ3);
            for (byte_i = 0; byte_i < (p->data_width/DQS_BIT_NUMBER);  byte_i++)
            {
            	mcSHOW_DBG_MSG(("DQM:(%d, %d) OEN:(%d, %d)", (u4Value2T >> (byte_i*4)) & 0x7, (u4Value05T >> (byte_i*4)) & 0x3, \
            		(u4Value2T >> (byte_i*4+16)) & 0x7, (u4Value05T >> (byte_i*4+16)) & 0x3));
            	mcSHOW_DBG_MSG(("DQM%d delay =  %d\n", byte_i, wrlevel_dq_delay[byte_i]));
            }
            #endif
            // set to best values for  DQ/DQM/DQ_OEN/DQM_OEN
            //MT8167 TX DQ/DQM
            // set to best values for  DQM, DQ
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ7, P_Fld(wrlevel_dq_delay[0], SHU1_R0_B0_DQ7_RK0_ARPI_DQM_B0) |
    					P_Fld(wrlevel_dq_delay[0], SHU1_R0_B0_DQ7_RK0_ARPI_DQ_B0));
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ7, P_Fld(wrlevel_dq_delay[1], SHU1_R0_B1_DQ7_RK0_ARPI_DQM_B1) |
    					P_Fld(wrlevel_dq_delay[1], SHU1_R0_B1_DQ7_RK0_ARPI_DQ_B1));
            //vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ7+(1<<POS_BANK_NUM), P_Fld(wrlevel_dq_delay[2], SHU1_R0_B0_DQ7_RK0_ARPI_DQM_B0) |
    		//			P_Fld(wrlevel_dq_delay[2], SHU1_R0_B0_DQ7_RK0_ARPI_DQ_B0));
            //vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ7+(1<<POS_BANK_NUM), P_Fld(wrlevel_dq_delay[3], SHU1_R0_B1_DQ7_RK0_ARPI_DQM_B1) |
    		//			P_Fld(wrlevel_dq_delay[3], SHU1_R0_B1_DQ7_RK0_ARPI_DQ_B1));
        }
        vSetRank(p,backup_rank);
        #endif
    }
    else
    {
        for (byte_i = 0; byte_i < (p->data_width/ucbit_num);  byte_i++)
        {
            mcSHOW_DBG_MSG(("DQS%d delay =  %d\n", byte_i, wrlevel_dqs_final_delay[byte_i]));
        }

        if (wrlevel_dqs_final_delay[ucSkewBit] == 0)
            p->density = 0;
        else
            p->density = wrlevel_dqs_final_delay[ucSkewBit] * 1000 / ucSkewPI;
        
        mcSHOW_DBG_MSG(("PI:DLY = %d:%d (%d)\n", ucSkewPI, wrlevel_dqs_final_delay[ucSkewBit], p->density));
        //Adjust DQS Delay Line
        for(ii=p->rank; ii<RANK_MAX; ii++)
        {
            //Adjust DQS Delay Line
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ1, P_Fld(0, SHU1_R0_B0_DQ1_RK0_TX_ARDQS0B_DLY_B0) | P_Fld(0, SHU1_R0_B0_DQ1_RK0_TX_ARDQS0_DLY_B0) \
                                                        | P_Fld(0, SHU1_R0_B0_DQ1_RK0_TX_ARDQS0B_DLYB_B0) | P_Fld(0, SHU1_R0_B0_DQ1_RK0_TX_ARDQS0_DLYB_B0));
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ1, P_Fld(wrlevel_dqs_final_delay[ucSkewBit]>>1, SHU1_R0_B1_DQ1_RK0_TX_ARDQS0B_DLY_B1) | P_Fld(wrlevel_dqs_final_delay[ucSkewBit]>>1, SHU1_R0_B1_DQ1_RK0_TX_ARDQS0_DLY_B1) \
                                                        | P_Fld(wrlevel_dqs_final_delay[ucSkewBit]>>1, SHU1_R0_B1_DQ1_RK0_TX_ARDQS0B_DLYB_B1) | P_Fld(wrlevel_dqs_final_delay[ucSkewBit]>>1, SHU1_R0_B1_DQ1_RK0_TX_ARDQS0_DLYB_B1));
            vSetRank(p,ii);
        }
        vSetRank(p,backup_rank);
        
        //restore wrlevel_dqs_final_delay
        memcpy(wrlevel_dqs_final_delay, wrlevel_dqs_delay, sizeof(wrlevel_dqs_final_delay));
    }

    DramcRankSwap(p, RANK_0);

#if REG_ACCESS_PORTING_DGB
    RegLogEnable =0;
#endif
    mcSHOW_DBG_MSG(("[DramcWriteLeveling] ====Done====\n"));

#if defined(DRAM_CALIB_LOG) || defined(DRAM_ETT)
    if (!u1DlyLine)
    {
        gDRAM_CALIB_LOG.RANK[p->rank].Write_Leveling.Final_Clk_delay =  ClockDelayMax;
        gDRAM_CALIB_LOG.RANK[p->rank].Write_Leveling.DQS0_delay = wrlevel_dqs_final_delay[0];
        gDRAM_CALIB_LOG.RANK[p->rank].Write_Leveling.DQS1_delay = wrlevel_dqs_final_delay[1];
        gDRAM_CALIB_LOG.RANK[p->rank].Write_Leveling.DQS2_delay = wrlevel_dqs_final_delay[2];
        gDRAM_CALIB_LOG.RANK[p->rank].Write_Leveling.DQS3_delay = wrlevel_dqs_final_delay[3];
    }
#endif

	//cc add
	vIO32WriteFldAlign(DDRPHY_B0_DQ3, 0x0, B0_DQ3_RG_RX_ARDQ_SMT_EN_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ3, 0x0, B1_DQ3_RG_RX_ARDQ_SMT_EN_B1);
	vIO32WriteFldAlign(DDRPHY_B01_MISC, 0x0, B01_MISC_RG_ARPI_SMT_EN_B01);
	vIO32WriteFldAlign(DDRPHY_CA_MISC, 0x0, CA_MISC_RG_ARPI_SMT_EN_CA);
	vIO32WriteFldAlign(DDRPHY_CA_CMD3, 0x0, CA_CMD3_RG_RX_ARCMD_SMT_EN);

	vIO32WriteFldAlign(DDRPHY_B0_DQ6, 0x0, B0_DQ6_RG_RX_ARDQ_BIAS_PS_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ6, 0x0, B1_DQ6_RG_RX_ARDQ_BIAS_PS_B1);
	vIO32WriteFldAlign(DDRPHY_CA_CMD6, 0x0, CA_CMD6_RG_RX_ARCMD_BIAS_PS);
	//cc add end


    Another_Rank_CKE_low(p, 1);

    return DRAM_OK;
}
#endif
#endif //SIMULATION_WRITE_LEVELING


#if SIMULATION_GATING
#define GATING_RODT_LATANCY_EN_PC3 1  //Need to enable when RODT enable
#define GATING_RODT_LATANCY_PC3_VALUE	4
#define GATING_PATTERN_NUM_LP3 0x46
#define GATING_GOLDEND_DQSCNT_LP3 0x2323

#define GATING_TXDLY_CHNAGE  1 //Gating txdly chcange & RANKINCTL setting

#if COMPILE_THIS_PART//cc mark to avoid compile error GATING_ADJUST_TXDLY_FOR_TRACKING
U8 u1TXDLY_Cal_min =0xff, u1TXDLY_Cal_max=0;
U8 ucbest_coarse_tune2T_backup[RANK_MAX][DQS_NUMBER];
U8 ucbest_coarse_tune0p5T_backup[RANK_MAX][DQS_NUMBER];
U8 ucbest_coarse_tune2T_P1_backup[RANK_MAX][DQS_NUMBER];
U8 ucbest_coarse_tune0p5T_P1_backup[RANK_MAX][DQS_NUMBER];
#endif

// Use gating old burst mode to find gating window boundary
// Set the begining of window as new burst mode gating window center.
#if 1 //COMPILE_PCDDR3
DRAM_STATUS_T DramcRxdqsGatingCal_PCDDR3(DRAMC_CTX_T *p)
{
        U8 ucRX_DLY_DQSIENSTB_LOOP,ucRX_DQS_CTL_LOOP;
        U32 u4value, u4err_value;
        U8 ucpass_begin[DQS_NUMBER], ucpass_count[DQS_NUMBER], ucCurrentPass;
        U8 ucmin_coarse_tune2T[DQS_NUMBER], ucmin_coarse_tune0p5T[DQS_NUMBER], ucmin_fine_tune[DQS_NUMBER];
        U8 ucpass_count_1[DQS_NUMBER], ucmin_coarse_tune2T_1[DQS_NUMBER], ucmin_coarse_tune0p5T_1[DQS_NUMBER], ucmin_fine_tune_1[DQS_NUMBER];
        U8 dqs_i,  ucdly_coarse_large, ucdly_coarse_0p5T, ucdly_fine_xT, ucDQS_GW_FINE_STEP;
        U8 uctmp_offset, uctmp_value;
        U8 ucbest_fine_tune[DQS_NUMBER], ucbest_coarse_tune0p5T[DQS_NUMBER], ucbest_coarse_tune2T[DQS_NUMBER];
        U8 ucbest_fine_tune_P1[DQS_NUMBER], ucbest_coarse_tune0p5T_P1[DQS_NUMBER], ucbest_coarse_tune2T_P1[DQS_NUMBER];

        U8 ucFreqDiv;
        U8 ucdly_coarse_large_P1, ucdly_coarse_0p5T_P1;

    #if GATING_ADJUST_TXDLY_FOR_TRACKING
        U8 u1TX_dly_DQSgated;
    #endif
    #if GATING_TXDLY_CHNAGE
        U32 u4ReadDQSINCTL, u4ReadTXDLY, u4RankINCTL_ROOT, u4Tmp2T, u4Tmp0p5T;
    #endif
    #if GATING_RODT_LATANCY_EN_PC3
        U8 ucdly_coarse_large_RODT, ucdly_coarse_0p5T_RODT;
        U8 ucdly_coarse_large_RODT_P1, ucdly_coarse_0p5T_RODT_P1;  //Elbrus new
        U8 ucbest_coarse_large_RODT[DQS_NUMBER], ucbest_coarse_0p5T_RODT[DQS_NUMBER];
        U8 ucbest_coarse_large_RODT_P1[DQS_NUMBER], ucbest_coarse_0p5T_RODT_P1[DQS_NUMBER];//Elbrus new
    #endif
        U8 ucCoarseTune, ucCoarseStart, ucCoarseEnd;
        U32 LP3_DataPerByte[DQS_NUMBER];
        U32 u4DebugCnt[DQS_NUMBER];
        U16 u2DebugCntPerByte;

        U32 u4BakReg_DRAMC_DQSCAL0, u4BakReg_DRAMC_STBCAL_F;
        U32 u4BakReg_DRAMC_WODT, u4BakReg_DRAMC_SPCMD, u4BakReg_DRAMC_REFCTRL0;
        U8 u1PassByteCount=0;

#ifdef ENABLE_CALIBRATION_WINDOW_LOG_FOR_FT
        U16 u2MinWinSize = 0xffff;
        U8 u1MinWinSizeByteidx;
#endif
        // error handling
        if (!p)
        {
            mcSHOW_ERR_MSG(("context is NULL\n"));
            return DRAM_FAIL;
        }

        //Register backup
        u4BakReg_DRAMC_DQSCAL0 = u4IO32Read4B(DRAMC_REG_STBCAL);
        u4BakReg_DRAMC_STBCAL_F = u4IO32Read4B(DRAMC_REG_STBCAL1);
        u4BakReg_DRAMC_WODT = u4IO32Read4B(DRAMC_REG_DDRCONF0);
        u4BakReg_DRAMC_SPCMD = u4IO32Read4B(DRAMC_REG_SPCMD);
        u4BakReg_DRAMC_REFCTRL0 = u4IO32Read4B(DRAMC_REG_REFCTRL0);

        //Disable perbank refresh, use all bank refresh, currently DUT problem
        vIO32WriteFldAlign(DRAMC_REG_REFCTRL0,  0, REFCTRL0_PBREFEN);

        // Disable HW gating first, 0x1c0[31], need to disable both UI and PI tracking or the gating delay reg won't be valid.
        DramcHWGatingOnOff(p, 0);

        //If DQS ring counter is different as our expectation, error flag is asserted and the status is in ddrphycfg 0xFC0 ~ 0xFCC
        //Enable this function by R_DMSTBENCMPEN=1 (0x348[18])
        //Set R_DMSTBCNT_LATCH_EN=1, 0x348[11]
        //Set R_DM4TO1MODE=0, 0x54[11]
        //Clear error flag by ddrphycfg 0x5c0[1] R_DMPHYRST
        vIO32WriteFldAlign(DRAMC_REG_STBCAL1, 1, STBCAL1_STBENCMPEN);
        vIO32WriteFldAlign(DRAMC_REG_STBCAL1, 1, STBCAL1_STBCNT_LATCH_EN);
        vIO32WriteFldAlign(DRAMC_REG_DDRCONF0, 0, DDRCONF0_DM4TO1MODE);

        //enable &reset DQS counter
        vIO32WriteFldAlign(DRAMC_REG_SPCMD, 1, SPCMD_DQSGCNTEN);
        mcDELAY_US(4);//wait 1 auto refresh after DQS Counter enable

        vIO32WriteFldAlign(DRAMC_REG_SPCMD, 1, SPCMD_DQSGCNTRST);
        mcDELAY_US(1);//delay 2T
        vIO32WriteFldAlign(DRAMC_REG_SPCMD, 0, SPCMD_DQSGCNTRST);

        vIO32WriteFldAlign(DDRPHY_MISC_CTRL1, u1GetRank(p), MISC_CTRL1_R_DMSTBENCMP_RK_OPT);

        //Initialize variables
        for (dqs_i=0; dqs_i<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); dqs_i++)
        {
            ucpass_begin[dqs_i] = 0;
            ucpass_count[dqs_i] = 0;
			ucpass_count_1[dqs_i] = 0;	//YMC add to avoid waring
			ucmin_coarse_tune0p5T_1[dqs_i] = 0;	//YMC add to avoid waring
			ucmin_coarse_tune2T_1[dqs_i] = 0;	//YMC add to avoid waring
			ucmin_fine_tune_1[dqs_i] = 0;	//YMC add to avoid waring
        }

        ucRX_DLY_DQSIENSTB_LOOP= 32;// PI fine tune 0->31

        mcSHOW_DBG_MSG(("\n[Gating]\n"));
		#if	dbg_print
		prom_puts("\n[Gating]\n");
		#endif
        vPrintCalibrationBasicInfo(p);

        ucFreqDiv= 2;
        ucDQS_GW_FINE_STEP = DQS_GW_FINE_STEP;

        if(p->frequency > DDR_DDR1333) {    //1333
        #if FOR_DV_SIMULATION //cc to speed up simulation
			ucCoarseStart = 13;
		#else
            ucCoarseStart = 5;
		#endif
        } else {//if(p->frequency >= DDR_DDR1066)  //1066
        #if FOR_DV_SIMULATION
            ucCoarseStart = 10; //cc to speed up simulation
		#else
            ucCoarseStart = 2;
		#endif
		}

        ucCoarseEnd = ucCoarseStart+24;

        ucRX_DQS_CTL_LOOP = 8; // Since Everest, no matter LP3 or LP4. ucRX_DQS_CTL_LOOP is 8.

    #if GATING_RODT_LATANCY_EN_PC3  //LP3 RODT is not enable, don't need to set the RODT settings.
        // Fix build warning, initialize variables.
        ucdly_coarse_large_RODT = 0;
        ucdly_coarse_0p5T_RODT = 0;

        ucdly_coarse_large_RODT_P1 = 2;
        ucdly_coarse_0p5T_RODT_P1 = 2;

        // 1.   DQSG latency =
        // (1)   R_DMR*DQSINCTL[3:0] (MCK) +
        // (2)   selph_TX_DLY[2:0] (MCK) +
        // (3)   selph_dly[2:0] (UI)

        // 2.   RODT latency =
        // (1)   R_DMTRODT[3:0] (MCK) +
        // (2)   selph_TX_DLY[2:0] (MCK) +
        // (3)   selph_dly[2:0] (UI)

		//cc porting from DDR4 Gating code
		{
			u4value = u4IO32ReadFldAlign(DRAMC_REG_SHURK0_DQSCTL, SHURK0_DQSCTL_DQSINCTL);
			vIO32WriteFldAlign(DRAMC_REG_SHU_ODTCTRL, u4value, SHU_ODTCTRL_RODT);
		}

    #endif

        for (ucCoarseTune = ucCoarseStart; ucCoarseTune < ucCoarseEnd; ucCoarseTune += DQS_GW_COARSE_STEP)
        {
            ucdly_coarse_large      = ucCoarseTune / ucRX_DQS_CTL_LOOP;
            ucdly_coarse_0p5T      = ucCoarseTune % ucRX_DQS_CTL_LOOP;

            ucdly_coarse_large_P1 = (ucCoarseTune + ucFreqDiv) / ucRX_DQS_CTL_LOOP;
            ucdly_coarse_0p5T_P1 =(ucCoarseTune + ucFreqDiv) % ucRX_DQS_CTL_LOOP;

			
		#if GATING_RODT_LATANCY_EN_PC3	//LP3 RODT is not enable, don't need to set the RODT settings.
			u4value = (ucdly_coarse_large <<3)+ucdly_coarse_0p5T;
		
			if(u4value >= GATING_RODT_LATANCY_PC3_VALUE)
			{
				u4value -= GATING_RODT_LATANCY_PC3_VALUE;
				ucdly_coarse_large_RODT 	= u4value>>3;
				ucdly_coarse_0p5T_RODT	   = u4value -(ucdly_coarse_large_RODT<<3);
		
				u4value = (ucdly_coarse_large_P1 <<3)+ucdly_coarse_0p5T_P1 - GATING_RODT_LATANCY_PC3_VALUE;
				ucdly_coarse_large_RODT_P1	   = u4value>>3;
				ucdly_coarse_0p5T_RODT_P1	  = u4value -(ucdly_coarse_large_RODT_P1<<3);
			}
			else
			{
				ucdly_coarse_large_RODT = 0;
				ucdly_coarse_0p5T_RODT = 0;
		
				ucdly_coarse_large_RODT_P1 = 2;
				ucdly_coarse_0p5T_RODT_P1 = 2;
		
				mcSHOW_ERR_MSG(("[DramcRxdqsGatingCal] Error: ucdly_coarse_large_RODT[%d] is already 0. RODT cannot be -%d UI\n", \
					dqs_i, GATING_RODT_LATANCY_PC3_VALUE));
				#if	dbg_print
				prom_puts("[DramcRxdqsGatingCal]ucdly_coarse_large_RODT is already 0. RODT cannot be -");
				prom_print_dec(GATING_RODT_LATANCY_PC3_VALUE);
				prom_puts(" UI\n");
				#endif
			}
		#endif

            //DramPhyCGReset(p, 1);// need to reset when UI update or PI change >=2

            // 4T or 2T coarse tune
            vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQSG0, \
                                        P_Fld((U32) ucdly_coarse_large, SHURK0_SELPH_DQSG0_TX_DLY_DQS0_GATED)| \
                                        P_Fld((U32) ucdly_coarse_large, SHURK0_SELPH_DQSG0_TX_DLY_DQS1_GATED)| \
                                        P_Fld((U32) ucdly_coarse_large, SHURK0_SELPH_DQSG0_TX_DLY_DQS2_GATED)| \
                                        P_Fld((U32) ucdly_coarse_large, SHURK0_SELPH_DQSG0_TX_DLY_DQS3_GATED)| \
                                        P_Fld((U32) ucdly_coarse_large_P1, SHURK0_SELPH_DQSG0_TX_DLY_DQS0_GATED_P1)| \
                                        P_Fld((U32) ucdly_coarse_large_P1, SHURK0_SELPH_DQSG0_TX_DLY_DQS1_GATED_P1)| \
                                        P_Fld((U32) ucdly_coarse_large_P1, SHURK0_SELPH_DQSG0_TX_DLY_DQS2_GATED_P1)| \
                                        P_Fld((U32) ucdly_coarse_large_P1, SHURK0_SELPH_DQSG0_TX_DLY_DQS3_GATED_P1));

            // 0.5T coarse tune
            vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQSG1, \
                                        P_Fld((U32) ucdly_coarse_0p5T, SHURK0_SELPH_DQSG1_REG_DLY_DQS0_GATED)| \
                                        P_Fld((U32) ucdly_coarse_0p5T, SHURK0_SELPH_DQSG1_REG_DLY_DQS1_GATED)| \
                                        P_Fld((U32) ucdly_coarse_0p5T, SHURK0_SELPH_DQSG1_REG_DLY_DQS2_GATED)| \
                                        P_Fld((U32) ucdly_coarse_0p5T, SHURK0_SELPH_DQSG1_REG_DLY_DQS3_GATED)| \
                                        P_Fld((U32) ucdly_coarse_0p5T_P1, SHURK0_SELPH_DQSG1_REG_DLY_DQS0_GATED_P1)| \
                                        P_Fld((U32) ucdly_coarse_0p5T_P1, SHURK0_SELPH_DQSG1_REG_DLY_DQS1_GATED_P1)| \
                                        P_Fld((U32) ucdly_coarse_0p5T_P1, SHURK0_SELPH_DQSG1_REG_DLY_DQS2_GATED_P1)| \
                                        P_Fld((U32) ucdly_coarse_0p5T_P1, SHURK0_SELPH_DQSG1_REG_DLY_DQS3_GATED_P1));

			#if GATING_RODT_LATANCY_EN_PC3  //LP3 RODT is not enable, don't need to set the RODT settings.
				vIO32WriteFldMulti((DRAMC_REG_SHURK0_SELPH_ODTEN0), \
					P_Fld((U32) ucdly_coarse_large_RODT, SHURK0_SELPH_ODTEN0_TXDLY_B0_RODTEN)| \
					P_Fld((U32) ucdly_coarse_large_RODT, SHURK0_SELPH_ODTEN0_TXDLY_B1_RODTEN)| \
					P_Fld((U32) ucdly_coarse_large_RODT, SHURK0_SELPH_ODTEN0_TXDLY_B2_RODTEN)| \
					P_Fld((U32) ucdly_coarse_large_RODT, SHURK0_SELPH_ODTEN0_TXDLY_B3_RODTEN)| \
					P_Fld((U32) ucdly_coarse_large_RODT_P1, SHURK0_SELPH_ODTEN0_TXDLY_B0_RODTEN_P1)| \
					P_Fld((U32) ucdly_coarse_large_RODT_P1, SHURK0_SELPH_ODTEN0_TXDLY_B1_RODTEN_P1)| \
					P_Fld((U32) ucdly_coarse_large_RODT_P1, SHURK0_SELPH_ODTEN0_TXDLY_B2_RODTEN_P1)| \
					P_Fld((U32) ucdly_coarse_large_RODT_P1, SHURK0_SELPH_ODTEN0_TXDLY_B3_RODTEN_P1));

				vIO32WriteFldMulti((DRAMC_REG_SHURK0_SELPH_ODTEN1), \
					P_Fld((U32) ucdly_coarse_0p5T_RODT, SHURK0_SELPH_ODTEN1_DLY_B0_RODTEN)| \
					P_Fld((U32) ucdly_coarse_0p5T_RODT, SHURK0_SELPH_ODTEN1_DLY_B1_RODTEN)| \
					P_Fld((U32) ucdly_coarse_0p5T_RODT, SHURK0_SELPH_ODTEN1_DLY_B2_RODTEN)| \
					P_Fld((U32) ucdly_coarse_0p5T_RODT, SHURK0_SELPH_ODTEN1_DLY_B3_RODTEN)| \
					P_Fld((U32) ucdly_coarse_0p5T_RODT_P1, SHURK0_SELPH_ODTEN1_DLY_B0_RODTEN_P1)| \
					P_Fld((U32) ucdly_coarse_0p5T_RODT_P1, SHURK0_SELPH_ODTEN1_DLY_B1_RODTEN_P1)| \
					P_Fld((U32) ucdly_coarse_0p5T_RODT_P1, SHURK0_SELPH_ODTEN1_DLY_B2_RODTEN_P1)| \
					P_Fld((U32) ucdly_coarse_0p5T_RODT_P1, SHURK0_SELPH_ODTEN1_DLY_B3_RODTEN_P1));

				//mcSHOW_DBG_MSG2(("RODT delay(2T, 0.5T) = (%d, %d)\n", ucdly_coarse_large_RODT, ucdly_coarse_0p5T_RODT));
				//mcFPRINTF((fp_A60501,"RODT delay(2T, 0.5T) = (%d, %d)\n", ucdly_coarse_large_RODT, ucdly_coarse_0p5T_RODT));
			#endif
			
            for (ucdly_fine_xT=DQS_GW_FINE_START; ucdly_fine_xT<DQS_GW_FINE_END; ucdly_fine_xT+=ucDQS_GW_FINE_STEP)
            {
                //ok we set a coarse/fine tune value already
                u4value = ucdly_fine_xT | (ucdly_fine_xT<<8) | (ucdly_fine_xT<<16) | (ucdly_fine_xT<<24);
                vIO32Write4B(DRAMC_REG_SHURK0_DQSIEN, u4value);

                //reset phy, reset read data counter
                DramPhyReset(p);

                //reset DQS counter
                vIO32WriteFldAlign(DRAMC_REG_SPCMD, 1, SPCMD_DQSGCNTRST);
                mcDELAY_US(1);//delay 2T
                vIO32WriteFldAlign(DRAMC_REG_SPCMD, 0, SPCMD_DQSGCNTRST);

                // enable TE2, audio pattern
                DramcEngine2(p, TE_OP_READ_CHECK, 0x55000000, 0xaa000000 |GATING_PATTERN_NUM_LP3, 1, 0, 0, 0);


                 //read DQS counter
                u4DebugCnt[0] = u4IO32Read4B(DRAMC_REG_DQSGNWCNT0);
                u4DebugCnt[1] = (u4DebugCnt[0] >> 16) & 0xffff;
                u4DebugCnt[0] &= 0xffff;
				#if 0	//YMC mark to avoid warning
                if(p->data_width == DATA_WIDTH_32BIT)
                {
                    u4DebugCnt[2] = u4IO32Read4B(DRAMC_REG_DQSGNWCNT1);
                    u4DebugCnt[3] = (u4DebugCnt[2] >> 16) & 0xffff;
                    u4DebugCnt[2] &= 0xffff;
                }
				#endif
                 u4err_value =0;

                 /*TINFO="%2d  %2d  %2d |(B3->B0) 0x%4x, 0x%4x, 0x%4x, 0x%4x | 0x%8x\n", ucdly_coarse_large, ucdly_coarse_0p5T, ucdly_fine_xT, u4DebugCnt[3], u4DebugCnt[2], u4DebugCnt[1], u4DebugCnt[0], u4err_value*/

                mcSHOW_DBG_MSG2(("(%2d %2d) | %2d %2d %2d |", ucCoarseTune, ucdly_fine_xT, ucdly_coarse_large, ucdly_coarse_0p5T, ucdly_fine_xT ));
				#if	dbg_print
				prom_puts("Coarse Tune:");
				prom_print_dec(ucCoarseTune);
				prom_puts(", Fine Tune:");
				prom_print_dec(ucdly_fine_xT);
				prom_puts(" | ");
				#endif

                for (dqs_i=0; dqs_i<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); dqs_i++)
                {
                    mcSHOW_DBG_MSG2(("0x%x ",u4DebugCnt[dqs_i]));
					#if	dbg_print
					prom_puts("DQS");
					prom_print_dec(dqs_i);
					prom_puts(", gating counter:0x");
					prom_print_hex(u4DebugCnt[dqs_i],4);
					prom_puts(" | ");
					#endif
                }

                mcSHOW_DBG_MSG2(("| %X",u4err_value));
                mcSHOW_DBG_MSG2(("\n"));
				#if	dbg_print
				prom_puts("\n");
				#endif

                //find gating window pass range per DQS separately
                for (dqs_i=0; dqs_i<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); dqs_i++)
                {
                    if(u1PassByteCount  & (1<<dqs_i))
                    {
                        // real window found, break to prevent finding fake window.
                        continue;
                    }
                    u2DebugCntPerByte =(U16) u4DebugCnt[dqs_i];

                    // check if current tap is pass
                    ucCurrentPass =0;

                    if ((p->data_width == DATA_WIDTH_16BIT) /* cc mark || (p->rank == RANK_1 && p->asymmetric)*/)
                    {
                        if(u2DebugCntPerByte==(GATING_GOLDEND_DQSCNT_LP3 << 1))
                        {
                                ucCurrentPass =1;
							#if !FOR_DV_SIMULATION //cc to speed up simulation
                                ucDQS_GW_FINE_STEP = 1;
							#endif
                        }

                    }
                    else
                    {
                       if(u2DebugCntPerByte==GATING_GOLDEND_DQSCNT_LP3)
	                    {
	                            ucCurrentPass =1;
	                            ucDQS_GW_FINE_STEP = 1;
	                    }
                    }
                    //if current tap is pass
                    if (ucCurrentPass)
                    {
                        if (ucpass_begin[dqs_i]==0)
                        {
                            //no pass tap before , so it is the begining of pass range
                            ucpass_begin[dqs_i] = 1;
                            ucpass_count_1[dqs_i] = 0;
                            ucmin_coarse_tune2T_1[dqs_i] = ucdly_coarse_large;
                            ucmin_coarse_tune0p5T_1[dqs_i] = ucdly_coarse_0p5T;
                            ucmin_fine_tune_1[dqs_i] = ucdly_fine_xT;

                            /*TINFO="[Byte %d]First pass (%d, %d, %d)\n", dqs_i,ucdly_coarse_large, ucdly_coarse_0p5T, ucdly_fine_xT*/
                            mcSHOW_DBG_MSG(("[Byte %d]First pass (%d, %d, %d)\n", dqs_i,ucdly_coarse_large, ucdly_coarse_0p5T, ucdly_fine_xT));
							#if	dbg_print
							prom_puts("DQS");
							prom_print_dec(dqs_i);
							prom_puts(", First pass (coarse_2T, coarse_0p5T, fine):");
							prom_print_dec(ucdly_coarse_large);
							prom_puts(", ");
							prom_print_dec(ucdly_coarse_0p5T);
							prom_puts(", ");
							prom_print_dec(ucdly_fine_xT);
							prom_puts("\n");
							#endif
                        }

                        if (ucpass_begin[dqs_i]==1)
                        {
                            //incr pass tap number
                            ucpass_count_1[dqs_i]++;
							#if	dbg_print
							if(ucpass_count_1[dqs_i]>1)
							{
								prom_puts("DQS");
								prom_print_dec(dqs_i);
								prom_puts(", Pass (coarse_2T, coarse_0p5T, fine):");
								prom_print_dec(ucdly_coarse_large);
								prom_puts(", ");
								prom_print_dec(ucdly_coarse_0p5T);
								prom_puts(", ");
								prom_print_dec(ucdly_fine_xT);
								prom_puts("\n");
							}
							#endif
                        }

                    }
                    else // current tap is fail
                    {
                        if (ucpass_begin[dqs_i]==1)
                        {
                            //at the end of pass range
                            ucpass_begin[dqs_i] = 0;

                            //save the max range settings, to avoid glitch
                            if (ucpass_count_1[dqs_i] > ucpass_count[dqs_i])
                            {
                                ucmin_coarse_tune2T[dqs_i] = ucmin_coarse_tune2T_1[dqs_i];
                                ucmin_coarse_tune0p5T[dqs_i] = ucmin_coarse_tune0p5T_1[dqs_i];
                                ucmin_fine_tune[dqs_i] = ucmin_fine_tune_1[dqs_i];
                                ucpass_count[dqs_i] = ucpass_count_1[dqs_i];

                                //didn't has lead/lag RG, use SW workaround

                                if (ucpass_count_1[dqs_i] >= 70) // if didn't check fail region and pass UI more than 2UI, then workaround back to 2UI
                                {
                                    ucpass_count_1[dqs_i] = 64; // 2UI
                                    ucpass_count[dqs_i] = ucpass_count_1[dqs_i];
                                }

                                /*TINFO="[Byte %d]Bigger pass win(%d, %d, %d)  Pass tap=%d\n", \
                                    dqs_i, ucmin_coarse_tune2T_1[dqs_i], ucmin_coarse_tune0p5T_1[dqs_i], ucmin_fine_tune_1[dqs_i], ucpass_count_1[dqs_i]*/
                                mcSHOW_DBG_MSG(("[Byte %d]Bigger pass win(%d, %d, %d)  Pass tap=%d\n", \
                                    dqs_i, ucdly_coarse_large, ucdly_coarse_0p5T, ucdly_fine_xT, ucpass_count_1[dqs_i]));
								#if	dbg_print
								prom_puts("DQS");
								prom_print_dec(dqs_i);
								prom_puts(", Bigger pass win end(coarse_2T, coarse_0p5T, fine):");
								prom_print_dec(ucdly_coarse_large);
								prom_puts(", ");
								prom_print_dec(ucdly_coarse_0p5T);
								prom_puts(", ");
								prom_print_dec(ucdly_fine_xT);
								prom_puts(", Pass tap:");
								prom_print_dec(ucpass_count_1[dqs_i]);
								prom_puts("\n");
								#endif
                                #if defined(DRAM_CALIB_LOG) || defined(DRAM_ETT)
                                gDRAM_CALIB_LOG.RANK[p->rank].GatingWindow.Gating_Win[dqs_i] = ucpass_count_1[dqs_i]*ucDQS_GW_FINE_STEP;
                                #endif
                                // LP4 pass window around 6 UI(burst mode), set 1~3 UI is pass
                                // LP3 pass window around 2 UI(pause mode), set 1~3 UI is pass
                                if((ucpass_count_1[dqs_i]*ucDQS_GW_FINE_STEP > 32) && (ucpass_count_1[dqs_i]*ucDQS_GW_FINE_STEP < 96))
                                {
                                    u1PassByteCount  |= (1<<dqs_i);
                                }

                                if(((p->data_width == DATA_WIDTH_16BIT) && (u1PassByteCount==0x3)) || \
									((p->data_width == DATA_WIDTH_32BIT) && (u1PassByteCount==0xf)))
                                {
                                    mcSHOW_DBG_MSG2(("All bytes gating window pass, Done, Early break!\n"));
									#if	dbg_print
									prom_puts("All bytes gating window pass, Done, Early break!\n");
									#endif
                                    ucdly_fine_xT = DQS_GW_FINE_END;//break loop
                                    ucCoarseTune = ucCoarseEnd;      //break loop
                                }
                            }
                        }
                    }
                }
            }
        }


        vSetCalibrationResult(p, DRAM_CALIBRATION_GATING, DRAM_OK);

        //check if there is no pass taps for each DQS
        for (dqs_i=0; dqs_i<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); dqs_i++)
        {
            if (ucpass_count[dqs_i]==0)
            {
                /*TINFO="error, no pass taps in DQS_%d !!!\n", dqs_i*/
                mcSHOW_ERR_MSG(("error, no pass taps in DQS_%d !!!\n", dqs_i));
				#if	dbg_print
				prom_puts("[DramcRxdqsGatingCal] Error: no pass taps in DQS");
				prom_print_dec(dqs_i);
				prom_puts("\n");
				#endif
                vSetCalibrationResult(p, DRAM_CALIBRATION_GATING, DRAM_FAIL);
            }

            #ifdef ENABLE_CALIBRATION_WINDOW_LOG_FOR_FT
            if(ucpass_count[dqs_i] < u2MinWinSize)
            {
                u2MinWinSize = ucpass_count[dqs_i];
                u1MinWinSizeByteidx= dqs_i;
            }
            #endif
        }

        #ifdef ENABLE_CALIBRATION_WINDOW_LOG_FOR_FT
        mcSHOW_DBG_MSG(("FT log: Gating min window : byte %d, size %d\n", u1MinWinSizeByteidx, u2MinWinSize*ucDQS_GW_FINE_STEP));
        #endif

        //find center of each byte
        for (dqs_i=0; dqs_i<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); dqs_i++)
        {
        // -- PI for Phase0 & Phase1 --
            uctmp_offset = ucpass_count[dqs_i]*ucDQS_GW_FINE_STEP/2;
            uctmp_value = ucmin_fine_tune[dqs_i]+uctmp_offset;
            ucbest_fine_tune[dqs_i] = uctmp_value% ucRX_DLY_DQSIENSTB_LOOP;
            ucbest_fine_tune_P1[dqs_i] = ucbest_fine_tune[dqs_i];

        // coarse tune 0.5T for Phase 0
            uctmp_offset = uctmp_value / ucRX_DLY_DQSIENSTB_LOOP;
            uctmp_value = ucmin_coarse_tune0p5T[dqs_i]+uctmp_offset;
            ucbest_coarse_tune0p5T[dqs_i] = uctmp_value% ucRX_DQS_CTL_LOOP;

        // coarse tune 2T for Phase 0
            uctmp_offset = uctmp_value/ucRX_DQS_CTL_LOOP;
            ucbest_coarse_tune2T[dqs_i] = ucmin_coarse_tune2T[dqs_i]+uctmp_offset;
        // coarse tune 0.5T for Phase 1
            uctmp_value = ucbest_coarse_tune0p5T[dqs_i]+ ucFreqDiv;
            ucbest_coarse_tune0p5T_P1[dqs_i] = uctmp_value% ucRX_DQS_CTL_LOOP;

        // coarse tune 2T for Phase 1
            uctmp_offset = uctmp_value/ucRX_DQS_CTL_LOOP;
            ucbest_coarse_tune2T_P1[dqs_i] = ucbest_coarse_tune2T[dqs_i]+uctmp_offset;
        }

        mcSHOW_DBG_MSG(("===============================================================================\n"));
        mcSHOW_DBG_MSG(("    dqs input gating widnow, final delay value\n    Frequency=%d  rank=%d\n", p->frequency, p->rank));
        mcSHOW_DBG_MSG(("===============================================================================\n"));
        //mcSHOW_DBG_MSG(("test2_1: 0x%x, test2_2: 0x%x, test pattern: %d\n",  p->test2_1,p->test2_2, p->test_pattern));

        for (dqs_i=0; dqs_i<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); dqs_i++)
        {
            /*TINFO="best DQS%d delay(2T, 0.5T, PI) = (%d, %d, %d)\n", dqs_i, ucbest_coarse_tune2T[dqs_i], ucbest_coarse_tune0p5T[dqs_i], ucbest_fine_tune[dqs_i]*/
            mcSHOW_DBG_MSG(("R%d FINAL: GW best DQS%d P0 delay(2T, 0.5T, PI) = (%d, %d, %d) [tap = %d]\n", p->rank, dqs_i, ucbest_coarse_tune2T[dqs_i], ucbest_coarse_tune0p5T[dqs_i], ucbest_fine_tune[dqs_i], ucpass_count[dqs_i]));
            #if GATING_ADJUST_TXDLY_FOR_TRACKING
            // find min gating TXDLY (should be in P0)
            u1TX_dly_DQSgated  = ((ucbest_coarse_tune2T[dqs_i] <<1)|((ucbest_coarse_tune0p5T[dqs_i] >>2)&0x1));

            if(u1TX_dly_DQSgated < u1TXDLY_Cal_min)
                u1TXDLY_Cal_min = u1TX_dly_DQSgated;

            ucbest_coarse_tune0p5T_backup[p->rank][dqs_i] = ucbest_coarse_tune0p5T[dqs_i];
            ucbest_coarse_tune2T_backup[p->rank][dqs_i] = ucbest_coarse_tune2T[dqs_i];
            #endif
        }
        mcSHOW_DBG_MSG2(("===============================================================================\n"));

        for (dqs_i=0; dqs_i<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); dqs_i++)
        {
            /*TINFO="best DQS%d P1 delay(2T, 0.5T, PI) = (%d, %d, %d)\n", dqs_i, ucbest_coarse_tune2T_P1[dqs_i], ucbest_coarse_tune0p5T_P1[dqs_i], ucbest_fine_tune[dqs_i]*/
            mcSHOW_DBG_MSG(("R%d FINAL: GW best DQS%d P1 delay(2T, 0.5T, PI) = (%d, %d, %d)\n", p->rank, dqs_i, ucbest_coarse_tune2T_P1[dqs_i], ucbest_coarse_tune0p5T_P1[dqs_i], ucbest_fine_tune[dqs_i]));
			#if	dbg_print
			prom_puts("Rank");
			prom_print_dec(p->rank);
			prom_puts(", DQS");
			prom_print_dec(dqs_i);
			prom_puts(", best gw delay(coarse_2T, coarse_0p5T, fine):");
			prom_print_dec(ucbest_coarse_tune2T[dqs_i]);
			prom_puts(", ");
			prom_print_dec(ucbest_coarse_tune0p5T[dqs_i]);
			prom_puts(", ");
			prom_print_dec(ucbest_fine_tune[dqs_i]);
			prom_puts("\n");
			#endif
            #if GATING_ADJUST_TXDLY_FOR_TRACKING
            // find max gating TXDLY (should be in P1)
            u1TX_dly_DQSgated  = ((ucbest_coarse_tune2T_P1[dqs_i] <<1)|((ucbest_coarse_tune0p5T_P1[dqs_i] >>2)&0x1));

            if(u1TX_dly_DQSgated > u1TXDLY_Cal_max)
                u1TXDLY_Cal_max = u1TX_dly_DQSgated;

            ucbest_coarse_tune0p5T_P1_backup[p->rank][dqs_i] = ucbest_coarse_tune0p5T_P1[dqs_i];
            ucbest_coarse_tune2T_P1_backup[p->rank][dqs_i] = ucbest_coarse_tune2T_P1[dqs_i];
            #endif
        }

        mcSHOW_DBG_MSG2(("===============================================================================\n"));

        //Restore registers
        vIO32Write4B(DRAMC_REG_STBCAL, u4BakReg_DRAMC_DQSCAL0);
        vIO32Write4B(DRAMC_REG_STBCAL1, u4BakReg_DRAMC_STBCAL_F);
        vIO32Write4B(DRAMC_REG_DDRCONF0, u4BakReg_DRAMC_WODT);
        vIO32Write4B(DRAMC_REG_SPCMD, u4BakReg_DRAMC_SPCMD);
        vIO32Write4B(DRAMC_REG_REFCTRL0, u4BakReg_DRAMC_REFCTRL0);

        // Set Coarse Tune Value to registers
        //DramPhyCGReset(p, 1);// need to reset when UI update or PI change >=2

	#if COMPILE_THIS_PART
        if (p->pinmux == PIN_MUX_TYPE_DDR3X4)
        {
            //swap B12 & B13
            mcSHOW_DBG_MSG(("DDR3X4 GW SWAP\n"));
            ucbest_coarse_tune2T[2] = ucbest_coarse_tune2T[1];
            ucbest_coarse_tune2T[3] = ucbest_coarse_tune2T[1];
            ucbest_coarse_tune2T_P1[2] = ucbest_coarse_tune2T_P1[1];
            ucbest_coarse_tune2T_P1[3] = ucbest_coarse_tune2T_P1[1];
            ucbest_coarse_tune0p5T[2] = ucbest_coarse_tune0p5T[1];
            ucbest_coarse_tune0p5T[3] = ucbest_coarse_tune0p5T[1];
            ucbest_coarse_tune0p5T_P1[2] = ucbest_coarse_tune0p5T_P1[1];
            ucbest_coarse_tune0p5T_P1[3] = ucbest_coarse_tune0p5T_P1[1];
            ucbest_fine_tune[2] = ucbest_fine_tune[1];
            ucbest_fine_tune[3] = ucbest_fine_tune[1];

            for (dqs_i=0; dqs_i<(p->data_width/4); dqs_i++)
            {
                mcSHOW_DBG_MSG(("DDR3X4 R%d FINAL: GW best DQS%d P0 delay(2T, 0.5T, PI) = (%d, %d, %d) [tap = %d]\n", p->rank, dqs_i, ucbest_coarse_tune2T[dqs_i], ucbest_coarse_tune0p5T[dqs_i], ucbest_fine_tune[dqs_i], ucpass_count[dqs_i]));        
            } 
            for (dqs_i=0; dqs_i<(p->data_width/4); dqs_i++)
            {
                mcSHOW_DBG_MSG(("DDR3X4 R%d FINAL: GW best DQS%d P1 delay(2T, 0.5T, PI) = (%d, %d, %d)\n", p->rank, dqs_i, ucbest_coarse_tune2T_P1[dqs_i], ucbest_coarse_tune0p5T_P1[dqs_i], ucbest_fine_tune[dqs_i]));
            } 

            ucbest_coarse_tune0p5T_backup[p->rank][2] = ucbest_coarse_tune0p5T_backup[p->rank][1];
            ucbest_coarse_tune2T_backup[p->rank][2] = ucbest_coarse_tune2T_backup[p->rank][1];
            ucbest_coarse_tune0p5T_backup[p->rank][3] = ucbest_coarse_tune0p5T_backup[p->rank][1];
            ucbest_coarse_tune2T_backup[p->rank][3] = ucbest_coarse_tune2T_backup[p->rank][1];

            ucbest_coarse_tune0p5T_P1_backup[p->rank][2] = ucbest_coarse_tune0p5T_P1_backup[p->rank][1];
            ucbest_coarse_tune2T_P1_backup[p->rank][2] = ucbest_coarse_tune2T_P1_backup[p->rank][1];
            ucbest_coarse_tune0p5T_P1_backup[p->rank][3] = ucbest_coarse_tune0p5T_P1_backup[p->rank][1];
            ucbest_coarse_tune2T_P1_backup[p->rank][3] = ucbest_coarse_tune2T_P1_backup[p->rank][1];
        }

	#endif
        // 4T or 2T coarse tune
        vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQSG0, \
                                    P_Fld((U32) ucbest_coarse_tune2T[0], SHURK0_SELPH_DQSG0_TX_DLY_DQS0_GATED)| \
                                    P_Fld((U32) ucbest_coarse_tune2T[1], SHURK0_SELPH_DQSG0_TX_DLY_DQS1_GATED)| \
                                    //P_Fld((U32) ucbest_coarse_tune2T[2], SHURK0_SELPH_DQSG0_TX_DLY_DQS2_GATED)| \	//YMC mark to avoid warning
                                    //P_Fld((U32) ucbest_coarse_tune2T[3], SHURK0_SELPH_DQSG0_TX_DLY_DQS3_GATED)| \	//YMC mark to avoid warning
                                    P_Fld((U32) ucbest_coarse_tune2T_P1[0], SHURK0_SELPH_DQSG0_TX_DLY_DQS0_GATED_P1)| \
                                    P_Fld((U32) ucbest_coarse_tune2T_P1[1], SHURK0_SELPH_DQSG0_TX_DLY_DQS1_GATED_P1)//| \
                                    //P_Fld((U32) ucbest_coarse_tune2T_P1[2], SHURK0_SELPH_DQSG0_TX_DLY_DQS2_GATED_P1)| \	//YMC mark to avoid warning
                                    //P_Fld((U32) ucbest_coarse_tune2T_P1[3], SHURK0_SELPH_DQSG0_TX_DLY_DQS3_GATED_P1)	//YMC mark to avoid warning
                                    );

        // 0.5T coarse tune
        vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQSG1, \
                                    P_Fld((U32) ucbest_coarse_tune0p5T[0], SHURK0_SELPH_DQSG1_REG_DLY_DQS0_GATED)| \
                                    P_Fld((U32) ucbest_coarse_tune0p5T[1], SHURK0_SELPH_DQSG1_REG_DLY_DQS1_GATED)| \
                                    //P_Fld((U32) ucbest_coarse_tune0p5T[2], SHURK0_SELPH_DQSG1_REG_DLY_DQS2_GATED)| \		//YMC mark to avoid warning
                                    //P_Fld((U32) ucbest_coarse_tune0p5T[3], SHURK0_SELPH_DQSG1_REG_DLY_DQS3_GATED)| \		//YMC mark to avoid warning
                                    P_Fld((U32) ucbest_coarse_tune0p5T_P1[0], SHURK0_SELPH_DQSG1_REG_DLY_DQS0_GATED_P1)| \
                                    P_Fld((U32) ucbest_coarse_tune0p5T_P1[1], SHURK0_SELPH_DQSG1_REG_DLY_DQS1_GATED_P1)//| \
                                    //P_Fld((U32) ucbest_coarse_tune0p5T_P1[2], SHURK0_SELPH_DQSG1_REG_DLY_DQS2_GATED_P1)| \	//YMC mark to avoid warning
                                    //P_Fld((U32) ucbest_coarse_tune0p5T_P1[3], SHURK0_SELPH_DQSG1_REG_DLY_DQS3_GATED_P1)		//YMC mark to avoid warning
                                    );

		
	#if GATING_RODT_LATANCY_EN_PC3    //cc porting from PCDDR3.
		// RODT = Gating - 11UI,
		for (dqs_i=0; dqs_i<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); dqs_i++)
		{
			uctmp_value = (ucbest_coarse_tune2T[dqs_i] <<3)+ucbest_coarse_tune0p5T[dqs_i];
	
			if(uctmp_value>=GATING_RODT_LATANCY_PC3_VALUE)
			{
				//P0
				uctmp_value -=GATING_RODT_LATANCY_PC3_VALUE;
				ucbest_coarse_large_RODT[dqs_i] = uctmp_value >>3;
				ucbest_coarse_0p5T_RODT[dqs_i]	= uctmp_value - (ucbest_coarse_large_RODT[dqs_i] <<3);
	
				//P1
				uctmp_value = (ucbest_coarse_tune2T_P1[dqs_i] <<3)+ucbest_coarse_tune0p5T_P1[dqs_i] -GATING_RODT_LATANCY_PC3_VALUE;
				ucbest_coarse_large_RODT_P1[dqs_i] = uctmp_value >>3;
				ucbest_coarse_0p5T_RODT_P1[dqs_i]  = uctmp_value - (ucbest_coarse_large_RODT_P1[dqs_i] <<3);
	
				mcSHOW_DBG_MSG2(("best RODT delay(2T, 0.5T) = (%d, %d)\n", ucbest_coarse_large_RODT[dqs_i], ucbest_coarse_0p5T_RODT[dqs_i]));
				#if	dbg_print
				prom_puts("DQS");
				prom_print_dec(dqs_i);
				prom_puts(", best RODT delay(2T, 0.5T):");
				prom_print_dec(ucbest_coarse_large_RODT[dqs_i]);
				prom_puts(", ");
				prom_print_dec(ucbest_coarse_0p5T_RODT[dqs_i]);
				prom_puts("\n");
				#endif
			}
			else //if(ucbest_coarse_tune2T[0] ==0)	//shouble not happen,  just only protect this happen
			{
				//P0
				ucbest_coarse_large_RODT[dqs_i] =0;
				ucbest_coarse_0p5T_RODT[dqs_i] = 0;
				//P1
				ucbest_coarse_large_RODT_P1[dqs_i] =2;
				ucbest_coarse_0p5T_RODT_P1[dqs_i] = 2;
	
				mcSHOW_ERR_MSG(("[DramcRxdqsGatingCal] Error: ucbest_coarse_tune2T[%d] is already 0. RODT cannot be -1 UI\n", dqs_i));
				#if	dbg_print
				prom_puts("[DramcRxdqsGatingCal] Error: ucbest_coarse_tune2T is already 0. RODT cannot be -1 UI\n");
				#endif
			}
		}
	
		vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_ODTEN0, \
			P_Fld((U32) ucbest_coarse_large_RODT[0], SHURK0_SELPH_ODTEN0_TXDLY_B0_RODTEN)| \
			P_Fld((U32) ucbest_coarse_large_RODT[1], SHURK0_SELPH_ODTEN0_TXDLY_B1_RODTEN)| \
			//P_Fld((U32) ucbest_coarse_large_RODT[2], SHURK0_SELPH_ODTEN0_TXDLY_B2_RODTEN)| \		//YMC mark to avoid warning
			//P_Fld((U32) ucbest_coarse_large_RODT[3], SHURK0_SELPH_ODTEN0_TXDLY_B3_RODTEN)| \		//YMC mark to avoid warning
			P_Fld((U32) ucbest_coarse_large_RODT_P1[0], SHURK0_SELPH_ODTEN0_TXDLY_B0_RODTEN_P1)| \
			P_Fld((U32) ucbest_coarse_large_RODT_P1[1], SHURK0_SELPH_ODTEN0_TXDLY_B1_RODTEN_P1)//| \
			//P_Fld((U32) ucbest_coarse_large_RODT_P1[2], SHURK0_SELPH_ODTEN0_TXDLY_B2_RODTEN_P1)| \	//YMC mark to avoid warning
			//P_Fld((U32) ucbest_coarse_large_RODT_P1[3], SHURK0_SELPH_ODTEN0_TXDLY_B3_RODTEN_P1)		//YMC mark to avoid warning
			);
	
		vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_ODTEN1, \
			P_Fld((U32) ucbest_coarse_0p5T_RODT[0], SHURK0_SELPH_ODTEN1_DLY_B0_RODTEN)| \
			P_Fld((U32) ucbest_coarse_0p5T_RODT[1], SHURK0_SELPH_ODTEN1_DLY_B1_RODTEN)| \
			//P_Fld((U32) ucbest_coarse_0p5T_RODT[2], SHURK0_SELPH_ODTEN1_DLY_B2_RODTEN)| \		//YMC mark to avoid warning
			//P_Fld((U32) ucbest_coarse_0p5T_RODT[3], SHURK0_SELPH_ODTEN1_DLY_B3_RODTEN)| \		//YMC mark to avoid warning
			P_Fld((U32) ucbest_coarse_0p5T_RODT_P1[0], SHURK0_SELPH_ODTEN1_DLY_B0_RODTEN_P1)| \
			P_Fld((U32) ucbest_coarse_0p5T_RODT_P1[1], SHURK0_SELPH_ODTEN1_DLY_B1_RODTEN_P1)//| \
			//P_Fld((U32) ucbest_coarse_0p5T_RODT_P1[2], SHURK0_SELPH_ODTEN1_DLY_B2_RODTEN_P1)| \	//YMC mark to avoid warning
			//P_Fld((U32) ucbest_coarse_0p5T_RODT_P1[3], SHURK0_SELPH_ODTEN1_DLY_B3_RODTEN_P1)	//YMC mark to avoid warning
			);
	
	#if ENABLE_RODT_TRACKING
		vIO32WriteFldMulti((DRAMC_REG_SHU_RODTENSTB), P_Fld(0x1f, SHU_RODTENSTB_RODTENSTB_EXT)|\
			P_Fld(8, SHU_RODTENSTB_RODTENSTB_OFFSET)|\
			P_Fld(p->odt_onoff, SHU_RODTENSTB_RODTEN_MCK_MODESEL));
	#endif
	
	
	#endif

        // Set Fine Tune Value to registers
        u4value = ucbest_fine_tune[0] | (ucbest_fine_tune[1]<<8); //| (ucbest_fine_tune[2]<<16) | (ucbest_fine_tune[3]<<24);	//YMC mark to avoid waring
        vIO32Write4B(DRAMC_REG_SHURK0_DQSIEN, u4value);

        //mcDELAY_US(1);//delay 2T
        //DramPhyCGReset(p, 0);
        DramPhyReset(p);   //reset phy, reset read data counter

        /*TINFO="[DramcRxdqsGatingCal] ====Done====\n"*/
        mcSHOW_DBG_MSG(("[DramcRxdqsGatingCal] ====Done====\n"));
		#if	dbg_print
		prom_puts("[DramcRxdqsGatingCal] ====Done====\n");
		#endif

#if defined(DRAM_CALIB_LOG) || defined(DRAM_ETT)
        gDRAM_CALIB_LOG.RANK[p->rank].GatingWindow.DQS0_2T = ucbest_coarse_tune2T_P1[0];
        gDRAM_CALIB_LOG.RANK[p->rank].GatingWindow.DQS0_05T = ucbest_coarse_tune0p5T_P1[0];
        gDRAM_CALIB_LOG.RANK[p->rank].GatingWindow.DQS0_PI = ucbest_fine_tune[0];
        gDRAM_CALIB_LOG.RANK[p->rank].GatingWindow.DQS1_2T = ucbest_coarse_tune2T_P1[1];
        gDRAM_CALIB_LOG.RANK[p->rank].GatingWindow.DQS1_05T = ucbest_coarse_tune0p5T_P1[1];
        gDRAM_CALIB_LOG.RANK[p->rank].GatingWindow.DQS1_PI = ucbest_fine_tune[1];
        gDRAM_CALIB_LOG.RANK[p->rank].GatingWindow.DQS2_2T = ucbest_coarse_tune2T_P1[2];
        gDRAM_CALIB_LOG.RANK[p->rank].GatingWindow.DQS2_05T = ucbest_coarse_tune0p5T_P1[2];
        gDRAM_CALIB_LOG.RANK[p->rank].GatingWindow.DQS2_PI = ucbest_fine_tune[2];
        gDRAM_CALIB_LOG.RANK[p->rank].GatingWindow.DQS3_2T = ucbest_coarse_tune2T_P1[3];
        gDRAM_CALIB_LOG.RANK[p->rank].GatingWindow.DQS3_05T = ucbest_coarse_tune0p5T_P1[3];
        gDRAM_CALIB_LOG.RANK[p->rank].GatingWindow.DQS3_PI = ucbest_fine_tune[3];
#endif

        return DRAM_OK;
}
#endif //COMPILE_PCDDR3

#if GATING_ADJUST_TXDLY_FOR_TRACKING
DRAM_STATUS_T DramcRxdqsGatingPostProcess(DRAMC_CTX_T *p)
{
     U8 dqs_i, u1RankRxDVS, ucbit_num;
     U8 u1RankIdx, u1RankMax, u1RankBak;
     S8 s1ChangeDQSINCTL;
     U32 backup_rank;

     U32 u4ReadDQSINCTL, u4ReadTXDLY[RANK_MAX][DQS_NUMBER], u4ReadTXDLY_P1[RANK_MAX][DQS_NUMBER], u4RankINCTL_ROOT, u4XRTR2R, reg_TX_dly_DQSgated_min;

     backup_rank = u1GetRank(p);

#if COMPILE_THIS_PART
     if (p->pinmux == PIN_MUX_TYPE_DDR3X4)
        ucbit_num = 4;
     else
#endif
        ucbit_num = DQS_BIT_NUMBER;

     // 1066 : reg_TX_dly_DQSgated (min) =2
     // 1600 : reg_TX_dly_DQSgated (min) =3
    if(p->frequency <= DDR_DDR1333)
         reg_TX_dly_DQSgated_min = 2;
    else
         reg_TX_dly_DQSgated_min= 3;


    // === Begin of DVS setting =====
    //RANKRXDVS = reg_TX_dly_DQSgated (min) -1 = Roundup(tDQSCKdiff/MCK)
    if(reg_TX_dly_DQSgated_min>1)
    {
         u1RankRxDVS = reg_TX_dly_DQSgated_min -1;
    }
     else
    {
        u1RankRxDVS=0;
        mcSHOW_ERR_MSG(("[DramcRxdqsGatingPostProcess] u1RankRxDVS <1,  Please check!\n"));
    }

    vIO32WriteFldAlign(DDRPHY_SHU1_B0_DQ7, u1RankRxDVS, SHU1_B0_DQ7_R_DMRANKRXDVS_B0);
    vIO32WriteFldAlign(DDRPHY_SHU1_B1_DQ7, u1RankRxDVS, SHU1_B1_DQ7_R_DMRANKRXDVS_B1);
    //cc mark vIO32WriteFldAlign(DDRPHY_SHU1_B0_DQ7+(1<<POS_BANK_NUM), u1RankRxDVS, SHU1_B0_DQ7_R_DMRANKRXDVS_B0);
    //cc mark vIO32WriteFldAlign(DDRPHY_SHU1_B1_DQ7+(1<<POS_BANK_NUM), u1RankRxDVS, SHU1_B1_DQ7_R_DMRANKRXDVS_B1);

     // === End of DVS setting =====

     s1ChangeDQSINCTL = reg_TX_dly_DQSgated_min- u1TXDLY_Cal_min;

     mcSHOW_DBG_MSG(("[DramcRxdqsGatingPostProcess] p->frequency %d\n", p->frequency));
     mcSHOW_DBG_MSG(("[DramcRxdqsGatingPostProcess] s1ChangeDQSINCTL %d, reg_TX_dly_DQSgated_min %d, u1TXDLY_Cal_min %d\n", s1ChangeDQSINCTL, reg_TX_dly_DQSgated_min, u1TXDLY_Cal_min));

#if defined(DRAM_CALIB_LOG) || defined(DRAM_ETT)
     gDRAM_CALIB_LOG.RxdqsGatingPostProcess.s1ChangeDQSINCTL = s1ChangeDQSINCTL;
     gDRAM_CALIB_LOG.RxdqsGatingPostProcess.reg_TX_dly_DQSgated_min = reg_TX_dly_DQSgated_min;
     gDRAM_CALIB_LOG.RxdqsGatingPostProcess.u1TXDLY_Cal_min = u1TXDLY_Cal_min;
#endif

     if(s1ChangeDQSINCTL!=0)  // need to change DQSINCTL and TXDLY of each byte
     {
         u1TXDLY_Cal_min += s1ChangeDQSINCTL;
         u1TXDLY_Cal_max += s1ChangeDQSINCTL;

        #if DUAL_RANK_ENABLE
        if (p->support_rank_num==RANK_DUAL)
            u1RankMax = RANK_MAX;
        else
        #endif
             u1RankMax =RANK_MAX; //cc modified from RANK_1-->RANK_MAX

        for(u1RankIdx=0; u1RankIdx<u1RankMax; u1RankIdx++)
        {
             mcSHOW_DBG_MSG2(("====DramcRxdqsGatingPostProcess (Rank = %d) ========================================\n", u1RankIdx));

             for (dqs_i=0; dqs_i<(p->data_width/ucbit_num); dqs_i++)
             {
                 u4ReadTXDLY[u1RankIdx][dqs_i]= ((ucbest_coarse_tune2T_backup[u1RankIdx][dqs_i]<<1) + ((ucbest_coarse_tune0p5T_backup[u1RankIdx][dqs_i]>>2) & 0x1));
                 u4ReadTXDLY_P1[u1RankIdx][dqs_i]= ((ucbest_coarse_tune2T_P1_backup[u1RankIdx][dqs_i]<<1) + ((ucbest_coarse_tune0p5T_P1_backup[u1RankIdx][dqs_i]>>2) & 0x1));

                 u4ReadTXDLY[u1RankIdx][dqs_i] += s1ChangeDQSINCTL;
                 u4ReadTXDLY_P1[u1RankIdx][dqs_i] += s1ChangeDQSINCTL;

                 ucbest_coarse_tune2T_backup[u1RankIdx][dqs_i] = (u4ReadTXDLY[u1RankIdx][dqs_i] >>1);
                 ucbest_coarse_tune0p5T_backup[u1RankIdx][dqs_i] = ((u4ReadTXDLY[u1RankIdx][dqs_i] & 0x1) <<2)+(ucbest_coarse_tune0p5T_backup[u1RankIdx][dqs_i] & 0x3);

                 ucbest_coarse_tune2T_P1_backup[u1RankIdx][dqs_i] = (u4ReadTXDLY_P1[u1RankIdx][dqs_i] >>1);
                 ucbest_coarse_tune0p5T_P1_backup[u1RankIdx][dqs_i] = ((u4ReadTXDLY_P1[u1RankIdx][dqs_i] & 0x1)<<2) +(ucbest_coarse_tune0p5T_P1_backup[u1RankIdx][dqs_i] & 0x3);

                 mcSHOW_DBG_MSG(("best DQS%d P0 delay(2T, 0.5T) = (%d, %d)\n", dqs_i, ucbest_coarse_tune2T_backup[u1RankIdx][dqs_i], ucbest_coarse_tune0p5T_backup[u1RankIdx][dqs_i]));
             }

             for (dqs_i=0; dqs_i<(p->data_width/ucbit_num); dqs_i++)
             {
                mcSHOW_DBG_MSG(("best DQS%d P1 delay(2T, 0.5T) = (%d, %d)\n", dqs_i, ucbest_coarse_tune2T_P1_backup[u1RankIdx][dqs_i], ucbest_coarse_tune0p5T_P1_backup[u1RankIdx][dqs_i]));
             }
        }

        for(u1RankIdx=0; u1RankIdx<u1RankMax; u1RankIdx++)
        {
            vSetRank(p, u1RankIdx);
            // 4T or 2T coarse tune
            vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQSG0, \
                                    P_Fld((U32) ucbest_coarse_tune2T_backup[u1RankIdx][0], SHURK0_SELPH_DQSG0_TX_DLY_DQS0_GATED)| \
                                    P_Fld((U32) ucbest_coarse_tune2T_backup[u1RankIdx][1], SHURK0_SELPH_DQSG0_TX_DLY_DQS1_GATED)| \
                                    P_Fld((U32) ucbest_coarse_tune2T_backup[u1RankIdx][2], SHURK0_SELPH_DQSG0_TX_DLY_DQS2_GATED)| \
                                    P_Fld((U32) ucbest_coarse_tune2T_backup[u1RankIdx][3], SHURK0_SELPH_DQSG0_TX_DLY_DQS3_GATED)| \
                                    P_Fld((U32) ucbest_coarse_tune2T_P1_backup[u1RankIdx][0], SHURK0_SELPH_DQSG0_TX_DLY_DQS0_GATED_P1)| \
                                    P_Fld((U32) ucbest_coarse_tune2T_P1_backup[u1RankIdx][1], SHURK0_SELPH_DQSG0_TX_DLY_DQS1_GATED_P1)| \
                                    P_Fld((U32) ucbest_coarse_tune2T_P1_backup[u1RankIdx][2], SHURK0_SELPH_DQSG0_TX_DLY_DQS2_GATED_P1)| \
                                    P_Fld((U32) ucbest_coarse_tune2T_P1_backup[u1RankIdx][3], SHURK0_SELPH_DQSG0_TX_DLY_DQS3_GATED_P1));

                // 0.5T coarse tune
            vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQSG1, \
                                    P_Fld((U32) ucbest_coarse_tune0p5T_backup[u1RankIdx][0], SHURK0_SELPH_DQSG1_REG_DLY_DQS0_GATED)| \
                                    P_Fld((U32) ucbest_coarse_tune0p5T_backup[u1RankIdx][1], SHURK0_SELPH_DQSG1_REG_DLY_DQS1_GATED)| \
                                    P_Fld((U32) ucbest_coarse_tune0p5T_backup[u1RankIdx][2], SHURK0_SELPH_DQSG1_REG_DLY_DQS2_GATED)| \
                                    P_Fld((U32) ucbest_coarse_tune0p5T_backup[u1RankIdx][3], SHURK0_SELPH_DQSG1_REG_DLY_DQS3_GATED)| \
                                    P_Fld((U32) ucbest_coarse_tune0p5T_P1_backup[u1RankIdx][0], SHURK0_SELPH_DQSG1_REG_DLY_DQS0_GATED_P1)| \
                                    P_Fld((U32) ucbest_coarse_tune0p5T_P1_backup[u1RankIdx][1], SHURK0_SELPH_DQSG1_REG_DLY_DQS1_GATED_P1)| \
                                    P_Fld((U32) ucbest_coarse_tune0p5T_P1_backup[u1RankIdx][2], SHURK0_SELPH_DQSG1_REG_DLY_DQS2_GATED_P1)| \
                                    P_Fld((U32) ucbest_coarse_tune0p5T_P1_backup[u1RankIdx][3], SHURK0_SELPH_DQSG1_REG_DLY_DQS3_GATED_P1));
        }
    }
    vSetRank(p, backup_rank);

    u4ReadDQSINCTL = u4IO32ReadFldAlign(DRAMC_REG_SHURK0_DQSCTL, SHURK0_DQSCTL_DQSINCTL);
    u4ReadDQSINCTL -= s1ChangeDQSINCTL;

    if(u4ReadDQSINCTL>=2)
        u4RankINCTL_ROOT = u4ReadDQSINCTL-2;
    else
    {
        u4RankINCTL_ROOT=0;
        mcSHOW_ERR_MSG(("[DramcRxdqsGatingPostProcess] DQSINCTL <2,  Risk for supporting 1066/RL8\n"));
    }

    //DISINCTL only apply to RANK0
    vSetRank(p, RANK_0);
    vIO32WriteFldAlign(DRAMC_REG_SHURK0_DQSCTL, u4ReadDQSINCTL, SHURK0_DQSCTL_DQSINCTL);  //Rank0 DQSINCTL
    vIO32WriteFldAlign(DRAMC_REG_SHURK1_DQSCTL, u4ReadDQSINCTL, SHURK1_DQSCTL_R1DQSINCTL); //Rank1 DQSINCTL, no use in A-PHY. Common DQSINCTL of both rank.
    vIO32WriteFldAlign(DRAMC_REG_SHU_RANKCTL, u4ReadDQSINCTL, SHU_RANKCTL_RANKINCTL_PHY);  //RANKINCTL_PHY = DQSINCTL
    vIO32WriteFldAlign(DRAMC_REG_SHU_RANKCTL, u4RankINCTL_ROOT, SHU_RANKCTL_RANKINCTL);  //RANKINCTL= DQSINCTL -2
    vIO32WriteFldAlign(DRAMC_REG_SHU_RANKCTL, u4RankINCTL_ROOT, SHU_RANKCTL_RANKINCTL_ROOT1);  //RANKINCTL_ROOT1= DQSINCTL -2

    //XRTR2R=A-phy forbidden margin(6T) + reg_TX_dly_DQSgated (max) +Roundup(tDQSCKdiff/MCK+0.25MCK)+1(05T sel_ph margin)-1(forbidden margin overlap part)
    //Roundup(tDQSCKdiff/MCK+1UI) =1~2 all LP3 and LP4 timing
    u4XRTR2R= 8 + u1TXDLY_Cal_max;  // 6+ u1TXDLY_Cal_max +2
    vIO32WriteFldAlign(DRAMC_REG_SHU_ACTIM_XRT, u4XRTR2R, SHU_ACTIM_XRT_XRTR2R);
    vSetRank(p, backup_rank);

    mcSHOW_DBG_MSG2(("TX_dly_DQSgated check: min %d  max %d,  s1ChangeDQSINCTL=%d\n", u1TXDLY_Cal_min, u1TXDLY_Cal_max, s1ChangeDQSINCTL));
    mcSHOW_DBG_MSG2(("DQSINCTL=%d, RANKINCTL=%d, u4XRTR2R=%d\n", u4ReadDQSINCTL, u4RankINCTL_ROOT, u4XRTR2R));

#if defined(DRAM_CALIB_LOG) || defined(DRAM_ETT)
    gDRAM_CALIB_LOG.RxdqsGatingPostProcess.TX_dly_DQSgated_check_min = u1TXDLY_Cal_min;
    gDRAM_CALIB_LOG.RxdqsGatingPostProcess.TX_dly_DQSgated_check_max = u1TXDLY_Cal_max;
    gDRAM_CALIB_LOG.RxdqsGatingPostProcess.DQSINCTL = u4ReadDQSINCTL;
    gDRAM_CALIB_LOG.RxdqsGatingPostProcess.RANKINCTL = u4RankINCTL_ROOT;
    gDRAM_CALIB_LOG.RxdqsGatingPostProcess.u4XRTR2R = u4XRTR2R;
#endif

	return DRAM_OK;
}
#endif


#if GATING_ADJUST_TXDLY_FOR_TRACKING
DRAM_STATUS_T DramcRxdqsGatingPreProcess(DRAMC_CTX_T *p)
{
    u1TXDLY_Cal_min =0xff;
    u1TXDLY_Cal_max=0;

	return DRAM_OK;
}
#endif
#endif //SIMULATION_GATING

//-------------------------------------------------------------------------
/** DramcRxWindowPerbitCal (v2 version)
 *  start the rx dqs perbit sw calibration.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 */
//-------------------------------------------------------------------------

static void SetRxDqDqsDelay(DRAMC_CTX_T *p, S16 iDelay)
{
    U8 u1ByteIdx;
    U32 u4value;
    U8 dl_value[8];
    U8 ucbit_num;

#if COMPILE_THIS_PART
    if (p->pinmux == PIN_MUX_TYPE_DDR3X4)
        ucbit_num = 4;
    else
#endif
        ucbit_num = DQS_BIT_NUMBER;

    if (iDelay <=0)
    {
        // Set DQS delay
        //B0 DQS
        vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ6, P_Fld(-iDelay, SHU1_R0_B0_DQ6_RK0_RX_ARDQS0_F_DLY_B0)
        			| P_Fld(-iDelay, SHU1_R0_B0_DQ6_RK0_RX_ARDQS0_R_DLY_B0));
        //B1 DQS
        vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ6, P_Fld(-iDelay, SHU1_R0_B1_DQ6_RK0_RX_ARDQS0_F_DLY_B1)
        			| P_Fld(-iDelay, SHU1_R0_B1_DQ6_RK0_RX_ARDQS0_R_DLY_B1));

        DramPhyReset(p);
    }
    else
    {
        // Adjust DQM output delay.

        for (u1ByteIdx=0; u1ByteIdx<(p->data_width/ucbit_num); u1ByteIdx++)
        {
            Set_RX_DQM_DelayLine_Phy_Byte(p, u1ByteIdx, iDelay);
        }

        // Adjust DQ output delay.
        u4value = ((U32) iDelay) | (((U32)iDelay)<<8) | (((U32)iDelay)<<16) | (((U32)iDelay)<<24);

        //every 2bit dq have the same delay register address
        for (u1ByteIdx=0; u1ByteIdx<(p->data_width/ucbit_num); u1ByteIdx++)
        {
            dl_value[0] = iDelay;
            dl_value[1] = iDelay;
            dl_value[2] = iDelay;
            dl_value[3] = iDelay;
            dl_value[4] = iDelay;
            dl_value[5] = iDelay;
            dl_value[6] = iDelay;
            dl_value[7] = iDelay;

            Set_RX_DQ_DelayLine_Phy_Byte(p, u1ByteIdx, dl_value);
        }
    }
}

#define RX_eye_scan 0

#if SIMULATION_RX_PERBIT

#if COMPILE_PCDDR3
#define RX_VREF_DEFALUT_VAULE	0x12
#else
#define RX_VREF_DEFALUT_VAULE	0x13
#endif

#if FOR_DV_SIMULATION
#define RX_VREF_SCAN_BEGIN		RX_VREF_DEFALUT_VAULE  /* cc notes: This is the recomended value by DE. */
#define RX_VREF_SCAN_STEP		4
#define RX_VREF_SCAN_END		(RX_VREF_SCAN_BEGIN+1)//32
#else
#define RX_VREF_SCAN_BEGIN		0
#define RX_VREF_SCAN_STEP		1
#if RX_eye_scan
#define RX_VREF_SCAN_END		64
#else
#define RX_VREF_SCAN_END		32
#endif
#endif

#if (COMPILE_PCDDR3 && DDR3_MPR_USED)
#define MPR_GOLDEN_DATA0 0x88880000
#define MPR_GOLDEN_DATA1 0x99991111
#define MPR_GOLDEN_DATA2 0xaaaa2222
#define MPR_GOLDEN_DATA3 0xbbbb3333

DRAM_STATUS_T DramcRxMPRInit(DRAMC_CTX_T *p)
{
	//U8 u1ReadDBIbak[2];
	//U32 u4MRS_reg_bak;

	// Disable Read DBI
	//u1ReadDBIbak[0] = u4IO32ReadFldAlign((DDRPHY_SHU1_B0_DQ7), SHU1_B0_DQ7_R_DMDQMDBI_SHU_B0);
	//u1ReadDBIbak[1] = u4IO32ReadFldAlign((DDRPHY_SHU1_B1_DQ7), SHU1_B1_DQ7_R_DMDQMDBI_SHU_B1);
	vIO32WriteFldAlign((DRAMC_REG_DRAMC_PD_CTRL), 0, DRAMC_PD_CTRL_PHYCLKDYNGEN);	 //PHYCLKDYNGEN=0
	vIO32WriteFldMulti(DRAMC_REG_CKECTRL, P_Fld(0x1, CKECTRL_CKEFIXON) 
			| P_Fld(0x1, CKECTRL_CKE1FIXON));
	vIO32WriteFldAlign((DDRPHY_SHU1_B0_DQ7),  0, SHU1_B0_DQ7_R_DMDQMDBI_SHU_B0);
	vIO32WriteFldAlign((DDRPHY_SHU1_B1_DQ7),  0, SHU1_B1_DQ7_R_DMDQMDBI_SHU_B1);
	vIO32WriteFldAlign((DRAMC_REG_SPCMDCTRL), 1, SPCMDCTRL_RDDQCDIS);

	/* Enable data flow from MPR */
	DramcModeRegWrite_PC3(p, 3, 0x004);
}

static U32 DramcRxMPRCalibrationRun(DRAMC_CTX_T *p)
{
    U32 u4Result, u4Response,u4MRRDATA0,u4MRRDATA1,u4MRRDATA2,u4MRRDATA3;
    U32 u4TimeCnt= TIME_OUT_CNT;

    //Issue RD DQ calibration
    //R_DMRDDQCEN, 0x1E4[7]=1 for RDDQC,  R_DMRDDQCDIS, 0x1EC[26]=1 to stop RDDQC burst
    //Wait rddqc_response=1, (dramc_conf_nao, 0x3b8[31])
    vIO32WriteFldAlign((DRAMC_REG_MRS), 0x0, MRS_MRSMA);
    vIO32WriteFldAlign((DRAMC_REG_SPCMD), 1, SPCMD_MRREN);

    do
    {
        u4Response = u4IO32ReadFldAlign((DRAMC_REG_SPCMDRESP), SPCMDRESP_MRR_RESPONSE);
        u4TimeCnt --;
        mcDELAY_US(1);
    }while((u4Response ==0) &&(u4TimeCnt>0));

    if(u4TimeCnt==0)//time out
    {
        mcSHOW_DBG_MSG(("[DramcRxWinRDDQC] u4Response fail (time out)\n"));
        //return DRAM_FAIL;
    }

    //Then read RDDQC compare result (dramc_conf_nao, 0x36c)
    u4MRRDATA0 = u4IO32ReadFldAlign((DRAMC_REG_MRRDATA0), MRRDATA0_MRR_DATA0); //pass patten 0x88880000
    u4MRRDATA1 = u4IO32ReadFldAlign((DRAMC_REG_MRRDATA1), MRRDATA1_MRR_DATA1); //pass patten 0x99991111
    u4MRRDATA2 = u4IO32ReadFldAlign((DRAMC_REG_MRRDATA2), MRRDATA2_MRR_DATA2); //pass patten 0xaaaa2222
    u4MRRDATA3 = u4IO32ReadFldAlign((DRAMC_REG_MRRDATA3), MRRDATA3_MRR_DATA3); //pass patten 0xbbbb3333
     //  mcSHOW_DBG_MSG(("[DramcRxWinRDDQC] u4MRRDATA0=0x%x   u4MRRDATA1=x%x  u4MRRDATA2=0x%x  u4MRRDATA3=0x%x\n",u4MRRDATA0,u4MRRDATA1,u4MRRDATA2,u4MRRDATA3));

//    u4MRRDATA0 = u4IO32ReadFldAlign(DRAMC_REG_MRR0DATA0, MRR0DATA0_MRR_DATA00)&0xFF00FF00; //pass patten 0x88880000
//    u4MRRDATA1 = u4IO32ReadFldAlign(DRAMC_REG_MRR0DATA1, MRR0DATA1_MRR_DATA01)&0xFF00FF00; //pass patten 0x99991111
//    u4MRRDATA2 = u4IO32ReadFldAlign(DRAMC_REG_MRR0DATA2, MRR0DATA2_MRR_DATA02)&0xFF00FF00; //pass patten 0xaaaa2222
//    u4MRRDATA3 = u4IO32ReadFldAlign(DRAMC_REG_MRR0DATA3, MRR0DATA3_MRR_DATA03)&0xFF00FF00; //pass patten 0xbbbb3333
     //  mcSHOW_DBG_MSG(("[DramcRxWinRDDQC] u4MRRDATA0=0x%x   u4MRRDATA1=x%x  u4MRRDATA2=0x%x  u4MRRDATA3=0x%x\n",u4MRRDATA0,u4MRRDATA1,u4MRRDATA2,u4MRRDATA3));

    if(u4MRRDATA0==MPR_GOLDEN_DATA0 && u4MRRDATA1==MPR_GOLDEN_DATA1 && u4MRRDATA2==MPR_GOLDEN_DATA2 && u4MRRDATA3==MPR_GOLDEN_DATA3 )
	{
		u4Result=0;
	}
    else
	{
		u4Result=0xffffffff;
	}
    //R_DMRDDQCEN, 0x1E4[7]=0
    vIO32WriteFldAlign((DRAMC_REG_SPCMD), 0, SPCMD_MRREN);
    //vIO32WriteFldAlign((DRAMC_REG_PERFCTL0), 0, PERFCTL0_RDDQCDIS);

    return u4Result;
}

void  DramcRxMPRCalibrationEnd(DRAMC_CTX_T *p)
{
    //Recover Read DBI
    //vIO32WriteFldAlign((DDRPHY_SHU1_B0_DQ7),  p->DBI_R_onoff, SHU1_B0_DQ7_R_DMDQMDBI_SHU_B0);
    //vIO32WriteFldAlign((DDRPHY_SHU1_B1_DQ7),  p->DBI_R_onoff, SHU1_B1_DQ7_R_DMDQMDBI_SHU_B1);

    // Recover MPC Rank
    vIO32WriteFldAlign((DRAMC_REG_MRS), 0, MRS_MPCRK); 
     DramcModeRegWrite_PC3(p, 3, 0x0);
   // vIO32WriteFldAlign((DRAMC_REG_DRAMC_PD_CTRL), 1, DRAMC_PD_CTRL_PHYCLKDYNGEN);   //PHYCLKDYNGEN=1
}
#endif

DRAM_STATUS_T DramcRxWindowPerbitCal(DRAMC_CTX_T *p, U8 u1UseTestEngine)
{
    U8 ii, u1BitIdx, u1ByteIdx;
    U8 ucbit_first, ucbit_last, ucbit_num;
    S16 iDelay, u4DelayBegin, u4DelayEnd, u4DelayStep=1;
    U32 uiFinishCount;
    U32 u4value, u4err_value, u4fail_bit;
    PASS_WIN_DATA_T WinPerBit[DQ_DATA_WIDTH], FinalWinPerBit[DQ_DATA_WIDTH], TempWinPerBit[DQ_DATA_WIDTH];
    S32 iDQSDlyPerbyte[4] = {0,0,0,0}, iDQMDlyPerbyte[4] = {0,0,0,0};//, iFinalDQSDly[DQS_NUMBER];
    U16 u2TempWinSum, u2MaxWinSum, u2TmpDQMSum;

    U16 u2WinSize;
    U16 u2MinWinSize = 0xffff;
    U8 u1MinWinSizeBitidx = 0;

    U8 dl_value[8]={0,0,0,0,0,0,0,0};
    U8 backup_rank;
	
	U8 u1VrefScanEnable;
	U16 u2VrefScanBegin, u2VrefScanEnd, u2VrefScanStep;
	U16 u2VrefValue;
	U16	u2VrefValue_opt;
	//YMC add for pinmux on DRAMC DQ and PHY DQ delay RG(PAD_ARDQ). index: phy DQ delay RG(PAD_ARDQ), val: DRAMC DQ
	U8 pinmux_KGD[DQ_DATA_WIDTH]={0,1,2,3,4,5,7,6,10,9,8,11,14,15,13,12};
	

    #if REG_ACCESS_PORTING_DGB
    RegLogEnable =1;
    mcSHOW_DBG_MSG(("\n[REG_ACCESS_PORTING_FUNC]   DramcRxWindowPerbitCal\n"));
    #endif

    // error handling
    if (!p)
    {
        mcSHOW_ERR_MSG(("context is NULL\n"));
        return DRAM_FAIL;
    }

    backup_rank = u1GetRank(p);

    // 1.delay DQ ,find the pass widnow (left boundary).
    // 2.delay DQS find the pass window (right boundary).
    // 3.Find the best DQ / DQS to satify the middle value of the overall pass window per bit
    // 4.Set DQS delay to the max per byte, delay DQ to de-skew
    mcSHOW_DBG_MSG(("\n[RX]\n"));
	#if	dbg_print
	prom_puts("\n[RX]\n");
	#endif
    vPrintCalibrationBasicInfo(p);
    mcSHOW_DBG_MSG2(("Start DQ delay to find pass range, UseTestEngine =%d\n", u1UseTestEngine));
    mcSHOW_DBG_MSG2(("x-axis is bit #; y-axis is DQ delay (%d~%d)\n", (-MAX_RX_DQSDLY_TAPS), MAX_RX_DQDLY_TAPS));
	#if	dbg_print
	prom_puts("Start DQ delay to find pass range, UseTestEngine = ");	
	prom_print_dec(u1UseTestEngine);
	prom_puts("\n");
	prom_puts("x-axis is bit #; y-axis is DQ delay\n");
	#endif
    //defult set result fail. When window found, update the result as oK
    if (u1UseTestEngine)
	    vSetCalibrationResult(p, DRAM_CALIBRATION_RX_PERBIT, DRAM_FAIL);
	else
	    vSetCalibrationResult(p, DRAM_CALIBRATION_RX_RDDQC, DRAM_FAIL);

	if (!u1UseTestEngine) {
		if (p->dram_type == TYPE_PCDDR3) {
		#if (COMPILE_PCDDR3 && DDR3_MPR_USED)
			DramcRxMPRInit(p);
		#endif
		}
	}

	if (u1UseTestEngine && p->enable_rx_scan_vref && p->rank == RANK_0)
		u1VrefScanEnable = 1;
	else
		u1VrefScanEnable = 0;

	#if RX_eye_scan
		u1VrefScanEnable = 1;
	#endif

	if (u1VrefScanEnable) {
		u2VrefValue_opt = RX_VREF_SCAN_BEGIN;
		u2VrefScanBegin = RX_VREF_SCAN_BEGIN;
		u2VrefScanEnd = RX_VREF_SCAN_END;
		u2VrefScanStep = RX_VREF_SCAN_STEP;
	} else {
		/* If disabled, shall set VREF to the defalut value in case
		 * VREF is changed to incorrect value
		 */
		u2VrefScanBegin = RX_VREF_DEFALUT_VAULE;
		u2VrefScanEnd = u2VrefScanBegin+1;
		u2VrefScanStep = RX_VREF_SCAN_STEP;
	}

    // initialize parameters
    u2TempWinSum =0;
	u2MaxWinSum = 0;
    uiFinishCount =0;

    for (u1BitIdx = 0; u1BitIdx < p->data_width; u1BitIdx++)
    {
        WinPerBit[u1BitIdx].first_pass = (S16)PASS_RANGE_NA;
        WinPerBit[u1BitIdx].last_pass = (S16)PASS_RANGE_NA;
        WinPerBit[u1BitIdx].win_center = (S16)PASS_RANGE_NA;
        FinalWinPerBit[u1BitIdx].first_pass = (S16)PASS_RANGE_NA;
        FinalWinPerBit[u1BitIdx].last_pass = (S16)PASS_RANGE_NA;
        FinalWinPerBit[u1BitIdx].win_center = (S16)PASS_RANGE_NA;
		TempWinPerBit[u1BitIdx].first_pass = (S16)PASS_RANGE_NA;
		TempWinPerBit[u1BitIdx].last_pass = (S16)PASS_RANGE_NA;
		TempWinPerBit[u1BitIdx].win_center = (S16)PASS_RANGE_NA;
    }

#if COMPILE_THIS_PART
    if (p->pinmux == PIN_MUX_TYPE_DDR3X4)
        ucbit_num = 4;
    else
#endif
        ucbit_num = DQS_BIT_NUMBER;

	//cc notes: how to set for DDR1866??
    if(p->frequency >= DDR_DDR1600)
    {
        u4DelayBegin= -48;
    }
    else if(p->frequency >= DDR_DDR1333)
    {
        u4DelayBegin= -70;
    }
    else if(p->frequency >= DDR_DDR1066)
    {
        u4DelayBegin= -96;
    }
    else
    {
        u4DelayBegin= -MAX_RX_DQSDLY_TAPS;
    }

    u4DelayEnd = MAX_RX_DQDLY_TAPS;
#if FOR_DV_SIMULATION
	u4DelayStep = 2; //cc to speed up simulation
#else
    u4DelayStep =1;
#endif
#if RX_eye_scan
	for (u2VrefValue = (u2VrefScanEnd-1); u2VrefValue < u2VrefScanEnd; u2VrefValue -= u2VrefScanStep)
#else
	for (u2VrefValue = u2VrefScanBegin; u2VrefValue < u2VrefScanEnd; u2VrefValue += u2VrefScanStep)
#endif
	{		
		// Adjust DQM output delay to 0
	    for (u1ByteIdx=0; u1ByteIdx<(p->data_width/ucbit_num); u1ByteIdx++)
	    {
	        Set_RX_DQM_DelayLine_Phy_Byte(p, u1ByteIdx, 0);
	    }

	    // Adjust DQ output delay to 0
	    //every 2bit dq have the same delay register address


	    for (u1ByteIdx=0; u1ByteIdx<(p->data_width/ucbit_num); u1ByteIdx++)
	    {
	        Set_RX_DQ_DelayLine_Phy_Byte(p, u1ByteIdx, dl_value);
	    }
		mcSHOW_DBG_MSG(("Loop Vref = %d\n", u2VrefValue));
		#if	dbg_print
		prom_puts("Loop Vref = ");
		prom_print_dec(u2VrefValue);
		prom_puts("\n");
		#endif
		/* Set RxVref and Enable Eye scan mode */
		vIO32WriteFldAlign(DDRPHY_SHU1_B0_DQ5, u2VrefValue, SHU1_B0_DQ5_RG_RX_ARDQ_VREF_SEL_B0);//cc change from DE 17.10.23 rev.
		vIO32WriteFldAlign(DDRPHY_SHU1_B1_DQ5, u2VrefValue, SHU1_B1_DQ5_RG_RX_ARDQ_VREF_SEL_B1);//cc change from DE 17.10.23 rev.
		
	    for (iDelay=u4DelayBegin; iDelay<=u4DelayEnd; iDelay+= u4DelayStep)
	    {
	        SetRxDqDqsDelay(p, iDelay);

	        if(u1UseTestEngine) {
	            u4err_value = TestEngineCompare(p);
	        } else {
				if (p->dram_type == TYPE_PCDDR3) {
				#if (COMPILE_PCDDR3 && DDR3_MPR_USED)
					u4err_value = DramcRxMPRCalibrationRun(p);
				#endif
				}
			}

	        // if(u4err_value != 0  || iDelay == u4DelayBegin)
	        #if	dbg_print
	        if(iDelay <= -10)
	        {
	   	       //-10
	            mcSHOW_DBG_MSG2(("%d, [0]", iDelay));
			   	prom_puts("-");	
				prom_print_dec(-iDelay);
				prom_puts(", [0]");	
	        }
	        else if((iDelay > -10) && (iDelay < 0))
	        {
	            //-9
	            mcSHOW_DBG_MSG2(("%d , [0]", iDelay));
				prom_puts("-");	
				prom_print_dec(-iDelay);
				prom_puts(" , [0]");	
	        }
	        else if((iDelay >= 0) && (iDelay < 10))
	        {
	            //9
	            mcSHOW_DBG_MSG2((" %d , [0]", iDelay));
				prom_puts(" ");
				prom_print_dec(iDelay);
				prom_puts(" , [0]");
	        }
	        else
	        {//10
	            mcSHOW_DBG_MSG2(("%d , [0]", iDelay));
				prom_print_dec(iDelay);
				prom_puts(" , [0]");
	        }
			#endif
	        //mcSHOW_DBG_MSG2(("u4err_value %x, u1MRRValue %x\n", u4err_value, u1MRRValue));

	        // check fail bit ,0 ok ,others fail
	        for (u1BitIdx = 0; u1BitIdx < p->data_width; u1BitIdx++)
	        {
	            u4fail_bit = u4err_value&((U32)1<<u1BitIdx);

	            if(WinPerBit[u1BitIdx].first_pass== PASS_RANGE_NA)
	            {
	                if(u4fail_bit==0) //compare correct: pass
	                {
	                    WinPerBit[u1BitIdx].first_pass = iDelay;
	                }
	            }
	            else if(WinPerBit[u1BitIdx].last_pass == PASS_RANGE_NA)
	            {
	                //mcSHOW_DBG_MSG(("fb%d \n", u4fail_bit));

	                if(u4fail_bit !=0) //compare error : fail
	                {
	                    WinPerBit[u1BitIdx].last_pass  = (iDelay-1);
	                }
	                else if (iDelay==u4DelayEnd)
	                {
	                    WinPerBit[u1BitIdx].last_pass  = iDelay;
	                }

	                if(WinPerBit[u1BitIdx].last_pass  !=PASS_RANGE_NA)
	                {
	                    if((WinPerBit[u1BitIdx].last_pass -WinPerBit[u1BitIdx].first_pass) >= (TempWinPerBit[u1BitIdx].last_pass -TempWinPerBit[u1BitIdx].first_pass))
	                    {
	                        #if 0 //for debug
	                        if(FinalWinPerBit[u1BitIdx].last_pass != PASS_RANGE_NA)
	                        {
	                            mcSHOW_DBG_MSG2(("Bit[%d] Bigger window update %d > %d\n", u1BitIdx, \
	                                (WinPerBit[u1BitIdx].last_pass -WinPerBit[u1BitIdx].first_pass), (FinalWinPerBit[u1BitIdx].last_pass -FinalWinPerBit[u1BitIdx].first_pass)));
	                        }
	                        #endif
	                        uiFinishCount |= (1<<u1BitIdx);
	                        //update bigger window size
	                        TempWinPerBit[u1BitIdx].first_pass = WinPerBit[u1BitIdx].first_pass;
	                        TempWinPerBit[u1BitIdx].last_pass = WinPerBit[u1BitIdx].last_pass;
	                    }
	                    //reset tmp window
	                    WinPerBit[u1BitIdx].first_pass = PASS_RANGE_NA;
	                    WinPerBit[u1BitIdx].last_pass = PASS_RANGE_NA;
	                }
	            }

	          //  if(u4err_value != 0 || iDelay == u4DelayBegin)
	            {
	            	#if	dbg_print
	                if(u1BitIdx%DQS_BIT_NUMBER ==0)
	                {
	                    mcSHOW_DBG_MSG2((" "));
						prom_puts(" ");
	                }

	                if (u4fail_bit == 0)
	                {
	                    mcSHOW_DBG_MSG2(("o"));
						prom_puts("o");
	                }
	                else
	                {
	                    mcSHOW_DBG_MSG2(("x"));
						prom_puts("x");
	                }
					#endif
	            }
	        }

	       // if(u4err_value != 0 || iDelay == u4DelayBegin)
	        {
	            mcSHOW_DBG_MSG2((" [MSB]\n"));
				#if	dbg_print
				prom_puts(" [MSB]\n");
				#endif
	        }

	    //if all bits widnow found and all bits turns to fail again, early break;
	    	if(((p->data_width== DATA_WIDTH_16BIT) &&(uiFinishCount == 0xffff)) || \
				((p->data_width== DATA_WIDTH_32BIT) &&(uiFinishCount == 0xffffffff)))
	        {
	            if(u1UseTestEngine)
	                vSetCalibrationResult(p, DRAM_CALIBRATION_RX_PERBIT, DRAM_OK);
				else				
					vSetCalibrationResult(p, DRAM_CALIBRATION_RX_RDDQC, DRAM_OK);
	            {
	       		 	if(((p->data_width== DATA_WIDTH_16BIT) &&((u4err_value&0xffff) == 0xffff)) || \
						((p->data_width== DATA_WIDTH_32BIT) &&(u4err_value == 0xffffffff)))
	                {
	                        mcSHOW_DBG_MSG2(("\nRX all bits window found, early break!\n"));
							#if	dbg_print
							prom_puts("\nRX all bits window found, early break!\n");
							#endif
	                        break;  //early break
	                }
	             }
	        }
	    }

		if (!u1UseTestEngine) {
			if (p->dram_type == TYPE_PCDDR3) {
			#if (COMPILE_PCDDR3 && DDR3_MPR_USED)
				u4err_value = DramcRxMPRCalibrationEnd(p);
			#endif
			}
		}

		/* Calculate the Total window for this Vref */
	    for (u1BitIdx = 0; u1BitIdx < p->data_width; u1BitIdx++)
	    {
	        u2WinSize = TempWinPerBit[u1BitIdx].last_pass-TempWinPerBit[u1BitIdx].first_pass;

	        u2TempWinSum += u2WinSize;  //Sum of CA Windows for vref selection
	    }

		mcSHOW_DBG_MSG(("Vref = %d, WinSum = %d > WinMax %d\n", u2VrefValue, u2TempWinSum, u2MaxWinSum));
		#if	dbg_print
		prom_puts("Vref = ");
		prom_print_dec(u2VrefValue);
		prom_puts(", WinSum = ");
		prom_print_dec(u2TempWinSum);
		prom_puts("\n");
		#endif
		if (u2TempWinSum > u2MaxWinSum) {			
			mcSHOW_DBG_MSG(("Better Rx Vref %d found. Winsum = %d\n", u2VrefValue, u2TempWinSum));
			if (u1VrefScanEnable)
			{
			prom_puts("Better Rx Vref found.\n");
				prom_puts("Vref = ");
				prom_print_dec(u2VrefValue);
				prom_puts("\n");
				u2VrefValue_opt = u2VrefValue;
				
			}
			
			for (u1BitIdx = 0; u1BitIdx < p->data_width; u1BitIdx++) {
				FinalWinPerBit[u1BitIdx].first_pass = TempWinPerBit[u1BitIdx].first_pass;
				FinalWinPerBit[u1BitIdx].last_pass = TempWinPerBit[u1BitIdx].last_pass;
			}
			u2MaxWinSum = u2TempWinSum;
		}

		
		for (u1BitIdx = 0; u1BitIdx < p->data_width; u1BitIdx++) {
			TempWinPerBit[u1BitIdx].first_pass = PASS_RANGE_NA;
			TempWinPerBit[u1BitIdx].last_pass = PASS_RANGE_NA;
		}
		
		uiFinishCount = 0;
		u2TempWinSum = 0;
	}

	u2TempWinSum = 0;
	for (u1BitIdx = 0; u1BitIdx < p->data_width; u1BitIdx++)
	{
		u2WinSize = FinalWinPerBit[u1BitIdx].last_pass-FinalWinPerBit[u1BitIdx].first_pass;
	
		u2TempWinSum += u2WinSize;	//Sum of DQ Windows for vref selection
	
		if(u2WinSize<u2MinWinSize)
		{
			u2MinWinSize = u2WinSize;
			u1MinWinSizeBitidx = u1BitIdx;
		}
	}

    #ifdef ENABLE_CALIBRATION_WINDOW_LOG_FOR_FT
    mcSHOW_DBG_MSG(("FT log: RX min window : bit %d, size %d\n", u1MinWinSizeBitidx, u2MinWinSize));
    if (u2MinWinSize < 15)
    {
        mcSHOW_DBG_MSG(("FT log: RX error bit%d window:%d is too small!!\n", u1MinWinSizeBitidx, u2MinWinSize));
        ASSERT(0);
    }
    #endif

    mcSHOW_DBG_MSG(("RX Window Sum %d\n", u2TempWinSum));
	#if	dbg_print
	prom_puts("\nOpt. window:\n");
	#endif
    for (u1BitIdx=0; u1BitIdx<p->data_width; u1BitIdx++)
    {
        FinalWinPerBit[u1BitIdx].win_center = (FinalWinPerBit[u1BitIdx].last_pass + FinalWinPerBit[u1BitIdx].first_pass)>>1;     // window center of each DQ bit

        mcSHOW_DBG_MSG(("R%d FINAL:  %d, %d (%d ~ %d) %d\n", p->rank, u1BitIdx, FinalWinPerBit[u1BitIdx].win_center, FinalWinPerBit[u1BitIdx].first_pass, FinalWinPerBit[u1BitIdx].last_pass, FinalWinPerBit[u1BitIdx].last_pass - FinalWinPerBit[u1BitIdx].first_pass + 1));
		#if	dbg_print
		prom_puts("Rank:");
		prom_print_dec(p->rank);
		prom_puts(", RX Bit:");
		prom_print_dec(u1BitIdx);
		prom_puts(", Window center:");
		if(FinalWinPerBit[u1BitIdx].win_center>=0)
		{
			prom_print_dec(FinalWinPerBit[u1BitIdx].win_center);
		}
		else
		{
			prom_puts("-");
			prom_print_dec(-FinalWinPerBit[u1BitIdx].win_center);
		}
		prom_puts(", Window first:");
		if(FinalWinPerBit[u1BitIdx].first_pass>=0)
		{
			prom_print_dec(FinalWinPerBit[u1BitIdx].first_pass);
		}
		else
		{
			prom_puts("-");
			prom_print_dec(-FinalWinPerBit[u1BitIdx].first_pass);
		}
		prom_puts(", Window last:");
		if(FinalWinPerBit[u1BitIdx].last_pass>=0)
		{
			prom_print_dec(FinalWinPerBit[u1BitIdx].last_pass);
		}
		else
		{
			prom_puts("-");
			prom_print_dec(-FinalWinPerBit[u1BitIdx].last_pass);
		}
		prom_puts("\n");
		#endif
        #ifdef ENABLE_CALIBRATION_WINDOW_LOG_FOR_FT
        if (FinalWinPerBit[u1BitIdx].win_center < 0)
        {
            mcSHOW_DBG_MSG(("FT log: RX error bit%d center:%d < 0!!\n", u1BitIdx, FinalWinPerBit[u1BitIdx].win_center));
            ASSERT(0);
        }
        #endif
    }

    // 3
    //As per byte, check max DQS delay in 8-bit. Except for the bit of max DQS delay, delay DQ to fulfill setup time = hold time
    for (u1ByteIdx = 0; u1ByteIdx < (DQ_DATA_WIDTH/DQS_BIT_NUMBER); u1ByteIdx++)
    {
        u2TmpDQMSum =0;

        ucbit_first =DQS_BIT_NUMBER*u1ByteIdx;
        ucbit_last = DQS_BIT_NUMBER*u1ByteIdx+DQS_BIT_NUMBER-1;
        iDQSDlyPerbyte[u1ByteIdx] = MAX_RX_DQSDLY_TAPS;

        for (u1BitIdx = ucbit_first; u1BitIdx <= ucbit_last; u1BitIdx++)
        {
            // find out min Center value
            if(FinalWinPerBit[u1BitIdx].win_center < iDQSDlyPerbyte[u1ByteIdx])
                iDQSDlyPerbyte[u1ByteIdx] = FinalWinPerBit[u1BitIdx].win_center;

            //mcSHOW_DBG_MSG(("bit#%2d : center=(%2d)\n", u1BitIdx, FinalWinPerBit[u1BitIdx].win_center));
        }

        //mcSHOW_DBG_MSG(("----seperate line----\n"));

        if (iDQSDlyPerbyte[u1ByteIdx]  > 0)  // Delay DQS=0, Delay DQ only
        {
            iDQSDlyPerbyte[u1ByteIdx]  = 0;
        }
        else  //Need to delay DQS
        {
            iDQSDlyPerbyte[u1ByteIdx]  = -iDQSDlyPerbyte[u1ByteIdx] ;
        }

        // we delay DQ or DQS to let DQS sample the middle of rx pass window for all the 8 bits,
        for (u1BitIdx = ucbit_first; u1BitIdx <= ucbit_last; u1BitIdx++)
        {
            FinalWinPerBit[u1BitIdx].best_dqdly = iDQSDlyPerbyte[u1ByteIdx] + FinalWinPerBit[u1BitIdx].win_center;
            u2TmpDQMSum += FinalWinPerBit[u1BitIdx].best_dqdly;
        }

        // calculate DQM as average of 8 DQ delay
        iDQMDlyPerbyte[u1ByteIdx] = u2TmpDQMSum/DQS_BIT_NUMBER;
    }

    vPrintCalibrationBasicInfo(p);
#if 0//cc mark
    mcSHOW_DBG_MSG(("DQS Delay :\nDQS0 = %d, DQS1 = %d, DQS2 = %d, DQS3 = %d\n", iDQSDlyPerbyte[0], iDQSDlyPerbyte[1], iDQSDlyPerbyte[2], iDQSDlyPerbyte[3]));
    mcSHOW_DBG_MSG(("DQM Delay :\nDQM0 = %d, DQM1 = %d, DQM2 = %d, DQM3 = %d\n", iDQMDlyPerbyte[0], iDQMDlyPerbyte[1], iDQMDlyPerbyte[2], iDQMDlyPerbyte[3]));
    mcSHOW_DBG_MSG(("DQ Delay :\n"));
#endif
	//cc add
	mcSHOW_DBG_MSG(("DQS Delay :\nDQS0 = %d, DQS1 = %d\n", iDQSDlyPerbyte[0], iDQSDlyPerbyte[1]));
    mcSHOW_DBG_MSG(("DQM Delay :\nDQM0 = %d, DQM1 = %d\n", iDQMDlyPerbyte[0], iDQMDlyPerbyte[1]));
    mcSHOW_DBG_MSG(("DQ Delay :\n"));
	#if	dbg_print
	prom_puts("DQS Delay :\nDQS0 = ");
	prom_print_dec(iDQSDlyPerbyte[0]);
	prom_puts(", DQS1 = ");
	prom_print_dec(iDQSDlyPerbyte[1]);
	prom_puts("\nDQM Delay :\nDQM0 = ");
	prom_print_dec(iDQMDlyPerbyte[0]);
	prom_puts(", DQM1 = ");
	prom_print_dec(iDQMDlyPerbyte[1]);
	prom_puts("\nDQ Delay :\n");
	#endif
	if (u1VrefScanEnable)
	{
		/* Set RxVref to opt. value */
		vIO32WriteFldAlign(DDRPHY_SHU1_B0_DQ5, u2VrefValue_opt, SHU1_B0_DQ5_RG_RX_ARDQ_VREF_SEL_B0);
		vIO32WriteFldAlign(DDRPHY_SHU1_B1_DQ5, u2VrefValue_opt, SHU1_B1_DQ5_RG_RX_ARDQ_VREF_SEL_B1);
	}
    for (u1BitIdx = 0; u1BitIdx < p->data_width; u1BitIdx=u1BitIdx+4)
    {
        mcSHOW_DBG_MSG(("DQ%d =%d, DQ%d =%d, DQ%d =%d, DQ%d =%d \n", 
			u1BitIdx, FinalWinPerBit[u1BitIdx].best_dqdly, 
			u1BitIdx+1, FinalWinPerBit[u1BitIdx+1].best_dqdly, 
			u1BitIdx+2, FinalWinPerBit[u1BitIdx+2].best_dqdly, 
			u1BitIdx+3, FinalWinPerBit[u1BitIdx+3].best_dqdly));
		#if	dbg_print
		prom_puts("DQ");
		prom_print_dec(u1BitIdx);
		prom_puts(" = ");
		prom_print_dec(FinalWinPerBit[u1BitIdx].best_dqdly);
		prom_puts(", DQ");
		prom_print_dec(u1BitIdx+1);
		prom_puts(" = ");
		prom_print_dec(FinalWinPerBit[u1BitIdx+1].best_dqdly);
		prom_puts(", DQ");
		prom_print_dec(u1BitIdx+2);
		prom_puts(" = ");
		prom_print_dec(FinalWinPerBit[u1BitIdx+2].best_dqdly);
		prom_puts(", DQ");
		prom_print_dec(u1BitIdx+3);
		prom_puts(" = ");
		prom_print_dec(FinalWinPerBit[u1BitIdx+3].best_dqdly);
		prom_puts("\n");
		#endif
		
    }
    mcSHOW_DBG_MSG(("________________________________________________________________________\n"));

#if COMPILE_THIS_PART
    // Set DQS & DQM delay
    /* p->rank = RANK_0, save to Reg Rank0 and Rank1, p->rank = RANK_1, save to Reg Rank1 */
    if (p->pinmux == PIN_MUX_TYPE_DDR3X4)
    {
        //swap B12 & B13
        mcSHOW_DBG_MSG(("DDR3X4 RX SWAP\n"));
        iDQSDlyPerbyte[2] = iDQSDlyPerbyte[1];
        iDQSDlyPerbyte[3] = iDQSDlyPerbyte[1];
        iDQMDlyPerbyte[2] = iDQMDlyPerbyte[1];
        iDQMDlyPerbyte[3] = iDQMDlyPerbyte[1];
        memcpy(FinalWinPerBit+16, FinalWinPerBit+8, sizeof(PASS_WIN_DATA_T) * DQS_BIT_NUMBER);
        memcpy(FinalWinPerBit+24, FinalWinPerBit+8, sizeof(PASS_WIN_DATA_T) * DQS_BIT_NUMBER);

        mcSHOW_DBG_MSG(("DDR3X4 DQS Delay :\nDQS0 = %d, DQS1 = %d, DQS2 = %d, DQS3 = %d\n", iDQSDlyPerbyte[0], iDQSDlyPerbyte[1], iDQSDlyPerbyte[2], iDQSDlyPerbyte[3]));
        mcSHOW_DBG_MSG(("DDR3X4 DQM Delay :\nDQM0 = %d, DQM1 = %d, DQM2 = %d, DQM3 = %d\n", iDQMDlyPerbyte[0], iDQMDlyPerbyte[1], iDQMDlyPerbyte[2], iDQMDlyPerbyte[3]));
        mcSHOW_DBG_MSG(("DDR3X4 DQ Delay :\n"));

        for (u1BitIdx = 16; u1BitIdx < DQ_DATA_WIDTH; u1BitIdx=u1BitIdx+4)
        {
            mcSHOW_DBG_MSG(("DDR3X4 DQ%d =%d, DQ%d =%d, DQ%d =%d, DQ%d =%d \n", u1BitIdx, FinalWinPerBit[u1BitIdx].best_dqdly, u1BitIdx+1, FinalWinPerBit[u1BitIdx+1].best_dqdly, u1BitIdx+2, FinalWinPerBit[u1BitIdx+2].best_dqdly, u1BitIdx+3, FinalWinPerBit[u1BitIdx+3].best_dqdly));
        }
    }
#endif

    for(ii=p->rank; ii<RANK_MAX; ii++)
    {
        vSetRank(p,ii);

        for (u1ByteIdx = 0; u1ByteIdx < (p->data_width/ucbit_num); u1ByteIdx++)
        {

        	Set_RX_DQM_DelayLine_Phy_Byte(p, u1ByteIdx,(U32)iDQMDlyPerbyte[u1ByteIdx]);
        }

        //B0 DQS
        vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ6, P_Fld(iDQSDlyPerbyte[0], SHU1_R0_B0_DQ6_RK0_RX_ARDQS0_F_DLY_B0)
        			| P_Fld(iDQSDlyPerbyte[0], SHU1_R0_B0_DQ6_RK0_RX_ARDQS0_R_DLY_B0));
        //B1 DQS
        vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ6, P_Fld(iDQSDlyPerbyte[1], SHU1_R0_B1_DQ6_RK0_RX_ARDQS0_F_DLY_B1)
        			| P_Fld(iDQSDlyPerbyte[1], SHU1_R0_B1_DQ6_RK0_RX_ARDQS0_R_DLY_B1));
	#if 0//cc mark 
		//B2 DQS
        vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ6+(1<<POS_BANK_NUM), P_Fld(iDQSDlyPerbyte[2], SHU1_R0_B0_DQ6_RK0_RX_ARDQS0_F_DLY_B0)
        			| P_Fld(iDQSDlyPerbyte[2], SHU1_R0_B0_DQ6_RK0_RX_ARDQS0_R_DLY_B0));
        //B3 DQS
        vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ6+(1<<POS_BANK_NUM), P_Fld(iDQSDlyPerbyte[3], SHU1_R0_B1_DQ6_RK0_RX_ARDQS0_F_DLY_B1)
        			| P_Fld(iDQSDlyPerbyte[3], SHU1_R0_B1_DQ6_RK0_RX_ARDQS0_R_DLY_B1));
	#endif
	}
    vSetRank(p, backup_rank);

    DramPhyReset(p);

    // set dq delay

#if COMPILE_THIS_PART //cc notes: Currently do not support 4bit pinmux
    if (p->en_4bitMux == ENABLE)
    {
         mcSHOW_DBG_MSG(("Rx 4bitMux is enabled\n"));

#if 1
        for(u1BitIdx = 0; u1BitIdx < p->data_width; u1BitIdx++)
        {
            mcSHOW_DBG_MSG(("%d ", FinalWinPerBit[u1BitIdx].best_dqdly));
        }
        mcSHOW_DBG_MSG(("\n"));
#endif

        for(u1BitIdx = 0; u1BitIdx < p->data_width; u1BitIdx++)
        {
            memcpy(&WinPerBit[Bit_DQ_Mapping[u1BitIdx]], &FinalWinPerBit[u1BitIdx], sizeof(PASS_WIN_DATA_T));
        }
        memcpy(FinalWinPerBit, WinPerBit, sizeof(PASS_WIN_DATA_T) * DQ_DATA_WIDTH);

#if 1
        for(u1BitIdx = 0; u1BitIdx < p->data_width; u1BitIdx++)
        {
            mcSHOW_DBG_MSG(("%d ", FinalWinPerBit[u1BitIdx].best_dqdly));
        }
        mcSHOW_DBG_MSG(("\n"));
#endif
    }
#endif //COMPILE_THIS_PART

    /* p->rank = RANK_0, save to Reg Rank0 and Rank1, p->rank = RANK_1, save to Reg Rank1 */
    for(ii=p->rank; ii<RANK_MAX; ii++)
    {
        vSetRank(p,ii);

        //every 2bit dq have the same delay register address
        for (u1ByteIdx=0; u1ByteIdx<(p->data_width/ucbit_num); u1ByteIdx++)
        {
            u1BitIdx = u1ByteIdx * DQS_BIT_NUMBER;
			if(p->pinmux == PIN_MUX_TYPE_DDR3KGD || p->pinmux == PIN_MUX_TYPE_DDR2KGD)
			{
				dl_value[0] = FinalWinPerBit[pinmux_KGD[u1BitIdx]].best_dqdly;
	            dl_value[1] = FinalWinPerBit[pinmux_KGD[u1BitIdx+1]].best_dqdly;
	            dl_value[2] = FinalWinPerBit[pinmux_KGD[u1BitIdx+2]].best_dqdly;
	            dl_value[3] = FinalWinPerBit[pinmux_KGD[u1BitIdx+3]].best_dqdly;
	            dl_value[4] = FinalWinPerBit[pinmux_KGD[u1BitIdx+4]].best_dqdly;
	            dl_value[5] = FinalWinPerBit[pinmux_KGD[u1BitIdx+5]].best_dqdly;
	            dl_value[6] = FinalWinPerBit[pinmux_KGD[u1BitIdx+6]].best_dqdly;
	            dl_value[7] = FinalWinPerBit[pinmux_KGD[u1BitIdx+7]].best_dqdly;
			}
			else
			{
	            dl_value[0] = FinalWinPerBit[u1BitIdx].best_dqdly;
	            dl_value[1] = FinalWinPerBit[u1BitIdx+1].best_dqdly;
	            dl_value[2] = FinalWinPerBit[u1BitIdx+2].best_dqdly;
	            dl_value[3] = FinalWinPerBit[u1BitIdx+3].best_dqdly;
	            dl_value[4] = FinalWinPerBit[u1BitIdx+4].best_dqdly;
	            dl_value[5] = FinalWinPerBit[u1BitIdx+5].best_dqdly;
	            dl_value[6] = FinalWinPerBit[u1BitIdx+6].best_dqdly;
	            dl_value[7] = FinalWinPerBit[u1BitIdx+7].best_dqdly;
			}

            Set_RX_DQ_DelayLine_Phy_Byte(p, u1ByteIdx, dl_value);
        }
    }
    vSetRank(p, backup_rank);


    mcSHOW_DBG_MSG(("[DramcRxWindowPerbitCal] ====Done====\n"));
	#if	dbg_print
	prom_puts("[DramcRxWindowPerbitCal] ====Done====\n");
	#endif

#if defined(DRAM_CALIB_LOG) || defined(DRAM_ETT)
    gDRAM_CALIB_LOG.RANK[p->rank].RxWindowPerbitCal.DQS0_delay = iDQSDlyPerbyte[0];
    gDRAM_CALIB_LOG.RANK[p->rank].RxWindowPerbitCal.DQS1_delay = iDQSDlyPerbyte[1];
    gDRAM_CALIB_LOG.RANK[p->rank].RxWindowPerbitCal.DQS2_delay = iDQSDlyPerbyte[2];
    gDRAM_CALIB_LOG.RANK[p->rank].RxWindowPerbitCal.DQS3_delay = iDQSDlyPerbyte[3];
    gDRAM_CALIB_LOG.RANK[p->rank].RxWindowPerbitCal.DQM0_delay = iDQMDlyPerbyte[0];
    gDRAM_CALIB_LOG.RANK[p->rank].RxWindowPerbitCal.DQM1_delay = iDQMDlyPerbyte[1];
    gDRAM_CALIB_LOG.RANK[p->rank].RxWindowPerbitCal.DQM2_delay = iDQMDlyPerbyte[2];
    gDRAM_CALIB_LOG.RANK[p->rank].RxWindowPerbitCal.DQM3_delay = iDQMDlyPerbyte[3];

    for (u1BitIdx = 0; u1BitIdx < p->data_width; u1BitIdx++)
    {
        gDRAM_CALIB_LOG.RANK[p->rank].RxWindowPerbitCal.WinPerBit[u1BitIdx].first_pass = FinalWinPerBit[u1BitIdx].first_pass;
        gDRAM_CALIB_LOG.RANK[p->rank].RxWindowPerbitCal.WinPerBit[u1BitIdx].last_pass = FinalWinPerBit[u1BitIdx].last_pass;
        gDRAM_CALIB_LOG.RANK[p->rank].RxWindowPerbitCal.WinPerBit[u1BitIdx].win_size = FinalWinPerBit[u1BitIdx].last_pass - FinalWinPerBit[u1BitIdx].first_pass + 1;
        gDRAM_CALIB_LOG.RANK[p->rank].RxWindowPerbitCal.WinPerBit[u1BitIdx].win_center = FinalWinPerBit[u1BitIdx].win_center;
        gDRAM_CALIB_LOG.RANK[p->rank].RxWindowPerbitCal.WinPerBit[u1BitIdx].left_margin= FinalWinPerBit[u1BitIdx].win_center - FinalWinPerBit[u1BitIdx].first_pass;
        gDRAM_CALIB_LOG.RANK[p->rank].RxWindowPerbitCal.WinPerBit[u1BitIdx].right_margin= FinalWinPerBit[u1BitIdx].last_pass - FinalWinPerBit[u1BitIdx].win_center;
        gDRAM_CALIB_LOG.RANK[p->rank].RxWindowPerbitCal.DQ_delay[u1BitIdx] = FinalWinPerBit[u1BitIdx].best_dqdly;
    }
#endif

#if REG_ACCESS_PORTING_DGB
    RegLogEnable =0;
#endif
    p->min_winsize = u2MinWinSize;
    p->sum_winsize = u2TempWinSum;

    return DRAM_OK;
}
#endif //SIMULATION_RX_PERBIT

#if SIMULATION_DATLAT
static void dle_factor_handler(DRAMC_CTX_T *p, U8 curr_val)
{
#if REG_ACCESS_PORTING_DGB
    RegLogEnable =1;
    mcSHOW_DBG_MSG(("\n[REG_ACCESS_PORTING_FUNC]   dle_factor_handler\n"));
#endif

    if(curr_val<3)
        curr_val =3;

    vIO32WriteFldMulti(DRAMC_REG_SHU_CONF1,
        P_Fld(curr_val, SHU_CONF1_DATLAT) |
        P_Fld(curr_val -2, SHU_CONF1_DATLAT_DSEL) | //cc change from -4 -> -2
        P_Fld(curr_val -3, SHU_CONF1_DATLAT_DSEL_PHY)); //cc change from -4 -> -3
	

    DramPhyReset(p);

#if REG_ACCESS_PORTING_DGB
    RegLogEnable =0;
#endif
}

//-------------------------------------------------------------------------
/** Dramc_ta2_rx_scans
 */
//-------------------------------------------------------------------------
static U32 Dramc_ta2_rx_scan(DRAMC_CTX_T *p)
{
    U32 u4err_value = 0xffffffff;
    S16 iDelay;
    U8 u1ByteIdx;

    U8 dl_value[8]={0,0,0,0,0,0,0,0};

    // Adjust DQM output delay to 0
    for(u1ByteIdx=0; u1ByteIdx<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); u1ByteIdx++)
    {
        Set_RX_DQM_DelayLine_Phy_Byte(p, u1ByteIdx, 0);
    }

    // Adjust DQ output delay to 0
    //every 2bit dq have the same delay register address
    for (u1ByteIdx=0; u1ByteIdx<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); u1ByteIdx++)
    {
        Set_RX_DQ_DelayLine_Phy_Byte(p, u1ByteIdx, dl_value);
    }

   // quick rx dqs search
   //mcSHOW_DBG_MSG(("quick rx dqs search\n"));
   for (iDelay=-32; iDelay<=32; iDelay+=4)
   {
       //mcSHOW_DBG_MSG(("%2d, ", iDelay));

       SetRxDqDqsDelay(p, iDelay);
       u4err_value = TestEngineCompare(p);

       if(u4err_value ==0)// rx dqs found.
            break;
    }

    mcSHOW_DBG_MSG(("RX DQS delay = %d, ", iDelay));

    return u4err_value;
}


static U8 aru1RxDatlatResult[RANK_MAX];

U8 DramcRxdatlatScan(DRAMC_CTX_T *p, DRAM_DATLAT_CALIBRATION_TYTE_T use_rxtx_scan)
{
    U8 ii, ucStartCalVal=0;
    U32 u4prv_register_080;
    U32 u4err_value= 0xffffffff;
    U8 ucfirst, ucbegin, ucsum, ucbest_step;

    // error handling
    if (!p)
    {
        mcSHOW_ERR_MSG(("context is NULL\n"));
        return DRAM_FAIL;
    }
#if REG_ACCESS_PORTING_DGB
    RegLogEnable =1;
    mcSHOW_DBG_MSG(("\n[REG_ACCESS_PORTING_FUNC]   DramcRxdatlatCal\n"));
#endif
	mcSHOW_DBG_MSG(("\n[DATLAT]\n"));
	#if	dbg_print
	prom_puts("\n[DATLAT]\n");
	#endif
    mcSHOW_DBG_MSG2(("==============================================================\n"));
    mcSHOW_DBG_MSG2((" Frequency=%d, Rank=%d, use_rxtx_scan=%d\n", p->frequency, p->rank, use_rxtx_scan));
    mcSHOW_DBG_MSG2(("==============================================================\n"));
	#if	dbg_print
	prom_puts("Frequency=");
	prom_print_dec(p->frequency);
	prom_puts(", Rank=");
	prom_print_dec(p->rank);
	prom_puts(", use_rxtx_scan=");
	prom_print_dec(use_rxtx_scan);
	prom_puts("\n");
	#endif

    // [11:10] DQIENQKEND 01 -> 00 for DATLAT calibration issue, DQS input enable will refer to DATLAT
    // if need to enable this (for power saving), do it after all calibration done
    //u4prv_register_0d8 = u4IO32Read4B(DRAMC_REG_MCKDLY);
    //vIO32WriteFldMulti(DRAMC_REG_PADCTRL, P_Fld(0, PADCTRL_DQIENQKEND) | P_Fld(0, PADCTRL_DQIENLATEBEGIN));

    // pre-save
    // 0x07c[6:4]   DATLAT bit2-bit0
    u4prv_register_080 = u4IO32Read4B(DRAMC_REG_SHU_CONF1);

    // init best_step to default
    ucbest_step = (U8) u4IO32ReadFldAlign(DRAMC_REG_SHU_CONF1, SHU_CONF1_DATLAT);
    mcSHOW_DBG_MSG(("DATLAT Default value = 0x%x\n", ucbest_step));
	#if	dbg_print
	prom_puts("DATLAT Default value = 0x");
	prom_print_hex(ucbest_step,1);
	prom_puts("\n");
	#endif
    // 1.set DATLAT 0-15 (0-21 for MT6595)
    // 2.enable engine1 or engine2
    // 3.check result  ,3~4 taps pass
    // 4.set DATLAT 2nd value for optimal

    // Initialize
    ucfirst = 0xff;
    ucbegin = 0;
    ucsum = 0;
#if FOR_DV_SIMULATION	//cc to speed up simulation
	if (p->dram_type == TYPE_PCDDR3)
		ucStartCalVal = 9;
	else if (p->dram_type == TYPE_PCDDR4)
		ucStartCalVal = 5;
#else
	ucStartCalVal = 5;
#endif

    for (ii = ucStartCalVal; ii < DATLAT_TAP_NUMBER; ii++)
    {
        // 1
        dle_factor_handler(p, ii);

        // 2
        if(use_rxtx_scan == fcDATLAT_USE_DEFAULT)
        {
            u4err_value = TestEngineCompare(p);
        }
        else  //if(use_rxtx_scan == fcDATLAT_USE_RX_SCAN)//LPDDR3, LP4 Should not enter if datlat calibration is after RDDQC
        {
            u4err_value = Dramc_ta2_rx_scan(p);
        }

        // 3
        if(p->data_width == DATA_WIDTH_16BIT)
        {
            u4err_value = u4err_value & 0x0000FFFF;
        }

        if (u4err_value == 0)
        {
            if (ucbegin == 0)
            {
                // first tap which is pass
                ucfirst = ii;
                ucbegin = 1;
            }
            if (ucbegin == 1)
            {
                ucsum++;

                if(ucsum >5)
                    break;  //early break.
            }
        }
        else
        {
            if (ucbegin == 1)
            {
                // pass range end
                ucbegin = 0xff;
            }
        }

        mcSHOW_DBG_MSG(("%d, 0x%X, sum=%d\n", ii, u4err_value, ucsum));
		#if	dbg_print
		prom_print_dec(ii);
		prom_puts(", err value = 0x");
		prom_print_hex(u4err_value,4);
		prom_puts(", sum=");
		prom_print_dec(ucsum);
		prom_puts("\n");
		#endif
    }

    // 4
    if (ucsum == 0)
    {
        mcSHOW_ERR_MSG(("no DATLAT taps pass, DATLAT calibration fail!!\n"));
		#if	dbg_print
		prom_puts("no DATLAT taps pass, DATLAT calibration fail!!\n");
		#endif
    }
    else if (ucsum <= 3)
    {
        ucbest_step = ucfirst + (ucsum>>1);
    }
    else // window is larger htan 3
    {
        ucbest_step = ucfirst + 2;
    }

    aru1RxDatlatResult[p->rank] = ucbest_step;

    mcSHOW_DBG_MSG(("pattern=%d first_step=%d total pass=%d best_step=%d\n", p->test_pattern, ucfirst, ucsum, ucbest_step));
    mcSHOW_DBG_MSG(("R%d FINAL: DATLAT = %d [%d ~ %d]\n", p->rank, ucbest_step, ucfirst, ucfirst+ucsum-1));
	#if	dbg_print
 	prom_puts("pattern = ");
 	prom_print_dec(p->test_pattern);
 	prom_puts(", first_step = ");
	prom_print_dec(ucfirst);
	prom_puts(", total pass = ");
	prom_print_dec(ucsum);
	prom_puts(", best_step = ");
	prom_print_dec(ucbest_step);
	prom_puts("\n");
	#endif

#if defined(DRAM_CALIB_LOG) || defined(DRAM_ETT)
    gDRAM_CALIB_LOG.RANK[p->rank].DATLAT.best_step = ucbest_step;
#endif

    if(ucsum <5)
    {
        mcSHOW_DBG_MSG2(("NOTICE]DatlatSum %d\n", ucsum));
		#if	dbg_print
		prom_puts("NOTICE]DatlatSum ");
		prom_print_dec(ucsum);
		prom_puts("\n");
		#endif
    }

    if (ucsum == 0)
    {
        mcSHOW_ERR_MSG(("DATLAT calibration fail, write back to default values!\n"));
		#if	dbg_print
		prom_puts("DATLAT calibration fail, write back to default values!\n");
		#endif
        vIO32Write4B(DRAMC_REG_SHU_CONF1, u4prv_register_080);
        vSetCalibrationResult(p, DRAM_CALIBRATION_DATLAT, DRAM_FAIL);
    }
    else
    {
        dle_factor_handler(p, ucbest_step);
        vSetCalibrationResult(p, DRAM_CALIBRATION_DATLAT, DRAM_OK);
    }

    // [11:10] DQIENQKEND 01 -> 00 for DATLAT calibration issue, DQS input enable will refer to DATLAT
    // if need to enable this (for power saving), do it after all calibration done
    vIO32WriteFldMulti(DRAMC_REG_PADCTRL, P_Fld(1, PADCTRL_DQIENQKEND) | P_Fld(1, PADCTRL_DQIENLATEBEGIN));

    mcSHOW_DBG_MSG(("[DramcRxdatlatCal] ====Done====\n"));
	#if	dbg_print
	prom_puts("[DramcRxdatlatCal] ====Done====\n");
	#endif

#if REG_ACCESS_PORTING_DGB
    RegLogEnable =0;
#endif

    return ucsum;
}

DRAM_STATUS_T DramcRxdatlatCal(DRAMC_CTX_T *p)
{
    U8 u1DatlatWindowSum;

    u1DatlatWindowSum = DramcRxdatlatScan(p, fcDATLAT_USE_DEFAULT);

    if(u1DatlatWindowSum <5)
    {
        mcSHOW_DBG_MSG(("\nu1DatlatWindowSum %d is too small(<5), Start RX + Datlat scan\n", u1DatlatWindowSum));
		#if	dbg_print
		prom_puts("\nu1DatlatWindowSum is too small(<5), Start RX + Datlat scan\n");
		#endif
        DramcRxdatlatScan(p, fcDATLAT_USE_RX_SCAN);
    }

	return DRAM_OK;
}
#if	DUAL_RANK_ENABLE	//YMC add to avoid warning
DRAM_STATUS_T DramcDualRankRxdatlatCal(DRAMC_CTX_T *p)
{
    U8 u1FinalDatlat, u1Datlat0, u1Datlat1;

    u1Datlat0 = aru1RxDatlatResult[0];
    u1Datlat1 = aru1RxDatlatResult[1];

    if(u1Datlat0> u1Datlat1)
    {
        u1FinalDatlat= u1Datlat0;
    }
    else
    {
        u1FinalDatlat= u1Datlat1;
    }

    dle_factor_handler(p, u1FinalDatlat);
    mcSHOW_DBG_MSG(("[DramcDualRankRxdatlatCal] RANK0: %d, RANK1: %d, Final_Datlat %d\n", u1Datlat0, u1Datlat1, u1FinalDatlat));

#if defined(DRAM_CALIB_LOG) || defined(DRAM_ETT)
    gDRAM_CALIB_LOG.DualRankRxdatlatCal.Rank0_Datlat = u1Datlat0;
    gDRAM_CALIB_LOG.DualRankRxdatlatCal.Rank1_Datlat = u1Datlat1;
    gDRAM_CALIB_LOG.DualRankRxdatlatCal.Final_Datlat = u1FinalDatlat;
#endif

    return DRAM_OK;

}
#endif
#endif //SIMULATION_DATLAT

#if SIMULATION_TX_PERBIT
//-------------------------------------------------------------------------
/** DramcTxWindowPerbitCal (v2)
 *  TX DQS per bit SW calibration.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  apply           (U8): 0 don't apply the register we set  1 apply the register we set ,default don't apply.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 */
//-------------------------------------------------------------------------
#define ENABLE_64_PI_TO_UI 1 //cc notes, DE says 1UI may not be 32 PI, but 1T is absolutely 64 PI
#if ENABLE_64_PI_TO_UI
#define TX_DQ_UI_TO_PI_TAP         64 // 1 PI = tCK/64, total 128 PI, 1UI = 32 PI
#else
#define TX_DQ_UI_TO_PI_TAP         32 // 1 PI = tCK/64, total 128 PI, 1UI = 32 PI
#endif

#define TX_VREF_RANGE_BEGIN       0
#define TX_VREF_RANGE_END           50 // binary 110010
#define TX_VREF_RANGE_STEP         2

static void TxWinTransferDelayToUIPI(DRAMC_CTX_T *p, U16 uiDelay, U8 u1UISmallBak, U8 u1UILargeBak, U8* pu1UILarge, U8* pu1UISmall, U8* pu1PI)
{
    U8 u1Small_ui_to_large;
    U16 u2TmpValue;

    //in LP4, 8 small UI =  1 large UI
    //in LP3, 4 small UI =  1 large UI
    //LPDDR3
    u1Small_ui_to_large =  2;

    #if 0
    *pu1PI = uiDelay% TX_DQ_UI_TO_PI_TAP;
    #else
    *pu1PI = uiDelay & (TX_DQ_UI_TO_PI_TAP-1);
    #endif

    #if ENABLE_64_PI_TO_UI
    u2TmpValue = (uiDelay /TX_DQ_UI_TO_PI_TAP)*2 +u1UISmallBak;
    #else
    u2TmpValue = uiDelay /TX_DQ_UI_TO_PI_TAP +u1UISmallBak;
    #endif

    #if 0
    *pu1UISmall = u2TmpValue % u1Small_ui_to_large;
    *pu1UILarge = u2TmpValue / u1Small_ui_to_large +u1UILargeBak;
    #else
    *pu1UISmall = u2TmpValue - ((u2TmpValue >> u1Small_ui_to_large) <<u1Small_ui_to_large);
    *pu1UILarge = (u2TmpValue >> u1Small_ui_to_large) +u1UILargeBak;
    #endif
}

#define TX_DQM_CALC_MAX_MIN_CENTER 1
#define TX_NEW_CENTER 1

/*
 * cc notes: TxWindow does not calculate per-bit delay for DQ. Instead, it will find the 
 * maximum_first_passed DQ delay value and the minimum_last_passed DQ delay cell, then set 
 * the delay value of all DQ bits to (maximum_first_passed + minimum_last_passed)/2.
 * This method may have side effect that some DQ is not delayed to the best sample position,
 * especially when the delay windows for different DQ bit have very small overlapping.
 *
 * To avoid this, we can calculate the minimum_win_center, then set per_byte delay to this value,
 * and then set per-bit delay as (win_center[dq_i] - minimum_win_center).
 *
 * The macro TX_PERBIT_ADJUST is used to control if such policy can be used.
 * Since the validation of this method cannot be verified in SIM ENV, so current, not used.
 */
#define TX_PERBIT_ADJUST		ENABLE_MIOCK_JMETER //cc notes: PERBIT_ADJUST must enable JMETER CAL.
#define TX_eye_scan 0
DRAM_STATUS_T DramcTxWindowPerbitCal(DRAMC_CTX_T *p, DRAM_TX_PER_BIT_CALIBRATION_TYTE_T calType)
{
    U8 u1BitIdx, u1ByteIdx, u1SmallestDQSByte=0;
    U8 ucindex, ucbit_num;
    U32 uiFinishCount;
    PASS_WIN_DATA_T WinPerBit[DQ_DATA_WIDTH], VrefWinPerBit[DQ_DATA_WIDTH], FinalWinPerBit[DQ_DATA_WIDTH];
	
    U16 uiDelay, uiDelay2, u2DQDelayBegin=0, u2DQDelayEnd=0, u2DelayStep;
    U8 ucdq_pi, ucdq_ui_small, ucdq_ui_large,ucdq_oen_ui_small, ucdq_oen_ui_large;
    static U8 dq_ui_small_bak, dq_ui_large_bak,  dq_oen_ui_small_bak, dq_oen_ui_large_bak;
    U8 ucdq_final_pi[DQS_NUMBER], ucdq_final_ui_large[DQS_NUMBER], ucdq_final_ui_small[DQS_NUMBER];
    U8 ucdq_final_oen_ui_large[DQS_NUMBER], ucdq_final_oen_ui_small[DQS_NUMBER];

	/* cc add for DQM */
	U8 ucdq_final_dqm_pi[DQS_NUMBER], ucdq_final_dqm_ui_large[DQS_NUMBER], ucdq_final_dqm_ui_small[DQS_NUMBER];
    U8 ucdq_final_dqm_oen_ui_large[DQS_NUMBER], ucdq_final_dqm_oen_ui_small[DQS_NUMBER];

    S16 s1temp1, s1temp2;
    U16 u2Center_min[DQS_NUMBER],u2Center_max[DQS_NUMBER];
    #if !TX_DQM_CALC_MAX_MIN_CENTER
    S16 s2sum_dly[DQS_NUMBER];
    #endif
    U32 u4err_value, u4fail_bit;
    U16 u2TempWinSum;
    U32 u4TempRegValue;
    U16 u2MinWinSize = 0xffff;
    U8 u1MinWinSizeBitidx = 0;
    #if TX_NEW_CENTER
    U16 u2First_max[DQS_NUMBER] = {0,0};	//YMC change to avoid warning
    U16 u2Last_min[DQS_NUMBER] = {0xffff,0xffff};	//YMC change to avoid warning
    #endif

	//cc add for PERBIT adjust
	#if TX_PERBIT_ADJUST	
    U8 u1EnableDelayCell=0;
    U8 u1DelayCellOfst[DQ_DATA_WIDTH];
    #endif
	
    U8 ii, backup_rank;
	#if TX_eye_scan
	U16 u2VrefValue,u2VrefValue_opt=0,u2MaxWinSum=0;
	#endif

    if (!p)
    {
        mcSHOW_ERR_MSG(("context is NULL\n"));
        return DRAM_FAIL;
    }

    backup_rank = u1GetRank(p);
	mcSHOW_DBG_MSG(("\n[TX]\n"));
	#if	dbg_print
	prom_puts("\n[TX]\n");
	#endif

#if COMPILE_THIS_PART
    if (p->pinmux == PIN_MUX_TYPE_DDR3X4)
        ucbit_num = 4;
    else
#endif
        ucbit_num = DQS_BIT_NUMBER;
    //A.set RX DQ/DQS in the middle of the pass region from read DQ/DQS calibration
    //B.Fix DQS (RG_PI_**_PBYTE*) at degree from write leveling.
    //   Move DQ (per byte) gradually from 90 to -45 degree to find the left boundary
    //   Move DQ (per byte) gradually from 90 to 225 degree to find the right boundary
    //C.For each DQ delay in step B, start engine test
    //D.After engine test, read per bit results from registers.
    //E.Set RG_PI_**_DQ* to lie in the average of the middle of the pass region in the same byte
    if(fgwrlevel_done)
    {
        // Find smallest DQS delay after write leveling. DQ PI scan from smallest DQS PI.
        u2DQDelayBegin =0xff;
        for(u1ByteIdx=0; u1ByteIdx<(p->data_width/ucbit_num); u1ByteIdx++)
        {
            if(wrlevel_dqs_final_delay[p->rank][u1ByteIdx] < u2DQDelayBegin)
            {
                u2DQDelayBegin = wrlevel_dqs_final_delay[p->rank][u1ByteIdx];
                u1SmallestDQSByte = u1ByteIdx;
            }
        }
    }
    else
    {
        u1SmallestDQSByte =0;
    }

    if(p->fgTXPerbifInit[p->rank]== FALSE)
    {
        // Scan from DQ delay = DQS delay
        // For everest, choose DQS 1 which is smaller and won't over 64.
        u4TempRegValue= u4IO32Read4B(DRAMC_REG_SHU_SELPH_DQS0);
        dq_ui_large_bak = (u4TempRegValue >> (u1SmallestDQSByte*4)) & 0x7;
        dq_oen_ui_large_bak= (u4TempRegValue >> (u1SmallestDQSByte*4+16)) & 0x7;

        u4TempRegValue= u4IO32Read4B(DRAMC_REG_SHU_SELPH_DQS1);
        dq_ui_small_bak = (u4TempRegValue >> (u1SmallestDQSByte*4)) & 0x7; //cc change from &3 --> &7
        //dq_ui_small_bak = (u4TempRegValue >> (u1SmallestDQSByte*4)) & 0x3;
        dq_oen_ui_small_bak= (u4TempRegValue >> (u1SmallestDQSByte*4+16)) & 0x7;; //cc change from &3 --> &7
        //dq_oen_ui_small_bak= (u4TempRegValue >> (u1SmallestDQSByte*4+16)) & 0x3;;

        p->fgTXPerbifInit[p->rank]= TRUE;
    }

    //u2DQDelayBegin = wrlevel_dqs_final_delay[p->rank][u1SmallestDQSByte];		//YMC mark to avoid warning
#if 0 //cc change since Tx will PASS for the first delay value. Using Azalea setting??
    if (u2DQDelayBegin >= 0x40)
        u2DQDelayBegin -= 0x40;
    u2DQDelayEnd = u2DQDelayBegin + 64; //Scan at least 1UI. Scan 64 to cover byte differece. if window found, early break.
#else
	u2DQDelayBegin = 0; //not wrlevel_dqs_final_delay to find left window_YMC
	u2DQDelayEnd = u2DQDelayBegin + 128; //Scan at least 1UI. Scan 64 to cover byte differece. if window found, early break.
#endif

#if FOR_DV_SIMULATION
	u2DelayStep = 2;
#else
	u2DelayStep = 1;
#endif

    vSetCalibrationResult(p, DRAM_CALIBRATION_TX_PERBIT, DRAM_FAIL);

    mcSHOW_DBG_MSG(("[DramcTxWindowPerbitCal] Frequency=%d, Rank=%d, calType=%d\n", p->frequency, p->rank, calType));
    mcSHOW_DBG_MSG(("[DramcTxWindowPerbitCal] Begin, TX DQ(%d, %d),  DQ OEN(%d, %d)\n", dq_ui_large_bak, dq_ui_small_bak,dq_oen_ui_large_bak,  dq_oen_ui_small_bak));
	#if	dbg_print
	prom_puts("Rank = ");
	prom_print_dec(p->rank);
	prom_puts(", calType = ");
	prom_print_dec(calType);
	prom_puts("\nBegin, TX DQ(LargeUI, SmallUI) = ");
	prom_print_dec(dq_ui_large_bak);
	prom_puts(", ");
	prom_print_dec(dq_ui_small_bak);
	prom_puts("\nDQ OEN(LargeUI, SmallUI) = ");
	prom_print_dec(dq_oen_ui_large_bak);
	prom_puts(", ");
	prom_print_dec(dq_oen_ui_small_bak);
	prom_puts("\n");
	#endif

#if TX_eye_scan
for (u2VrefValue = 63; u2VrefValue < 64; u2VrefValue -= 1)
#endif	//TX_eye_scan
{
    // initialize parameters
    uiFinishCount = 0;
    u2TempWinSum =0;

    for (u1BitIdx = 0; u1BitIdx < p->data_width; u1BitIdx++)
    {
        WinPerBit[u1BitIdx].first_pass = (S16)PASS_RANGE_NA;
        WinPerBit[u1BitIdx].last_pass = (S16)PASS_RANGE_NA;
        VrefWinPerBit[u1BitIdx].first_pass = (S16)PASS_RANGE_NA;
        VrefWinPerBit[u1BitIdx].last_pass = (S16)PASS_RANGE_NA;
    }
		#if TX_eye_scan
		#if	dbg_print
		prom_puts("Loop Vref = ");
		prom_print_dec(u2VrefValue);
		prom_puts("\n");
		#endif
		vIO32WriteFldMulti(DDRPHY_MISC_VREF_CTRL, \
		P_Fld((u2VrefValue), MISC_VREF_CTRL_RG_RVREF_SEL_DQ) | \
		P_Fld(0x1, MISC_VREF_CTRL_RG_RVREF_VREF_DQ_EN) | \
		P_Fld(0x1, MISC_VREF_CTRL_RG_RVREF_DDR3_DQ_SEL) | \
		P_Fld(0x0, MISC_VREF_CTRL_RG_RVREF_DDR4_DQ_SEL) );
		#endif
    //Move DQ delay ,  1 PI = tCK/64, total 128 PI, 1UI = 32 PI
    //For data rate 3200, max tDQS2DQ is 2.56UI (82 PI)
    //For data rate 4266, max tDQS2DQ is 3.41UI (109 PI)
    for (uiDelay = u2DQDelayBegin; uiDelay <=u2DQDelayEnd; uiDelay += u2DelayStep)
    //    for (uiDelay = u2DQDelayBegin; uiDelay <=u2DQDelayEnd; uiDelay+=3)  //DBI test
    {
        TxWinTransferDelayToUIPI(p, uiDelay, dq_ui_small_bak, dq_ui_large_bak, &ucdq_ui_large, &ucdq_ui_small, &ucdq_pi);
        TxWinTransferDelayToUIPI(p, uiDelay, dq_oen_ui_small_bak, dq_oen_ui_large_bak, &ucdq_oen_ui_large, &ucdq_oen_ui_small, &ucdq_pi);

        if(calType ==TX_DQ_DQS_MOVE_DQ_ONLY || calType== TX_DQ_DQS_MOVE_DQ_DQM)
        {
            //TXDLY_DQ , TXDLY_OEN_DQ
            vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ0, \
                                P_Fld(ucdq_ui_large, SHURK0_SELPH_DQ0_TXDLY_DQ0) | \
                                P_Fld(ucdq_ui_large, SHURK0_SELPH_DQ0_TXDLY_DQ1) | \
                                P_Fld(ucdq_ui_large, SHURK0_SELPH_DQ0_TXDLY_DQ2) | \
                                P_Fld(ucdq_ui_large, SHURK0_SELPH_DQ0_TXDLY_DQ3) | \
                                P_Fld(ucdq_oen_ui_large, SHURK0_SELPH_DQ0_TXDLY_OEN_DQ0) | \
                                P_Fld(ucdq_oen_ui_large, SHURK0_SELPH_DQ0_TXDLY_OEN_DQ1) | \
                                P_Fld(ucdq_oen_ui_large, SHURK0_SELPH_DQ0_TXDLY_OEN_DQ2) | \
                                P_Fld(ucdq_oen_ui_large, SHURK0_SELPH_DQ0_TXDLY_OEN_DQ3));

            // DLY_DQ[2:0]
            vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ2, \
                                P_Fld(ucdq_ui_small, SHURK0_SELPH_DQ2_DLY_DQ0) | \
                                P_Fld(ucdq_ui_small, SHURK0_SELPH_DQ2_DLY_DQ1) | \
                                P_Fld(ucdq_ui_small, SHURK0_SELPH_DQ2_DLY_DQ2) | \
                                P_Fld(ucdq_ui_small, SHURK0_SELPH_DQ2_DLY_DQ3) | \
                                P_Fld(ucdq_oen_ui_small, SHURK0_SELPH_DQ2_DLY_OEN_DQ0) | \
                                P_Fld(ucdq_oen_ui_small, SHURK0_SELPH_DQ2_DLY_OEN_DQ1) | \
                                P_Fld(ucdq_oen_ui_small, SHURK0_SELPH_DQ2_DLY_OEN_DQ2) | \
                                P_Fld(ucdq_oen_ui_small, SHURK0_SELPH_DQ2_DLY_OEN_DQ3));
        }

        if(calType ==TX_DQ_DQS_MOVE_DQM_ONLY || calType== TX_DQ_DQS_MOVE_DQ_DQM)
        {
            //TXDLY_DQM , TXDLY_OEN_DQM
            vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ1, \
                                P_Fld(ucdq_ui_large, SHURK0_SELPH_DQ1_TXDLY_DQM0) | \
                                P_Fld(ucdq_ui_large, SHURK0_SELPH_DQ1_TXDLY_DQM1) | \
                                P_Fld(ucdq_ui_large, SHURK0_SELPH_DQ1_TXDLY_DQM2) | \
                                P_Fld(ucdq_ui_large, SHURK0_SELPH_DQ1_TXDLY_DQM3) | \
                                P_Fld(ucdq_oen_ui_large, SHURK0_SELPH_DQ1_TXDLY_OEN_DQM0) | \
                                P_Fld(ucdq_oen_ui_large, SHURK0_SELPH_DQ1_TXDLY_OEN_DQM1) | \
                                P_Fld(ucdq_oen_ui_large, SHURK0_SELPH_DQ1_TXDLY_OEN_DQM2) | \
                                P_Fld(ucdq_oen_ui_large, SHURK0_SELPH_DQ1_TXDLY_OEN_DQM3));

            // DLY_DQM[2:0]
            vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ3, \
                                P_Fld(ucdq_ui_small, SHURK0_SELPH_DQ3_DLY_DQM0) | \
                                P_Fld(ucdq_ui_small, SHURK0_SELPH_DQ3_DLY_DQM1) | \
                                P_Fld(ucdq_ui_small, SHURK0_SELPH_DQ3_DLY_DQM2) | \
                                P_Fld(ucdq_ui_small, SHURK0_SELPH_DQ3_DLY_DQM3) | \
                                P_Fld(ucdq_oen_ui_small, SHURK0_SELPH_DQ3_DLY_OEN_DQM0) | \
                                P_Fld(ucdq_oen_ui_small, SHURK0_SELPH_DQ3_DLY_OEN_DQM1) | \
                                P_Fld(ucdq_oen_ui_small, SHURK0_SELPH_DQ3_DLY_OEN_DQM2) | \
                                P_Fld(ucdq_oen_ui_small, SHURK0_SELPH_DQ3_DLY_OEN_DQM3));
        }

        //set to registers, PI DQ (per byte)
        if(calType ==TX_DQ_DQS_MOVE_DQ_ONLY || calType== TX_DQ_DQS_MOVE_DQ_DQM)
        {
            // set to best values for  DQ
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ7, P_Fld(ucdq_pi, SHU1_R0_B0_DQ7_RK0_ARPI_DQ_B0));
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ7, P_Fld(ucdq_pi, SHU1_R0_B1_DQ7_RK0_ARPI_DQ_B1));
            //cc mark vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ7+(1<<POS_BANK_NUM), P_Fld(ucdq_pi, SHU1_R0_B0_DQ7_RK0_ARPI_DQ_B0));
            //cc mark vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ7+(1<<POS_BANK_NUM), P_Fld(ucdq_pi, SHU1_R0_B1_DQ7_RK0_ARPI_DQ_B1));
        }
        if(calType ==TX_DQ_DQS_MOVE_DQM_ONLY || calType== TX_DQ_DQS_MOVE_DQ_DQM)
        {
            // set to best values for  DQM
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ7, P_Fld(ucdq_pi, SHU1_R0_B0_DQ7_RK0_ARPI_DQM_B0));
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ7, P_Fld(ucdq_pi, SHU1_R0_B1_DQ7_RK0_ARPI_DQM_B1));
            //cc mark vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ7+(1<<POS_BANK_NUM), P_Fld(ucdq_pi, SHU1_R0_B0_DQ7_RK0_ARPI_DQM_B0));
            //cc mark vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ7+(1<<POS_BANK_NUM), P_Fld(ucdq_pi, SHU1_R0_B1_DQ7_RK0_ARPI_DQM_B1));
        }

        u4err_value= TestEngineCompare(p);

        //mcSHOW_DBG_MSG(("Delay=%3d |%2d %2d %3d| %2d %2d| 0x%8x [0]",uiDelay, ucdq_ui_large,ucdq_ui_small, ucdq_pi, ucdq_oen_ui_large,ucdq_oen_ui_small, u4err_value));
        //cc mark to always print. if(u4err_value != 0 || uiDelay == u2DQDelayBegin)
        {
            mcSHOW_DBG_MSG2(("%2d |%2d %2d %2d|[0]",uiDelay, ucdq_ui_large,ucdq_ui_small, ucdq_pi));
			#if	dbg_print
			prom_print_dec(uiDelay);
			prom_puts(" | ");	
			prom_print_dec(ucdq_ui_large);
			prom_puts(" ");
			prom_print_dec(ucdq_ui_small);
			prom_puts(" ");
			prom_print_dec(ucdq_pi);
			prom_puts("|[0]");
			#endif
        }

        // check fail bit ,0 ok ,others fail
        for (u1BitIdx = 0; u1BitIdx < p->data_width; u1BitIdx++)
        {
            u4fail_bit = u4err_value&((U32)1<<u1BitIdx);

            //cc mark to always print. if(u4err_value != 0 || uiDelay == u2DQDelayBegin)
            {
            	#if	dbg_print
                if(u1BitIdx%DQS_BIT_NUMBER ==0)
                {
                    mcSHOW_DBG_MSG2((" "));
					prom_puts(" ");
                }

                if (u4fail_bit == 0)
                {
                    mcSHOW_DBG_MSG2(("o"));
					prom_puts("o");
                }
                else
                {
                    mcSHOW_DBG_MSG2(("x"));
					prom_puts("x");
                }
				#endif
            }

            if(WinPerBit[u1BitIdx].first_pass== PASS_RANGE_NA)
            {
                if(u4fail_bit==0) //compare correct: pass
                {
                    WinPerBit[u1BitIdx].first_pass = uiDelay;
                }
            }
            else if(WinPerBit[u1BitIdx].last_pass == PASS_RANGE_NA)
            {
                if(u4fail_bit !=0) //compare error : fail
                {
                    WinPerBit[u1BitIdx].last_pass  = (uiDelay-1);
                }
                else if (uiDelay==u2DQDelayEnd)
                //else if (uiDelay==MAX_TX_DQDLY_TAPS)
                {
                    WinPerBit[u1BitIdx].last_pass  = uiDelay;
                }

                if(WinPerBit[u1BitIdx].last_pass  !=PASS_RANGE_NA)
                {
                    if((WinPerBit[u1BitIdx].last_pass -WinPerBit[u1BitIdx].first_pass) >= (VrefWinPerBit[u1BitIdx].last_pass -VrefWinPerBit[u1BitIdx].first_pass))
                    {
                        #if 0 //for debug
                        if(VrefWinPerBit[u1BitIdx].last_pass != PASS_RANGE_NA)
                        {
                            mcSHOW_DBG_MSG2(("Bit[%d] Bigger window update %d > %d\n", u1BitIdx, \
                                (WinPerBit[u1BitIdx].last_pass -WinPerBit[u1BitIdx].first_pass), (VrefWinPerBit[u1BitIdx].last_pass -VrefWinPerBit[u1BitIdx].first_pass)));

                        }
                        #endif
                        uiFinishCount |= (1<<u1BitIdx);
                        //update bigger window size
                        VrefWinPerBit[u1BitIdx].first_pass = WinPerBit[u1BitIdx].first_pass;
                        VrefWinPerBit[u1BitIdx].last_pass = WinPerBit[u1BitIdx].last_pass;
                    }

                    //reset tmp window
                    WinPerBit[u1BitIdx].first_pass = PASS_RANGE_NA;
                    WinPerBit[u1BitIdx].last_pass = PASS_RANGE_NA;
                }
            }
        }

        //cc mark to always print. if(u4err_value != 0 || uiDelay == u2DQDelayBegin)
        {
            mcSHOW_DBG_MSG2((" [MSB]\n"));
			#if	dbg_print
			prom_puts(" [MSB]\n");
			#endif
        }

        //if all bits widnow found and all bits turns to fail again, early break;
        if(((p->data_width== DATA_WIDTH_16BIT) &&(uiFinishCount == 0xffff)) || \
        ((p->data_width== DATA_WIDTH_32BIT) &&(uiFinishCount == 0xffffffff)))
        {
            vSetCalibrationResult(p, DRAM_CALIBRATION_TX_PERBIT, DRAM_OK);

            if(((p->data_width== DATA_WIDTH_16BIT) &&((u4err_value&0xffff) == 0xffff)) || \
			((p->data_width== DATA_WIDTH_32BIT) &&(u4err_value == 0xffffffff)))
            {
                    mcSHOW_DBG_MSG2(("TX calibration finding left boundary early break. PI DQ delay=%d\n", uiDelay));
					#if	dbg_print
					prom_puts("TX calibration finding left boundary early break. PI DQ delay = ");
					prom_print_dec(uiDelay);
					prom_puts("\n");
					#endif
                break;  //early break
            }
        }
    }

    #if 0
    if (p->en_4bitMux == ENABLE)
    {
        mcSHOW_DBG_MSG(("Tx 4bitMux is enabled\n"));

		#if 1
		for(u1BitIdx = 0; u1BitIdx < p->data_width; u1BitIdx++)
		{
			mcSHOW_DBG_MSG(("%d:%d ", VrefWinPerBit[u1BitIdx].first_pass, VrefWinPerBit[u1BitIdx].last_pass));
		}
		mcSHOW_DBG_MSG(("\n"));
		#endif

		for(u1BitIdx = 0; u1BitIdx < p->data_width; u1BitIdx++)
		{
			memcpy(&WinPerBit[Bit_DQ_Mapping[u1BitIdx]], &VrefWinPerBit[u1BitIdx], sizeof(PASS_WIN_DATA_T));
		}
		memcpy(VrefWinPerBit, WinPerBit, sizeof(PASS_WIN_DATA_T) * DQ_DATA_WIDTH);

		#if 1
		for(u1BitIdx = 0; u1BitIdx < p->data_width; u1BitIdx++)
		{
			mcSHOW_DBG_MSG(("%d:%d ", VrefWinPerBit[u1BitIdx].first_pass, VrefWinPerBit[u1BitIdx].last_pass));
			mcSHOW_DBG_MSG(("\n"));
		}
		#endif

    }
    #endif
    for (u1BitIdx = 0; u1BitIdx < p->data_width; u1BitIdx++)
    {
        VrefWinPerBit[u1BitIdx].win_size = VrefWinPerBit[u1BitIdx].last_pass- VrefWinPerBit[u1BitIdx].first_pass;

        if(VrefWinPerBit[u1BitIdx].win_size<u2MinWinSize)
        {
            u2MinWinSize = VrefWinPerBit[u1BitIdx].win_size;
            u1MinWinSizeBitidx = u1BitIdx;
        }

        u2TempWinSum += VrefWinPerBit[u1BitIdx].win_size;  //Sum of DQ Windows for vref selection
    }
	#if	TX_eye_scan
	#if	dbg_print
	prom_puts("Vref = ");
	prom_print_dec(u2VrefValue);
	prom_puts(", WinSum = ");
	prom_print_dec(u2TempWinSum);
	prom_puts("\n");
	#endif
	if (u2TempWinSum > u2MaxWinSum) {			
			prom_puts("Better Tx Vref found.\n");
			prom_puts("Vref = ");
			prom_print_dec(u2VrefValue);
			prom_puts("\n");
			u2VrefValue_opt = u2VrefValue;	
			u2MaxWinSum = u2TempWinSum;
	}
	#if 0
	while(1)
	{
		delay_a_while(3000000);
		if((ADDR_READ_REG(0x1fbf0204)&0x1)==0)
		{
			break;
		}
	}
	#endif
	#else
    mcSHOW_DBG_MSG2(("Fra Min Bit=%d, winsize=%d ===\n",u1MinWinSizeBitidx, u2MinWinSize));
	#if	dbg_print
	prom_puts("Min Bit = ");
	prom_print_dec(u1MinWinSizeBitidx);
	prom_puts(", winsize = ");
	prom_print_dec(u2MinWinSize);
	prom_puts("\n");
	#endif

    #ifdef ENABLE_CALIBRATION_WINDOW_LOG_FOR_FT
    mcSHOW_DBG_MSG(("FT log: TX min window : bit %d, size %d\n", u1MinWinSizeBitidx, u2MinWinSize));
    #endif

    mcSHOW_DBG_MSG(("TX Window Sum %d\n", u2TempWinSum));
	#if	dbg_print
	prom_puts("TX Window Sum = ");
	prom_print_dec(u2TempWinSum);
	prom_puts("\n");
	#endif
	#endif	//TX_eye_scan
}
	#if TX_eye_scan
	vIO32WriteFldAlign(DDRPHY_MISC_VREF_CTRL, (u2VrefValue_opt), MISC_VREF_CTRL_RG_RVREF_SEL_DQ);
	#endif
    //Calculate the center of DQ pass window
    // Record center sum of each byte
    for (u1ByteIdx=0; u1ByteIdx<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); u1ByteIdx++)
    {
        u2Center_min[u1ByteIdx] = 0xffff;
        u2Center_max[u1ByteIdx] = 0;
        #if !TX_DQM_CALC_MAX_MIN_CENTER
        s2sum_dly[u1ByteIdx] = 0;
        #endif

        #if TX_NEW_CENTER
        u2Last_min[u1ByteIdx] = 0xffff;
        u2First_max[u1ByteIdx] = 0;
        #endif

        for (u1BitIdx=0; u1BitIdx<DQS_BIT_NUMBER; u1BitIdx++)
        {
            ucindex = u1ByteIdx * DQS_BIT_NUMBER + u1BitIdx;
            FinalWinPerBit[ucindex].first_pass = VrefWinPerBit[ucindex].first_pass;
            FinalWinPerBit[ucindex].last_pass =  VrefWinPerBit[ucindex].last_pass;
            FinalWinPerBit[ucindex].win_size = VrefWinPerBit[ucindex].win_size;
            FinalWinPerBit[ucindex].win_center = (FinalWinPerBit[ucindex].first_pass + FinalWinPerBit[ucindex].last_pass) >> 1;
           
            if(FinalWinPerBit[ucindex].win_center < u2Center_min[u1ByteIdx])
                u2Center_min[u1ByteIdx] = FinalWinPerBit[ucindex].win_center;

            if(FinalWinPerBit[ucindex].win_center > u2Center_max[u1ByteIdx])
                u2Center_max[u1ByteIdx] = FinalWinPerBit[ucindex].win_center;
            #if !TX_DQM_CALC_MAX_MIN_CENTER
            s2sum_dly[u1ByteIdx] += FinalWinPerBit[ucindex].win_center;
            #endif

            #if TX_NEW_CENTER
            if (FinalWinPerBit[ucindex].first_pass > u2First_max[u1ByteIdx])
                u2First_max[u1ByteIdx] = FinalWinPerBit[ucindex].first_pass;

            if (FinalWinPerBit[ucindex].last_pass < u2Last_min[u1ByteIdx])
                u2Last_min[u1ByteIdx] = FinalWinPerBit[ucindex].last_pass;
            #endif
        }
    }

    //mcSHOW_DBG_MSG(("==================================================================\n"));
    //mcSHOW_DBG_MSG(("PI DQ (per byte) window\nx=pass dq delay value (min~max)center \ny=0-7bit DQ of every group\n"));
    //mcSHOW_DBG_MSG(("==================================================================\n"));
    //mcSHOW_DBG_MSG(("bit    Byte0    bit      Byte1     bit     Byte2     bit     Byte3\n"));
    vPrintCalibrationBasicInfo(p);

    for (u1BitIdx = 0; u1BitIdx < DQS_BIT_NUMBER; u1BitIdx++)
    {

        mcSHOW_DBG_MSG(("R%d FINAL: TX Bit%d (%d~%d) %d %d,   Bit%d (%d~%d) %d %d,", p->rank, \
            u1BitIdx, FinalWinPerBit[u1BitIdx].first_pass, FinalWinPerBit[u1BitIdx].last_pass, FinalWinPerBit[u1BitIdx].win_size, FinalWinPerBit[u1BitIdx].win_center, \
            u1BitIdx+8, FinalWinPerBit[u1BitIdx+8].first_pass, FinalWinPerBit[u1BitIdx+8].last_pass, FinalWinPerBit[u1BitIdx+8].win_size, FinalWinPerBit[u1BitIdx+8].win_center));
		#if	dbg_print
		int dbg_i;
		prom_puts("Rank:");
		prom_print_dec(p->rank);
		prom_puts(" FINAL: ");
		for(dbg_i=0; dbg_i<2; dbg_i++)
		{
			prom_puts("TX Bit:");
			prom_print_dec(u1BitIdx+8*dbg_i);
			prom_puts(", Window center:");
			prom_print_dec(FinalWinPerBit[u1BitIdx+8*dbg_i].win_center);
			prom_puts(", Window first:");
			prom_print_dec(FinalWinPerBit[u1BitIdx+8*dbg_i].first_pass);
			prom_puts(", Window last:");
			prom_print_dec(FinalWinPerBit[u1BitIdx+8*dbg_i].last_pass);
			prom_puts(", Window size:");
			prom_print_dec(FinalWinPerBit[u1BitIdx+8*dbg_i].win_size);
			prom_puts(" | ");
		}
		prom_puts("\n");
		#endif
        if(p->data_width == DATA_WIDTH_32BIT)
        {

            mcSHOW_DBG_MSG(("  Bit%d (%d~%d) %d %d,   Bit%d (%d~%d) %d %d\n", \
                u1BitIdx+16, FinalWinPerBit[u1BitIdx+16].first_pass, FinalWinPerBit[u1BitIdx+16].last_pass, FinalWinPerBit[u1BitIdx+16].win_size, FinalWinPerBit[u1BitIdx+16].win_center, \
                u1BitIdx+24, FinalWinPerBit[u1BitIdx+24].first_pass, FinalWinPerBit[u1BitIdx+24].last_pass, FinalWinPerBit[u1BitIdx+24].win_size, FinalWinPerBit[u1BitIdx+24].win_center ));
        }
        else
        {
            mcSHOW_DBG_MSG(("\n"));
        }
    }
    mcSHOW_DBG_MSG(("\n==================================================================\n"));

    //Calculate the center of DQ pass window
    //average the center delay
    for (u1ByteIdx=0; u1ByteIdx<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); u1ByteIdx++)
    {
        #if TX_DQM_CALC_MAX_MIN_CENTER
        uiDelay = ((u2Center_min[u1ByteIdx] + u2Center_max[u1ByteIdx])>>1); //(max +min)/2
        #else
        s1temp1 = s2sum_dly[u1ByteIdx] /DQS_BIT_NUMBER;
        s1temp2 = s1temp1+1;

        if ((s2sum_dly[u1ByteIdx] -s1temp1*DQS_BIT_NUMBER) > (s1temp2*DQS_BIT_NUMBER-s2sum_dly[u1ByteIdx] ))
        {
            uiDelay = (U16)s1temp2;
        }
        else
        {
            uiDelay = (U16)s1temp1;
        }
        #endif

        #if TX_NEW_CENTER
        uiDelay2 = (u2Last_min[u1ByteIdx] + u2First_max[u1ByteIdx]) >> 1;
        #else
        uiDelay2 = uiDelay;
        #endif
		
	#if TX_PERBIT_ADJUST
		//DQ
		TxWinTransferDelayToUIPI(p, u2Center_min[u1ByteIdx], dq_ui_small_bak, dq_ui_large_bak, &ucdq_final_ui_large[u1ByteIdx], &ucdq_final_ui_small[u1ByteIdx], &ucdq_final_pi[u1ByteIdx]);
		TxWinTransferDelayToUIPI(p, u2Center_min[u1ByteIdx], dq_oen_ui_small_bak, dq_oen_ui_large_bak, &ucdq_final_oen_ui_large[u1ByteIdx], &ucdq_final_oen_ui_small[u1ByteIdx], &ucdq_final_pi[u1ByteIdx]);

		//cc add for DQM
		TxWinTransferDelayToUIPI(p, uiDelay, dq_ui_small_bak, dq_ui_large_bak, &ucdq_final_dqm_ui_large[u1ByteIdx], &ucdq_final_dqm_ui_small[u1ByteIdx], &ucdq_final_dqm_pi[u1ByteIdx]);
		TxWinTransferDelayToUIPI(p, uiDelay, dq_oen_ui_small_bak, dq_oen_ui_large_bak, &ucdq_final_dqm_oen_ui_large[u1ByteIdx], &ucdq_final_dqm_oen_ui_small[u1ByteIdx], &ucdq_final_dqm_pi[u1ByteIdx]);

		mcSHOW_DBG_MSG(("Byte%d, PI DQ Delay %d Per-byte delay %d\n", u1ByteIdx, uiDelay, u2Center_min[u1ByteIdx]));
        mcSHOW_DBG_MSG(("Final DQ PI Delay(LargeUI, SmallUI, PI) =(%d ,%d, %d)\n", ucdq_final_ui_large[u1ByteIdx], ucdq_final_ui_small[u1ByteIdx], ucdq_final_pi[u1ByteIdx]));
        mcSHOW_DBG_MSG(("OEN DQ PI Delay(LargeUI, SmallUI, PI) =(%d ,%d, %d)\n\n", ucdq_final_oen_ui_large[u1ByteIdx], ucdq_final_oen_ui_small[u1ByteIdx], ucdq_final_pi[u1ByteIdx]));
		#if	dbg_print
		prom_puts("Byte:");
		prom_print_dec(u1ByteIdx);
		prom_puts(", PI DQ Delay:");
		prom_print_dec(uiDelay);
		prom_puts(", Per-byte delay:");
		prom_print_dec(u2Center_min[u1ByteIdx]);
		prom_puts("\nFinal DQ PI Delay(LargeUI, SmallUI, PI) = ");
		prom_print_dec(ucdq_final_ui_large[u1ByteIdx]);
		prom_puts(" ");
		prom_print_dec(ucdq_final_ui_small[u1ByteIdx]);
		prom_puts(" ");
		prom_print_dec(ucdq_final_pi[u1ByteIdx]);
		prom_puts("\nOEN DQ PI Delay(LargeUI, SmallUI, PI) = ");
		prom_print_dec(ucdq_final_oen_ui_large[u1ByteIdx]);
		prom_puts(" ");
		prom_print_dec(ucdq_final_oen_ui_small[u1ByteIdx]);
		prom_puts(" ");
		prom_print_dec(ucdq_final_pi[u1ByteIdx]);
		prom_puts("\n");
		#endif
	#else
        TxWinTransferDelayToUIPI(p, uiDelay2, dq_ui_small_bak, dq_ui_large_bak, &ucdq_final_ui_large[u1ByteIdx], &ucdq_final_ui_small[u1ByteIdx], &ucdq_final_pi[u1ByteIdx]);
        TxWinTransferDelayToUIPI(p, uiDelay2, dq_oen_ui_small_bak, dq_oen_ui_large_bak, &ucdq_final_oen_ui_large[u1ByteIdx], &ucdq_final_oen_ui_small[u1ByteIdx], &ucdq_final_pi[u1ByteIdx]);

		/* cc add. For previous code, the DQM delay is the same as DQ */
		TxWinTransferDelayToUIPI(p, uiDelay2, dq_ui_small_bak, dq_ui_large_bak, &ucdq_final_dqm_ui_large[u1ByteIdx], &ucdq_final_dqm_ui_small[u1ByteIdx], &ucdq_final_dqm_pi[u1ByteIdx]);
		TxWinTransferDelayToUIPI(p, uiDelay2, dq_oen_ui_small_bak, dq_oen_ui_large_bak, &ucdq_final_dqm_oen_ui_large[u1ByteIdx], &ucdq_final_dqm_oen_ui_small[u1ByteIdx], &ucdq_final_dqm_pi[u1ByteIdx]);

		mcSHOW_DBG_MSG(("Byte%d, PI DQ Delay %d Delay2 %d\n", u1ByteIdx, uiDelay, uiDelay2));
        mcSHOW_DBG_MSG(("Final DQ PI Delay(LargeUI, SmallUI, PI) =(%d ,%d, %d)\n", ucdq_final_ui_large[u1ByteIdx], ucdq_final_ui_small[u1ByteIdx], ucdq_final_pi[u1ByteIdx]));
        mcSHOW_DBG_MSG(("OEN DQ PI Delay(LargeUI, SmallUI, PI) =(%d ,%d, %d)\n\n", ucdq_final_oen_ui_large[u1ByteIdx], ucdq_final_oen_ui_small[u1ByteIdx], ucdq_final_pi[u1ByteIdx]));
		#if	dbg_print
		prom_puts("Byte:");
		prom_print_dec(u1ByteIdx);
		prom_puts(", PI DQ Delay:");
		prom_print_dec(uiDelay);
		prom_puts(", Delay2:");
		prom_print_dec(uiDelay2);
		prom_puts("\nFinal DQ PI Delay(LargeUI, SmallUI, PI) = ");
		prom_print_dec(ucdq_final_ui_large[u1ByteIdx]);
		prom_puts(", ");
		prom_print_dec(ucdq_final_ui_small[u1ByteIdx]);
		prom_puts(", ");
		prom_print_dec(ucdq_final_pi[u1ByteIdx]);
		prom_puts("\nOEN DQ PI Delay(LargeUI, SmallUI, PI) = ");
		prom_print_dec(ucdq_final_oen_ui_large[u1ByteIdx]);
		prom_puts(", ");
		prom_print_dec(ucdq_final_oen_ui_small[u1ByteIdx]);
		prom_puts(", ");
		prom_print_dec(ucdq_final_pi[u1ByteIdx]);
		prom_puts("\n");
		#endif
	#endif
	

#if defined(DRAM_CALIB_LOG) || defined(DRAM_ETT)
        for (u1BitIdx = 0; u1BitIdx < DQS_BIT_NUMBER; u1BitIdx++)
        {
            gDRAM_CALIB_LOG.RANK[p->rank].TxWindowPerbitCal.WinPerBit[u1ByteIdx *DQS_BIT_NUMBER +u1BitIdx].first_pass = FinalWinPerBit[u1ByteIdx *DQS_BIT_NUMBER +u1BitIdx].first_pass;
            gDRAM_CALIB_LOG.RANK[p->rank].TxWindowPerbitCal.WinPerBit[u1ByteIdx *DQS_BIT_NUMBER +u1BitIdx].last_pass = FinalWinPerBit[u1ByteIdx *DQS_BIT_NUMBER +u1BitIdx].last_pass;
            gDRAM_CALIB_LOG.RANK[p->rank].TxWindowPerbitCal.WinPerBit[u1ByteIdx *DQS_BIT_NUMBER +u1BitIdx].win_size = FinalWinPerBit[u1ByteIdx *DQS_BIT_NUMBER +u1BitIdx].win_size;
            gDRAM_CALIB_LOG.RANK[p->rank].TxWindowPerbitCal.WinPerBit[u1ByteIdx *DQS_BIT_NUMBER +u1BitIdx].win_center = FinalWinPerBit[u1ByteIdx *DQS_BIT_NUMBER +u1BitIdx].win_center;
            gDRAM_CALIB_LOG.RANK[p->rank].TxWindowPerbitCal.WinPerBit[u1ByteIdx *DQS_BIT_NUMBER +u1BitIdx].left_margin = uiDelay2 - FinalWinPerBit[u1ByteIdx *DQS_BIT_NUMBER +u1BitIdx].first_pass;  // left margin
            gDRAM_CALIB_LOG.RANK[p->rank].TxWindowPerbitCal.WinPerBit[u1ByteIdx *DQS_BIT_NUMBER +u1BitIdx].right_margin = FinalWinPerBit[u1ByteIdx *DQS_BIT_NUMBER +u1BitIdx].last_pass - uiDelay2;  // right margin
        }
        gDRAM_CALIB_LOG.RANK[p->rank].TxWindowPerbitCal.PI_DQ_delay[u1ByteIdx] = uiDelay2;
        gDRAM_CALIB_LOG.RANK[p->rank].TxWindowPerbitCal.Large_UI[u1ByteIdx] = ucdq_final_ui_large[u1ByteIdx];
        gDRAM_CALIB_LOG.RANK[p->rank].TxWindowPerbitCal.Small_UI[u1ByteIdx] = ucdq_final_ui_small[u1ByteIdx];
        gDRAM_CALIB_LOG.RANK[p->rank].TxWindowPerbitCal.PI[u1ByteIdx] = ucdq_final_pi[u1ByteIdx];
#endif
    }

#if REG_ACCESS_PORTING_DGB
    RegLogEnable =1;
#endif

#if COMPILE_THIS_PART
    if (p->pinmux == PIN_MUX_TYPE_DDR3X4)
    {   
        //swap B12 & B13
        mcSHOW_DBG_MSG(("DDR3X4 TX SWAP\n"));
        ucdq_final_ui_large[2] = ucdq_final_ui_large[1];
        ucdq_final_ui_large[3] = ucdq_final_ui_large[1];
        ucdq_final_oen_ui_large[2] = ucdq_final_oen_ui_large[1];
        ucdq_final_oen_ui_large[3] = ucdq_final_oen_ui_large[1];
        ucdq_final_ui_small[2] = ucdq_final_ui_small[1];
        ucdq_final_ui_small[3] = ucdq_final_ui_small[1];
        ucdq_final_oen_ui_small[2] = ucdq_final_oen_ui_small[1];
        ucdq_final_oen_ui_small[3] = ucdq_final_oen_ui_small[1];
        ucdq_final_pi[2] = ucdq_final_pi[1];
        ucdq_final_pi[3] = ucdq_final_pi[1];

        for (u1ByteIdx=0; u1ByteIdx<(p->data_width/ucbit_num); u1ByteIdx++)
        {
            mcSHOW_DBG_MSG(("DDR3X4 Final DQM(DQ) PI Delay(LargeUI, SmallUI, PI) =(%d ,%d, %d)\n", ucdq_final_ui_large[u1ByteIdx], ucdq_final_ui_small[u1ByteIdx], ucdq_final_pi[u1ByteIdx]));
            mcSHOW_DBG_MSG(("DDR3X4 OEN DQM(DQ) PI Delay(LargeUI, SmallUI, PI) =(%d ,%d, %d)\n\n", ucdq_final_oen_ui_large[u1ByteIdx], ucdq_final_oen_ui_small[u1ByteIdx], ucdq_final_pi[u1ByteIdx]));        
        }
    }
#endif

    /* p->rank = RANK_0, save to Reg Rank0 and Rank1, p->rank = RANK_1, save to Reg Rank1 */
    for(ii=p->rank; ii<RANK_MAX; ii++)
    {
        vSetRank(p,ii);

	#if TX_PERBIT_ADJUST
		for (u1ByteIdx=0; u1ByteIdx<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); u1ByteIdx++)
		{		 
			for (u1BitIdx=0; u1BitIdx<DQS_BIT_NUMBER; u1BitIdx++)
			{
				ucindex = u1ByteIdx * DQS_BIT_NUMBER + u1BitIdx;
				WinPerBit[ucindex].best_dqdly = FinalWinPerBit[ucindex].win_center - u2Center_min[u1ByteIdx];
			#if ENABLE_MIOCK_JMETER
				if (u2gdelay_cell_ps != 0) {
					 u1DelayCellOfst[ucindex] = (WinPerBit[ucindex].best_dqdly*100000000/((p->frequency/2)*64))/u2gdelay_cell_ps;
				} else
			#endif
				{
					u1DelayCellOfst[ucindex] = 0;
				}
				mcSHOW_DBG_MSG(("RK %d, byte %d, bit %d - dly: %d\n", ii, u1ByteIdx, ucindex, WinPerBit[ucindex].best_dqdly));
				#if	dbg_print
				prom_puts("Rank:");
				prom_print_dec(ii);
				prom_puts(", byte:");
				prom_print_dec(u1ByteIdx);
				prom_puts(", bit:");
				prom_print_dec(ucindex);
				prom_puts(", dly:");
				prom_print_dec(WinPerBit[ucindex].best_dqdly);
				prom_puts("\n");
				#endif
				//cc mark. what for ?? WinPerBit[ucindex].best_dqdly = (WinPerBit[ucindex].best_dqdly*p->density + 200)/1000;				 
				//cc mark mcSHOW_DBG_MSG(("(%d) ", WinPerBit[ucindex].best_dqdly));
			}

			mcSHOW_DBG_MSG(("\n"));
		}
	#endif

        if(calType ==TX_DQ_DQS_MOVE_DQ_ONLY || calType== TX_DQ_DQS_MOVE_DQ_DQM)
        {			
            //TXDLY_DQ , TXDLY_OEN_DQ
            vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ0, \
                                    P_Fld(ucdq_final_ui_large[0], SHURK0_SELPH_DQ0_TXDLY_DQ0) | \
                                    P_Fld(ucdq_final_ui_large[1], SHURK0_SELPH_DQ0_TXDLY_DQ1) | \
                                    //P_Fld(ucdq_final_ui_large[2], SHURK0_SELPH_DQ0_TXDLY_DQ2) | \	//YMC mark to avoid warning
                                    //P_Fld(ucdq_final_ui_large[3], SHURK0_SELPH_DQ0_TXDLY_DQ3) | \	//YMC mark to avoid warning
                                    P_Fld(ucdq_final_oen_ui_large[0], SHURK0_SELPH_DQ0_TXDLY_OEN_DQ0) | \
                                    P_Fld(ucdq_final_oen_ui_large[1], SHURK0_SELPH_DQ0_TXDLY_OEN_DQ1) //| \
                                    //P_Fld(ucdq_final_oen_ui_large[2], SHURK0_SELPH_DQ0_TXDLY_OEN_DQ2) | \	//YMC mark to avoid warning
                                    //P_Fld(ucdq_final_oen_ui_large[3], SHURK0_SELPH_DQ0_TXDLY_OEN_DQ3)		//YMC mark to avoid warning
                                    );

             // DLY_DQ[2:0]
             vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ2, \
                                    P_Fld(ucdq_final_ui_small[0], SHURK0_SELPH_DQ2_DLY_DQ0) | \
                                    P_Fld(ucdq_final_ui_small[1], SHURK0_SELPH_DQ2_DLY_DQ1) | \
                                    //P_Fld(ucdq_final_ui_small[2], SHURK0_SELPH_DQ2_DLY_DQ2) | \		//YMC mark to avoid warning
                                    //P_Fld(ucdq_final_ui_small[3], SHURK0_SELPH_DQ2_DLY_DQ3) | \		//YMC mark to avoid warning
                                    P_Fld(ucdq_final_oen_ui_small[0], SHURK0_SELPH_DQ2_DLY_OEN_DQ0) | \
                                    P_Fld(ucdq_final_oen_ui_small[1], SHURK0_SELPH_DQ2_DLY_OEN_DQ1) //| \
                                    //P_Fld(ucdq_final_oen_ui_small[2], SHURK0_SELPH_DQ2_DLY_OEN_DQ2) | \	//YMC mark to avoid warning
                                    //P_Fld(ucdq_final_oen_ui_small[3], SHURK0_SELPH_DQ2_DLY_OEN_DQ3)		//YMC mark to avoid warning
                                    );
             
             // set to best values for  DQ
             vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ7, P_Fld(ucdq_final_pi[0], SHU1_R0_B0_DQ7_RK0_ARPI_DQ_B0));
             vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ7, P_Fld(ucdq_final_pi[1], SHU1_R0_B1_DQ7_RK0_ARPI_DQ_B1));
             //cc mark vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ7+(1<<POS_BANK_NUM), P_Fld(ucdq_final_pi[2], SHU1_R0_B0_DQ7_RK0_ARPI_DQ_B0));
             //cc mark vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ7+(1<<POS_BANK_NUM), P_Fld(ucdq_final_pi[3], SHU1_R0_B1_DQ7_RK0_ARPI_DQ_B1));
		#if TX_PERBIT_ADJUST
			vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ0, \
                                   P_Fld(u1DelayCellOfst[0], SHU1_R0_B0_DQ0_RK0_TX_ARDQ0_DLY_B0) | \
                                   P_Fld(u1DelayCellOfst[1], SHU1_R0_B0_DQ0_RK0_TX_ARDQ1_DLY_B0) | \
                                   P_Fld(u1DelayCellOfst[2], SHU1_R0_B0_DQ0_RK0_TX_ARDQ2_DLY_B0) | \
                                   P_Fld(u1DelayCellOfst[3], SHU1_R0_B0_DQ0_RK0_TX_ARDQ3_DLY_B0) | \
                                   P_Fld(u1DelayCellOfst[4], SHU1_R0_B0_DQ0_RK0_TX_ARDQ4_DLY_B0) | \
                                   P_Fld(u1DelayCellOfst[5], SHU1_R0_B0_DQ0_RK0_TX_ARDQ5_DLY_B0) | \
                                   P_Fld(u1DelayCellOfst[6], SHU1_R0_B0_DQ0_RK0_TX_ARDQ6_DLY_B0) | \
                                   P_Fld(u1DelayCellOfst[7], SHU1_R0_B0_DQ0_RK0_TX_ARDQ7_DLY_B0));
             vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ0, \
                                   P_Fld(u1DelayCellOfst[8], SHU1_R0_B1_DQ0_RK0_TX_ARDQ0_DLY_B1) | \
                                   P_Fld(u1DelayCellOfst[9], SHU1_R0_B1_DQ0_RK0_TX_ARDQ1_DLY_B1) | \
                                   P_Fld(u1DelayCellOfst[10], SHU1_R0_B1_DQ0_RK0_TX_ARDQ2_DLY_B1) | \
                                   P_Fld(u1DelayCellOfst[11], SHU1_R0_B1_DQ0_RK0_TX_ARDQ3_DLY_B1) | \
                                   P_Fld(u1DelayCellOfst[12], SHU1_R0_B1_DQ0_RK0_TX_ARDQ4_DLY_B1) | \
                                   P_Fld(u1DelayCellOfst[13], SHU1_R0_B1_DQ0_RK0_TX_ARDQ5_DLY_B1) | \
                                   P_Fld(u1DelayCellOfst[14], SHU1_R0_B1_DQ0_RK0_TX_ARDQ6_DLY_B1) | \
                                   P_Fld(u1DelayCellOfst[15], SHU1_R0_B1_DQ0_RK0_TX_ARDQ7_DLY_B1));
		#endif
		}

        if(calType ==TX_DQ_DQS_MOVE_DQM_ONLY || calType== TX_DQ_DQS_MOVE_DQ_DQM)
        {
            //TXDLY_DQM , TXDLY_OEN_DQM
            vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ1, \
                                     P_Fld(ucdq_final_dqm_ui_large[0], SHURK0_SELPH_DQ1_TXDLY_DQM0) | \
                                     P_Fld(ucdq_final_dqm_ui_large[1], SHURK0_SELPH_DQ1_TXDLY_DQM1) | \
                                     P_Fld(ucdq_final_dqm_ui_large[2], SHURK0_SELPH_DQ1_TXDLY_DQM2) | \
                                     P_Fld(ucdq_final_dqm_ui_large[3], SHURK0_SELPH_DQ1_TXDLY_DQM3) | \
                                     P_Fld(ucdq_final_dqm_oen_ui_large[0], SHURK0_SELPH_DQ1_TXDLY_OEN_DQM0) | \
                                     P_Fld(ucdq_final_dqm_oen_ui_large[1], SHURK0_SELPH_DQ1_TXDLY_OEN_DQM1) | \
                                     P_Fld(ucdq_final_dqm_oen_ui_large[2], SHURK0_SELPH_DQ1_TXDLY_OEN_DQM2) | \
                                     P_Fld(ucdq_final_dqm_oen_ui_large[3], SHURK0_SELPH_DQ1_TXDLY_OEN_DQM3));

            // DLY_DQM[2:0]
            vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ3, \
                                     P_Fld(ucdq_final_dqm_ui_small[0], SHURK0_SELPH_DQ3_DLY_DQM0) | \
                                     P_Fld(ucdq_final_dqm_ui_small[1], SHURK0_SELPH_DQ3_DLY_DQM1) | \
                                     P_Fld(ucdq_final_dqm_ui_small[2], SHURK0_SELPH_DQ3_DLY_DQM2) | \
                                     P_Fld(ucdq_final_dqm_ui_small[3], SHURK0_SELPH_DQ3_DLY_DQM3) | \
                                     P_Fld(ucdq_final_dqm_oen_ui_small[0], SHURK0_SELPH_DQ3_DLY_OEN_DQM0) | \
                                     P_Fld(ucdq_final_dqm_oen_ui_small[1], SHURK0_SELPH_DQ3_DLY_OEN_DQM1) | \
                                     P_Fld(ucdq_final_dqm_oen_ui_small[2], SHURK0_SELPH_DQ3_DLY_OEN_DQM2) | \
                                     P_Fld(ucdq_final_dqm_oen_ui_small[3], SHURK0_SELPH_DQ3_DLY_OEN_DQM3));

            // set to best values for  DQM
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ7, P_Fld(ucdq_final_dqm_pi[0], SHU1_R0_B0_DQ7_RK0_ARPI_DQM_B0));
            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ7, P_Fld(ucdq_final_dqm_pi[1], SHU1_R0_B1_DQ7_RK0_ARPI_DQM_B1));
            //cc mark vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ7+(1<<POS_BANK_NUM), P_Fld(ucdq_final_pi[2], SHU1_R0_B0_DQ7_RK0_ARPI_DQM_B0));
            //cc mark vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ7+(1<<POS_BANK_NUM), P_Fld(ucdq_final_pi[3], SHU1_R0_B1_DQ7_RK0_ARPI_DQM_B1));
        }
    }

    vSetRank(p, backup_rank);

#if COMPILE_THIS_PART
	//cc notes: ??? what for?? Do almost the same thing as above code...
    if ((p->pinmux == PIN_MUX_TYPE_DDR3X4) && (p->density) && (calType ==TX_DQ_DQS_MOVE_DQ_ONLY || calType== TX_DQ_DQS_MOVE_DQ_DQM))//perbit dly line enable
    {
        for (u1ByteIdx=0; u1ByteIdx<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); u1ByteIdx++)
        {
            TxWinTransferDelayToUIPI(p, u2Center_min[u1ByteIdx], dq_ui_small_bak, dq_ui_large_bak, &ucdq_final_ui_large[u1ByteIdx], &ucdq_final_ui_small[u1ByteIdx], &ucdq_final_pi[u1ByteIdx]);
            TxWinTransferDelayToUIPI(p, u2Center_min[u1ByteIdx], dq_oen_ui_small_bak, dq_oen_ui_large_bak, &ucdq_final_oen_ui_large[u1ByteIdx], &ucdq_final_oen_ui_small[u1ByteIdx], &ucdq_final_pi[u1ByteIdx]);
            mcSHOW_DBG_MSG(("DDR3X4 Byte%d, PI DQ Delay %d \n", u1ByteIdx, u2Center_min[u1ByteIdx]));
        }

        //swap B12 & B13
        mcSHOW_DBG_MSG(("DDR3X4 TX Per Bit SWAP\n"));
        ucdq_final_ui_large[2] = ucdq_final_ui_large[1];
        ucdq_final_ui_large[3] = ucdq_final_ui_large[1];
        ucdq_final_oen_ui_large[2] = ucdq_final_oen_ui_large[1];
        ucdq_final_oen_ui_large[3] = ucdq_final_oen_ui_large[1];
        ucdq_final_ui_small[2] = ucdq_final_ui_small[1];
        ucdq_final_ui_small[3] = ucdq_final_ui_small[1];
        ucdq_final_oen_ui_small[2] = ucdq_final_oen_ui_small[1];
        ucdq_final_oen_ui_small[3] = ucdq_final_oen_ui_small[1];
        ucdq_final_pi[2] = ucdq_final_pi[1];
        ucdq_final_pi[3] = ucdq_final_pi[1];

        for (u1ByteIdx=0; u1ByteIdx<(p->data_width/ucbit_num); u1ByteIdx++)
        {
            mcSHOW_DBG_MSG(("DDR3X4 Final DQ PI Delay(LargeUI, SmallUI, PI) =(%d ,%d, %d)\n", ucdq_final_ui_large[u1ByteIdx], ucdq_final_ui_small[u1ByteIdx], ucdq_final_pi[u1ByteIdx]));
            mcSHOW_DBG_MSG(("DDR3X4 OEN DQ PI Delay(LargeUI, SmallUI, PI) =(%d ,%d, %d)\n\n", ucdq_final_oen_ui_large[u1ByteIdx], ucdq_final_oen_ui_small[u1ByteIdx], ucdq_final_pi[u1ByteIdx]));
        }

        //TX Delay Line DQ de-skew 
        mcSHOW_DBG_MSG(("DDR3X4 de-skew(%d) PI(DLY): \n", p->density));
        for (u1ByteIdx=0; u1ByteIdx<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); u1ByteIdx++)
        {        
            for (u1BitIdx=0; u1BitIdx<DQS_BIT_NUMBER; u1BitIdx++)
            {
                ucindex = u1ByteIdx * DQS_BIT_NUMBER + u1BitIdx;
                WinPerBit[ucindex].best_dqdly = FinalWinPerBit[ucindex].win_center - u2Center_min[u1ByteIdx];
                mcSHOW_DBG_MSG(("%d", WinPerBit[ucindex].best_dqdly));
                WinPerBit[ucindex].best_dqdly = (WinPerBit[ucindex].best_dqdly*p->density + 200)/1000;               
                mcSHOW_DBG_MSG(("(%d) ", WinPerBit[ucindex].best_dqdly));
            }

            mcSHOW_DBG_MSG(("\n"));
        }
        for(ii=p->rank; ii<RANK_MAX; ii++)
        {
            vSetRank(p,ii);

            //TXDLY_DQ , TXDLY_OEN_DQ
            vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ0, \
                                    P_Fld(ucdq_final_ui_large[0], SHURK0_SELPH_DQ0_TXDLY_DQ0) | \
                                    P_Fld(ucdq_final_ui_large[1], SHURK0_SELPH_DQ0_TXDLY_DQ1) | \
                                    P_Fld(ucdq_final_ui_large[2], SHURK0_SELPH_DQ0_TXDLY_DQ2) | \
                                    P_Fld(ucdq_final_ui_large[3], SHURK0_SELPH_DQ0_TXDLY_DQ3) | \
                                    P_Fld(ucdq_final_oen_ui_large[0], SHURK0_SELPH_DQ0_TXDLY_OEN_DQ0) | \
                                    P_Fld(ucdq_final_oen_ui_large[1], SHURK0_SELPH_DQ0_TXDLY_OEN_DQ1) | \
                                    P_Fld(ucdq_final_oen_ui_large[2], SHURK0_SELPH_DQ0_TXDLY_OEN_DQ2) | \
                                    P_Fld(ucdq_final_oen_ui_large[3], SHURK0_SELPH_DQ0_TXDLY_OEN_DQ3));
        
             // DLY_DQ[2:0]
             vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ2, \
                                    P_Fld(ucdq_final_ui_small[0], SHURK0_SELPH_DQ2_DLY_DQ0) | \
                                    P_Fld(ucdq_final_ui_small[1], SHURK0_SELPH_DQ2_DLY_DQ1) | \
                                    P_Fld(ucdq_final_ui_small[2], SHURK0_SELPH_DQ2_DLY_DQ2) | \
                                    P_Fld(ucdq_final_ui_small[3], SHURK0_SELPH_DQ2_DLY_DQ3) | \
                                    P_Fld(ucdq_final_oen_ui_small[0], SHURK0_SELPH_DQ2_DLY_OEN_DQ0) | \
                                    P_Fld(ucdq_final_oen_ui_small[1], SHURK0_SELPH_DQ2_DLY_OEN_DQ1) | \
                                    P_Fld(ucdq_final_oen_ui_small[2], SHURK0_SELPH_DQ2_DLY_OEN_DQ2) | \
                                    P_Fld(ucdq_final_oen_ui_small[3], SHURK0_SELPH_DQ2_DLY_OEN_DQ3));
             
             // set to best values for  DQ
             vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ7, P_Fld(ucdq_final_pi[0], SHU1_R0_B0_DQ7_RK0_ARPI_DQ_B0));
             vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ7, P_Fld(ucdq_final_pi[1], SHU1_R0_B1_DQ7_RK0_ARPI_DQ_B1));
             //cc mark vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ7+(1<<POS_BANK_NUM), P_Fld(ucdq_final_pi[2], SHU1_R0_B0_DQ7_RK0_ARPI_DQ_B0));
             //cc mar vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ7+(1<<POS_BANK_NUM), P_Fld(ucdq_final_pi[3], SHU1_R0_B1_DQ7_RK0_ARPI_DQ_B1));

            vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ0, \
                                   P_Fld(u1DelayCellOfst[0], SHU1_R0_B0_DQ0_RK0_TX_ARDQ0_DLY_B0) | \
                                   P_Fld(u1DelayCellOfst[1], SHU1_R0_B0_DQ0_RK0_TX_ARDQ1_DLY_B0) | \
                                   P_Fld(u1DelayCellOfst[2], SHU1_R0_B0_DQ0_RK0_TX_ARDQ2_DLY_B0) | \
                                   P_Fld(u1DelayCellOfst[3], SHU1_R0_B0_DQ0_RK0_TX_ARDQ3_DLY_B0) | \
                                   P_Fld(u1DelayCellOfst[4], SHU1_R0_B0_DQ0_RK0_TX_ARDQ4_DLY_B0) | \
                                   P_Fld(u1DelayCellOfst[5], SHU1_R0_B0_DQ0_RK0_TX_ARDQ5_DLY_B0) | \
                                   P_Fld(u1DelayCellOfst[6], SHU1_R0_B0_DQ0_RK0_TX_ARDQ6_DLY_B0) | \
                                   P_Fld(u1DelayCellOfst[7], SHU1_R0_B0_DQ0_RK0_TX_ARDQ7_DLY_B0));
             vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ0, \
                                   P_Fld(u1DelayCellOfst[8], SHU1_R0_B1_DQ0_RK0_TX_ARDQ0_DLY_B1) | \
                                   P_Fld(u1DelayCellOfst[9], SHU1_R0_B1_DQ0_RK0_TX_ARDQ1_DLY_B1) | \
                                   P_Fld(u1DelayCellOfst[10], SHU1_R0_B1_DQ0_RK0_TX_ARDQ2_DLY_B1) | \
                                   P_Fld(u1DelayCellOfst[11], SHU1_R0_B1_DQ0_RK0_TX_ARDQ3_DLY_B1) | \
                                   P_Fld(u1DelayCellOfst[12], SHU1_R0_B1_DQ0_RK0_TX_ARDQ4_DLY_B1) | \
                                   P_Fld(u1DelayCellOfst[13], SHU1_R0_B1_DQ0_RK0_TX_ARDQ5_DLY_B1) | \
                                   P_Fld(u1DelayCellOfst[14], SHU1_R0_B1_DQ0_RK0_TX_ARDQ6_DLY_B1) | \
                                   P_Fld(u1DelayCellOfst[15], SHU1_R0_B1_DQ0_RK0_TX_ARDQ7_DLY_B1));
        }        
        vSetRank(p, backup_rank); 
    }
#endif

#if REG_ACCESS_PORTING_DGB
    RegLogEnable =0;
#endif

	p->min_winsize = u2MinWinSize;
	p->sum_winsize = u2TempWinSum;

    mcSHOW_DBG_MSG(("[DramcTxWindowPerbitCal] ====Done====\n"));
	#if	dbg_print
	prom_puts("[DramcTxWindowPerbitCal] ====Done====\n");
	#endif
    return DRAM_OK;
}
#endif //SIMULATION_TX_PERBIT

#if (RX_EYE_SCAN || TX_EYE_SCAN)
U32 DramcDmaEngine(DRAMC_CTX_T *p, DRAM_DMA_OP_T op, U32 src_addr, U32 dst_addr, U32 trans_len, 
			U8 burst_len, DRAM_DMA_CHECK_RESULT_T check_result, U8 channel_num)
{
	return 0;
}
#endif

/* cc notes: Rx & Tx eye scan code is porting from Boar (MT5887).
 */
#if RX_EYE_SCAN

void DramEyeStbenReset(DRAMC_CTX_T *p)
{
    //vIO32WriteFldAlign((DRAMC_REG_GDDR3CTL1), 1, GDDR3CTL1_RDATRST);// read data counter reset

    vIO32WriteFldAlign((DDRPHY_B0_DQ5), 0, B0_DQ5_RG_RX_ARDQ_EYE_STBEN_RESETB_B0);
    vIO32WriteFldAlign((DDRPHY_B1_DQ5), 0, B1_DQ5_RG_RX_ARDQ_EYE_STBEN_RESETB_B1);

    mcDELAY_US(1);//delay 10ns

    vIO32WriteFldAlign((DDRPHY_B0_DQ5), 1, B0_DQ5_RG_RX_ARDQ_EYE_STBEN_RESETB_B0);
    vIO32WriteFldAlign((DDRPHY_B1_DQ5), 1, B1_DQ5_RG_RX_ARDQ_EYE_STBEN_RESETB_B1);
}


#define RX_EYE_VREF_LEVEL   32
#define RX_EYE_DELAY_LEVEL  63//40
#define RX_EYE_SAVE_TO_FILE 1
#define ENALBE_EYE_SCAN_LOG_DURING_PROCESS 0
#define RX_EYE_SCAN_VREF_EXTEND 0

typedef enum
{
    RX_EYE_SCAN_MOVE_DQS_ENTER=0,
    RX_EYE_SCAN_MOVE_DQS_END,
} RX_EYE_SCAN_MOVE_DQS_STATUS_T;

typedef enum
{
    RX_EYE_SCAN_SCAN_AT_BOOT_TIME=0,
    RX_EYE_SCAN_SCAN_AT_RUN_TIME,
} RX_EYE_SCAN_TIME_T;


void DramcRxEyeScanInit(DRAMC_CTX_T *p)
{
    U8 u1ByteIdx;
    //Enable DQ eye scan
    vIO32WriteFldAlign((DRAMC_REG_EYESCAN), 1, EYESCAN_RG_EX_EYE_SCAN_EN);

    //Disable MIOCK jitter meter mode (RG_??_RX_DQS_MIOCK_SEL=0, RG_RX_MIOCK_JIT_EN=0)
    //RG_RX_MIOCK_JIT_EN=0
    vIO32WriteFldAlign((DRAMC_REG_EYESCAN), 0, EYESCAN_RG_RX_MIOCK_JIT_EN);

    //RG_??_RX_EYE_SCAN_EN,   RG_??_RX_VREF_EN,  RG_??_RX_SMT_EN
    vIO32WriteFldAlign((DDRPHY_B0_DQ5), 1, B0_DQ5_RG_RX_ARDQ_EYE_EN_B0);
    vIO32WriteFldAlign((DDRPHY_B1_DQ5), 1, B1_DQ5_RG_RX_ARDQ_EYE_EN_B1);
    vIO32WriteFldAlign((DDRPHY_B0_DQ5), 1, B0_DQ5_RG_RX_ARDQ_EYE_VREF_EN_B0);
    vIO32WriteFldAlign((DDRPHY_B1_DQ5), 1, B1_DQ5_RG_RX_ARDQ_EYE_VREF_EN_B1);
    vIO32WriteFldAlign((DDRPHY_B0_DQ3), 1, B0_DQ3_RG_RX_ARDQ_SMT_EN_B0);
    vIO32WriteFldAlign((DDRPHY_B1_DQ3), 1, B1_DQ3_RG_RX_ARDQ_SMT_EN_B1);

    DramEyeStbenReset(p);
    DramPhyReset(p);
}

#if 1
static void DramcRxEyeRuntimeDQS(DRAMC_CTX_T *p, U8 enter_or_end)
{
    U32 u4TimeCnt =1000;
    U8 u1Ready[DQS_NUMBER], u1Ready_all, u1Wait=1;

        vIO32WriteFldAlign((DDRPHY_B0_DQ6), enter_or_end, B0_DQ6_RG_RX_ARDQ_EYE_OE_GATE_EN_B0);
        vIO32WriteFldAlign((DDRPHY_B1_DQ6), enter_or_end, B1_DQ6_RG_RX_ARDQ_EYE_OE_GATE_EN_B1);

        do
        {
            u1Ready[0]  =(U8)u4IO32ReadFldAlign((DDRPHY_MISC_PHY_RGS0), MISC_PHY_RGS0_RGS_RX_ARDQS0_DLY_RDY_EYE_B01);
            u1Ready[1] = (U8)u4IO32ReadFldAlign((DDRPHY_MISC_PHY_RGS0), MISC_PHY_RGS0_RGS_RX_ARDQS1_DLY_RDY_EYE);

            if(enter_or_end ==RX_EYE_SCAN_MOVE_DQS_ENTER)
            {
               u1Ready_all = u1Ready[0] || u1Ready[1];
            }
            else//RX_EYE_SCAN_MOVE_DQS_END
            {
               u1Ready_all = u1Ready[0] && u1Ready[1];
            }

            u1Wait = (u1Ready_all != enter_or_end) ? 1 : 0;
            u4TimeCnt--;
        }
        while((u1Wait ==1) && (u4TimeCnt >0));

            ///TODO:  need to be fix, Yulia
        //if(u4TimeCnt ==0)
       //    mcSHOW_DBG_MSG(("DQS setup timeout (enter_or_end = %d) %d %d\n", enter_or_end, u1Ready[0] , u1Ready[1]));
#if 0///TODO://ENABLE_LP3_SW
    else //LPDDR3
    {
        vIO32WriteFldAlign((DDRPHY_B0_DQ6), enter_or_end, B0_DQ6_RG_RX_ARDQ_EYE_OE_GATE_EN_B0);
        vIO32WriteFldAlign((DDRPHY_B1_DQ6), enter_or_end, B1_DQ6_RG_RX_ARDQ_EYE_OE_GATE_EN_B1);
        do
        {
            u1Ready[0] = (U8)u4IO32ReadFldAlign(DDRPHY_PHY_RO_1+(aru1PhyMap2Channel[0]<< POS_BANK_NUM), PHY_RO_1_RGS_RX_ARDQS0_DLY_RDY_EYE);
            u1Ready[1] = (U8)u4IO32ReadFldAlign(DDRPHY_PHY_RO_1+(aru1PhyMap2Channel[1]<< POS_BANK_NUM), PHY_RO_1_RGS_RX_ARDQS0_DLY_RDY_EYE);
            u1Ready[2] = (U8)u4IO32ReadFldAlign(DDRPHY_PHY_RO_1+(aru1PhyMap2Channel[0]<< POS_BANK_NUM), PHY_RO_1_RGS_RX_ARDQS1_DLY_RDY_EYE);
            u1Ready[3] = (U8)u4IO32ReadFldAlign(DDRPHY_PHY_RO_1+(aru1PhyMap2Channel[1]<< POS_BANK_NUM), PHY_RO_1_RGS_RX_ARDQS1_DLY_RDY_EYE);

            if(enter_or_end ==RX_EYE_SCAN_MOVE_DQS_ENTER)
            {
                u1Ready_all = u1Ready[0] ||u1Ready[1]||u1Ready[2]||u1Ready[3];
            }
            else//RX_EYE_SCAN_MOVE_DQS_END
            {
                u1Ready_all = u1Ready[0] && u1Ready[1] && u1Ready[2] && u1Ready[3];
            }

            u1Wait = (u1Ready_all != enter_or_end) ? 1 : 0;

            u4TimeCnt--;

         }
         while((u1Wait ==1) && (u4TimeCnt >0));

         if(u4TimeCnt ==0)
         {
             mcSHOW_DBG_MSG(("DQS setup timeout (enter_or_end = %d) (u1Ready_all %d) u4TimeCnt %d,%d %d %d %d\n", enter_or_end, u1Ready_all, u4TimeCnt, u1Ready[0], u1Ready[1], u1Ready[2], u1Ready[3]));
         }
    }
#endif
}
#endif


void mem_test_address_calculation(DRAMC_CTX_T * p, U32 uiSrcAddr, U32*pu4Dest)
{
    U32 u4RankSize;

	/* cc notes: This shall be set according to DRAM(die) spec.
	 * Also, address mapping (if DMA engine used) shall be considered??
	 */
    u4RankSize = 0x40000000; 

    if(p->support_rank_num == RANK_SINGLE)
    {
        *pu4Dest = uiSrcAddr + (u4RankSize >>1);
    }
    else
    {
        *pu4Dest = uiSrcAddr + u4RankSize;
    }
}

//-------------------------------------------------------------------------
/** DramcRxEyeScan
 *  start the rx dq eye scan.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param dq_no            (U8): 0~7.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 */
//-------------------------------------------------------------------------
DRAM_STATUS_T DramcRxEyeScanRun(DRAMC_CTX_T *p, RX_EYE_SCAN_TIME_T boot_or_run, U8 dq_no)
{
    S16 s2vref, s2dq_dly, s2DelayBegin, s2DelayStep, s2DelayEnd;
    U32  u4err_value = 0xffffffff, u4sample_cnt=0, u4error_cnt[DQS_NUMBER];
    #if RX_EYE_SAVE_TO_FILE
    static U32 arErrorCount[DQS_NUMBER][RX_EYE_VREF_LEVEL][RX_EYE_DELAY_LEVEL];
    static U16 arVrefLevelForLog[RX_EYE_VREF_LEVEL];
    U16 u2VrefIdx, u2DelayIdx, ii,jj, kk;
    #endif
    U16 u2VrefBegin, u2VrefEnd, u2VrefStep;

    U32 uiSrcAddr, uiDestAddr, uiLen;

    U8 u1DramCBit, u1PHYBit;

#if RX_EYE_SCAN_VREF_EXTEND
    U8 Vref_DDR4_SEL =0;
#endif
    // error handling
    if (!p)
    {
        mcSHOW_ERR_MSG(("context is NULL\n"));
        return DRAM_FAIL;
    }

    if (dq_no > 7)
    {
        mcSHOW_ERR_MSG(("DQ number should be 0~15 for 2 bytes\n"));
		#if dbg_print
		prom_puts("DQ number should be 0~7 for 2 bytes\n");
		#endif
        return DRAM_FAIL;
    }

	//DDRPHY.SHU1_B*_DQ7.R_DMDQMDBI_SHU_B* = R_DMDQMDBI_EYE_SHU_B* 
	//0: DBI-OFF
	//1: DBI-ON
	//cc mark. DDR3 has no DBI. vIO32WriteFldAlign((DDRPHY_SHU1_B0_DQ7), p->DBI_R_onoff, SHU1_B0_DQ7_R_DMDQMDBI_SHU_EYE_B0);
	//cc mark. DDR3 has no DBI. vIO32WriteFldAlign((DDRPHY_SHU1_B1_DQ7), p->DBI_R_onoff, SHU1_B1_DQ7_R_DMDQMDBI_EYE_SHU_B1);

    // check if SoC platform has "%" operation?!
    #if RX_EYE_SAVE_TO_FILE
    for(ii=0; ii<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); ii++)
    {
        for(jj=0; jj<RX_EYE_VREF_LEVEL; jj++)
        {
            for(kk=0; kk<RX_EYE_DELAY_LEVEL; kk++)
            {
                arErrorCount[ii][jj][kk] =0xffff;
            }
        }
    }
    #endif

    if(boot_or_run==RX_EYE_SCAN_SCAN_AT_RUN_TIME)
    {
        //uiSrcAddr = DDR_BASE+0x10000000;
        uiSrcAddr = 0x50000000;
        uiLen = 0xff000;

	#if DUAL_RANK_ENABLE
		mem_test_address_calculation(p, uiSrcAddr, &uiDestAddr);
	#else
		uiDestAddr=uiSrcAddr+0x10000000;
	#endif
    }

    //DRAMC: RG_RX_DQ_EYE_SEL (0~7) for 4 bytes
    u1DramCBit = dq_no% DQS_BIT_NUMBER;
    u1PHYBit = dq_no % DQS_BIT_NUMBER;
    vIO32WriteFldAlign((DRAMC_REG_EYESCAN), u1DramCBit, EYESCAN_RX_DQ_EYE_SEL);

    //select DQ to be scanned (0~7)
    //DDRPHY: RG_??_RX_DQ_EYE_SEL
    vIO32WriteFldAlign((DDRPHY_B0_DQ5), u1PHYBit, B0_DQ5_RG_RX_ARDQ_EYE_SEL_B0);
    vIO32WriteFldAlign((DDRPHY_B1_DQ5), u1PHYBit, B1_DQ5_RG_RX_ARDQ_EYE_SEL_B1);

    vIO32WriteFldAlign((DDRPHY_B0_DQ6), !boot_or_run, B0_DQ6_RG_RX_ARDQ_EYE_DLY_DQS_BYPASS_B0);
    vIO32WriteFldAlign((DDRPHY_B1_DQ6), !boot_or_run, B1_DQ6_RG_RX_ARDQ_EYE_DLY_DQS_BYPASS_B1);

    #if ENALBE_EYE_SCAN_LOG_DURING_PROCESS
    mcSHOW_DBG_MSG(("===============================================================================\n"));
    mcSHOW_DBG_MSG(("    DQ RX eye scan (channel=%d, dq_%d, DramCBit %d, PHYBit %d)\n", p->channel, dq_no, u1DramCBit, u1PHYBit));
    mcSHOW_DBG_MSG(("===============================================================================\n"));
	#if dbg_print
	prom_puts("===============================================================================\n");
	prom_puts("DQ RX eye scan (dq_");
	prom_print_dec(dq_no);
	prom_puts(", DramCBit ");
	prom_print_dec(u1DramCBit);
	prom_puts(", PHYBit ");
	prom_print_dec(u1PHYBit);
	prom_puts(")\n");
	prom_puts("===============================================================================\n");
	#endif
    #endif
    u2VrefBegin =RX_VREF_SCAN_BEGIN;
    u2VrefEnd =RX_VREF_SCAN_END;
    u2VrefStep=RX_VREF_SCAN_STEP;

    #if RX_EYE_SAVE_TO_FILE
    u2VrefIdx =0;
	u2DelayIdx=0;	//YMC add to avoid warning
    #endif

    #if RX_EYE_SCAN_VREF_EXTEND
    s2vref =(u2VrefEnd-1);

    while(1)
    #else
    //cc change for(s2vref =(u2VrefEnd-1); s2vref > u2VrefBegin; s2vref-=u2VrefStep)
    for(s2vref = u2VrefBegin; s2vref < u2VrefEnd; s2vref+=u2VrefStep)
    //s2vref =0xa;
    #endif
    {
        #if 1//cc mark to log VREF always. RX_EYE_SCAN_VREF_EXTEND
        arVrefLevelForLog[u2VrefIdx] = s2vref;
        #endif

        if(boot_or_run ==RX_EYE_SCAN_SCAN_AT_RUN_TIME)
        {
            DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, uiSrcAddr, uiDestAddr, uiLen, 8, DMA_CHECK_DATA_ACCESS_ONLY_AND_NO_WAIT, 2);
            DramcRxEyeRuntimeDQS(p, RX_EYE_SCAN_MOVE_DQS_ENTER);
        }
		
        s2vref = s2vref | (1<<5); //cc unmark. According to DE's init flow, shall set bit[5] to 1'b1
        //Set Vref voltage
        vIO32WriteFldAlign((DDRPHY_B0_DQ5), s2vref, B0_DQ5_RG_RX_ARDQ_EYE_VREF_SEL_B0);
        vIO32WriteFldAlign((DDRPHY_B1_DQ5), s2vref, B1_DQ5_RG_RX_ARDQ_EYE_VREF_SEL_B1);

        //Wait for Vref settles down, 1us is enough
        mcDELAY_US(1);

        //Set DQS delay (RG_??_RX_DQS_EYE_DLY) to 0
        vIO32WriteFldAlign(DDRPHY_B0_DQ4, 0,RG_FLD_FULL);
        vIO32WriteFldAlign(DDRPHY_B1_DQ4, 0,RG_FLD_FULL);

        if(boot_or_run ==RX_EYE_SCAN_SCAN_AT_RUN_TIME)
        {
            DramcRxEyeRuntimeDQS(p, RX_EYE_SCAN_MOVE_DQS_END);
        }
        else //if(boot_or_run ==0)
        {
            DramEyeStbenReset(p);
            DramPhyReset(p);
        }

	#if 0 //cc mark for 7580 only supports 1 channel
            if(p->channel ==CHANNEL_A)
            {
                s2DelayBegin = -35;
                s2DelayEnd = 15;
            }
            else if(p->channel ==CHANNEL_B)
            {
                s2DelayBegin = -30;
                s2DelayEnd = 25;
            }
			else if(p->channel ==CHANNEL_C)
            {
                s2DelayBegin = -30;
                s2DelayEnd = 25;
            }
			else //CHD
            {
                s2DelayBegin = -30;
                s2DelayEnd = 25;
            }
	#else		
		s2DelayBegin = -35;
		s2DelayEnd = 15;
		s2DelayStep = 4;
	#endif

        for (s2dq_dly=s2DelayBegin; s2dq_dly <s2DelayEnd; s2dq_dly+=s2DelayStep)
        {

            if(boot_or_run == RX_EYE_SCAN_SCAN_AT_RUN_TIME)
            {
                //Reset eye scan counters (reg_sw_rst): 1 to 0
                vIO32WriteFldAlign((DRAMC_REG_EYESCAN), 1, EYESCAN_REG_SW_RST);
                mcDELAY_US(1);
                vIO32WriteFldAlign((DRAMC_REG_EYESCAN), 0, EYESCAN_REG_SW_RST);

                DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, uiSrcAddr, uiDestAddr, uiLen, 8, DMA_CHECK_DATA_ACCESS_ONLY_AND_NO_WAIT, 2);
            }

            if(s2dq_dly >0)  // move DQ delay
            {
                    //Set DQ delay (RG_??_RX_DQ_EYE_DLY)
                    vIO32WriteFldAlign((DDRPHY_B0_DQ4), s2dq_dly, B0_DQ4_RG_RX_ARDQ_EYE_R_DLY_B0);
                    vIO32WriteFldAlign((DDRPHY_B0_DQ4), s2dq_dly, B0_DQ4_RG_RX_ARDQ_EYE_F_DLY_B0);
                    vIO32WriteFldAlign((DDRPHY_B1_DQ4), s2dq_dly, B1_DQ4_RG_RX_ARDQ_EYE_R_DLY_B1);
                    vIO32WriteFldAlign((DDRPHY_B1_DQ4), s2dq_dly, B1_DQ4_RG_RX_ARDQ_EYE_F_DLY_B1);
            }
            else// move DQS delay
            {
                if(boot_or_run ==RX_EYE_SCAN_SCAN_AT_RUN_TIME)
                {
                    DramcRxEyeRuntimeDQS(p, RX_EYE_SCAN_MOVE_DQS_ENTER);
                }

                //Set DQS delay (RG_??_RX_DQS_EYE_DLY)
                vIO32WriteFldAlign((DDRPHY_B0_DQ4), -s2dq_dly, B0_DQ4_RG_RX_ARDQS_EYE_R_DLY_B0);
                vIO32WriteFldAlign((DDRPHY_B0_DQ4), -s2dq_dly, B0_DQ4_RG_RX_ARDQS_EYE_F_DLY_B0);
                vIO32WriteFldAlign((DDRPHY_B1_DQ4), -s2dq_dly, B1_DQ4_RG_RX_ARDQS_EYE_R_DLY_B1);
                vIO32WriteFldAlign((DDRPHY_B1_DQ4), -s2dq_dly, B1_DQ4_RG_RX_ARDQS_EYE_F_DLY_B1);

                if(boot_or_run ==RX_EYE_SCAN_SCAN_AT_RUN_TIME)
                {
                    DramcRxEyeRuntimeDQS(p, RX_EYE_SCAN_MOVE_DQS_END);
                }
                else //if(boot_or_run ==RX_EYE_SCAN_SCAN_AT_BOOT_TIME)
                {
                    DramEyeStbenReset(p);
                    DramPhyReset(p);
                }
            }

            if(boot_or_run ==RX_EYE_SCAN_SCAN_AT_BOOT_TIME)//boot time
            {
                //Reset eye scan counters (reg_sw_rst): 1 to 0
                vIO32WriteFldAlign((DRAMC_REG_EYESCAN), 1, EYESCAN_REG_SW_RST);
                mcDELAY_US(1);
                vIO32WriteFldAlign((DRAMC_REG_EYESCAN), 0, EYESCAN_REG_SW_RST);

                u4err_value= TestEngineCompare(p);
            }
            else
            {
                //run time
                //mcDELAY_MS(1);
                u4err_value = DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, uiSrcAddr, uiDestAddr, uiLen, 8, DMA_CHECK_COMAPRE_RESULT_ONLY, 2);  // comapre DMA result
            }

            //Read the counter values from registers (toggle_cnt*, dq_err_cnt*);
            //At run time, the counter will change all the time. Therefore, the counter of different bytes and toggle count will be data of different time.
            u4sample_cnt = u4IO32ReadFldAlign((DRAMC_REG_TOGGLE_CNT), TOGGLE_CNT_TOGGLE_CNT);
            u4error_cnt[0] = u4IO32ReadFldAlign((DRAMC_REG_DQ_ERR_CNT0), DQ_ERR_CNT0_DQ_ERR_CNT0);
            u4error_cnt[1] = u4IO32ReadFldAlign((DRAMC_REG_DQ_ERR_CNT1), DQ_ERR_CNT1_DQ_ERR_CNT1);

	        #ifdef ETT_PRINT_FORMAT
	        //mcSHOW_DBG_MSG(("RX Vref= %d, Delay= %d, ErrorValue=%d, Toggle %d, ErrorCnt %d\n",  s2vref, s2dq_dly, u4err_value, u4sample_cnt, u4error_cnt[dq_no/8]));
	        #else
	        mcSHOW_DBG_MSG(("RX Vref= %2d, Delay= %4d, ErrorValue=%d, Toggle %4d, ErrorCnt %4d\n",  s2vref, s2dq_dly, u4err_value, u4sample_cnt, u4error_cnt[dq_no/8]));
			#if dbg_print
			prom_puts("RX Vref = ");
			prom_print_dec(s2vref);
			prom_puts(", Delay = ");
			if(s2dq_dly>=0)
				prom_print_dec(s2dq_dly);
			else
			{
				prom_puts("-");
				prom_print_dec(-s2dq_dly);
			}
			prom_puts(", ErrorValue = ");
			prom_print_dec(u4err_value);
			prom_puts(", Toggle = ");
			prom_print_dec(u4sample_cnt);
			prom_puts(", ErrorCnt = ");
			prom_print_dec(u4error_cnt[dq_no/8]);
			prom_puts("\n");
			#endif
	        #endif

            #if RX_EYE_SAVE_TO_FILE
            arErrorCount[0][u2VrefIdx][u2DelayIdx] = u4error_cnt[0];
            arErrorCount[1][u2VrefIdx][u2DelayIdx] = u4error_cnt[1];
            if(p->dram_type == TYPE_LPDDR3)
            {
                arErrorCount[2][u2VrefIdx][u2DelayIdx] = u4error_cnt[2];
                arErrorCount[3][u2VrefIdx][u2DelayIdx] = u4error_cnt[3];
            }

            if(u2DelayIdx<RX_EYE_DELAY_LEVEL)
            {
                u2DelayIdx ++;
            }
            else
            {
                mcSHOW_DBG_MSG(("\n\n[DramcRxEyeScan] WARNING: Out of error count array size, Delay %d\n\n", u2DelayIdx));
				#if dbg_print
				prom_puts("\n\n[DramcRxEyeScan] WARNING: Out of error count array size, Delay ");
				prom_print_dec(u2DelayIdx);
				prom_puts("\n\n");
				#endif
            }
            #endif
        }

        #if RX_EYE_SAVE_TO_FILE
        if(u2VrefIdx<RX_EYE_VREF_LEVEL)
        {
            u2VrefIdx ++;
        }
        else
        {
            mcSHOW_DBG_MSG(("\n\n[DramcRxEyeScan] WARNING: Out of error count array size, Vref %d\n\n", u2VrefIdx));
			#if dbg_print
			prom_puts("\n\n[DramcRxEyeScan] WARNING: Out of error count array size, Vref ");
			prom_print_dec(u2VrefIdx);
			prom_puts("\n\n");
			#endif
        }
        #endif

        #if RX_EYE_SCAN_VREF_EXTEND
        s2vref-=u2VrefStep;

        if( s2vref < u2VrefBegin)  // switch to LP4 scan range
        {
            if(Vref_DDR4_SEL==0)
            {
                s2vref =(u2VrefEnd-1);
                Vref_DDR4_SEL =1; // LP4 range

                vIO32WriteFldAlign(DDRPHY_B0_DQ6, 0, B0_DQ6_RG_RX_ARDQ_DDR3_SEL_B0);
                vIO32WriteFldAlign(DDRPHY_B1_DQ6, 0, B1_DQ6_RG_RX_ARDQ_DDR3_SEL_B1);

                vIO32WriteFldAlign(DDRPHY_B0_DQ6, 1, B0_DQ6_RG_RX_ARDQ_DDR4_SEL_B0);
                vIO32WriteFldAlign(DDRPHY_B1_DQ6, 1, B1_DQ6_RG_RX_ARDQ_DDR4_SEL_B1);

                vIO32WriteFldAlign(DDRPHY_SHU1_B0_DQ5, 0x3f, SHU1_B0_DQ5_RG_RX_ARDQ_VREF_SEL_B0);
                vIO32WriteFldAlign(DDRPHY_SHU1_B1_DQ5, 0x3f, SHU1_B1_DQ5_RG_RX_ARDQ_VREF_SEL_B1);
                mcSHOW_DBG_MSG(("\n\n[DramcRxEyeScan] Change to LP4 Vref range\n\n"));
				#if dbg_print
				prom_puts("\n\n[DramcRxEyeScan] Change to LP4 Vref range\n\n");
				#endif
            }
            else
            {
                // Restore LP3 oringinal Vref setttings
                //cc mark for field not found...vIO32WriteFldAlign(DDRPHY_B0_DQ6, 1, B0_DQ6_RG_RX_ARDQ_DDR3_SEL_B0);
                //cc mark for field not found...vIO32WriteFldAlign(DDRPHY_B1_DQ6, 1, B1_DQ6_RG_RX_ARDQ_DDR3_SEL_B1);

                vIO32WriteFldAlign(DDRPHY_B0_DQ6, 0, B0_DQ6_RG_RX_ARDQ_DDR4_SEL_B0);
                vIO32WriteFldAlign(DDRPHY_B1_DQ6, 0, B1_DQ6_RG_RX_ARDQ_DDR4_SEL_B1);

                vIO32WriteFldAlign(DDRPHY_SHU1_B0_DQ5, 0xe, SHU1_B0_DQ5_RG_RX_ARDQ_VREF_SEL_B0);
                vIO32WriteFldAlign(DDRPHY_SHU1_B1_DQ5, 0xe, SHU1_B1_DQ5_RG_RX_ARDQ_VREF_SEL_B1);
                mcSHOW_DBG_MSG(("\n\n[DramcRxEyeScan] Restore LP3 Vref range\n\n"));
				#if dbg_print
				prom_puts("\n\n[DramcRxEyeScan] Restore LP3 Vref range\n\n");
				#endif
                break;
            }
        }
        #endif

        #if ENALBE_EYE_SCAN_LOG_DURING_PROCESS
        mcSHOW_DBG_MSG(("\n"));
		#if dbg_print
		prom_puts("\n");
		#endif
        #else
        mcSHOW_DBG_MSG(("."));
        #endif
    }

    #if !ENALBE_EYE_SCAN_LOG_DURING_PROCESS
    mcSHOW_DBG_MSG(("\n"));
    #endif

#if 0  // When run time eye scan, don't disable RX eye scan.
    //Disable DQ eye scan (RG_RX_EYE_SCAN_EN=0)
    vIO32WriteFldAlign((DRAMC_REG_STBCAL_F), 0, STBCAL_F_RG_EX_EYE_SCAN_EN);

        vIO32WriteFldAlign((DDRPHY_EYE3), 1, EYE3_RG_RX_ARDQ_EYE_DLY_DQS_BYPASS_B0);
        vIO32WriteFldAlign((DDRPHY_EYEB1_3), 1, EYEB1_3_RG_RX_ARDQ_EYE_DLY_DQS_BYPASS_B1);

        //Disable DQ eye scan (RG_RX_EYE_SCAN_EN=0, RG_RX_*RDQ_VREF_EN_B*=0, RG_RX_*RDQ_EYE_VREF_EN_B*=0, RG_RX_*RDQ_SMT_EN_B*=0)
        vIO32WriteFldAlign((DDRPHY_EYE2), 0, EYE2_RG_RX_ARDQ_EYE_VREF_EN_B0);
        vIO32WriteFldAlign((DDRPHY_EYEB1_2), 0, EYEB1_2_RG_RX_ARDQ_EYE_VREF_EN_B1);
        vIO32WriteFldAlign((DDRPHY_TXDQ3), 0, TXDQ3_RG_RX_ARDQ_SMT_EN_B0);
        vIO32WriteFldAlign((DDRPHY_RXDQ13),0, RXDQ13_RG_RX_ARDQ_SMT_EN_B1);

#endif
    #if RX_EYE_SAVE_TO_FILE
    for(ii=0; ii<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); ii++)
    {
        mcSHOW_DBG_MSG(("===============================================================================\n"));
        mcSHOW_DBG_MSG(("    DQ RX eye scan (Byte %d, dq_%d, DramCBit %d, PHYBit %d)\n", ii, dq_no, u1DramCBit, u1PHYBit));
        mcSHOW_DBG_MSG(("===============================================================================\n"));
        mcFPRINTF((fp_A60501, "===============================================================================\n"));
        mcFPRINTF((fp_A60501,"    DQ RX eye scan Byte %d, dq_%d, DramCBit %d, PHYBit %d)\n", ii, dq_no, u1DramCBit, u1PHYBit));
        mcFPRINTF((fp_A60501, "===============================================================================\n"));
		#if dbg_print
		prom_puts("===============================================================================\n");
		prom_puts("DQ RX eye scan (Byte ");
		prom_print_dec(ii);
		prom_puts(", dq_");
		prom_print_dec(dq_no);
		prom_puts(", DramCBit ");
		prom_print_dec(u1DramCBit);
		prom_puts(", PHYBit ");
		prom_print_dec(u1PHYBit);
		prom_puts(")\n");
		prom_puts("===============================================================================\n");
		#endif

        for(jj=0; jj<RX_EYE_VREF_LEVEL; jj++)
        {
            #if 1//cc change to log vref. RX_EYE_SCAN_VREF_EXTEND
            mcSHOW_DBG_MSG(("%d. Vref = %X, ", jj, arVrefLevelForLog[jj]));
			#if dbg_print
			prom_print_dec(jj);
			prom_puts(". Vref = 0x");
			prom_print_hex(arVrefLevelForLog[jj],4);
			prom_puts(", ");
			#endif
            #endif

            for(kk=0; kk<RX_EYE_DELAY_LEVEL; kk++)
            {
                //cc mark if(arErrorCount[ii][jj][kk] != 0xffff)
                {
                    mcSHOW_DBG_MSG(("%x, ", arErrorCount[ii][jj][kk]));
					#if dbg_print
					prom_puts("0x");
					prom_print_hex(arErrorCount[ii][jj][kk],8);
					prom_puts(", ");
					#endif
                    mcFPRINTF((fp_A60501, "%8d, ", arErrorCount[ii][jj][kk]));
                }
            }
            mcSHOW_DBG_MSG(("\n"));
			#if dbg_print
			prom_puts("\n");
			#endif
            mcFPRINTF((fp_A60501, "\n"));
        }
        mcSHOW_DBG_MSG(("\n"));
		#if dbg_print
		prom_puts("\n");
		#endif
        mcFPRINTF((fp_A60501, "\n"));
    }
    return DRAM_OK;

    // log example
    /*
 ===============================================================================
     DQ RX eye scan (channel=1, byte=0, dq_0)
 ===============================================================================
 2450, 2311, 2304, 2299, 1721, 1628, 1466,  946,  814,  635,  507,  232,   48,   26,    7,    1,    3,    4,    1,    5,   24,  140,  386, 1435, 2304, 2560, 2560, 2575, 2816, 2816, 2816, 2853, 3072, 3072, 3072,
 2313, 2304, 2304, 2202, 1655, 1620, 1223,  906,  713,  564,  278,   70,   28,    1,    0,    0,    0,    0,    0,    0,    1,    6,  130,  225, 1823, 2548, 2560, 2564, 2813, 2816, 2816, 3069, 3072, 3072, 3072,
 2305, 2304, 2304, 1786, 1620, 1555,  961,  816,  609,  444,   60,   43,    4,    0,    0,    0,    0,    0,    0,    0,    0,    0,    8,  127,  789, 2265, 2560, 2560, 2571, 2816, 2994, 3072, 3072, 3072, 3072,
 2305, 2304, 2246, 1686, 1589, 1248,  909,  660,  577,  231,   47,   31,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    5,  191, 1233, 2559, 2560, 2563, 2600, 3071, 3072, 3072, 3072, 3072,
 2304, 2304, 2028, 1627, 1375, 1027,  848,  639,  516,   88,   30,    1,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   54,  617, 2175, 2554, 2560, 2571, 3072, 3072, 3072, 3072, 3072,
 2304, 2269, 1757, 1569, 1192,  947,  700,  559,  274,   44,    2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  208, 1005, 2300, 2560, 2808, 2874, 3072, 3072, 3072, 3072,
 2302, 2141, 1672, 1350, 1000,  834,  602,  412,   63,   33,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   67,  633, 1644, 2547, 2816, 2985, 3072, 3072, 3072, 3072,
 2301, 1936, 1559, 1159,  852,  658,  504,  223,   45,    7,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  278,  949, 2414, 2816, 3065, 3072, 3072, 3072, 3072,
 2189, 1920, 1444,  994,  692,  561,  327,   66,   42,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  110,  367, 1968, 2847, 3072, 3072, 3072, 3072, 3072,
 2059, 1967, 1452,  826,  609,  441,  105,   44,   29,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    1,  161, 1640, 2965, 3072, 3072, 3072, 3072, 3072,
 2208, 1948, 1561,  946,  548,  261,   46,   42,    5,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    1,  370, 1392, 2816, 3066, 3072, 3072, 3072, 3072,
 2303, 2019, 1546, 1074,  404,   57,   44,   30,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  250,  466, 1631, 2651, 2995, 3072, 3072, 3072, 3072,
 2305, 2270, 1573, 1279,  390,   72,   41,    4,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  418,  738, 1850, 2634, 3057, 3072, 3072, 3072, 3072,
 2305, 2303, 1655, 1334,  723,  171,   25,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  255,  712, 1468, 2149, 2709, 3072, 3072, 3072, 3072, 3072,
 2305, 2247, 1953, 1323,  947,  426,   14,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    1,  492, 1271, 1854, 2308, 2573, 3072, 3072, 3072, 3072, 3072,
 2303, 2221, 2211, 1449, 1150,  704,   91,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  320,  767, 1775, 2193, 2552, 2562, 2923, 3072, 3072, 3072, 3072,

 ===============================================================================
     DQ RX eye scan (channel=1, byte=1, dq_0)
 ===============================================================================
 2311, 2307, 2304, 2278, 2016, 1674, 1280, 1214,  929,  503,  377,  365,  359,  343,  349,  359,  373,  377,  396,  446, 1102, 1977, 2550, 2563, 2563, 2572, 2816, 2816, 2816, 2816, 3066, 3071, 3072, 3072, 3072,
 2308, 2304, 2304, 2150, 1832, 1458, 1270, 1137,  615,  362,  232,  149,  139,  140,  144,  163,  211,  269,  336,  359,  673, 1300, 2223, 2555, 2565, 2564, 2782, 2816, 2816, 2816, 3070, 3072, 3072, 3072, 3072,
 2304, 2304, 2304, 2050, 1708, 1307, 1198,  924,  388,  168,   59,   38,   24,   28,   27,   44,   82,  129,  199,  325,  354,  854, 1433, 2367, 2561, 2563, 2572, 2816, 2816, 2927, 3072, 3072, 3072, 3072, 3072,
 2305, 2304, 2303, 2013, 1516, 1268, 1101,  545,  179,   54,    3,    2,    0,    1,    1,    2,    1,   28,   88,  141,  322,  590, 1097, 1621, 2535, 2564, 2562, 2816, 2816, 3069, 3072, 3072, 3072, 3072, 3072,
 2304, 2304, 2168, 1838, 1332, 1213,  815,  304,   73,    2,    4,    0,    0,    0,    1,    1,    1,    1,    2,   24,  135,  295,  726, 1170, 2182, 2560, 2562, 2579, 2816, 3072, 3072, 3072, 3072, 3072, 3072,
 2304, 2304, 1937, 1672, 1271, 1125,  474,  124,   22,    0,    0,    0,    0,    1,    0,    0,    2,    2,    0,    0,   28,  166,  513,  703, 1374, 2506, 2561, 2565, 2998, 3072, 3072, 3072, 3072, 3072, 3072,
 2304, 2299, 1813, 1468, 1201,  873,  178,   47,    3,    1,    0,    0,    0,    0,    0,    0,    0,    0,    0,    1,    2,   58,  189,  588, 1062, 1768, 2560, 2563, 2878, 3072, 3072, 3072, 3072, 3072, 3072,
 2304, 2086, 1778, 1308, 1153,  475,   56,   31,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    1,    0,  134,  206,  669, 1149, 2491, 2574, 2820, 3066, 3072, 3072, 3072, 3072, 3072,
 2246, 1862, 1645, 1201,  844,  166,   41,    6,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    1,   11,  112,  503,  949, 1626, 2808, 2818, 3072, 3072, 3072, 3072, 3072, 3072,
 2011, 1775, 1426, 1170,  430,   61,   10,    1,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    1,    1,   13,  185,  665, 1173, 2617, 2904, 3072, 3072, 3072, 3072, 3072, 3072,
 1967, 1725, 1219,  842,  224,   43,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    1,   78,  431, 1161, 2096, 3062, 3072, 3072, 3072, 3072, 3072, 3072,
 2044, 1749, 1098,  433,   83,   18,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    1,    5,  188,  940, 1751, 3060, 3072, 3072, 3072, 3072, 3072, 3072,
 2048, 1878,  940,  316,   45,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    1,  316,  853, 2013, 3016, 3072, 3072, 3072, 3072, 3072, 3072,
 2048, 1978,  934,  294,   56,    1,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    1,  260,  925, 2077, 2667, 3072, 3072, 3072, 3072, 3072, 3072,
 2048, 1974, 1267,  402,   91,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  211,  313, 1214, 2095, 2596, 3065, 3072, 3072, 3072, 3072, 3072,
 2056, 1963, 1556,  535,  130,    2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    1,  254,  728, 1563, 2004, 2892, 3072, 3072, 3072, 3072, 3072, 3072,
   */
#endif
}
#endif //RX_EYE_SCAN

#if TX_EYE_SCAN
U16 aru2TXCaliDelay[DQS_NUMBER];
U16 aru2TXCaliDelay_OEN[DQS_NUMBER];

#define TX_EYE_VREF_LEVEL   32
#define TX_EYE_DELAY_LEVEL  32//40
#define TX_EYE_SAVE_TO_FILE 1
#define ENALBE_EYE_SCAN_LOG_DURING_PROCESS 1
#define TX_EYE_SCAN_VREF_EXTEND 0 //cc change from 1->0


void DramcTxEyeScanInit(DRAMC_CTX_T *p)
{
#if 0
    vIO32WriteFldMulti((DDRPHY_PLL15)+(1 <<POS_BANK_NUM), P_Fld(0, PLL15_RG_RPHYPLL_TST_SEL) | P_Fld(1, PLL15_RG_RPHYPLL_TST_EN) | \
                            P_Fld(0, PLL15_RG_RPHYPLL_TSTCK_EN) | P_Fld(0, PLL15_RG_RPHYPLL_TSTFM_EN) | \
                            P_Fld(0, PLL15_RG_RPHYPLL_TSTOD_EN) | P_Fld(1, PLL15_RG_RPHYPLL_TSTOP_EN) | \
                            P_Fld(1, PLL15_RG_RVREF_VREF_EN) );
    vIO32WriteFldMulti((DDRPHY_PLL15)+(1 <<POS_BANK_NUM), P_Fld(0x3f, PLL15_RG_RVREF_SEL_DQ) | \
                            P_Fld(1, PLL15_RG_RVREF_DDR3_SEL) | P_Fld(0, PLL15_RG_RVREF_DDR4_SEL));
#endif
}

DRAM_STATUS_T DramcTxEyeScanDelay(DRAMC_CTX_T *p, S16 uiDelay[], S16 uiOenDelay[])
{
#ifndef OLYMPUS_TO_BE_PORTING
    U8 u1ByteIdx;
    U8 ucdq_pi[DQS_NUMBER], ucdq_ui_small[DQS_NUMBER], ucdq_ui_large[DQS_NUMBER];
    U8 ucdq_oen_ui_small[DQS_NUMBER], ucdq_oen_ui_large[DQS_NUMBER];

    for(u1ByteIdx=0; u1ByteIdx<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); u1ByteIdx++)
    {
        TxWinTransferDelayToUIPI(p, uiDelay[u1ByteIdx], 0, 0, &(ucdq_ui_large[u1ByteIdx]), &ucdq_ui_small[u1ByteIdx], &ucdq_pi[u1ByteIdx]);
        TxWinTransferDelayToUIPI(p, uiOenDelay[u1ByteIdx], 0, 0, &ucdq_oen_ui_large[u1ByteIdx], &ucdq_oen_ui_small[u1ByteIdx], &ucdq_pi[u1ByteIdx]);

        #if ENALBE_EYE_SCAN_LOG_DURING_PROCESS
        #ifdef ETT_PRINT_FORMAT
        mcSHOW_DBG_MSG(("Delay = %d |%d %d %d \n",uiDelay[u1ByteIdx], ucdq_ui_large[u1ByteIdx],ucdq_ui_small[u1ByteIdx], ucdq_pi[u1ByteIdx]));
        #else
        mcSHOW_DBG_MSG(("Delay = %d |%4d %4d %4d \n",uiDelay[u1ByteIdx], ucdq_ui_large[u1ByteIdx],ucdq_ui_small[u1ByteIdx], ucdq_pi[u1ByteIdx]));
        #endif
        mcFPRINTF((fp_A60501, "Delay = %d | %4d %4d %4d \n",uiDelay[u1ByteIdx], ucdq_ui_large[u1ByteIdx],ucdq_ui_small[u1ByteIdx], ucdq_pi[u1ByteIdx]));
        #endif //#if ENALBE_EYE_SCAN_LOG_DURING_PROCESS
    }

    //mcSHOW_DBG_MSG(("\nTX manual DQ delay = %d\n",uiDelay));
    //mcSHOW_DBG_MSG(("Reg Values = (%d, %d, %d)\n",ucdq_ui_large, ucdq_ui_small,ucdq_pi ));
    //mcSHOW_DBG_MSG(("Reg OEN Values = (%d, %d, %d)\n",ucdq_oen_ui_large, ucdq_oen_ui_small, ucdq_pi));

    //TXDLY_DQ , TXDLY_OEN_DQ
    vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ0, \
                                        P_Fld(ucdq_ui_large[0], SHURK0_SELPH_DQ0_TXDLY_DQ0) | \
                                        P_Fld(ucdq_ui_large[1], SHURK0_SELPH_DQ0_TXDLY_DQ1) | \
                                        P_Fld(ucdq_ui_large[2], SHURK0_SELPH_DQ0_TXDLY_DQ2) | \
                                        P_Fld(ucdq_ui_large[3], SHURK0_SELPH_DQ0_TXDLY_DQ3) | \
                                        P_Fld(ucdq_oen_ui_large[0], SHURK0_SELPH_DQ0_TXDLY_OEN_DQ0) | \
                                        P_Fld(ucdq_oen_ui_large[1], SHURK0_SELPH_DQ0_TXDLY_OEN_DQ1) | \
                                        P_Fld(ucdq_oen_ui_large[2], SHURK0_SELPH_DQ0_TXDLY_OEN_DQ2) | \
                                        P_Fld(ucdq_oen_ui_large[3], SHURK0_SELPH_DQ0_TXDLY_OEN_DQ3));

    // DLY_DQ[1:0]
   vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ2, \
                                    P_Fld(ucdq_ui_small[0], SHURK0_SELPH_DQ2_DLY_DQ0) | \
                                    P_Fld(ucdq_ui_small[1], SHURK0_SELPH_DQ2_DLY_DQ1) | \
                                    P_Fld(ucdq_ui_small[2], SHURK0_SELPH_DQ2_DLY_DQ2) | \
                                    P_Fld(ucdq_ui_small[3], SHURK0_SELPH_DQ2_DLY_DQ3) | \
                                    P_Fld(ucdq_oen_ui_small[0], SHURK0_SELPH_DQ2_DLY_OEN_DQ0) | \
                                    P_Fld(ucdq_oen_ui_small[1], SHURK0_SELPH_DQ2_DLY_OEN_DQ1) | \
                                    P_Fld(ucdq_oen_ui_small[2], SHURK0_SELPH_DQ2_DLY_OEN_DQ2) | \
                                    P_Fld(ucdq_oen_ui_small[3], SHURK0_SELPH_DQ2_DLY_OEN_DQ3));
#if 0//cc mark. what for???
	// DLY_DQ[2]
   vIO32WriteFldMulti((DRAMC_REG_SELPH22), \
                                    P_Fld(ucdq_ui_small[0]>>2, SELPH22_DLY_DQ0_2) | \
                                    P_Fld(ucdq_ui_small[1]>>2, SELPH22_DLY_DQ1_2)| \
                                    P_Fld(ucdq_ui_small[2]>>2, SELPH22_DLY_DQ2_2) | \
                                    P_Fld(ucdq_ui_small[3]>>2, SELPH22_DLY_DQ3_2) | \
                                    P_Fld(ucdq_oen_ui_small[0]>>2, SELPH22_DLY_OEN_DQ0_2) | \
                                    P_Fld(ucdq_oen_ui_small[1]>>2, SELPH22_DLY_OEN_DQ1_2) | \
                                    P_Fld(ucdq_oen_ui_small[2]>>2, SELPH22_DLY_OEN_DQ2_2) | \
                                    P_Fld(ucdq_oen_ui_small[3]>>2, SELPH22_DLY_OEN_DQ3_2));
#endif

    //TXDLY_DQM , TXDLY_OEN_DQM
    vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ1, \
                                        P_Fld(ucdq_ui_large[0], SHURK0_SELPH_DQ1_TXDLY_DQM0) | \
                                        P_Fld(ucdq_ui_large[1], SHURK0_SELPH_DQ1_TXDLY_DQM1) | \
                                        P_Fld(ucdq_ui_large[2], SHURK0_SELPH_DQ1_TXDLY_DQM2) | \
                                        P_Fld(ucdq_ui_large[3], SHURK0_SELPH_DQ1_TXDLY_DQM3) | \
                                        P_Fld(ucdq_oen_ui_large[0], SHURK0_SELPH_DQ1_TXDLY_OEN_DQM0) | \
                                        P_Fld(ucdq_oen_ui_large[1], SHURK0_SELPH_DQ1_TXDLY_OEN_DQM1) | \
                                        P_Fld(ucdq_oen_ui_large[2], SHURK0_SELPH_DQ1_TXDLY_OEN_DQM2) | \
                                        P_Fld(ucdq_oen_ui_large[3], SHURK0_SELPH_DQ1_TXDLY_OEN_DQM3));

    // DLY_DQM[1:0]
   vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ3, \
                                    P_Fld(ucdq_ui_small[0], SHURK0_SELPH_DQ3_DLY_DQM0) | \
                                    P_Fld(ucdq_ui_small[1], SHURK0_SELPH_DQ3_DLY_DQM1) | \
                                    P_Fld(ucdq_ui_small[2], SHURK0_SELPH_DQ3_DLY_DQM2) | \
                                    P_Fld(ucdq_ui_small[3], SHURK0_SELPH_DQ3_DLY_DQM3) | \
                                    P_Fld(ucdq_oen_ui_small[0], SHURK0_SELPH_DQ3_DLY_OEN_DQM0) | \
                                    P_Fld(ucdq_oen_ui_small[1], SHURK0_SELPH_DQ3_DLY_OEN_DQM1) | \
                                    P_Fld(ucdq_oen_ui_small[2], SHURK0_SELPH_DQ3_DLY_OEN_DQM2) | \
                                    P_Fld(ucdq_oen_ui_small[3], SHURK0_SELPH_DQ3_DLY_OEN_DQM3));
#if 0//cc mark. what for????
    // DLY_DQM[2]
   vIO32WriteFldMulti((DRAMC_REG_SELPH22), \
                                    P_Fld(ucdq_ui_small[0]>>2, SELPH22_DLY_DQM0_2) | \
                                    P_Fld(ucdq_ui_small[1]>>2, SELPH22_DLY_DQM1_2)| \
                                    P_Fld(ucdq_ui_small[2]>>2, SELPH22_DLY_DQM2_2) | \
                                    P_Fld(ucdq_ui_small[3]>>2, SELPH22_DLY_DQM3_2) | \
                                    P_Fld(ucdq_oen_ui_small[0]>>2, SELPH22_DLY_OEN_DQM0_2) | \
                                    P_Fld(ucdq_oen_ui_small[1]>>2, SELPH22_DLY_OEN_DQM1_2) | \
                                    P_Fld(ucdq_oen_ui_small[2]>>2, SELPH22_DLY_OEN_DQM2_2) | \
                                    P_Fld(ucdq_oen_ui_small[3]>>2, SELPH22_DLY_OEN_DQM3_2));
#endif

	//cc porting from 7580 TxPerBitCal
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ7, P_Fld(ucdq_pi, SHU1_R0_B0_DQ7_RK0_ARPI_DQ_B0));
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ7, P_Fld(ucdq_pi, SHU1_R0_B1_DQ7_RK0_ARPI_DQ_B1));
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ7, P_Fld(ucdq_pi, SHU1_R0_B0_DQ7_RK0_ARPI_DQM_B0));
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ7, P_Fld(ucdq_pi, SHU1_R0_B1_DQ7_RK0_ARPI_DQM_B1));
	//cc porting end

    //set to registers, PI DQ (per byte)
        // update TX DQ PI delay, for rank 0 // need to take care rank 1 and 2
    //cc mark    vIO32WriteFldMulti((DDRPHY_ARPI_DQ), \
    //cc mark                                    P_Fld(ucdq_pi, ARPI_DQ_RK0_ARPI_DQ_B0) | P_Fld(ucdq_pi, ARPI_DQ_RK0_ARPI_DQ_B1));

#if 0
    u4err_value= TestEngineCompare(p);
#endif
    //DramPhyReset(p);

#endif
	return DRAM_OK;
}


//-------------------------------------------------------------------------
/** DramcTxEyeScan
 *  start the rx dq eye scan.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param dq_no            (U8): 0~7.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 */
//-------------------------------------------------------------------------
//#define EMI_APB_BASE    0x10203000
#define EMI_APB_BASE    0x10219000

DRAM_STATUS_T DramcTxEyeScanRun(DRAMC_CTX_T *p, U8 u1PatternDMA)
{
#ifndef OLYMPUS_TO_BE_PORTING

    S16 s2vref, s2dq_dly, s2Delay[DQS_NUMBER],  s2Delay_OEN[DQS_NUMBER];
    U32  u4err_value = 0xffffffff, u4sample_cnt=0, u4error_cnt[DQS_NUMBER];
    #if TX_EYE_SAVE_TO_FILE
    static U8 arErrorCount[DQ_DATA_WIDTH][TX_EYE_VREF_LEVEL][TX_EYE_DELAY_LEVEL];
    static U16 arVrefLevelForLog[TX_EYE_VREF_LEVEL];
    U16 u2VrefIdx, u2DelayIdx, ii,jj, kk;
    #endif
    U16 u2VrefBegin, u2VrefEnd, u2VrefStep;
    U8 u1ByteIdx, u1BitIdx;
    U32 uiSrcAddr, uiDestAddr, uiLen;

#if TX_EYE_SCAN_VREF_EXTEND
    U8 Vref_DDR4_SEL =0;
#endif
    // error handling
    if (!p)
    {
        mcSHOW_ERR_MSG(("context is NULL\n"));
        return DRAM_FAIL;
    }

    // check if SoC platform has "%" operation?!
    #if TX_EYE_SAVE_TO_FILE
    for(ii=0; ii<DQ_DATA_WIDTH; ii++)
    {
        for(jj=0; jj<TX_EYE_VREF_LEVEL; jj++)
        {
            for(kk=0; kk<TX_EYE_DELAY_LEVEL; kk++)
            {
                arErrorCount[ii][jj][kk] =0xff;
            }
        }
    }
    #endif

    //DMA init address
    if(u1PatternDMA)
    {
        uiSrcAddr = 0x50000000;
        uiDestAddr = 0x58000000;
        uiLen = 0xff000;
		
	#if 0 //cc mark for 7580 only support CHA. No need to change EMI setting
		*(volatile unsigned *)(EMI_APB_BASE+0x00000000) &= (~0x01);	// disable dual channel.  SW hang. Don't use change channel on-the-fly

		if(p->channel ==CHANNEL_A)
       	{
       		*(volatile unsigned *)(EMI_APB_BASE+ 0x000) &= (~(0x01<<18));
      	}
       	else
       	{
       	    *(volatile unsigned *)(EMI_APB_BASE+ 0x000) |= ((0x01<<18));
       	}
	#endif
        DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, uiSrcAddr, uiDestAddr, uiLen, 8, DMA_PREPARE_DATA_ONLY, 1);
    }

    u2VrefBegin =0;
    u2VrefEnd =0x3f;
    u2VrefStep=4;

    #if TX_EYE_SAVE_TO_FILE
    u2VrefIdx =0;
    #endif

    #if TX_EYE_SCAN_VREF_EXTEND
    s2vref =u2VrefEnd;

    while(1)
    #else
    for(s2vref =u2VrefEnd; s2vref > u2VrefBegin; s2vref-=u2VrefStep)
    #endif
    {
        #if TX_EYE_SCAN_VREF_EXTEND
        arVrefLevelForLog[u2VrefIdx] = s2vref;
        #endif

		s2vref |= 1 << 5;//cc add according to DE's Init flow		
        //Set Vref voltage. cc notes. This is for DDR3. For DDR4, use Mode Register??
        vIO32WriteFldAlign(DDRPHY_MISC_VREF_CTRL, s2vref, MISC_VREF_CTRL_RG_RVREF_SEL_DQ);

        //Wait for Vref settles down, 1sec. is enough
        mcDELAY_US(1000000);

 	 #if TX_EYE_SAVE_TO_FILE
        u2DelayIdx=0;
        #endif

        for (s2dq_dly=-15; s2dq_dly <20; s2dq_dly+=1)
        {
            for(u1ByteIdx=0; u1ByteIdx<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); u1ByteIdx++)
            {
                s2Delay[u1ByteIdx] = (S16)aru2TXCaliDelay[u1ByteIdx] +s2dq_dly;
                s2Delay_OEN[u1ByteIdx] = (S16)aru2TXCaliDelay_OEN[u1ByteIdx] +s2dq_dly;
            }

            DramcTxEyeScanDelay(p, s2Delay, s2Delay_OEN);

            if(u1PatternDMA)
            {
                //cc mark if(p->support_channel_num == CHANNEL_SINGLE)
                {
                    DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, uiSrcAddr, uiDestAddr, uiLen, 8, DMA_CHECK_DATA_ACCESS_ONLY_AND_NO_WAIT, 1);
                    u4err_value = DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, uiSrcAddr, uiDestAddr, uiLen, 8, DMA_CHECK_COMAPRE_RESULT_ONLY, 1);
                }
                //cc mark else
                //cc mark {
                //cc mark     DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, uiSrcAddr, uiDestAddr, uiLen, 8, DMA_CHECK_DATA_ACCESS_ONLY_AND_NO_WAIT, 2);
                //cc mark     u4err_value = DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, uiSrcAddr, uiDestAddr, uiLen, 8, DMA_CHECK_COMAPRE_RESULT_ONLY, 2);
                //cc mark }
            }
            else
                u4err_value= TestEngineCompare(p);

            #if ENALBE_EYE_SCAN_LOG_DURING_PROCESS
            #ifdef ETT_PRINT_FORMAT
            mcSHOW_DBG_MSG(("TX Vref= %d, Delay= %d,  u4err_value=%X\n", s2vref, s2dq_dly, u4err_value));
            #else
            mcSHOW_DBG_MSG(("TX Vref= %2d, Delay= %4d, u4err_value= %x\n", s2vref, s2dq_dly, u4err_value));
			#if dbg_print
			prom_puts("TX Vref = ");
			prom_print_dec(s2vref);
			prom_puts(", Delay = ");
			if(s2dq_dly>=0)
				prom_print_dec(s2dq_dly);
			else
			{
				prom_puts("-");
				prom_print_dec(-s2dq_dly);
			}
			prom_puts(", u4err_value = 0x");
			prom_print_hex(u4err_value,8);
			prom_puts("\n");
			#endif
            #endif
            #endif

            #if TX_EYE_SAVE_TO_FILE

            for(u1BitIdx=0; u1BitIdx<DQ_DATA_WIDTH; u1BitIdx++)
            {
                arErrorCount[u1BitIdx][u2VrefIdx][u2DelayIdx] = ((u4err_value >>u1BitIdx)& 0x1);
            }

            if(u2DelayIdx<TX_EYE_DELAY_LEVEL)
            {
                u2DelayIdx ++;
            }
            else
            {
                mcSHOW_DBG_MSG(("\n\n[DramcTxEyeScan] WARNING: Out of error count array size, Delay %d\n\n", u2DelayIdx));
				#if dbg_print
				prom_puts("\n\n[DramcTxEyeScan] WARNING: Out of error count array size, Delay ");
				prom_print_dec(u2DelayIdx);
				prom_puts("\n\n");
				#endif
            }
            #endif
        }

        #if TX_EYE_SAVE_TO_FILE
        if(u2VrefIdx<TX_EYE_VREF_LEVEL)
        {
            u2VrefIdx ++;
        }
        else
        {
            mcSHOW_DBG_MSG(("\n\n[DramcTxEyeScan] WARNING: Out of error count array size, Vref %d\n\n", u2VrefIdx));
			#if dbg_print
			prom_puts("\n\n[DramcTxEyeScan] WARNING: Out of error count array size, Vref ");
			prom_print_dec(u2VrefIdx);
			prom_puts("\n\n");
			#endif
        }
        #endif

        #if TX_EYE_SCAN_VREF_EXTEND
        s2vref -= u2VrefStep;

        if( s2vref < u2VrefBegin)  // switch to LP4 scan range
        {
            if(Vref_DDR4_SEL==0)
            {
                s2vref =u2VrefEnd;
                Vref_DDR4_SEL =1; // LP4 range

				if (p->dram_type == TYPE_PCDDR3) {
	                vIO32WriteFldAlign(DDRPHY_MISC_VREF_CTRL, 1, MISC_VREF_CTRL_RG_RVREF_DDR3_SEL);
	                vIO32WriteFldAlign(DDRPHY_MISC_VREF_CTRL, 0, MISC_VREF_CTRL_RG_RVREF_DDR4_SEL);
				} else if (p->dram_type == TYPE_PCDDR4) {					
	                vIO32WriteFldAlign(DDRPHY_MISC_VREF_CTRL, 0, MISC_VREF_CTRL_RG_RVREF_DDR3_SEL);
	                vIO32WriteFldAlign(DDRPHY_MISC_VREF_CTRL, 1, MISC_VREF_CTRL_RG_RVREF_DDR4_SEL);
				}
				
                //vIO32WriteFldAlign((DDRPHY_PLL15)+(1 <<POS_BANK_NUM), 0x3f, PLL15_RG_RVREF_SEL_DQ);
                mcSHOW_DBG_MSG(("\n\n[DramcTxEyeScan] Change to LP4 Vref range\n\n"));
				#if dbg_print
				prom_puts("\n\n[DramcTxEyeScan] Change to LP4 Vref range\n\n");
				#endif
            }
            else
            {
                // Restore LP3 oringinal Vref setttings
                vIO32WriteFldAlign(DDRPHY_MISC_VREF_CTRL, 1, MISC_VREF_CTRL_RG_RVREF_DDR3_SEL);
                vIO32WriteFldAlign(DDRPHY_MISC_VREF_CTRL, 0, MISC_VREF_CTRL_RG_RVREF_DDR4_SEL);
                vIO32WriteFldAlign(DDRPHY_MISC_VREF_CTRL, 0xf, MISC_VREF_CTRL_RG_RVREF_SEL_DQ);
                mcSHOW_DBG_MSG(("\n\n[DramcTxEyeScan] Restore LP3 Vref range\n\n"));
				#if dbg_print
				prom_puts("\n\n[DramcTxEyeScan] Restore LP3 Vref range\n\n");
				#endif

                mcDELAY_US(1000000);

                for(u1ByteIdx=0; u1ByteIdx<(DQ_DATA_WIDTH/DQS_BIT_NUMBER); u1ByteIdx++)
                {
                    s2Delay[u1ByteIdx] = (S16)aru2TXCaliDelay[u1ByteIdx];
                    s2Delay_OEN[u1ByteIdx] = (S16)aru2TXCaliDelay_OEN[u1ByteIdx];
                }
                DramcTxEyeScanDelay(p, s2Delay, s2Delay_OEN);

                if(u1PatternDMA)
                {
                    DramcDmaEngine((DRAMC_CTX_T *)p, DMA_OP_READ_WRITE, uiSrcAddr, uiDestAddr, uiLen, 8, DMA_CHECK_DATA_ACCESS_ONLY_AND_NO_WAIT, 1);
                }

                break;
            }
        }
        #endif

        #if ENALBE_EYE_SCAN_LOG_DURING_PROCESS
        mcSHOW_DBG_MSG(("\n"));
		#if dbg_print
		prom_puts("\n");
		#endif
        #else
        mcSHOW_DBG_MSG(("."));
        #endif
    }

    #if !ENALBE_EYE_SCAN_LOG_DURING_PROCESS
    mcSHOW_DBG_MSG(("\n"));
    #endif

    #if TX_EYE_SAVE_TO_FILE
    for(ii=0; ii<DQ_DATA_WIDTH; ii++)
    {
        mcSHOW_DBG_MSG(("===============================================================================\n"));
        mcSHOW_DBG_MSG(("    DQ TX eye scan (dq_%d)\n", ii));
        mcSHOW_DBG_MSG(("===============================================================================\n"));
        mcFPRINTF((fp_A60501, "===============================================================================\n"));
        mcFPRINTF((fp_A60501,"    DQ TX eye scan (dq_%d)\n", ii));
        mcFPRINTF((fp_A60501, "===============================================================================\n"));
		#if dbg_print
		prom_puts("===============================================================================\n");
		prom_puts("DQ TX eye scan (dq_");
		prom_print_dec(ii);
		prom_puts(")\n");
		prom_puts("===============================================================================\n");
		#endif
		
        for(jj=0; jj<TX_EYE_VREF_LEVEL; jj++)
        {
            #if TX_EYE_SCAN_VREF_EXTEND
            mcSHOW_DBG_MSG(("%d. Vref = %X, ", jj, arVrefLevelForLog[jj]));
			#if dbg_print
			prom_print_dec(jj);
			prom_puts(". Vref = 0x");
			prom_print_hex(arVrefLevelForLog[jj],4);
			prom_puts(", ");
			#endif
            #endif

            for(kk=0; kk<TX_EYE_DELAY_LEVEL; kk++)
            {
                if(arErrorCount[ii][jj][kk] != 0xff)
                {
                    mcSHOW_DBG_MSG(("%d, ", arErrorCount[ii][jj][kk]));
					#if dbg_print
					prom_print_dec(arErrorCount[ii][jj][kk]);
					prom_puts(", ");
					#endif
                    mcFPRINTF((fp_A60501, "%8d, ", arErrorCount[ii][jj][kk]));
                     mcDELAY_US(1000);
                }
            }
            mcSHOW_DBG_MSG(("\n"));
			#if dbg_print
			prom_puts("\n");
			#endif
            mcFPRINTF((fp_A60501, "\n"));
        }
        mcSHOW_DBG_MSG(("\n"));
		#if dbg_print
		prom_puts("\n");
		#endif
        mcFPRINTF((fp_A60501, "\n"));
    }
    #endif
    return DRAM_OK;

    // log example
    /*
 ===============================================================================
     DQ TX eye scan (channel=0    dq_18)
 ===============================================================================
 0. Vref = 0000003F  1   1   1   1   1   1   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 1. Vref = 0000003B  1   1   1   1   1   1   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 2. Vref = 00000037  1   1   1   1   1   1   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 3. Vref = 00000033  1   1   1   1   1   1   0   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 4. Vref = 0000002F  1   1   1   1   1   1   0   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 5. Vref = 0000002B  1   1   1   1   1   1   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 6. Vref = 00000027  1   1   1   1   1   1   0   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 7. Vref = 00000023  1   1   1   1   1   1   0   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 8. Vref = 0000001F  1   1   1   1   1   0   0   0   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1
 9. Vref = 0000001B  1   1   1   1   1   0   0   0   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1
 10. Vref = 00000017 1   1   1   1   1   0   0   0   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1
 11. Vref = 00000013 1   1   1   1   1   0   0   0   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1
 12. Vref = 0000000F 1   1   1   1   1   0   0   0   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1
 13. Vref = 0000000B 1   1   1   1   1   0   0   0   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1
 14. Vref = 00000007 1   1   1   1   1   0   0   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 15. Vref = 00000003 1   1   1   1   1   0   0   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 16. Vref = 0000003F 1   1   1   1   1   0   0   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 17. Vref = 0000003B 1   1   1   1   1   0   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 18. Vref = 00000037 1   1   1   1   1   1   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 19. Vref = 00000033 1   1   1   1   1   0   0   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 20. Vref = 0000002F 1   1   1   1   1   1   0   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 21. Vref = 0000002B 1   1   1   1   1   1   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 22. Vref = 00000027 1   1   1   1   1   1   0   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 23. Vref = 00000023 1   1   1   1   1   1   0   0   0   0   0   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 24. Vref = 0000001F 1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 25. Vref = 0000001B 1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 26. Vref = 00000017 1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 27. Vref = 00000013 1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 28. Vref = 0000000F 1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 29. Vref = 0000000B 1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 30. Vref = 00000007 1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1
 31. Vref = 00000003 1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1
   */
#endif
}
#endif //TX_EYE_SCAN

#if DRAMC_SELFTEST_AFTER_CAL
void DramcSelftestRun(DRAMC_CTX_T *p)
{
	U32 u1testresult,u4result;
	U32 u4loop_count = 0;
	
	// disable DDRCONF0_DM64BITEN with scramble fucntion
    vIO32WriteFldAlign(DRAMC_REG_DDRCONF0, 0x0, DDRCONF0_DM64BITEN);

	vIO32WriteFldAlign(DRAMC_REG_TEST2_3, 0x12000400, RG_FLD_FULL);
	vIO32WriteFldAlign(DRAMC_REG_TEST2_0, 0x55aa, RG_FLD_FULL);
	vIO32WriteFldAlign(DRAMC_REG_TEST2_1, 0, RG_FLD_FULL);

	//cc set Length=512MB. Shall be changed if real Device capacity change
	vIO32WriteFldAlign(DRAMC_REG_TEST2_2, 0x20000000, RG_FLD_FULL); 
	
	vIO32WriteFldAlign(DRAMC_REG_TEST2_3, 0x92000400, RG_FLD_FULL);

	mcSHOW_DBG_MSG(("[DRAMC_SELFTEST_AFTER_CAL DramcSelftestRun] \n"));
	#if 1//dbg_print
	prom_puts("[DramcSelftestRun] \n");
	#endif
    while((u4IO32ReadFldAlign(DRAMC_REG_TESTRPT, TESTRPT_DM_CMP_CPT)) == 0)
    {
        //ucstatus |= ucDram_Register_Read(mcSET_(DRAMC_REG_TESTRPT), &u4value);
        mcDELAY_US(CMP_CPT_POLLING_PERIOD);
        u4loop_count++;
     //if ((u4loop_count > 3) &&(u4loop_count <= MAX_CMP_CPT_WAIT_LOOP))
        //{
            //mcSHOW_ERR_MSG(("TESTRPT_DM_CMP_CPT (Write): %d\n", u4loop_count));
        //}
        //else if(u4loop_count > MAX_CMP_CPT_WAIT_LOOP)
        //{
           /*TINFO="fcWAVEFORM_MEASURE_A %d: time out\n", u4loop_count*/
           //mcSHOW_DBG_MSG(("fcWAVEFORM_MEASURE_A %d :time out\n", u4loop_count));
          // mcFPRINTF((fp_A60501, "fcWAVEFORM_MEASURE_A %d: time out\n", u4loop_count));
            //break;
        //}
    }

	vIO32WriteFldAlign(DRAMC_REG_TEST2_3, 0x12000400, RG_FLD_FULL);
	mcDELAY_US(1);
	vIO32WriteFldAlign(DRAMC_REG_TEST2_3, 0x52000400, RG_FLD_FULL);

    u4loop_count = 0;
    while((u4IO32ReadFldAlign((DRAMC_REG_TESTRPT), TESTRPT_DM_CMP_CPT))==0)
    {
        mcDELAY_US(CMP_CPT_POLLING_PERIOD);
        u4loop_count++;
       // if ((u4loop_count > 3)&&(u4loop_count <= MAX_CMP_CPT_WAIT_LOOP))
        //{
            //mcSHOW_ERR_MSG(("TESTRPT_DM_CMP_CPT (Read): %d\n", u4loop_count));
        //}
        //else if(u4loop_count > MAX_CMP_CPT_WAIT_LOOP)
        //{
           /*TINFO="fcWAVEFORM_MEASURE_B %d: time out\n", u4loop_count*/
           //mcSHOW_DBG_MSG(("fcWAVEFORM_MEASURE_B %d: time out\n", u4loop_count));
           //mcFPRINTF((fp_A60501, "fcWAVEFORM_MEASURE_B %d: time out\n", u4loop_count));
           //break;
        //}
    }
	mcSHOW_DBG_MSG(("[DRAMC_SELFTEST_AFTER_CAL DramcSelftestRun] while end\n"));
	#if 1//dbg_print
	prom_puts("[DramcSelftestRun] while end\n");
	#endif
	mcDELAY_US(1);
  //  mcSHOW_DBG_MSG(("DRAMC_REG_TESTRPT=0x%x\n",DRAMC_REG_TESTRPT));

	u4result = u4IO32Read4B(DRAMC_REG_CMP_ERR);

	u1testresult=u4IO32ReadFldAlign((DRAMC_REG_TESTRPT), TESTRPT_DM_CMP_ERR);

	vIO32WriteFldAlign(DRAMC_REG_TEST2_3, 0x02000400, RG_FLD_FULL);
	mcDELAY_US(1);
	mcSHOW_DBG_MSG(("[DRAMC_SELFTEST_AFTER_CAL] result=%d  u4result=0x%x\n", u1testresult,u4result));
	#if 1//dbg_print
	prom_puts("[DramcSelftestRun] result=");
	prom_print_dec(u1testresult);
	prom_puts("  u4result=0x");
	prom_print_hex(u4result,8);
	prom_puts("\n");
	#endif
}

#endif


