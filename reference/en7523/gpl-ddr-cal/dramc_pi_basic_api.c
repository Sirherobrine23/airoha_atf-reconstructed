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
 * $RCSfile: pi_basic_api.c,v $
 * $Revision: #5 $
 *
 *---------------------------------------------------------------------------*/

/** @file pi_basic_api.c
 *  Basic DRAMC API implementation
 */

//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------
#include "dramc_common.h"
#include "x_hal_io.h"

#if 0//FOR_DV_SIMULATION /* add by cc - line */
#include "emi.h"
#include "emi_hw.h"

#if __FLASH_TOOL_DA__
#include "../pll/pll_dev.h"
#else
#include <pll.h>
#endif
#endif /* add by cc - line */
//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

#if REG_ACCESS_PORTING_DGB
extern U8 RegLogEnable;
#endif
extern const U32 uiDDR_PHY_Mapping[][32];

enum
{
    AC_TIMING_DRAM_TYPE=0,
    AC_TIMING_FREQUENCY,
    AC_TIMING_DBI_ONOFF,//  new
    AC_TIMING_BYTE_MODE,//  new

    AC_TIMING_TRAS,
    AC_TIMING_TRP, //5
    AC_TIMING_TRPAB,
    AC_TIMING_TRC,
    AC_TIMING_TRFC,
    AC_TIMING_TRFCPB,
    AC_TIMING_TXP, //10
    AC_TIMING_TRTP,
    AC_TIMING_TRCD,
    AC_TIMING_TWR,
    AC_TIMING_TWTR,
    AC_TIMING_TRRD,//15
    AC_TIMING_TFAW,
    AC_TIMING_TRTW_ODT_OFF,//  new (External)
    AC_TIMING_TRTW_ODT_ON,//  new (External)
    AC_TIMING_REFCNT, //(REFFRERUN = 0)
    AC_TIMING_REFCNT_FR_CLK, //(REFFRERUN = 1) //20
    AC_TIMING_TXREFCNT,
    AC_TIMING_TZQCS,
    //  new (External)
    AC_TIMING_TRTPD,
    AC_TIMING_TWTPD,
    AC_TIMING_TMRR2W_ODT_OFF, //25
    AC_TIMING_TMRR2W_ODT_ON,
    // End of  new

	//cc add for PC4 
	AC_TIMING_TWRMPR,
	AC_TIMING_BGTWTR,
	AC_TIMING_BGTCCD,
	//cc add end

    AC_TIMING_TRAS_05T, //30
    AC_TIMING_TRP_05T,
    AC_TIMING_TRPAB_05T,
    AC_TIMING_TRC_05T,
    AC_TIMING_TRFC_05T,
    AC_TIMING_TRFCPB_05T, //35
    AC_TIMING_TXP_05T,
    AC_TIMING_TRTP_05T,
    AC_TIMING_TRCD_05T,
    AC_TIMING_TWR_05T,
    AC_TIMING_TWTR_05T, //40
    AC_TIMING_TRRD_05T,
    AC_TIMING_TFAW_05T,
    AC_TIMING_TRTW_05T_ODT_OFF,
    AC_TIMING_TRTW_05T_ODT_ON,

	//cc add for PC4
	AC_TIMING_BGTWTR_05T, //45
	AC_TIMING_BGTCCD_05T,
	//cc add end

    //  new (External)
    AC_TIMING_TRTPD_05T,
    AC_TIMING_TWTPD_05T,
    // End of  new

    AC_TIMING_XRTW2W,
    AC_TIMING_XRTW2R, //50
    AC_TIMING_XRTR2W,
    AC_TIMING_XRTR2R,

    AC_TIMING_DMCATRAIN_INTV,   //  new, LP4 internal

    AC_TIMING_DQSINCTL_FOR_GATING,
    AC_TIMING_DATLAT, //55
    AC_TIMING_MODE_REG_WL,
    AC_TIMING_MODE_REG_RL,
	AC_TIMING_REFCNT_FR_CLK_20M,	//YMC add for 20MHz clock option
	
    AC_TIMING_ITEM_NUM
};

//cc add 
#if COMPILE_PCDDR3
#define AC_TIMING_NUM_PCDDR3	3
#else
#define AC_TIMING_NUM_PCDDR3	0
#endif

#if COMPILE_PCDDR4
#define AC_TIMING_NUM_PCDDR4	2
#else
#define AC_TIMING_NUM_PCDDR4	0
#endif

#define TOTAL_AC_TIMING_NUMBER	(AC_TIMING_NUM_PCDDR3 + AC_TIMING_NUM_PCDDR4)

const U16 ACTimingTable[TOTAL_AC_TIMING_NUMBER][AC_TIMING_ITEM_NUM] = {
#if COMPILE_PCDDR3
	/* PC3, DDR_1333 */
	{
		TYPE_PCDDR3,
		1333,
		DBI_OFF,
		CBT_NORMAL_MODE,

		0x4, 	/* TRAS */
		0x4,	/* TRP */
		0x0,	/* TRPAB */
		0x9,	/* TRC */
		0x6A,	/* TRFC */
		0x18,	/* TRFCPB */
		0x0,	/* TXP */
		0x2,	/* TRTP */
		0x4, 	/* TRCD */
		0xA,	/* TWR */	//YMC chg to follow spec.(1333, CL9, CWL7)
		0x6,	/* TWTR */	//YMC chg to follow spec.(1333, CL9, CWL7)
		0x2,	/* TRRD */
		0x7,	/* TFAW */
		0x2,	/* RTW(ODT_OFF) */	//YMC chg to follow spec.(1333, CL9, CWL7)
		0x2,	/* RTW(ODT_ON) */	//YMC chg to follow spec.(1333, CL9, CWL7)
		0x0,	/* REFCNT(CLKFREERUN=0) - cc notes: not used now. need confirm */
		0x60,	/* REFCNT(CLKFREERUN=1) */	//YMC chg to follow spec.(1333, CL9, CWL7)
		0xff,	/* TXREFCNT */
		0x1e,	/* TZQCS */
		0x8,	/* TRTPD */
		0x9,	/* TWTPD */
		0x1,	/* TMRRTW(ODT_OFF) */
		0x3,	/* TMRRTW(ODT_ON) */

		0x0,	/* TWRMPR */
		0x0,	/* BGTWTR */
		0x0,	/* BGTCCD */
		
	    0x0, 	/* TRAS_05T */
	    0x1, 	/* TRP_05T */
	    0x0, 	/* TRPAB_05T */
	    0x1, 	/* TRC_05T */
	    0x0, 	/* TRFC_05T */
	    0x0, 	/* TRFCPB_05T */
	    0x1, 	/* TXP_05T */
	    0x0, 	/* TRTP_05T */
	    0x1, 	/* TRCD_05T */
	    0x0, 	/* TWR_05T */
	    0x1, 	/* TWTR_05T */
	    0x0, 	/* TRRD_05T */
	    0x0, 	/* TFAW_05T */
	    0x0, 	/* TRTW_05T_ODTOFF */
	    0x0, 	/* TRTW_05T_ODTON */	    
		0x0,	/* BGTWTR_05T */
		0x0,	/* BGTCCD_05T */
		0x0,	/* TRTPD_05T */
		0x0,	/* TWTPD_05T */
		
		0x8,	/* XRTWTW */
		0x4,	/* XRTWTR -- cc notes: when RODT enabled, XRWTR shall consider timing */
		0x4,	/* XRTRTW */
		0xb,	/* XRTRTR */

		0x0,	/* DMCATRAIN -- cc notes: not used */
		
	#if TEST_ODT_ON
		0x1,
	#else
		0x2,	/* DQSINCTL */
	#endif
		0xe,	/* DATLAT */
		0x0,	/* MODE_REG_WL -- cc notes: not used now, need confirm */
		0x0,	/* MODE_REG_WL -- cc notes: not used now, need confirm */
		0x4d,	/*REFCNT_FR_CLK_20M*/	//YMC add for 20MHz clock option
	},

	/* PC3, DDR_1600 */	//YMC add 1600 AC timing setting
	{
		TYPE_PCDDR3,
		1600,
		DBI_OFF,
		CBT_NORMAL_MODE,

		0x6, 	/* TRAS */
		0x5,	/* TRP */
		0x0,	/* TRPAB */
		0xC,	/* TRC */
		0x81,	/* TRFC */
		0x18,	/* TRFCPB */
		0x1,	/* TXP */
		0x2,	/* TRTP */
		0x5, 	/* TRCD */
		0xB,	/* TWR */	
		0x7,	/* TWTR */	
		0x2,	/* TRRD */
		0x8,	/* TFAW */
		0x2,	/* RTW(ODT_OFF) */	
		0x2,	/* RTW(ODT_ON) */	
		0x0,	/* REFCNT(CLKFREERUN=0) - cc notes: not used now. need confirm */
		0x60,	/* REFCNT(CLKFREERUN=1) */	
		0xff,	/* TXREFCNT */
		0x1e,	/* TZQCS */
		0x8,	/* TRTPD */
		0x9,	/* TWTPD */
		0x1,	/* TMRRTW(ODT_OFF) */
		0x3,	/* TMRRTW(ODT_ON) */

		0x0,	/* TWRMPR */
		0x0,	/* BGTWTR */
		0x0,	/* BGTCCD */
		
	    0x0, 	/* TRAS_05T */
	    0x1, 	/* TRP_05T */
	    0x0, 	/* TRPAB_05T */
	    0x1, 	/* TRC_05T */
	    0x0, 	/* TRFC_05T */
	    0x0, 	/* TRFCPB_05T */
	    0x1, 	/* TXP_05T */
	    0x0, 	/* TRTP_05T */
	    0x1, 	/* TRCD_05T */
	    0x0, 	/* TWR_05T */
	    0x1, 	/* TWTR_05T */
	    0x0, 	/* TRRD_05T */
	    0x0, 	/* TFAW_05T */
	    0x0, 	/* TRTW_05T_ODTOFF */
	    0x0, 	/* TRTW_05T_ODTON */	    
		0x0,	/* BGTWTR_05T */
		0x0,	/* BGTCCD_05T */
		0x0,	/* TRTPD_05T */
		0x0,	/* TWTPD_05T */
		
		0x8,	/* XRTWTW */
		0x4,	/* XRTWTR -- cc notes: when RODT enabled, XRWTR shall consider timing */
		0x4,	/* XRTRTW */
		0xb,	/* XRTRTR */

		0x0,	/* DMCATRAIN -- cc notes: not used */
		
	#if TEST_ODT_ON
		0x1,
	#else
		0x2,	/* DQSINCTL */
	#endif
		0xe,	/* DATLAT */
		0x0,	/* MODE_REG_WL -- cc notes: not used now, need confirm */
		0x0,	/* MODE_REG_WL -- cc notes: not used now, need confirm */
		0x4d,	/*REFCNT_FR_CLK_20M*/	//YMC add for 20MHz clock option
	},
	
	/* PC3, DDR_1866 */
	{
		TYPE_PCDDR3,
		1866,
		DBI_OFF,
		CBT_NORMAL_MODE,

		0x8,//6 	/* TRAS */
		0x6,	/* TRP */	//YMC chg to follow spec.(1866, CL13, CWL9)
		0x0,	/* TRPAB */
		0xF,	/* TRC */	//YMC chg to follow spec.(1866, CL13, CWL9)
		0x98,	/* TRFC */	//YMC chg to follow spec.(1866, CL13, CWL9)
		0x18,	/* TRFCPB */
		0x1,	/* TXP */
		0x3,	/* TRTP */
		0x6, 	/* TRCD */	//YMC chg to follow spec.(1866, CL13, CWL9)
		0xd,//B	/* TWR */
		0x8,	/* TWTR */
		0x2,	/* TRRD */
		0x9,	/* TFAW */
		0x3,	/* RTW(ODT_OFF) */	//YMC chg to follow spec.(1866, CL13, CWL9)
		0x3,	/* RTW(ODT_ON) */	//YMC chg to follow spec.(1866, CL13, CWL9)
		0x0,	/* REFCNT(CLKFREERUN=0) - cc notes: not used now. need confirm */
		0x60,	/* REFCNT(CLKFREERUN=1) */	//YMC chg to follow spec.(1866, CL13, CWL9)
		0xff,	/* TXREFCNT */
		0x24,	/* TZQCS */	//YMC chg to follow spec.(1866, CL13, CWL9)
		0x8,	/* TRTPD */
		0x9,	/* TWTPD */
		0x1,	/* TMRRTW(ODT_OFF) */
		0x3,	/* TMRRTW(ODT_ON) */

		0x0,	/* TWRMPR */
		0x0,	/* BGTWTR */
		0x0,	/* BGTCCD */
		
	    0x0, 	/* TRAS_05T */
	    0x1, 	/* TRP_05T */
	    0x0, 	/* TRPAB_05T */
	    0x1, 	/* TRC_05T */
	    0x0, 	/* TRFC_05T */
	    0x0, 	/* TRFCPB_05T */
	    0x1, 	/* TXP_05T */
	    0x0, 	/* TRTP_05T */
	    0x1, 	/* TRCD_05T */
	    0x0, 	/* TWR_05T */
	    0x1, 	/* TWTR_05T */
	    0x0, 	/* TRRD_05T */
	    0x0, 	/* TFAW_05T */
	    0x0, 	/* TRTW_05T_ODTOFF */
	    0x0, 	/* TRTW_05T_ODTON */
		0x0,	/* BGTWTR_05T */
		0x0,	/* BGTCCD_05T */
		0x0,	/* TRTPD_05T */
		0x0,	/* TWTPD_05T */
		
		0x8,	/* XRTWTW */
		0x4,	/* XRTWTR -- cc notes: when RODT enabled, XRWTR shall consider timing */
		0x5,	/* XRTRTW */
		0xb,	/* XRTRTR */
		
		0x0,	/* DMCATRAIN -- cc notes: not used */

#if TEST_ODT_ON
		0x1,
#else
		0x2,	/* DQSINCTL */
#endif
		0xe,	/* DATLAT */
		0x0,	/* MODE_REG_WL -- cc notes: not used now, need confirm */
		0x0,	/* MODE_REG_WL -- cc notes: not used now, need confirm */
		0x4d,	/*REFCNT_FR_CLK_20M*/	//YMC add for 20MHz clock option
	},
#endif
};
//cc add end

#if ENABLE_AUTO_DETECTION
void vDramcACTimingOptimize(DRAMC_CTX_T *p)
{
    U8 u1TRFC=0, u1TRFCpb=0, u1TRFC_05T=0, u1TXREFCNT=0;

    if(p->density !=0)  //already read MR8 for density update
    {
        switch(p->density)
        {
            case 0x6://4Gb per die  tRFCab=130, tRFCpb = 60
                u1TRFC = 41;
				u1TRFCpb = 13;
                u1TRFC_05T=0;
                u1TXREFCNT = 54;
                break;

            case 0xE://6Gb, tRFCab=210, tRFCpb = 90
            case 0x7://8Gb, tRFCab=210, tRFCpb = 90
            case 0xD://12Gb, tRFCab=210, tRFCpb = 90
            case 0x8://16Gb, tRFCab=210, tRFCpb = 90
            default:
                u1TRFC = 73;
				u1TRFCpb = 25;
                u1TRFC_05T=0;
                u1TXREFCNT = 86;
        }

        vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM3, P_Fld(u1TRFC, SHU_ACTIM3_TRFC)	|
									P_Fld(u1TRFCpb, SHU_ACTIM3_TRFCPB));
        vIO32WriteFldAlign(DRAMC_REG_SHU_AC_TIME_05T, u1TRFC_05T, SHU_AC_TIME_05T_TRFC_05T);
        vIO32WriteFldAlign(DRAMC_REG_SHU_ACTIM4, u1TXREFCNT, SHU_ACTIM4_TXREFCNT);
    }

    mcSHOW_DBG_MSG(("[vDramcACTimingOptimize] Density %d, u1TRFC %d, u1TRFCpb %d, u1TRFC_05T %d, u1TXREFCNT %d\n", p->density, u1TRFC, u1TRFCpb, u1TRFC_05T, u1TXREFCNT));
}
#endif

#if ENABLE_AUTO_DETECTION /* add by cc - line */
void vDramcUpdateEmiSetting(DRAMC_CTX_T *p, DRAM_INFO_BY_MRR_T *DramInfo)
{
    U8 u1RankIdx, u1Row, u1Column;
    U32 uiTmp = 0;
    EMI_SETTINGS *emi_set = mt_get_emisettings();

    if (((U64)DramInfo->u4MR8Density[0] + (U64)DramInfo->u4MR8Density[1]) > 0xC0000000)
    {
        DramInfo->u4MR8Density[1] >>= 1;
        mcSHOW_DBG_MSG(("Not support 4GB, can use only 3GB\n"));
    }

    emi_set->DRAM_RANK_SIZE[0] = DramInfo->u4MR8Density[0];
    emi_set->DRAM_RANK_SIZE[1] = DramInfo->u4MR8Density[1];
    emi_set->EMI_CONA_VAL = (1<<1) | ((p->support_rank_num-1) << 17);

    for(u1RankIdx = 0; u1RankIdx< p->support_rank_num; u1RankIdx++)
    {
        uiTmp = DramInfo->u4MR8Density[u1RankIdx];
        //detect CONA
        switch (uiTmp >> DramInfo->u4MR8DieNumber[u1RankIdx])
        {
            //6Gb/8Gb
            case 0x30000000:
            case 0x40000000:
                u1Row = 2;
                u1Column = 1 + DramInfo->u4MR8DieNumber[u1RankIdx];
            break;
            //12Gb/16Gb
            case 0x60000000:
            case 0x80000000:
                u1Row = 2;
                u1Column = 2 + DramInfo->u4MR8DieNumber[u1RankIdx];
            break;
            //4Gb
            case 0x20000000://4
            default:
                u1Row = 1;
                u1Column = 1 + DramInfo->u4MR8DieNumber[u1RankIdx];
            break;
        }

        emi_set->EMI_CONA_VAL |= (u1Column & 0x3) << (4+(u1RankIdx<<1));
        emi_set->EMI_CONA_VAL |= (u1Row & 0x3) << (12+(u1RankIdx<<1));

        //detect CONH
        if (uiTmp == 0x30000000 || uiTmp == 0x60000000) //6Gb or 12Gb
        {
            emi_set->EMI_CONH_VAL &= ~(0xf << (16 + u1RankIdx*4));
            emi_set->EMI_CONH_VAL |= ((uiTmp>>12) << u1RankIdx*4);
        }
    }

    *(volatile unsigned *)EMI_CONA = emi_set->EMI_CONA_VAL;
    *(volatile unsigned *)EMI_CONH = emi_set->EMI_CONH_VAL;
    mcSHOW_DBG_MSG(("[vDramcUpdateEmiSetting] EMI_CONA 0x%x, EMI_CONH 0x%x\n", *(volatile unsigned *)EMI_CONA, *(volatile unsigned *)EMI_CONH));

}
#endif

void v4bitPinmuxEnableVerify(DRAMC_CTX_T *p)
{
#if COMPILE_THIS_PART
    U8 backup_rank, u1RankIdx;
    U32 u4reg, u4B0, u4B1, u4B2, u4B3, u4Idx;

    if (p->en_4bitMux == DISABLE)
        return;

    backup_rank = u1GetRank(p);
    for(u1RankIdx=0; u1RankIdx<p->support_rank_num; u1RankIdx++)
    {
        vSetRank(p, u1RankIdx);
        for (u4Idx=DRAMC_REG_SHURK0_SELPH_DQ0; u4Idx<=DRAMC_REG_SHURK0_SELPH_DQ3; u4Idx+=4)
        {
            //DQ 2T, DQM 2T, DQ 0.5T, DQM 0.5T
            u4reg = u4IO32Read4B(u4Idx);
            u4B0 = u4reg & 0x00070007;
            u4B1 = (u4reg & 0x00700070) >> 4;
            u4B2 = u4reg & 0x07000700;
            u4B3 = (u4reg & 0x70007000) >> 4;

            if ((u4B0 != u4B1) || (u4B2 != u4B3))
            {
                mcSHOW_ERR_MSG(("v4bitPinmuxEnableVerify reg[%d]=0x%x\n", u4Idx, u4reg));
                ASSERT(0);
            }
        }
    }

    vSetRank(p, backup_rank);
#endif
}

DRAM_STATUS_T DdrUpdateACTimingReg(DRAMC_CTX_T *p, const U32* ACTable)
{
#if COMPILE_THIS_PART
    if(ACTable == NULL)
        return DRAM_FAIL;

    vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM0,
                                    P_Fld( ACTable[AC_TIMING_TRCD], SHU_ACTIM0_TRCD)   |
                                    P_Fld( ACTable[AC_TIMING_TRRD], SHU_ACTIM0_TRRD)    |
                                    P_Fld( ACTable[AC_TIMING_TWR], SHU_ACTIM0_TWR)      |
                                    P_Fld( ACTable[AC_TIMING_TWTR], SHU_ACTIM0_TWTR));

    vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM1,
                                    P_Fld( ACTable[AC_TIMING_TRC], SHU_ACTIM1_TRC)         |
                                    P_Fld( ACTable[AC_TIMING_TRAS], SHU_ACTIM1_TRAS)     |
                                    P_Fld( ACTable[AC_TIMING_TRP] ,SHU_ACTIM1_TRP)         |
                                    P_Fld( ACTable[AC_TIMING_TRPAB], SHU_ACTIM1_TRPAB));


    vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM2,
                                    P_Fld( ACTable[AC_TIMING_TFAW], SHU_ACTIM2_TFAW)         |
                                    P_Fld( ACTable[AC_TIMING_TRTW_ODT_OFF + p->odt_onoff], SHU_ACTIM2_TR2W)               |
                                    P_Fld( ACTable[AC_TIMING_TRTP], SHU_ACTIM2_TRTP)         |
                                    P_Fld( ACTable[AC_TIMING_TXP], SHU_ACTIM2_TXP));

    vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM3,
                                    P_Fld( ACTable[AC_TIMING_REFCNT], SHU_ACTIM3_REFCNT)	 |
                                    P_Fld( ACTable[AC_TIMING_TRFC], SHU_ACTIM3_TRFC)         |
                                    P_Fld( ACTable[AC_TIMING_TRFCPB], SHU_ACTIM3_TRFCPB));

    vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM4,
                                    P_Fld( ACTable[AC_TIMING_TZQCS], SHU_ACTIM4_TZQCS)         |
                                    P_Fld( ACTable[AC_TIMING_REFCNT_FR_CLK], SHU_ACTIM4_REFCNT_FR_CLK)               |
                                    P_Fld( ACTable[AC_TIMING_TXREFCNT], SHU_ACTIM4_TXREFCNT));

    vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM5,
                                    P_Fld( ACTable[AC_TIMING_TMRR2W_ODT_OFF+ p->odt_onoff], SHU_ACTIM5_TMRR2W) |
                                    P_Fld( ACTable[AC_TIMING_TWTPD], SHU_ACTIM5_TWTPD) |
                                    P_Fld( ACTable[AC_TIMING_TRTPD], SHU_ACTIM5_TR2PD));

    // AC timing 0.5T
    vIO32WriteFldMulti(DRAMC_REG_SHU_AC_TIME_05T,
                                    P_Fld(ACTable[AC_TIMING_TWTR_05T], SHU_AC_TIME_05T_TWTR_M05T)  |
                                    P_Fld(ACTable[AC_TIMING_TRTW_05T_ODT_OFF+ p->odt_onoff],        SHU_AC_TIME_05T_TR2W_05T)           |
                                    P_Fld(ACTable[AC_TIMING_TWTPD_05T],       SHU_AC_TIME_05T_TWTPD_M05T ) |
                                    P_Fld(ACTable[AC_TIMING_TRTPD_05T],     SHU_AC_TIME_05T_TR2PD_05T)         |
                                    P_Fld(ACTable[AC_TIMING_TFAW_05T],        SHU_AC_TIME_05T_TFAW_05T)               |
                                    P_Fld(ACTable[AC_TIMING_TRRD_05T],       SHU_AC_TIME_05T_TRRD_05T)         |
                                    P_Fld(ACTable[AC_TIMING_TWR_05T],   SHU_AC_TIME_05T_TWR_M05T)           |
                                    P_Fld(ACTable[AC_TIMING_TRAS_05T] ,    SHU_AC_TIME_05T_TRAS_05T ) |
                                    P_Fld(ACTable[AC_TIMING_TRPAB_05T],   SHU_AC_TIME_05T_TRPAB_05T)         |
                                    P_Fld(ACTable[AC_TIMING_TRP_05T],   SHU_AC_TIME_05T_TRP_05T)               |
                                    P_Fld(ACTable[AC_TIMING_TRCD_05T] ,  SHU_AC_TIME_05T_TRCD_05T)         |
                                    P_Fld(ACTable[AC_TIMING_TRTP_05T],   SHU_AC_TIME_05T_TRTP_05T)        |
                                    P_Fld(ACTable[AC_TIMING_TXP_05T],   SHU_AC_TIME_05T_TXP_05T)         |
                                    P_Fld(ACTable[AC_TIMING_TRFC_05T],   SHU_AC_TIME_05T_TRFC_05T)               |
                                    P_Fld(ACTable[AC_TIMING_TRFCPB_05T] ,  SHU_AC_TIME_05T_TRFCPB_05T)         |
                                    P_Fld(ACTable[AC_TIMING_TRC_05T],   SHU_AC_TIME_05T_TRC_05T));
#endif
	return DRAM_OK;
}

#if COMPILE_THIS_PART
DRAM_STATUS_T DdrUpdateACTiming_EMI(DRAMC_CTX_T *p)
{
    U8 u1TimingIdx=0xff, u1TmpIdx;
    U32 ACTiming[AC_TIMING_ITEM_NUM];
    U8 u1TmpDramType = p->dram_type;
	EMI_SETTINGS *emi_set = mt_get_emisettings();
	AC_TIMING_EXTERNAL_T *ACRegFromEmi = &(emi_set->AcTimeEMI);

   if(ACRegFromEmi == NULL)
        return DRAM_FAIL;

	mcSHOW_DBG_MSG2(("[DdrUpdateACTiming_EMI] ====Begin====\n"));

    //Get AC timing from emi setting
    ACTiming[AC_TIMING_DRAM_TYPE] =  p->dram_type;
    ACTiming[AC_TIMING_FREQUENCY] =  ACRegFromEmi->AC_TIME_EMI_FREQUENCY ;
    ACTiming[AC_TIMING_TRAS] =  ACRegFromEmi->AC_TIME_EMI_TRAS ;
    ACTiming[AC_TIMING_TRP] =  ACRegFromEmi->AC_TIME_EMI_TRP ;

    ACTiming[AC_TIMING_TRPAB] =  ACRegFromEmi->AC_TIME_EMI_TRPAB ;
    ACTiming[AC_TIMING_TRC] =  ACRegFromEmi->AC_TIME_EMI_TRC ;
    ACTiming[AC_TIMING_TRFC] =  ACRegFromEmi->AC_TIME_EMI_TRFC ;
    ACTiming[AC_TIMING_TRFCPB] =  ACRegFromEmi->AC_TIME_EMI_TRFCPB ;

    ACTiming[AC_TIMING_TXP] =  ACRegFromEmi->AC_TIME_EMI_TXP ;
    ACTiming[AC_TIMING_TRTP] =  ACRegFromEmi->AC_TIME_EMI_TRTP ;
    ACTiming[AC_TIMING_TRCD] =  ACRegFromEmi->AC_TIME_EMI_TRCD ;
    ACTiming[AC_TIMING_TWR] =  ACRegFromEmi->AC_TIME_EMI_TWR ;
    ACTiming[AC_TIMING_TWTR] =  ACRegFromEmi->AC_TIME_EMI_TWTR ;
    ACTiming[AC_TIMING_TRRD] =  ACRegFromEmi->AC_TIME_EMI_TRRD ;
    ACTiming[AC_TIMING_TFAW] =  ACRegFromEmi->AC_TIME_EMI_TFAW ;
    ACTiming[AC_TIMING_TRTW_ODT_OFF] =  ACRegFromEmi->AC_TIME_EMI_TRTW_ODT_OFF ;
    ACTiming[AC_TIMING_TRTW_ODT_ON] =  ACRegFromEmi->AC_TIME_EMI_TRTW_ODT_ON ;
    ACTiming[AC_TIMING_REFCNT] =  ACRegFromEmi->AC_TIME_EMI_REFCNT ; //(REFFRERUN = 0)
    ACTiming[AC_TIMING_REFCNT_FR_CLK] =  ACRegFromEmi->AC_TIME_EMI_REFCNT_FR_CLK ; //(REFFRERUN = 1)
    ACTiming[AC_TIMING_TXREFCNT] =  ACRegFromEmi->AC_TIME_EMI_TXREFCNT ;
    ACTiming[AC_TIMING_TZQCS] =  ACRegFromEmi->AC_TIME_EMI_TZQCS ;

    ACTiming[AC_TIMING_TRTPD] =  ACRegFromEmi->AC_TIME_EMI_TRTPD ;// oylmpus new
    ACTiming[AC_TIMING_TWTPD] =  ACRegFromEmi->AC_TIME_EMI_TWTPD ;// oylmpus new
    ACTiming[AC_TIMING_TMRR2W_ODT_OFF] =  ACRegFromEmi->AC_TIME_EMI_TMRR2W_ODT_OFF ;// oylmpus new
    ACTiming[AC_TIMING_TMRR2W_ODT_ON] =  ACRegFromEmi->AC_TIME_EMI_TMRR2W_ODT_ON;// oylmpus new

    ACTiming[AC_TIMING_TRAS_05T] =  ACRegFromEmi->AC_TIME_EMI_TRAS_05T ;
    ACTiming[AC_TIMING_TRP_05T] =  ACRegFromEmi->AC_TIME_EMI_TRP_05T ;
    ACTiming[AC_TIMING_TRPAB_05T] =  ACRegFromEmi->AC_TIME_EMI_TRPAB_05T ;
    ACTiming[AC_TIMING_TRC_05T] =  ACRegFromEmi->AC_TIME_EMI_TRC_05T ;
    ACTiming[AC_TIMING_TRFC_05T] =  ACRegFromEmi->AC_TIME_EMI_TRFC_05T ;
    ACTiming[AC_TIMING_TRFCPB_05T] =  ACRegFromEmi->AC_TIME_EMI_TRFCPB_05T ;
    ACTiming[AC_TIMING_TXP_05T] =  ACRegFromEmi->AC_TIME_EMI_TXP_05T ;
    ACTiming[AC_TIMING_TRTP_05T] =  ACRegFromEmi->AC_TIME_EMI_TRTP_05T ;
    ACTiming[AC_TIMING_TRCD_05T] =  ACRegFromEmi->AC_TIME_EMI_TRCD_05T ;
    ACTiming[AC_TIMING_TWR_05T] =  ACRegFromEmi->AC_TIME_EMI_TWR_05T ;
    ACTiming[AC_TIMING_TWTR_05T] =  ACRegFromEmi->AC_TIME_EMI_TWTR_05T ;

    ACTiming[AC_TIMING_TRRD_05T] =  ACRegFromEmi->AC_TIME_EMI_TRRD_05T ;
    ACTiming[AC_TIMING_TFAW_05T] =  ACRegFromEmi->AC_TIME_EMI_TFAW_05T ;
    ACTiming[AC_TIMING_TRTW_05T_ODT_OFF] =  ACRegFromEmi->AC_TIME_EMI_TRTW_ODT_OFF_05T ;
    ACTiming[AC_TIMING_TRTW_05T_ODT_ON] =  ACRegFromEmi->AC_TIME_EMI_TRTW_ODT_ON_05T ;

    ACTiming[AC_TIMING_TRTPD_05T] =  ACRegFromEmi->AC_TIME_EMI_TRTPD_05T ;
    ACTiming[AC_TIMING_TWTPD_05T] =  ACRegFromEmi->AC_TIME_EMI_TWTPD_05T ;

    DdrUpdateACTimingReg(p, ACTiming);

	mcSHOW_DBG_MSG2(("[DdrUpdateACTiming_EMI] ====done====\n"));

    return DRAM_OK;
}
#endif

#ifdef FIRST_BRING_UP
#if COMPILE_THIS_PART
void freq_meter_test_mode(void)
{
	U32 val = 0;
	vIO32WriteFldAlign(DDRPHY_SHU1_PLL9, 0x1, SHU1_PLL9_RG_RPHYPLL_MONCK_EN);
	vIO32WriteFldAlign(DDRPHY_SHU1_PLL11, 0x1, SHU1_PLL11_RG_RCLRPLL_MONCK_EN);
	val = u4IO32ReadFldAlign(DDRPHY_SHU1_PLL0, SHU1_PLL0_RG_RPHYPLL_TOP_REV);
	val |= (0xf<<8);
	vIO32WriteFldAlign(DDRPHY_SHU1_PLL0, val, SHU1_PLL0_RG_RPHYPLL_TOP_REV);
	vIO32WriteFldAlign(DDRPHY_SHU1_PLL9, 0x1, SHU1_PLL9_RG_RPHYPLL_MONREF_EN);
	vIO32WriteFldAlign(DDRPHY_SHU1_PLL11, 0x1, SHU1_PLL11_RG_RCLRPLL_MONREF_EN);
	val = u4IO32ReadFldAlign(DDRPHY_SHU1_CA_CMD7, SHU1_CA_CMD7_RG_ARCMD_REV);
	val |= (1<<4);
	vIO32WriteFldAlign(DDRPHY_SHU1_CA_CMD7, val, SHU1_CA_CMD7_RG_ARCMD_REV);
	val = u4IO32ReadFldAlign(DDRPHY_SHU1_B0_DQ7, SHU1_B0_DQ7_RG_ARDQ_REV_B0);
	val |= (1<<4);
	vIO32WriteFldAlign(DDRPHY_SHU1_B0_DQ7, val, SHU1_B0_DQ7_RG_ARDQ_REV_B0);
	val = u4IO32ReadFldAlign(DDRPHY_SHU1_B1_DQ7, SHU1_B1_DQ7_RG_ARDQ_REV_B1);
	val |= (1<<4);
	vIO32WriteFldAlign(DDRPHY_SHU1_B1_DQ7, val, SHU1_B1_DQ7_RG_ARDQ_REV_B1);
	val = u4IO32ReadFldAlign(DDRPHY_SHU1_B0_DQ7+(1<<POS_BANK_NUM), SHU1_B0_DQ7_RG_ARDQ_REV_B0);
	val |= (1<<4);
	vIO32WriteFldAlign(DDRPHY_SHU1_B0_DQ7+(1<<POS_BANK_NUM), val, SHU1_B0_DQ7_RG_ARDQ_REV_B0);
	val = u4IO32ReadFldAlign(DDRPHY_SHU1_B1_DQ7+(1<<POS_BANK_NUM), SHU1_B1_DQ7_RG_ARDQ_REV_B1);
	val |= (1<<4);
	vIO32WriteFldAlign(DDRPHY_SHU1_B1_DQ7+(1<<POS_BANK_NUM), val, SHU1_B1_DQ7_RG_ARDQ_REV_B1);
	vIO32WriteFldAlign(DDRPHY_MISC_VREF_CTRL, 0x0 ,MISC_VREF_CTRL_RG_RVREF_VREF_EN);
	vIO32WriteFldMulti(DDRPHY_PLL3, P_Fld(0x0, PLL3_RG_RPHYPLL_TST_SEL)
				| P_Fld(0x1, PLL3_RG_RPHYPLL_TSTLVROD_EN)
				| P_Fld(0x1, PLL3_RG_RPHYPLL_TST_EN)
				| P_Fld(0x1, PLL3_RG_RPHYPLL_TSTCK_EN)
				| P_Fld(0x1, PLL3_RG_RPHYPLL_TSTFM_EN)
				| P_Fld(0x1, PLL3_RG_RPHYPLL_TSTOD_EN)
				| P_Fld(0x0, PLL3_RG_RPHYPLL_TSTOP_EN));
}

void freq_meter_test_mode_dc(void)
{
	vIO32WriteFldAlign(DDRPHY_SHU1_PLL9, 0x1, SHU1_PLL9_RG_RPHYPLL_MONVC_EN);
	vIO32WriteFldAlign(DDRPHY_SHU1_PLL11, 0x1, SHU1_PLL11_RG_RCLRPLL_MONVC_EN);
	vIO32WriteFldMulti(DDRPHY_MISC_VREF_CTRL, P_Fld(0x1, MISC_VREF_CTRL_RG_RVREF_VREF_EN)
                | P_Fld(0x1, MISC_VREF_CTRL_RG_RVREF_DDR3_SEL)
                | P_Fld(0x0, MISC_VREF_CTRL_RG_RVREF_DDR4_SEL)
                | P_Fld(0xe, MISC_VREF_CTRL_RG_RVREF_SEL_DQ));
	vIO32WriteFldMulti(DDRPHY_PLL3, P_Fld(0x1, PLL3_RG_RPHYPLL_TST_SEL)
				| P_Fld(0x0, PLL3_RG_RPHYPLL_TSTLVROD_EN)
				| P_Fld(0x1, PLL3_RG_RPHYPLL_TST_EN)
				| P_Fld(0x0, PLL3_RG_RPHYPLL_TSTCK_EN)
				| P_Fld(0x0, PLL3_RG_RPHYPLL_TSTFM_EN)
				| P_Fld(0x0, PLL3_RG_RPHYPLL_TSTOD_EN)
				| P_Fld(0x1, PLL3_RG_RPHYPLL_TSTOP_EN));
}
#endif
#endif

void Set_MRR_Pinmux_Mapping(DRAMC_CTX_T *p)
{
#if COMPILE_THIS_PART //cc notes: seems that only for LPXXX need to do this
    U32 uiLPDDR_MRR_Mapping[16], i;

    if (p->dram_type != TYPE_LPDDR3 && p->dram_type != TYPE_LPDDR2)
    {
    	return;
    }

    memcpy(uiLPDDR_MRR_Mapping, (U32 *)uiDDR_PHY_Mapping[(U8)(p->pinmux)], sizeof(uiLPDDR_MRR_Mapping));               

    if (p->en_4bitMux == ENABLE)
    {
        for (i=0; i<sizeof(uiLPDDR_MRR_Mapping)/sizeof(uiLPDDR_MRR_Mapping[0]); i++)
        {
            if ((uiLPDDR_MRR_Mapping[i] >= 4 && uiLPDDR_MRR_Mapping[i] <= 7) || (uiLPDDR_MRR_Mapping[i] >= 20 && uiLPDDR_MRR_Mapping[i] <= 23))
                uiLPDDR_MRR_Mapping[i] += 4;
            else if ((uiLPDDR_MRR_Mapping[i] >= 8 && uiLPDDR_MRR_Mapping[i] <= 11) || (uiLPDDR_MRR_Mapping[i] >= 24 && uiLPDDR_MRR_Mapping[i] <= 27))
                uiLPDDR_MRR_Mapping[i] -= 4;
            
            mcSHOW_DBG_MSG(("%d ", uiLPDDR_MRR_Mapping[i]));
        }

        mcSHOW_DBG_MSG(("\n"));
    }

    //Set MRR pin mux
    vIO32WriteFldMulti(DRAMC_REG_MRR_BIT_MUX1, P_Fld( uiLPDDR_MRR_Mapping[0], MRR_BIT_MUX1_MRR_BIT0_SEL) | P_Fld( uiLPDDR_MRR_Mapping[1], MRR_BIT_MUX1_MRR_BIT1_SEL) |
                                                               P_Fld( uiLPDDR_MRR_Mapping[2], MRR_BIT_MUX1_MRR_BIT2_SEL) | P_Fld( uiLPDDR_MRR_Mapping[3], MRR_BIT_MUX1_MRR_BIT3_SEL));
    vIO32WriteFldMulti(DRAMC_REG_MRR_BIT_MUX2, P_Fld( uiLPDDR_MRR_Mapping[4], MRR_BIT_MUX2_MRR_BIT4_SEL) | P_Fld( uiLPDDR_MRR_Mapping[5], MRR_BIT_MUX2_MRR_BIT5_SEL) |
                                                               P_Fld( uiLPDDR_MRR_Mapping[6], MRR_BIT_MUX2_MRR_BIT6_SEL) | P_Fld( uiLPDDR_MRR_Mapping[7], MRR_BIT_MUX2_MRR_BIT7_SEL));
    vIO32WriteFldMulti(DRAMC_REG_MRR_BIT_MUX3, P_Fld( uiLPDDR_MRR_Mapping[8], MRR_BIT_MUX3_MRR_BIT8_SEL) | P_Fld( uiLPDDR_MRR_Mapping[9], MRR_BIT_MUX3_MRR_BIT9_SEL) |
                                                               P_Fld( uiLPDDR_MRR_Mapping[10], MRR_BIT_MUX3_MRR_BIT10_SEL) | P_Fld(uiLPDDR_MRR_Mapping[11], MRR_BIT_MUX3_MRR_BIT11_SEL));
    vIO32WriteFldMulti(DRAMC_REG_MRR_BIT_MUX4, P_Fld( uiLPDDR_MRR_Mapping[12], MRR_BIT_MUX4_MRR_BIT12_SEL) | P_Fld(uiLPDDR_MRR_Mapping[13], MRR_BIT_MUX4_MRR_BIT13_SEL) |
                                                              P_Fld( uiLPDDR_MRR_Mapping[14], MRR_BIT_MUX4_MRR_BIT14_SEL) | P_Fld(uiLPDDR_MRR_Mapping[15], MRR_BIT_MUX4_MRR_BIT15_SEL));
#endif
}

void Global_Option_Init(DRAMC_CTX_T *p)
{
    SaveCurrDramCtx(p);
    vSetRankNumber(p);
    Set_MRR_Pinmux_Mapping(p);
}

DRAM_STATUS_T SscEnable(DRAMC_CTX_T *p)
{
    if (p->ssc_en == ENABLE)
    {
        mcSHOW_DBG_MSG(("Enable SSC\n"));
		#if 1//dbg_print
		prom_puts("Enable SSC\n");
		#endif
        vIO32WriteFldAlign(DDRPHY_PLL7, 1, PLL7_RG_RCLRPLL_SDM_SSC_EN);
    }
    return DRAM_OK;
}

#if COMPILE_THIS_PART
static U32 DramCKGen_Cal(DRAMC_CTX_T *p)
{
    U32 u4dll_pd_status, u4delay_code,u4Result;


        u4dll_pd_status = u4IO32ReadFldAlign(DDRPHY_MISC_APHY_PD_STATUS, MISC_APHY_PD_STATUS_ARCKGEN_DLL_PD_EN_CA);
	 mcSHOW_DBG_MSG(("[DramCKGen_Cal] u4dll_pd_status=%d\n",u4dll_pd_status));

	if(u4dll_pd_status==0)
		{
		for(u4delay_code=SW_CAL_CKGEN_STAET;u4delay_code>0;u4delay_code--)
			{
			   // vIO32WriteFldAlign((DDRPHY_CA_CKGEN_DLL0), u4delay_code, CA_CKGEN_DLL0_RG_ARCKGEN_DLL_DELAY_CODE_CA);
			    vIO32WriteFldAlign((DDRPHY_SHU1_CA_CMD6), u4delay_code+0x48, SHU1_CA_CMD6_RG_ARPI_CAP_SEL_CA);
			        u4dll_pd_status = u4IO32ReadFldAlign(DDRPHY_MISC_APHY_PD_STATUS, MISC_APHY_PD_STATUS_ARCKGEN_DLL_PD_EN_CA);
			    if(u4dll_pd_status==1)
			    	{
			    	u4Result=DRAM_OK;
			        mcSHOW_DBG_MSG(("[DramCKGen_Cal_0] Pass  u4delay_code=%x  u4dll_pd_status=%d\n",u4delay_code,u4dll_pd_status));
			    	break;
			    	}
			    else if(u4dll_pd_status==0 && u4delay_code==0)
			    	{
			        mcSHOW_DBG_MSG(("[DramCKGen_Cal_0] fail  u4delay_code=%x  u4dll_pd_status=%d\n",u4delay_code,u4dll_pd_status));
			    	u4Result=DRAM_FAIL;
			    	}
			        mcSHOW_DBG_MSG(("[DramCKGen_Cal_0] u4delay_code=%x  u4dll_pd_status=%d\n",u4delay_code,u4dll_pd_status));

			}
		}
	else
		{
		for(u4delay_code=SW_CAL_CKGEN_STAET;u4delay_code<0xFF;u4delay_code++)
			{
			    //vIO32WriteFldAlign((DDRPHY_CA_CKGEN_DLL0), u4delay_code, CA_CKGEN_DLL0_RG_ARCKGEN_DLL_DELAY_CODE_CA);
			    vIO32WriteFldAlign((DDRPHY_SHU1_CA_CMD6), u4delay_code+0x48, SHU1_CA_CMD6_RG_ARPI_CAP_SEL_CA);
			        u4dll_pd_status = u4IO32ReadFldAlign(DDRPHY_MISC_APHY_PD_STATUS, MISC_APHY_PD_STATUS_ARCKGEN_DLL_PD_EN_CA);
			    if(u4dll_pd_status==0)
			    	{
			    	u4Result=DRAM_OK;
			        mcSHOW_DBG_MSG(("[DramCKGen_Cal_1] Pass  u4delay_code=%x  u4dll_pd_status=%d\n",u4delay_code,u4dll_pd_status));
			    	break;
			    	}
			    else if(u4dll_pd_status==1 && u4delay_code==0xFF)
			    	{
			        mcSHOW_DBG_MSG(("[DramCKGen_Cal_1] fail  u4delay_code=%x  u4dll_pd_status=%d\n",u4delay_code,u4dll_pd_status));
			    	u4Result=DRAM_FAIL;
			    	}
			        mcSHOW_DBG_MSG(("[DramCKGen_Cal_0] u4delay_code=%x  u4dll_pd_status=%d\n",u4delay_code,u4dll_pd_status));

			}
		}

        u4dll_pd_status = u4IO32ReadFldAlign(DDRPHY_MISC_APHY_PD_STATUS, MISC_APHY_PD_STATUS_ARCKGEN_DLL_PD_EN_B01);
	 mcSHOW_DBG_MSG(("[DramCKGen_Cal] u4dll_pd_status=%d\n",u4dll_pd_status));

	if(u4dll_pd_status==0)
		{
		for(u4delay_code=SW_CAL_CKGEN_STAET;u4delay_code>0;u4delay_code--)
			{
			    vIO32WriteFldAlign((DDRPHY_B0_CKGEN_DLL0), u4delay_code, B0_CKGEN_DLL0_RG_ARCKGEN_DLL_DELAY_CODE_B01);
			    vIO32WriteFldAlign((DDRPHY_SHU1_B0_DQ6), u4delay_code+0x48, SHU1_B0_DQ6_RG_ARPI_CAP_SEL_B0);
			        u4dll_pd_status = u4IO32ReadFldAlign(DDRPHY_MISC_APHY_PD_STATUS, MISC_APHY_PD_STATUS_ARCKGEN_DLL_PD_EN_B01);
			    if(u4dll_pd_status==1)
			    	{
			    	u4Result=DRAM_OK;
			        mcSHOW_DBG_MSG(("[DramCKGen_Cal_0] Pass  u4delay_code=%x  u4dll_pd_status=%d\n",u4delay_code,u4dll_pd_status));
			    	break;
			    	}
			    else if(u4dll_pd_status==0 && u4delay_code==0)
			    	{
			        mcSHOW_DBG_MSG(("[DramCKGen_Cal_0] fail  u4delay_code=%x  u4dll_pd_status=%d\n",u4delay_code,u4dll_pd_status));
			    	u4Result=DRAM_FAIL;
			    	}
			        mcSHOW_DBG_MSG(("[DramCKGen_Cal_0] u4delay_code=%x  u4dll_pd_status=%d\n",u4delay_code,u4dll_pd_status));


			}
		}
	else
		{
		for(u4delay_code=SW_CAL_CKGEN_STAET;u4delay_code<0xFF;u4delay_code++)
			{
			    vIO32WriteFldAlign((DDRPHY_B0_CKGEN_DLL0), u4delay_code, B0_CKGEN_DLL0_RG_ARCKGEN_DLL_DELAY_CODE_B01);
			    vIO32WriteFldAlign((DDRPHY_SHU1_B0_DQ6), u4delay_code+0x48, SHU1_B0_DQ6_RG_ARPI_CAP_SEL_B0);
			        u4dll_pd_status = u4IO32ReadFldAlign(DDRPHY_MISC_APHY_PD_STATUS, MISC_APHY_PD_STATUS_ARCKGEN_DLL_PD_EN_B01);
			    if(u4dll_pd_status==0)
			    	{
			    	u4Result=DRAM_OK;
			        mcSHOW_DBG_MSG(("[DramCKGen_Cal_1] Pass  u4delay_code=%x u4dll_pd_status=%d\n",u4delay_code,u4dll_pd_status));
			    	break;
			    	}
			    else if(u4dll_pd_status==1 && u4delay_code==0xFF)
			    	{
			        mcSHOW_DBG_MSG(("[DramCKGen_Cal_1] fail  u4delay_code=%x u4dll_pd_status=%d\n",u4delay_code,u4dll_pd_status));
			    	u4Result=DRAM_FAIL;
			    	}
			        mcSHOW_DBG_MSG(("[DramCKGen_Cal_0] u4delay_code=%x  u4dll_pd_status=%d\n",u4delay_code,u4dll_pd_status));

			}
		}
		mcSHOW_DBG_MSG(("[DramCKGen_Cal]Channel =%d u4Result=%d\n",p->channel,u4Result));

    return u4Result;
}
#endif

void DDRPhyPLLSetting_EN7523(DRAMC_CTX_T *p)
{
	//0 ===DDR4_2666_intial_setting_shu1 begin===
	U32 u4SDM_PCW,u4PREDIV,u4POSDIV,u4FBDIV;
	U32 FVCO;
	U32 u4Result;
	U32 u4clock;
	U32 u4value;
	U32 u4recov;

	 /* cc to be confirmed. porting from 8167 */
	 //MIDPI
#if 1//cc mark. These reigsters not exist in 7580
    vIO32WriteFldMulti(DDRPHY_SHU1_CA_CMD6, P_Fld(0x0, SHU1_CA_CMD6_RG_ARPI_MIDPI_EN_CA)
    			| P_Fld(0x0, SHU1_CA_CMD6_RG_ARPI_MIDPI_CKDIV4_EN_CA));
    vIO32WriteFldMulti(DDRPHY_SHU1_B0_DQ6, P_Fld(0x0, SHU1_B0_DQ6_RG_ARPI_MIDPI_EN_B0)
    			| P_Fld(0x0, SHU1_B0_DQ6_RG_ARPI_MIDPI_CKDIV4_EN_B0));
    vIO32WriteFldMulti(DDRPHY_SHU1_B1_DQ6, P_Fld(0x0, SHU1_B1_DQ6_RG_ARPI_MIDPI_EN_B1)
    			| P_Fld(0x0, SHU1_B1_DQ6_RG_ARPI_MIDPI_CKDIV4_EN_B1));
#endif

    vIO32WriteFldMulti(DDRPHY_PLL4, P_Fld(0x0, PLL4_RG_RPHYPLL_MCK8X_EN)
    			| P_Fld(0x1, PLL4_RG_RPHYPLL_MCK8X_SEL)
    			| P_Fld(0x0, PLL4_RG_RPHYPLL_RESETB));

	//PLL
    vIO32WriteFldAlign(DDRPHY_PLL1, 0x0, PLL1_RG_RPHYPLL_EN);
    vIO32WriteFldAlign(DDRPHY_PLL2, 0x0, PLL2_RG_RCLRPLL_EN);
    vIO32WriteFldAlign(DDRPHY_PLL1, 0x1, PLL1_RG_RPHYPLL_MCK16X_EN);


	//cc add - CKGEN DLL
	vIO32WriteFldAlign(DDRPHY_B01_CKGEN_DLL1, 0x0, B01_CKGEN_DLL1_RG_ARCKGEN_DCDL_EN_B01);	
	//vIO32WriteFldAlign(DDRPHY_CA_CKGEN_DLL1, 0x0, CA_CKGEN_DLL1_RG_CKGEN_DCDL_EN_CA);	
	//vIO32WriteFldAlign(DDRPHY_CA_CKGEN_DLL1, 0x0, CA_CKGEN_DLL1_RG_ARCKGEN_DLL_EN_CA);
	vIO32WriteFldAlign(DDRPHY_B01_CKGEN_DLL1, 0x0, B01_CKGEN_DLL1_RG_ARCKGEN_DLL_EN_B01);
	//cc add end
	
    //DLL
    vIO32WriteFldAlign(DDRPHY_CA_DLL_ARPI5, 0x0, CA_DLL_ARPI5_RG_ARDLL_PHDET_EN_CA);
    vIO32WriteFldAlign(DDRPHY_B0_DLL_ARPI5, 0x0, B0_DLL_ARPI5_RG_ARDLL_PHDET_EN_B0);
    vIO32WriteFldAlign(DDRPHY_B1_DLL_ARPI5, 0x0, B1_DLL_ARPI5_RG_ARDLL_PHDET_EN_B1);

    //RESETB
    vIO32WriteFldAlign(DDRPHY_CA_DLL_ARPI0, 0x0, CA_DLL_ARPI0_RG_ARPI_RESETB_CA);
    vIO32WriteFldAlign(DDRPHY_B0_DLL_ARPI0, 0x0, B0_DLL_ARPI0_RG_ARPI_RESETB_B0);
    vIO32WriteFldAlign(DDRPHY_B1_DLL_ARPI0, 0x0, B1_DLL_ARPI0_RG_ARPI_RESETB_B1);
	
    //CG for DLL release
    vIO32WriteFldAlign(DDRPHY_CA_DLL_ARPI2, 0x1, CA_DLL_ARPI2_RG_ARPI_MPDIV_CG_CA);
    vIO32WriteFldMulti(DDRPHY_B0_DLL_ARPI2, P_Fld(0x1, B0_DLL_ARPI2_RG_ARPI_CG_FB_B0)
    			| P_Fld(0x1, B0_DLL_ARPI2_RG_ARPI_MPDIV_CG_B0));
    //cc mark-> _B0 will do both bytes. vIO32WriteFldMulti(DDRPHY_B1_DLL_ARPI2, P_Fld(0x1, B1_DLL_ARPI2_RG_ARPI_CG_FB_B1)
    //			| P_Fld(0x1, B1_DLL_ARPI2_RG_ARPI_MPDIV_CG_B1));

	
	/* PLL Setting: cc add -- config PLL from the table released by DE. */	
	vIO32WriteFldAlign(DDRPHY_SHU1_PLL0, 0x6, SHU1_PLL0_RG_RPHYPLL_TOP_REV); 

	vIO32WriteFldAlign(DDRPHY_SHU1_PLL9, 0x0, SHU1_PLL9_RG_RPHYPLL_RST_DLY); 
	vIO32WriteFldAlign(DDRPHY_SHU1_PLL11, 0x0, SHU1_PLL11_RG_RCLRPLL_RST_DLY);
	
	switch (p->frequency) {
	case DDR_DDR1600:
		mcSHOW_DBG_MSG(("DDR1600 PLL setting init\n"));
		#if 1//dbg_print
		prom_puts("DDR1600 PLL setting init\n");
		#endif
		if(p->clk_src == PLL_Fin_20M)
		{
			vIO32WriteFldMulti(DDRPHY_SHU1_PLL10, P_Fld(0x0, SHU1_PLL10_RG_RCLRPLL_PREDIV)
				| P_Fld(0x0, SHU1_PLL10_RG_RCLRPLL_POSDIV));								//YMC change for EN7523 1600 PLL setting.	
			vIO32WriteFldMulti(DDRPHY_SHU1_PLL8, P_Fld(0x0, SHU1_PLL8_RG_RPHYPLL_PREDIV)
				| P_Fld(0x0, SHU1_PLL8_RG_RPHYPLL_POSDIV));									//YMC change for EN7523 1600 PLL setting.
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL6, 0x0, SHU1_PLL6_RG_RCLKRPLL_SDM_FRA_EN);	//YMC change for 1600 integer mode.//cc change from 0->1 from DE golden
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL4, 0x0, SHU1_PLL4_RG_RPHYPLL_SDM_FRA_EN);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL7, 0x50000000, SHU1_PLL7_RG_RCLRPLL_SDM_PCW);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL5, 0x50000000, SHU1_PLL5_RG_RPHYPLL_SDM_PCW);

			vIO32WriteFldMulti(DDRPHY_PLL7, P_Fld(0x1, PLL7_RG_RCLRPLL_SDM_SSC_PH_INIT)
				| P_Fld(((0x1 << 8) | (0x4D << 0)), PLL7_RG_RCLRPLL_SDM_SSC_PRD));
			vIO32WriteFldMulti(DDRPHY_PLL5, P_Fld(0x1, PLL5_RG_RPHYPLL_SDM_SSC_PH_INIT)
				| P_Fld(((0x1 << 8) | (0x4D << 0)), PLL5_RG_RPHYPLL_SDM_SSC_PRD));
			vIO32WriteFldMulti(DDRPHY_PLL8, P_Fld((0x3 << 8) | (0x25 << 0), PLL8_RG_RCLRPLL_SDM_SSC_DELTA1)
				| P_Fld((0x4 << 8) | (0xEA << 0), PLL8_RG_RCLRPLL_SDM_SSC_DELTA));		
			vIO32WriteFldMulti(DDRPHY_PLL6, P_Fld((0x3 << 8) | (0x25 << 0), PLL6_RG_RPHYPLL_SDM_SSC_DELTA1)
				| P_Fld((0x4 << 8) | (0xEA << 0), PLL6_RG_RPHYPLL_SDM_SSC_DELTA));
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL11, 0x1, SHU1_PLL11_RG_CLRPLL_FBDIV_SEL);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL9, 0x1, SHU1_PLL9_RG_PHYPLL_FBDIV_SEL);
		}
		else
		{
			vIO32WriteFldMulti(DDRPHY_SHU1_PLL10, P_Fld(0x0, SHU1_PLL10_RG_RCLRPLL_PREDIV)
				| P_Fld(0x0, SHU1_PLL10_RG_RCLRPLL_POSDIV));								//YMC change for EN7523 1600 PLL setting.	
			vIO32WriteFldMulti(DDRPHY_SHU1_PLL8, P_Fld(0x0, SHU1_PLL8_RG_RPHYPLL_PREDIV)
				| P_Fld(0x0, SHU1_PLL8_RG_RPHYPLL_POSDIV));									//YMC change for EN7523 1600 PLL setting.
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL6, 0x0, SHU1_PLL6_RG_RCLKRPLL_SDM_FRA_EN);	//YMC change for 1600 integer mode.//cc change from 0->1 from DE golden
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL4, 0x0, SHU1_PLL4_RG_RPHYPLL_SDM_FRA_EN);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL7, 0x40000000, SHU1_PLL7_RG_RCLRPLL_SDM_PCW);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL5, 0x40000000, SHU1_PLL5_RG_RPHYPLL_SDM_PCW);

			vIO32WriteFldMulti(DDRPHY_PLL7, P_Fld(0x1, PLL7_RG_RCLRPLL_SDM_SSC_PH_INIT)
				| P_Fld(((0x1 << 8) | (0xA1 << 0)), PLL7_RG_RCLRPLL_SDM_SSC_PRD));
			vIO32WriteFldMulti(DDRPHY_PLL5, P_Fld(0x1, PLL5_RG_RPHYPLL_SDM_SSC_PH_INIT)
				| P_Fld(((0x1 << 8) | (0xA1 << 0)), PLL5_RG_RPHYPLL_SDM_SSC_PRD));
			vIO32WriteFldMulti(DDRPHY_PLL8, P_Fld((0x3 << 8) | (0x25 << 0), PLL8_RG_RCLRPLL_SDM_SSC_DELTA1)
				| P_Fld((0x3 << 8) | (0x25 << 0), PLL8_RG_RCLRPLL_SDM_SSC_DELTA));		
			vIO32WriteFldMulti(DDRPHY_PLL6, P_Fld((0x3 << 8) | (0x25 << 0), PLL6_RG_RPHYPLL_SDM_SSC_DELTA1)
				| P_Fld((0x3 << 8) | (0x25 << 0), PLL6_RG_RPHYPLL_SDM_SSC_DELTA));
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL11, 0x1, SHU1_PLL11_RG_CLRPLL_FBDIV_SEL);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL9, 0x1, SHU1_PLL9_RG_PHYPLL_FBDIV_SEL);

		}

		break;

	case DDR_DDR1866:
		mcSHOW_DBG_MSG(("DDR1866 PLL setting init[tg changed]\n"));
		#if 1//dbg_print
		prom_puts("DDR1866 PLL setting init\n");
		#endif
		if(p->clk_src == PLL_Fin_20M)
		{
			vIO32WriteFldMulti(DDRPHY_SHU1_PLL10, P_Fld(0x0, SHU1_PLL10_RG_RCLRPLL_PREDIV)
				| P_Fld(0x1, SHU1_PLL10_RG_RCLRPLL_POSDIV));
			vIO32WriteFldMulti(DDRPHY_SHU1_PLL8, P_Fld(0x0, SHU1_PLL8_RG_RPHYPLL_PREDIV)
				| P_Fld(0x1, SHU1_PLL8_RG_RPHYPLL_POSDIV));
	        //vIO32WriteFldAlign(DDRPHY_SHU1_PLL6, 0x1, SHU1_PLL6_RG_RCLRPLL_BP);//tg added
	        //vIO32WriteFldAlign(DDRPHY_SHU1_PLL4, 0x1, SHU1_PLL4_RG_RPHYPLL_BP); //tg added
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL6, 0x0, SHU1_PLL6_RG_RCLKRPLL_SDM_FRA_EN);	//YMC change for 1850 integer mode.  //cc change from 0->1 from DE golden
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL4, 0x0, SHU1_PLL4_RG_RPHYPLL_SDM_FRA_EN); 	//YMC change for 1850 integer mode. //tg change 0->1
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL7, 0xB9000000, SHU1_PLL7_RG_RCLRPLL_SDM_PCW);	//YMC change for 1850 integer mode.
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL5, 0xB9000000, SHU1_PLL5_RG_RPHYPLL_SDM_PCW);	//YMC change for 1850 integer mode.
			vIO32WriteFldMulti(DDRPHY_PLL7, P_Fld(0x1, PLL7_RG_RCLRPLL_SDM_SSC_PH_INIT)
				| P_Fld(((0x1 << 8) | (0x4D << 0)), PLL7_RG_RCLRPLL_SDM_SSC_PRD));
			vIO32WriteFldMulti(DDRPHY_PLL5, P_Fld(0x1, PLL5_RG_RPHYPLL_SDM_SSC_PH_INIT)
				| P_Fld(((0x1 << 8) | (0x4D << 0)), PLL5_RG_RPHYPLL_SDM_SSC_PRD));
			vIO32WriteFldMulti(DDRPHY_PLL8, P_Fld(0x0, PLL8_RG_RCLRPLL_SDM_SSC_DELTA1)
				| P_Fld((0xB<< 8) | (0x5E << 0), PLL8_RG_RCLRPLL_SDM_SSC_DELTA));	//-2% YMC chg for 1850 integer mode.
			vIO32WriteFldMulti(DDRPHY_PLL6, P_Fld(0x0, PLL6_RG_RPHYPLL_SDM_SSC_DELTA1)
				| P_Fld((0xB << 8) | (0x5E << 0), PLL6_RG_RPHYPLL_SDM_SSC_DELTA));	//-2% YMC chg for 1850 integer mode.
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL11, 0x1, SHU1_PLL11_RG_CLRPLL_FBDIV_SEL);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL9, 0x1, SHU1_PLL9_RG_PHYPLL_FBDIV_SEL);
		}
		else
		{
			vIO32WriteFldMulti(DDRPHY_SHU1_PLL10, P_Fld(0x0, SHU1_PLL10_RG_RCLRPLL_PREDIV)
				| P_Fld(0x0, SHU1_PLL10_RG_RCLRPLL_POSDIV));
			vIO32WriteFldMulti(DDRPHY_SHU1_PLL8, P_Fld(0x0, SHU1_PLL8_RG_RPHYPLL_PREDIV)
				| P_Fld(0x0, SHU1_PLL8_RG_RPHYPLL_POSDIV));
	        //vIO32WriteFldAlign(DDRPHY_SHU1_PLL6, 0x1, SHU1_PLL6_RG_RCLRPLL_BP);//tg added
	        //vIO32WriteFldAlign(DDRPHY_SHU1_PLL4, 0x1, SHU1_PLL4_RG_RPHYPLL_BP); //tg added
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL6, 0x0, SHU1_PLL6_RG_RCLKRPLL_SDM_FRA_EN);	//YMC change for 1850 integer mode.  //cc change from 0->1 from DE golden
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL4, 0x0, SHU1_PLL4_RG_RPHYPLL_SDM_FRA_EN); 	//YMC change for 1850 integer mode. //tg change 0->1
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL7, 0x4A000000, SHU1_PLL7_RG_RCLRPLL_SDM_PCW);	//YMC change for 1850 integer mode.
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL5, 0x4A000000, SHU1_PLL5_RG_RPHYPLL_SDM_PCW);	//YMC change for 1850 integer mode.
			vIO32WriteFldMulti(DDRPHY_PLL7, P_Fld(0x1, PLL7_RG_RCLRPLL_SDM_SSC_PH_INIT)
				| P_Fld(((0x1 << 8) | (0xA1 << 0)), PLL7_RG_RCLRPLL_SDM_SSC_PRD));
			vIO32WriteFldMulti(DDRPHY_PLL5, P_Fld(0x1, PLL5_RG_RPHYPLL_SDM_SSC_PH_INIT)
				| P_Fld(((0x1 << 8) | (0xA1 << 0)), PLL5_RG_RPHYPLL_SDM_SSC_PRD));
			vIO32WriteFldMulti(DDRPHY_PLL8, P_Fld(0x0, PLL8_RG_RCLRPLL_SDM_SSC_DELTA1)
				| P_Fld((0x3<< 8) | (0xA3 << 0), PLL8_RG_RCLRPLL_SDM_SSC_DELTA));	//-2% YMC chg for 1850 integer mode.
			vIO32WriteFldMulti(DDRPHY_PLL6, P_Fld(0x0, PLL6_RG_RPHYPLL_SDM_SSC_DELTA1)
				| P_Fld((0x3 << 8) | (0xA3 << 0), PLL6_RG_RPHYPLL_SDM_SSC_DELTA));	//-2% YMC chg for 1850 integer mode.
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL11, 0x1, SHU1_PLL11_RG_CLRPLL_FBDIV_SEL);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL9, 0x1, SHU1_PLL9_RG_PHYPLL_FBDIV_SEL);
		}

		break;
	case DDR_DDR1333:		
		mcSHOW_DBG_MSG(("DDR1333 PLL setting init\n"));
		if(p->clk_src == PLL_Fin_20M)
		{
			vIO32WriteFldMulti(DDRPHY_SHU1_PLL10, P_Fld(0x0, SHU1_PLL10_RG_RCLRPLL_PREDIV)
				| P_Fld(0x1, SHU1_PLL10_RG_RCLRPLL_POSDIV));
			vIO32WriteFldMulti(DDRPHY_SHU1_PLL8, P_Fld(0x0, SHU1_PLL8_RG_RPHYPLL_PREDIV)
				| P_Fld(0x1, SHU1_PLL8_RG_RPHYPLL_POSDIV));
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL6, 0x0, SHU1_PLL6_RG_RCLKRPLL_SDM_FRA_EN);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL4, 0x0, SHU1_PLL4_RG_RPHYPLL_SDM_FRA_EN);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL7, 0x85000000, SHU1_PLL7_RG_RCLRPLL_SDM_PCW);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL5, 0x85000000, SHU1_PLL5_RG_RPHYPLL_SDM_PCW);
			
			vIO32WriteFldMulti(DDRPHY_PLL7, P_Fld(0x1, PLL7_RG_RCLRPLL_SDM_SSC_PH_INIT)
				| P_Fld(((0x1 << 8) | (0x4D << 0)), PLL7_RG_RCLRPLL_SDM_SSC_PRD));
			vIO32WriteFldMulti(DDRPHY_PLL5, P_Fld(0x1, PLL5_RG_RPHYPLL_SDM_SSC_PH_INIT)
				| P_Fld(((0x1 << 8) | (0x4D << 0)), PLL5_RG_RPHYPLL_SDM_SSC_PRD));
			vIO32WriteFldMulti(DDRPHY_PLL8, P_Fld(0x0, PLL8_RG_RCLRPLL_SDM_SSC_DELTA1)
				| P_Fld((0x8 << 8) | (0x2C << 0), PLL8_RG_RCLRPLL_SDM_SSC_DELTA));		
			vIO32WriteFldMulti(DDRPHY_PLL6, P_Fld(0x0, PLL6_RG_RPHYPLL_SDM_SSC_DELTA1)
				| P_Fld((0x8 << 8) | (0x2C << 0), PLL6_RG_RPHYPLL_SDM_SSC_DELTA));
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL11, 0x1, SHU1_PLL11_RG_CLRPLL_FBDIV_SEL);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL9, 0x1, SHU1_PLL9_RG_PHYPLL_FBDIV_SEL);
		}
		else
		{
			vIO32WriteFldMulti(DDRPHY_SHU1_PLL10, P_Fld(0x0, SHU1_PLL10_RG_RCLRPLL_PREDIV)
				| P_Fld(0x0, SHU1_PLL10_RG_RCLRPLL_POSDIV));
			vIO32WriteFldMulti(DDRPHY_SHU1_PLL8, P_Fld(0x0, SHU1_PLL8_RG_RPHYPLL_PREDIV)
				| P_Fld(0x0, SHU1_PLL8_RG_RPHYPLL_POSDIV));
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL6, 0x0, SHU1_PLL6_RG_RCLKRPLL_SDM_FRA_EN);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL4, 0x0, SHU1_PLL4_RG_RPHYPLL_SDM_FRA_EN);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL7, 0x35000000, SHU1_PLL7_RG_RCLRPLL_SDM_PCW);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL5, 0x35000000, SHU1_PLL5_RG_RPHYPLL_SDM_PCW);
			
			vIO32WriteFldMulti(DDRPHY_PLL7, P_Fld(0x1, PLL7_RG_RCLRPLL_SDM_SSC_PH_INIT)
				| P_Fld(((0x1 << 8) | (0xC1 << 0)), PLL7_RG_RCLRPLL_SDM_SSC_PRD));
			vIO32WriteFldMulti(DDRPHY_PLL5, P_Fld(0x1, PLL5_RG_RPHYPLL_SDM_SSC_PH_INIT)
				| P_Fld(((0x1 << 8) | (0xC1 << 0)), PLL5_RG_RPHYPLL_SDM_SSC_PRD));
			vIO32WriteFldMulti(DDRPHY_PLL8, P_Fld(0x0, PLL8_RG_RCLRPLL_SDM_SSC_DELTA1)
				| P_Fld((0x2 << 8) | (0x9B << 0), PLL8_RG_RCLRPLL_SDM_SSC_DELTA));		
			vIO32WriteFldMulti(DDRPHY_PLL6, P_Fld(0x0, PLL6_RG_RPHYPLL_SDM_SSC_DELTA1)
				| P_Fld((0x2 << 8) | (0x9B << 0), PLL6_RG_RPHYPLL_SDM_SSC_DELTA));
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL11, 0x1, SHU1_PLL11_RG_CLRPLL_FBDIV_SEL);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL9, 0x1, SHU1_PLL9_RG_PHYPLL_FBDIV_SEL);
		}
		
		break;

	case DDR_DDR1066:		
		mcSHOW_DBG_MSG(("DDR1066 PLL setting init\n"));
		if(p->clk_src == PLL_Fin_20M)
		{
			vIO32WriteFldMulti(DDRPHY_SHU1_PLL10, P_Fld(0x0, SHU1_PLL10_RG_RCLRPLL_PREDIV)
				| P_Fld(0x1, SHU1_PLL10_RG_RCLRPLL_POSDIV));
			vIO32WriteFldMulti(DDRPHY_SHU1_PLL8, P_Fld(0x0, SHU1_PLL8_RG_RPHYPLL_PREDIV)
				| P_Fld(0x1, SHU1_PLL8_RG_RPHYPLL_POSDIV));
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL6, 0x0, SHU1_PLL6_RG_RCLKRPLL_SDM_FRA_EN);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL4, 0x0, SHU1_PLL4_RG_RPHYPLL_SDM_FRA_EN);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL7, 0x69000000, SHU1_PLL7_RG_RCLRPLL_SDM_PCW);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL5, 0x69000000, SHU1_PLL5_RG_RPHYPLL_SDM_PCW);
			
			vIO32WriteFldMulti(DDRPHY_PLL7, P_Fld(0x1, PLL7_RG_RCLRPLL_SDM_SSC_PH_INIT)
				| P_Fld(((0x1 << 8) | (0x4d << 0)), PLL7_RG_RCLRPLL_SDM_SSC_PRD));
			vIO32WriteFldMulti(DDRPHY_PLL5, P_Fld(0x1, PLL5_RG_RPHYPLL_SDM_SSC_PH_INIT)
				| P_Fld(((0x1 << 8) | (0x4d << 0)), PLL5_RG_RPHYPLL_SDM_SSC_PRD));
			vIO32WriteFldMulti(DDRPHY_PLL8, P_Fld(0x0, PLL8_RG_RCLRPLL_SDM_SSC_DELTA1)
				| P_Fld((0x6 << 8) | (0x74 << 0), PLL8_RG_RCLRPLL_SDM_SSC_DELTA));		
			vIO32WriteFldMulti(DDRPHY_PLL6, P_Fld(0x0, PLL6_RG_RPHYPLL_SDM_SSC_DELTA1)
				| P_Fld((0x6 << 8) | (0x74 << 0), PLL6_RG_RPHYPLL_SDM_SSC_DELTA));
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL11, 0x1, SHU1_PLL11_RG_CLRPLL_FBDIV_SEL);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL9, 0x1, SHU1_PLL9_RG_PHYPLL_FBDIV_SEL);
		}
		else
		{
			vIO32WriteFldMulti(DDRPHY_SHU1_PLL10, P_Fld(0x0, SHU1_PLL10_RG_RCLRPLL_PREDIV)
				| P_Fld(0x0, SHU1_PLL10_RG_RCLRPLL_POSDIV));
			vIO32WriteFldMulti(DDRPHY_SHU1_PLL8, P_Fld(0x0, SHU1_PLL8_RG_RPHYPLL_PREDIV)
				| P_Fld(0x0, SHU1_PLL8_RG_RPHYPLL_POSDIV));
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL6, 0x0, SHU1_PLL6_RG_RCLKRPLL_SDM_FRA_EN);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL4, 0x0, SHU1_PLL4_RG_RPHYPLL_SDM_FRA_EN);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL7, 0x2A000000, SHU1_PLL7_RG_RCLRPLL_SDM_PCW);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL5, 0x2A000000, SHU1_PLL5_RG_RPHYPLL_SDM_PCW);
			
			vIO32WriteFldMulti(DDRPHY_PLL7, P_Fld(0x1, PLL7_RG_RCLRPLL_SDM_SSC_PH_INIT)
				| P_Fld(((0x1 << 8) | (0xA1 << 0)), PLL7_RG_RCLRPLL_SDM_SSC_PRD));
			vIO32WriteFldMulti(DDRPHY_PLL5, P_Fld(0x1, PLL5_RG_RPHYPLL_SDM_SSC_PH_INIT)
				| P_Fld(((0x1 << 8) | (0xA1 << 0)), PLL5_RG_RPHYPLL_SDM_SSC_PRD));
			vIO32WriteFldMulti(DDRPHY_PLL8, P_Fld(0x0, PLL8_RG_RCLRPLL_SDM_SSC_DELTA1)
				| P_Fld((0x2 << 8) | (0x10 << 0), PLL8_RG_RCLRPLL_SDM_SSC_DELTA));		
			vIO32WriteFldMulti(DDRPHY_PLL6, P_Fld(0x0, PLL6_RG_RPHYPLL_SDM_SSC_DELTA1)
				| P_Fld((0x2 << 8) | (0x10 << 0), PLL6_RG_RPHYPLL_SDM_SSC_DELTA));
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL11, 0x1, SHU1_PLL11_RG_CLRPLL_FBDIV_SEL);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL9, 0x1, SHU1_PLL9_RG_PHYPLL_FBDIV_SEL);
		}
		
		break;

	default:
#if 0
		//cc notes: EN7523 PLL calculation.
		mcSHOW_DBG_MSG(("PLL defalut case. DDR%d PLL setting init\n", p->frequency));
		prom_puts("PLL defalut case.\n");

		u4PREDIV = u4IO32ReadFldAlign(DDRPHY_SHU1_PLL10, SHU1_PLL10_RG_RCLRPLL_PREDIV);
		u4POSDIV = u4IO32ReadFldAlign(DDRPHY_SHU1_PLL10, SHU1_PLL10_RG_RCLRPLL_POSDIV);
		u4FBDIV = u4IO32ReadFldAlign(DDRPHY_SHU1_PLL11, SHU1_PLL11_RG_CLRPLL_FBDIV_SEL);
		vIO32WriteFldAlign(DDRPHY_SHU1_PLL6, 0x1, SHU1_PLL6_RG_RCLKRPLL_SDM_FRA_EN);//cc change from 0->1 from DE golden
		vIO32WriteFldAlign(DDRPHY_SHU1_PLL4, 0x0, SHU1_PLL4_RG_RPHYPLL_SDM_FRA_EN);
		
		//cc add for low frequency shall double required freq since CKGEN will do different process
		u4recov = 0;
		if (p->frequency < DDR_DDR1600) {
			u4recov = 1;
			p->frequency = p->frequency << 1;
		}
		
		FVCO = p->frequency << u4POSDIV;

		/* Adjust FVCO first */
		while (FVCO > 3732) {
			mcSHOW_DBG_MSG(("FVCO %dMHz is too large! Try to adjust...\n", FVCO));
			prom_puts("FVCO ");
			prom_print_dec(FVCO);
			prom_puts("MHz is too large! Try to adjust...\n");
			if (u4POSDIV > 0) {
				u4POSDIV =  u4POSDIV - 1;
			} else {
				mcSHOW_DBG_MSG(("FVCO cannot adjust...\n"));
				prom_puts("FVCO cannot adjust...\n");
				break;
			}
			FVCO = p->frequency << u4POSDIV;
		}

		while (FVCO < 1660) {
			mcSHOW_DBG_MSG(("FVCO %dMHz is too small! Try to adjust...\n", FVCO));
			prom_puts("FVCO ");
			prom_print_dec(FVCO);
			prom_puts("MHz is too small! Try to adjust...\n");
			if (u4POSDIV < 4) {
				u4POSDIV =  u4POSDIV + 1;
			} else {
				mcSHOW_DBG_MSG(("FVCO cannot adjust...\n"));
				prom_puts("FVCO cannot adjust...\n");
				break;
			}
			FVCO = p->frequency << u4POSDIV;
		}
		
		vIO32WriteFldAlign(DDRPHY_SHU1_PLL10, u4POSDIV, SHU1_PLL10_RG_RCLRPLL_POSDIV);

		/* cc notes: for PCW calculation, only consider the Integer part. */
		u4SDM_PCW = ((p->frequency / (25 >> u4PREDIV)) >> u4FBDIV) << u4POSDIV;

		/* After u4POSDIV is ajusted, it shall be fixed. */
		if (u4SDM_PCW > 0x78) {
			mcSHOW_DBG_MSG(("PCW 0x%x is too large! Try to adjust...\n", u4SDM_PCW));
			prom_puts("PCW = 0x");
			prom_print_hex(u4SDM_PCW,8);
			prom_puts(" is too large! Try to adjust...\n");

			while (u4SDM_PCW > 0x78) {
				if (u4PREDIV > 0) {
					u4PREDIV = u4PREDIV - 1;
				} else if (u4FBDIV < 1) {
					u4FBDIV = u4FBDIV + 1;
				} else {
					mcSHOW_DBG_MSG(("Cannot adjust...\n"));
					prom_puts("Cannot adjust...\n");
					break;
				}
				
				u4SDM_PCW = ((p->frequency / (25 >> u4PREDIV)) >> u4FBDIV) << u4POSDIV;
			}
		vIO32WriteFldMulti(DDRPHY_SHU1_PLL10, P_Fld(u4PREDIV, SHU1_PLL10_RG_RCLRPLL_PREDIV)
			| P_Fld(u4POSDIV, SHU1_PLL10_RG_RCLRPLL_POSDIV));
		vIO32WriteFldMulti(DDRPHY_SHU1_PLL8, P_Fld(u4PREDIV, SHU1_PLL8_RG_RPHYPLL_PREDIV)
			| P_Fld(u4POSDIV, SHU1_PLL8_RG_RPHYPLL_POSDIV));
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL11, u4FBDIV, SHU1_PLL11_RG_CLRPLL_FBDIV_SEL);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL9, u4FBDIV, SHU1_PLL9_RG_PHYPLL_FBDIV_SEL);
		}

		if (u4SDM_PCW < 0xA) {
			mcSHOW_DBG_MSG(("PCW 0x%x is too Small! Try to adjust...\n", u4SDM_PCW));
			prom_puts("PCW = 0x");
			prom_print_hex(u4SDM_PCW,8);
			prom_puts(" is too Small! Try to adjust...\n");

			while (u4SDM_PCW < 0xA) {
				if (u4PREDIV < 2) {
					u4PREDIV = u4PREDIV + 1;
				} else if (u4FBDIV > 0) {
					u4FBDIV = u4FBDIV - 1;
				} else {
					mcSHOW_DBG_MSG(("Cannot adjust...\n"));
					prom_puts("Cannot adjust...\n");
					break;
				}

				u4SDM_PCW = ((p->frequency / (25 >> u4PREDIV)) >> u4FBDIV) << u4POSDIV;
			}
			vIO32WriteFldMulti(DDRPHY_SHU1_PLL10, P_Fld(u4PREDIV, SHU1_PLL10_RG_RCLRPLL_PREDIV)
				| P_Fld(u4POSDIV, SHU1_PLL10_RG_RCLRPLL_POSDIV));
			vIO32WriteFldMulti(DDRPHY_SHU1_PLL8, P_Fld(u4PREDIV, SHU1_PLL8_RG_RPHYPLL_PREDIV)
				| P_Fld(u4POSDIV, SHU1_PLL8_RG_RPHYPLL_POSDIV));
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL11, u4FBDIV, SHU1_PLL11_RG_CLRPLL_FBDIV_SEL);
			vIO32WriteFldAlign(DDRPHY_SHU1_PLL9, u4FBDIV, SHU1_PLL9_RG_PHYPLL_FBDIV_SEL);
		}

		mcSHOW_DBG_MSG(("PREDIV %d, POSDIV %d, FBDIV %d, PCW 0x%x\n", \
			u4PREDIV, u4POSDIV, u4FBDIV, u4SDM_PCW));
		
		vIO32WriteFldAlign(DDRPHY_SHU1_PLL7, u4SDM_PCW << 24, SHU1_PLL7_RG_RCLRPLL_SDM_PCW);
		vIO32WriteFldAlign(DDRPHY_SHU1_PLL5, u4SDM_PCW << 24, SHU1_PLL5_RG_RPHYPLL_SDM_PCW);

		//cc add to recover frequency
		if (u4recov == 1)
			p->frequency = p->frequency >> 1;
	#if 0	
		vIO32WriteFldMulti(DDRPHY_PLL7, P_Fld(0x1, PLL7_RG_RCLRPLL_SDM_SSC_PH_INIT)
			| P_Fld(((0x1 << 8) | (0xA1 << 0)), PLL7_RG_RCLRPLL_SDM_SSC_PRD));
		vIO32WriteFldMulti(DDRPHY_PLL5, P_Fld(0x1, PLL5_RG_RPHYPLL_SDM_SSC_PH_INIT)
			| P_Fld(((0x1 << 8) | (0xA1 << 0)), PLL5_RG_RPHYPLL_SDM_SSC_PRD));
		vIO32WriteFldMulti(DDRPHY_PLL8, P_Fld((0x1 << 8) | (0xD2 << 0), PLL8_RG_RCLRPLL_SDM_SSC_DELTA1)
			| P_Fld((0x1 << 8) | (0xD2 << 0), PLL8_RG_RCLRPLL_SDM_SSC_DELTA));		
		vIO32WriteFldMulti(DDRPHY_PLL6, P_Fld((0x1 << 8) | (0xD2 << 0), PLL6_RG_RPHYPLL_SDM_SSC_DELTA1)
			| P_Fld((0x1 << 8) | (0xD2 << 0), PLL6_RG_RPHYPLL_SDM_SSC_DELTA));
	#endif
#endif
		break;
	}
	// cc add end
	
	mcSHOW_DBG_MSG(("DDRPHY_SHU1_PLL7=0x%x\n",u4IO32Read4B(DDRPHY_SHU1_PLL7)));
	//cc mark -- ssc setting has been set in PLL part. ApplySSCSettingReg(p, 2, 30, 1);
	mcDELAY_US(1);
	
	vIO32WriteFldAlign(DDRPHY_SHU1_PLL1, 0x0, SHU1_PLL1_RG_RPHYPLLGP_CK_SEL);

	/* DLL Seting	*/
   vIO32WriteFldMulti(DDRPHY_CA_DLL_ARPI0, P_Fld(0x1, CA_DLL_ARPI0_RG_ARDLL_PD_CK_SEL_CA)
			   | P_Fld(0x0, CA_DLL_ARPI0_RG_ARDLL_FASTPJ_CK_SEL_CA));
   vIO32WriteFldMulti(DDRPHY_B0_DLL_ARPI0, P_Fld(0x0, B0_DLL_ARPI0_RG_ARDLL_PD_CK_SEL_B0)
			   | P_Fld(0x1, B0_DLL_ARPI0_RG_ARDLL_FASTPJ_CK_SEL_B0));
   vIO32WriteFldMulti(DDRPHY_B1_DLL_ARPI0, P_Fld(0x0, B1_DLL_ARPI0_RG_ARDLL_PD_CK_SEL_B1)
			   | P_Fld(0x1, B1_DLL_ARPI0_RG_ARDLL_FASTPJ_CK_SEL_B1));

   vIO32WriteFldMulti(DDRPHY_CA_DLL_ARPI5, P_Fld(0x0, CA_DLL_ARPI5_RG_ARDLL_PHDET_OUT_SEL_CA)
			   | P_Fld(0x0, CA_DLL_ARPI5_RG_ARDLL_PHDET_IN_SWAP_CA)
			   | P_Fld(0x0, CA_DLL_ARPI5_RG_ARDLL_FJ_OUT_MODE_CA)
			   | P_Fld(0x0, CA_DLL_ARPI5_RG_ARDLL_FJ_OUT_MODE_SEL_CA)
			   | P_Fld(0x6, CA_DLL_ARPI5_RG_ARDLL_GAIN_CA)
			   | P_Fld(0x9, CA_DLL_ARPI5_RG_ARDLL_IDLECNT_CA)
			   | P_Fld(0x8, CA_DLL_ARPI5_RG_ARDLL_P_GAIN_CA)
			   | P_Fld(0x1, CA_DLL_ARPI5_RG_ARDLL_PHJUMP_EN_CA)
			   | P_Fld(0x1, CA_DLL_ARPI5_RG_ARDLL_PHDIV_CA)
			   | P_Fld(0x0, CA_DLL_ARPI5_RG_ARDLL_DIV_DEC_CA)
			   | P_Fld(0x0, CA_DLL_ARPI5_RG_ARDLL_DIV_MCTL_CA));
   vIO32WriteFldMulti(DDRPHY_B0_DLL_ARPI5, P_Fld(0x1, B0_DLL_ARPI5_RG_ARDLL_PHDET_OUT_SEL_B0)
			   | P_Fld(0x1, B0_DLL_ARPI5_RG_ARDLL_PHDET_IN_SWAP_B0)
			   | P_Fld(0x0, B0_DLL_ARPI5_RG_ARDLL_FJ_OUT_MODE_B0)
			   | P_Fld(0x0, B0_DLL_ARPI5_RG_ARDLL_FJ_OUT_MODE_SEL_B0)
			   | P_Fld(0x7, B0_DLL_ARPI5_RG_ARDLL_GAIN_B0)
			   | P_Fld(0x7, B0_DLL_ARPI5_RG_ARDLL_IDLECNT_B0)
			   | P_Fld(0x8, B0_DLL_ARPI5_RG_ARDLL_P_GAIN_B0)
			   | P_Fld(0x1, B0_DLL_ARPI5_RG_ARDLL_PHJUMP_EN_B0)
			   | P_Fld(0x1, B0_DLL_ARPI5_RG_ARDLL_PHDIV_B0)
			   | P_Fld(0x0, B0_DLL_ARPI5_RG_ARDLL_DIV_DEC_B0)
			   | P_Fld(0x0, B0_DLL_ARPI5_RG_ARDLL_DIV_MCTL_B0));
   vIO32WriteFldMulti(DDRPHY_B1_DLL_ARPI5, P_Fld(0x1, B1_DLL_ARPI5_RG_ARDLL_PHDET_OUT_SEL_B1)
			   | P_Fld(0x1, B1_DLL_ARPI5_RG_ARDLL_PHDET_IN_SWAP_B1)
			   | P_Fld(0x0, B1_DLL_ARPI5_RG_ARDLL_FJ_OUT_MODE_B1)
			   | P_Fld(0x0, B1_DLL_ARPI5_RG_ARDLL_FJ_OUT_MODE_SEL_B1)
			   | P_Fld(0x7, B1_DLL_ARPI5_RG_ARDLL_GAIN_B1)
			   | P_Fld(0x7, B1_DLL_ARPI5_RG_ARDLL_IDLECNT_B1)
			   | P_Fld(0x8, B1_DLL_ARPI5_RG_ARDLL_P_GAIN_B1)
			   | P_Fld(0x1, B1_DLL_ARPI5_RG_ARDLL_PHJUMP_EN_B1)
			   | P_Fld(0x1, B1_DLL_ARPI5_RG_ARDLL_PHDIV_B1)
			   | P_Fld(0x0, B1_DLL_ARPI5_RG_ARDLL_DIV_DEC_B1)
			   | P_Fld(0x0, B1_DLL_ARPI5_RG_ARDLL_DIV_MCTL_B1));

   /*PI Setting*/
   /* Azalea code */
   vIO32WriteFldMulti(DDRPHY_B0_DLL_ARPI2, P_Fld(0x1, B0_DLL_ARPI2_RG_ARPI_CG_MCK_B0)
			| P_Fld(0x1, B0_DLL_ARPI2_RG_ARPI_CG_MCK_FB2DLL_B0)
			| P_Fld(0x1, B0_DLL_ARPI2_RG_ARPI_CG_MCTL_B0)
			| P_Fld(0x1, B0_DLL_ARPI2_RG_ARPI_CG_FB_B0)
			| P_Fld(0x1, B0_DLL_ARPI2_RG_ARPI_CG_DQS_B0)
			| P_Fld(0x1, B0_DLL_ARPI2_RG_ARPI_CG_DQM_B0)
			| P_Fld(0x1, B0_DLL_ARPI2_RG_ARPI_CG_DQ_B0)
			| P_Fld(0x1, B0_DLL_ARPI2_RG_ARPI_CG_DQSIEN_B0)
			| P_Fld(0x1, B0_DLL_ARPI2_RG_ARPI_MPDIV_CG_B0));
	vIO32WriteFldMulti(DDRPHY_B1_DLL_ARPI2, P_Fld(0x1, B1_DLL_ARPI2_RG_ARPI_CG_MCK_B1)
			| P_Fld(0x1, B1_DLL_ARPI2_RG_ARPI_CG_MCK_FB2DLL_B1)
			| P_Fld(0x1, B1_DLL_ARPI2_RG_ARPI_CG_MCTL_B1)
			//cc mark _B1 not used. | P_Fld(0x1, B1_DLL_ARPI2_RG_ARPI_CG_FB_B1)
			| P_Fld(0x1, B1_DLL_ARPI2_RG_ARPI_CG_DQS_B1)
			| P_Fld(0x1, B1_DLL_ARPI2_RG_ARPI_CG_DQM_B1)
			| P_Fld(0x1, B1_DLL_ARPI2_RG_ARPI_CG_DQ_B1)
			| P_Fld(0x1, B1_DLL_ARPI2_RG_ARPI_CG_DQSIEN_B1));
			//cc mark _B1 not used. | P_Fld(0x1, B1_DLL_ARPI2_RG_ARPI_MPDIV_CG_B1));
	vIO32WriteFldMulti(DDRPHY_CA_DLL_ARPI2, P_Fld(0x1, CA_DLL_ARPI2_RG_ARPI_CG_MCK_CA)
			| P_Fld(0x1, CA_DLL_ARPI2_RG_ARPI_CG_MCK_FB2DLL_CA)
			| P_Fld(0x1, CA_DLL_ARPI2_RG_ARPI_CG_MCTL_CA)
			| P_Fld(0x1, CA_DLL_ARPI2_RG_ARRPI_CG_FB_CA)
			| P_Fld(0x1, CA_DLL_ARPI2_RG_ARPI_CG_CS)
			| P_Fld(0x1, CA_DLL_ARPI2_RG_ARPI_CG_CLK)
			| P_Fld(0x1, CA_DLL_ARPI2_RG_ARPI_CG_CMD)
			| P_Fld(0x1, CA_DLL_ARPI2_RG_ARPI_CG_CLKIEN)
			| P_Fld(0x1, CA_DLL_ARPI2_RG_ARPI_MPDIV_CG_CA));
	/* End of azalea code */

	/* Enable PLL */
	vIO32WriteFldAlign(DDRPHY_PLL2, 0x1, PLL2_RG_RCLRPLL_EN);
	vIO32WriteFldAlign(DDRPHY_PLL1, 0x0, PLL1_RG_RPHYPLL_EN); // add by cc

	mcDELAY_US(31);
	SscEnable(p);

	vIO32WriteFldAlign(DDRPHY_B0_DQ5, 0x1, B0_DQ5_RG_RX_ARDQ_VREF_EN_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ5, 0x1, B1_DQ5_RG_RX_ARDQ_VREF_EN_B1);
	vIO32WriteFldAlign(DDRPHY_SHU1_B0_DQ5, 0x0D, SHU1_B0_DQ5_RG_RX_ARDQ_VREF_SEL_B0);	// LP4 and LP4x with term: 0xe
	vIO32WriteFldAlign(DDRPHY_SHU1_B1_DQ5, 0x0D, SHU1_B1_DQ5_RG_RX_ARDQ_VREF_SEL_B1);	// LP4 and LP4x with term: 0xe

	vIO32WriteFldAlign(DDRPHY_B0_DQ5, 0x0D, B0_DQ5_RG_RX_ARDQ_EYE_VREF_SEL_B0); //add by zj
	vIO32WriteFldAlign(DDRPHY_B1_DQ5, 0x0D, B1_DQ5_RG_RX_ARDQ_EYE_VREF_SEL_B1); //add by zj

	vIO32WriteFldAlign(DDRPHY_B0_DQ5, 0, B0_DQ5_RG_RX_ARDQ_EYE_VREF_EN_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ5, 0, B1_DQ5_RG_RX_ARDQ_EYE_VREF_EN_B1);
	vIO32WriteFldMulti(DDRPHY_PLL4, P_Fld(0x1, PLL4_RG_RPHYPLL_ADA_MCK8X_EN)
			| P_Fld(0x1, PLL4_RG_RPHYPLL_AD_MCK8X_EN)
			| P_Fld(0x1, PLL4_RG_RPHYPLL_MCK8X_SEL)
			| P_Fld(0x1, PLL4_RG_RPHYPLL_RESETB)
			| P_Fld(0x1, PLL4_RG_RPHYPLL_MCK8X_EN)); // MCK8X_EN: add by cc

	/* Release PI_RESETB */
	vIO32WriteFldAlign(DDRPHY_CA_DLL_ARPI0, 0x1, CA_DLL_ARPI0_RG_ARPI_RESETB_CA);
	vIO32WriteFldAlign(DDRPHY_B0_DLL_ARPI0, 0x1, B0_DLL_ARPI0_RG_ARPI_RESETB_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DLL_ARPI0, 0x1, B1_DLL_ARPI0_RG_ARPI_RESETB_B1);

	mcDELAY_US(1);

	/* CKGEN */
	#ifdef CKGEN_SW_MOED
		vIO32WriteFldAlign(DDRPHY_B01_CKGEN_DLL0, 0x1, B01_CKGEN_DLL0_RG_ARCKGEN_DLL_OUT_SEL_B01);
		//cc mark vIO32WriteFldAlign(DDRPHY_B0_DLL_ARPI4, 0x1, B0_DLL_ARPI4_RG_ARPI_ONLINE_CAL_B01);
		//vIO32WriteFldAlign(DDRPHY_CA_CKGEN_DLL0, 0x1, CA_CKGEN_DLL0_RG_ARCKGEN_DLL_OUT_SEL_CA);
		//cc mark vIO32WriteFldAlign(DDRPHY_CA_DLL_ARPI4, 0x1, CA_DLL_ARPI4_RG_ARPI_ONLINE_CAL_CA);
		vIO32WriteFldAlign((DDRPHY_B01_CKGEN_DLL0), 0x80, B01_CKGEN_DLL0_RG_ARCKGEN_DLL_DELAY_CODE_B01);
		vIO32WriteFldAlign((DDRPHY_SHU1_B0_DQ6), 0x80+0x48, SHU1_B0_DQ6_RG_ARPI_CAP_SEL_B0);
		//vIO32WriteFldAlign((DDRPHY_CA_CKGEN_DLL0), 0x80, CA_CKGEN_DLL0_RG_ARCKGEN_DLL_DELAY_CODE_CA);
		vIO32WriteFldAlign((DDRPHY_SHU1_CA_CMD6), 0x80+0x48, SHU1_CA_CMD6_RG_ARPI_CAP_SEL_CA);
#endif
	
		/* CKGEN DLL setting */
#ifdef CKGEN_SW_MOED
		u4Result=DramCKGen_Cal(p);
#else
		// add by cc for auto mode
	if(p->frequency < DDR_DDR1600)
	{
		//B01
		vIO32WriteFldMulti(DDRPHY_B01_CKGEN_DLL1, P_Fld(0x0, B01_CKGEN_DLL1_RG_ARCKGEN_PD_EN_B01)
			| P_Fld(0x1, B01_CKGEN_DLL1_RG_ARCKGEN_FD_EN_B01)
			| P_Fld(0x0, B01_CKGEN_DLL1_RG_ARCKGEN_DLL_PHJUMP_EN_B01)
			| P_Fld(0x0, B01_CKGEN_DLL1_RG_ARCKGEN_DLL_FAST_PSPJ_B01)
			| P_Fld(0x0, B01_CKGEN_DLL1_RG_ARCKGEN_DCDL_EN_B01)
			| P_Fld(0x1, B01_CKGEN_DLL1_RG_ARCKGEN_CKDIV4_EN_B01)
			| P_Fld(0xF, B01_CKGEN_DLL1_RG_ARCKGEN_PI_CAP_SEL_B01)
			| P_Fld(0x1, B01_CKGEN_DLL1_RG_ARCKGEN_PI_EXT_CAP_SEL_B01)
			| P_Fld(0x0, B01_CKGEN_DLL1_RG_ARCKGEN_ONLINE_CAL_B01));
		vIO32WriteFldMulti(DDRPHY_B01_CKGEN_DLL0, P_Fld(0x20, B01_CKGEN_DLL0_RG_ARCKGEN_DLL_RESERVE_B01)
			| P_Fld(0x3, B01_CKGEN_DLL0_RG_ARCKGEN_DLL_OUT_SEL_B01)
			| P_Fld(0x8, B01_CKGEN_DLL0_RG_ARCKGEN_DLL_P_GAIN_B01)
			| P_Fld(0x7, B01_CKGEN_DLL0_RG_ARCKGEN_DLL_IDLECNT_B01)
			| P_Fld(0x0, B01_CKGEN_DLL0_RG_ARCKGEN_DLL_DELAY_CODE_B01)
			| P_Fld(0x0, B01_CKGEN_DLL0_RG_ARCKGEN_DLL_MON_SEL_B01));
		vIO32WriteFldAlign(DDRPHY_SHU1_B0_DQ6, 0x0, SHU1_B0_DQ6_RG_ARPI_RESERVE_B0); // to be confirmed
		vIO32WriteFldAlign(DDRPHY_SHU1_B1_DQ6, 0x0, SHU1_B1_DQ6_RG_ARPI_RESERVE_B1); // to be confirmed
	
		//CA -- to be confirmed if CA needed
		#if 0
		vIO32WriteFldMulti(DDRPHY_CA_CKGEN_DLL1, P_Fld(0x0, CA_CKGEN_DLL1_RG_ARCKGEN_PD_EN_CA)
			| P_Fld(0x1, CA_CKGEN_DLL1_RG_ARCKGEN_FD_EN_CA)
			| P_Fld(0x0, CA_CKGEN_DLL1_RG_ARCKGEN_DLL_PHJUMP_EN_CA)
			| P_Fld(0x0, CA_CKGEN_DLL1_RG_ARCKGEN_DLL_FAST_PSPJ_CA)
			| P_Fld(0x0, CA_CKGEN_DLL1_RG_CKGEN_DCDL_EN_CA)
			| P_Fld(0x1, CA_CKGEN_DLL1_RG_ARCKGEN_CKDIV4_EN_CA)
			| P_Fld(0xF, CA_CKGEN_DLL1_RG_ARCKGEN_PI_CAP_SEL_CA)
			| P_Fld(0x1, CA_CKGEN_DLL1_RG_ARCKGEN_PI_EXT_CAP_SEL_CA)
			| P_Fld(0x0, CA_CKGEN_DLL1_RG_ARCKGEN_ONLINE_CAL_CA));
        #endif
        #if 0
		vIO32WriteFldMulti(DDRPHY_CA_CKGEN_DLL0, P_Fld(0x20, CA_CKGEN_DLL0_RG_ARCKGEN_DLL_RESERVE_CA)
			| P_Fld(0x3, CA_CKGEN_DLL0_RG_ARCKGEN_DLL_OUT_SEL_CA)
			| P_Fld(0x8, CA_CKGEN_DLL0_RG_ARCKGEN_DLL_P_GAIN_CA)
			| P_Fld(0x7, CA_CKGEN_DLL0_RG_ARCKGEN_DLL_IDLECNT_CA)
			| P_Fld(0x0, CA_CKGEN_DLL0_RG_ARCKGEN_DLL_DELAY_CODE_CA)
			| P_Fld(0x0, CA_CKGEN_DLL0_RG_ARCKGEN_DLL_MON_SEL_CA));
	    #endif
		vIO32WriteFldAlign(DDRPHY_SHU1_CA_CMD6, 0x0, SHU1_CA_CMD6_RG_ARPI_RESERVE_CA);		
		vIO32WriteFldAlign(DDRPHY_B01_MISC, 0x1, B01_MISC_RG_ARPI_SMT_EN_B01); 
		vIO32WriteFldAlign(DDRPHY_CA_MISC, 0x1, CA_MISC_RG_ARPI_SMT_EN_CA); 

		/* CKGEN DLL enable */
		//vIO32WriteFldAlign(DDRPHY_CA_CKGEN_DLL1, 0x0, CA_CKGEN_DLL1_RG_ARCKGEN_DLL_EN_CA);
		vIO32WriteFldAlign(DDRPHY_B01_CKGEN_DLL1, 0x0, B01_CKGEN_DLL1_RG_ARCKGEN_DLL_EN_B01);
	} else {
		//B01
		vIO32WriteFldMulti(DDRPHY_B01_CKGEN_DLL1, P_Fld(0x1, B01_CKGEN_DLL1_RG_ARCKGEN_PD_EN_B01)
			| P_Fld(0x1, B01_CKGEN_DLL1_RG_ARCKGEN_FD_EN_B01)
			| P_Fld(0x0, B01_CKGEN_DLL1_RG_ARCKGEN_DLL_PHJUMP_EN_B01)
			| P_Fld(0x0, B01_CKGEN_DLL1_RG_ARCKGEN_DLL_FAST_PSPJ_B01)
			| P_Fld(0x1, B01_CKGEN_DLL1_RG_ARCKGEN_DCDL_EN_B01)
			| P_Fld(0x0, B01_CKGEN_DLL1_RG_ARCKGEN_CKDIV4_EN_B01)
			| P_Fld(0x0, B01_CKGEN_DLL1_RG_ARCKGEN_PI_CAP_SEL_B01)
			| P_Fld(0x1, B01_CKGEN_DLL1_RG_ARCKGEN_PI_EXT_CAP_SEL_B01)
			| P_Fld(0x0, B01_CKGEN_DLL1_RG_ARCKGEN_ONLINE_CAL_B01));
		vIO32WriteFldMulti(DDRPHY_B01_CKGEN_DLL0, P_Fld(0x0, B01_CKGEN_DLL0_RG_ARCKGEN_DLL_RESERVE_B01)
			| P_Fld(0x0, B01_CKGEN_DLL0_RG_ARCKGEN_DLL_OUT_SEL_B01)
			| P_Fld(0x8, B01_CKGEN_DLL0_RG_ARCKGEN_DLL_P_GAIN_B01)
			| P_Fld(0x7, B01_CKGEN_DLL0_RG_ARCKGEN_DLL_IDLECNT_B01)
			| P_Fld(0x0, B01_CKGEN_DLL0_RG_ARCKGEN_DLL_DELAY_CODE_B01)
			| P_Fld(0x0, B01_CKGEN_DLL0_RG_ARCKGEN_DLL_MON_SEL_B01));
		vIO32WriteFldAlign(DDRPHY_SHU1_B0_DQ6, 0x0, SHU1_B0_DQ6_RG_ARPI_RESERVE_B0); // to be confirmed
		vIO32WriteFldAlign(DDRPHY_SHU1_B1_DQ6, 0x0, SHU1_B1_DQ6_RG_ARPI_RESERVE_B1); // to be confirmed
	
		//CA -- to be confirmed if CA needed
		#if 0
		vIO32WriteFldMulti(DDRPHY_CA_CKGEN_DLL1, P_Fld(0x1, CA_CKGEN_DLL1_RG_ARCKGEN_PD_EN_CA)
			| P_Fld(0x1, CA_CKGEN_DLL1_RG_ARCKGEN_FD_EN_CA)
			| P_Fld(0x0, CA_CKGEN_DLL1_RG_ARCKGEN_DLL_PHJUMP_EN_CA)
			| P_Fld(0x0, CA_CKGEN_DLL1_RG_ARCKGEN_DLL_FAST_PSPJ_CA)
			| P_Fld(0x1, CA_CKGEN_DLL1_RG_CKGEN_DCDL_EN_CA)
			| P_Fld(0x0, CA_CKGEN_DLL1_RG_ARCKGEN_CKDIV4_EN_CA)
			| P_Fld(0x0, CA_CKGEN_DLL1_RG_ARCKGEN_PI_CAP_SEL_CA)
			| P_Fld(0x1, CA_CKGEN_DLL1_RG_ARCKGEN_PI_EXT_CAP_SEL_CA)
			| P_Fld(0x0, CA_CKGEN_DLL1_RG_ARCKGEN_ONLINE_CAL_CA));
        #endif
        #if 0
		vIO32WriteFldMulti(DDRPHY_CA_CKGEN_DLL0, P_Fld(0x0, CA_CKGEN_DLL0_RG_ARCKGEN_DLL_RESERVE_CA)
			| P_Fld(0x0, CA_CKGEN_DLL0_RG_ARCKGEN_DLL_OUT_SEL_CA)
			| P_Fld(0x8, CA_CKGEN_DLL0_RG_ARCKGEN_DLL_P_GAIN_CA)
			| P_Fld(0x7, CA_CKGEN_DLL0_RG_ARCKGEN_DLL_IDLECNT_CA)
			| P_Fld(0x0, CA_CKGEN_DLL0_RG_ARCKGEN_DLL_DELAY_CODE_CA)
			| P_Fld(0x0, CA_CKGEN_DLL0_RG_ARCKGEN_DLL_MON_SEL_CA));
	    #endif
		vIO32WriteFldAlign(DDRPHY_SHU1_CA_CMD6, 0x0, SHU1_CA_CMD6_RG_ARPI_RESERVE_CA);	
		vIO32WriteFldAlign(DDRPHY_B01_MISC, 0x1, B01_MISC_RG_ARPI_SMT_EN_B01); 
		vIO32WriteFldAlign(DDRPHY_CA_MISC, 0x1, CA_MISC_RG_ARPI_SMT_EN_CA); 

		/* CKGEN DLL enable */
		//vIO32WriteFldAlign(DDRPHY_CA_CKGEN_DLL1, 0x1, CA_CKGEN_DLL1_RG_ARCKGEN_DLL_EN_CA);
		mcDELAY_US(1);		
		vIO32WriteFldAlign(DDRPHY_B01_CKGEN_DLL1, 0x1, B01_CKGEN_DLL1_RG_ARCKGEN_DLL_EN_B01);
	}
		//cc add end
#endif

   vIO32WriteFldMulti(DDRPHY_CA_DLL_ARPI4, P_Fld(0x0, CA_DLL_ARPI4_RG_ARPI_BYPASS_MCTL_CA)
			   | P_Fld(0x0, CA_DLL_ARPI4_RG_ARPI_BYPASS_FB_CA)
			   | P_Fld(0x0, CA_DLL_ARPI4_RG_ARPI_BYPASS_CS)
			   | P_Fld(0x0, CA_DLL_ARPI4_RG_ARPI_BYPASS_CLK)
			   | P_Fld(0x0, CA_DLL_ARPI4_RG_ARPI_BYPASS_CMD)
			   | P_Fld(0x0, CA_DLL_ARPI4_RG_ARPI_BYPASS_CLKIEN));
   vIO32WriteFldMulti(DDRPHY_B0_DLL_ARPI4, P_Fld(0x0, B0_DLL_ARPI4_RG_ARPI_BYPASS_MCTL_B0)
			   | P_Fld(0x0, B0_DLL_ARPI4_RG_ARPI_BYPASS_FB_B0)
			   | P_Fld(0x0, B0_DLL_ARPI4_RG_ARPI_BYPASS_DQS_B0)
			   | P_Fld(0x0, B0_DLL_ARPI4_RG_ARPI_BYPASS_DQM_B0)
			   | P_Fld(0x0, B0_DLL_ARPI4_RG_ARPI_BYPASS_DQ_B0)
			   | P_Fld(0x0, B0_DLL_ARPI4_RG_ARPI_BYPASS_DQSIEN_B0));
   vIO32WriteFldMulti(DDRPHY_B1_DLL_ARPI4, P_Fld(0x0, B1_DLL_ARPI4_RG_ARPI_BYPASS_MCTL_B1)
			   | P_Fld(0x0, B1_DLL_ARPI4_RG_ARPI_BYPASS_FB_B1)
			   | P_Fld(0x0, B1_DLL_ARPI4_RG_ARPI_BYPASS_DQS_B1)
			   | P_Fld(0x0, B1_DLL_ARPI4_RG_ARPI_BYPASS_DQM_B1)
			   | P_Fld(0x0, B1_DLL_ARPI4_RG_ARPI_BYPASS_DQ_B1)
			   | P_Fld(0x0, B1_DLL_ARPI4_RG_ARPI_BYPASS_DQSIEN_B1));
   
   /* PI EN - RPI_*_EN* & *_JUMP_EN* */
   vIO32WriteFldMulti(DDRPHY_CA_DLL_ARPI1, P_Fld(0x1, CA_DLL_ARPI1_RG_ARPISM_MCK_SEL_CA)//cc change from 0x0->0x1 from DE golden.
			   | P_Fld(0x1, CA_DLL_ARPI1_RG_ARPI_MCTL_JUMP_EN_CA)
			   | P_Fld(0x1, CA_DLL_ARPI1_RG_ARPI_FB_JUMP_EN_CA)
			   | P_Fld(0x1, CA_DLL_ARPI1_RG_ARPI_CS_JUMP_EN)
			   | P_Fld(0x1, CA_DLL_ARPI1_RG_ARPI_CLK_JUMP_EN)
			   | P_Fld(0x1, CA_DLL_ARPI1_RG_ARPI_CMD_JUMP_EN)
			   | P_Fld(0x1, CA_DLL_ARPI1_RG_ARPI_CLKIEN_JUMP_EN)
			   | P_Fld(0x0, CA_DLL_ARPI1_RG_ARPI_SET_UPDN_CA)); // add by cc
   vIO32WriteFldMulti(DDRPHY_B0_DLL_ARPI1, P_Fld(0x1, B0_DLL_ARPI1_RG_ARPISM_MCK_SEL_B0)//cc change from 0x0->0x1 from DE golden.
			   | P_Fld(0x1, B0_DLL_ARPI1_RG_ARPI_MCTL_JUMP_EN_B0)
			   | P_Fld(0x1, B0_DLL_ARPI1_RG_ARPI_FB_JUMP_EN_B0)
			   | P_Fld(0x1, B0_DLL_ARPI1_RG_ARPI_DQS_JUMP_EN_B0)
			   | P_Fld(0x1, B0_DLL_ARPI1_RG_ARPI_DQM_JUMP_EN_B0)
			   | P_Fld(0x1, B0_DLL_ARPI1_RG_ARPI_DQ_JUMP_EN_B0)
			   | P_Fld(0x1, B0_DLL_ARPI1_RG_ARPI_DQSIEN_JUMP_EN_B0)
			   | P_Fld(0x0, B0_DLL_ARPI1_RG_ARPI_OFFSET_DQSIEN_B0)
			   | P_Fld(0x0, B0_DLL_ARPI1_RG_ARPI_SET_UPDN_B0)); // add by cc
   vIO32WriteFldMulti(DDRPHY_B1_DLL_ARPI1, P_Fld(0x1, B1_DLL_ARPI1_RG_ARPISM_MCK_SEL_B1)//cc change from 0x0->0x1 from DE golden.
			   | P_Fld(0x1, B1_DLL_ARPI1_RG_ARPI_MCTL_JUMP_EN_B1)
			   | P_Fld(0x1, B1_DLL_ARPI1_RG_ARPI_FB_JUMP_EN_B1)
			   | P_Fld(0x1, B1_DLL_ARPI1_RG_ARPI_DQS_JUMP_EN_B1)
			   | P_Fld(0x1, B1_DLL_ARPI1_RG_ARPI_DQM_JUMP_EN_B1)
			   | P_Fld(0x1, B1_DLL_ARPI1_RG_ARPI_DQ_JUMP_EN_B1)
			   | P_Fld(0x1, B1_DLL_ARPI1_RG_ARPI_DQSIEN_JUMP_EN_B1)
			   | P_Fld(0x0, B1_DLL_ARPI1_RG_ARPI_OFFSET_DQSIEN_B1)			   
			   | P_Fld(0x0, B1_DLL_ARPI1_RG_ARPI_SET_UPDN_B1)); // add by cc

   vIO32WriteFldMulti(DDRPHY_CA_DLL_ARPI3, P_Fld(0x1, CA_DLL_ARPI3_RG_ARPI_MCTL_EN_CA)
			   | P_Fld(0x1, CA_DLL_ARPI3_RG_ARPI_FB_EN_CA)
			   | P_Fld(0x1, CA_DLL_ARPI3_RG_ARPI_CS_EN)
			   | P_Fld(0x1, CA_DLL_ARPI3_RG_ARPI_CLK_EN)
			   | P_Fld(0x1, CA_DLL_ARPI3_RG_ARPI_CMD_EN)
			   | P_Fld(0x0, CA_DLL_ARPI3_RG_ARPI_CLKIEN_EN)); // add by cc
   vIO32WriteFldMulti(DDRPHY_B0_DLL_ARPI3, P_Fld(0x0, B0_DLL_ARPI3_RG_ARPI_MCTL_EN_B0)
			   | P_Fld(0x1, B0_DLL_ARPI3_RG_ARPI_FB_EN_B0)
			   | P_Fld(0x1, B0_DLL_ARPI3_RG_ARPI_DQS_EN_B0)
			   | P_Fld(0x1, B0_DLL_ARPI3_RG_ARPI_DQM_EN_B0)
			   | P_Fld(0x1, B0_DLL_ARPI3_RG_ARPI_DQ_EN_B0)
			   | P_Fld(0x1, B0_DLL_ARPI3_RG_ARPI_DQSIEN_EN_B0));
   vIO32WriteFldMulti(DDRPHY_B1_DLL_ARPI3, P_Fld(0x0, B1_DLL_ARPI3_RG_ARPI_MCTL_EN_B1)
			   | P_Fld(0x1, B1_DLL_ARPI3_RG_ARPI_FB_EN_B1)
			   | P_Fld(0x1, B1_DLL_ARPI3_RG_ARPI_DQS_EN_B1)
			   | P_Fld(0x1, B1_DLL_ARPI3_RG_ARPI_DQM_EN_B1)
			   | P_Fld(0x1, B1_DLL_ARPI3_RG_ARPI_DQ_EN_B1)
			   | P_Fld(0x1, B1_DLL_ARPI3_RG_ARPI_DQSIEN_EN_B1));
   
   /* newly add by cc */
   vIO32WriteFldMulti(DDRPHY_CA_MISC, P_Fld(0x1, CA_MISC_RG_ARPI_BYPASS_SR_CLK)
			   | P_Fld(0x1, CA_MISC_RG_ARPI_BYPASS_SR_CMD)
			   | P_Fld(0x0, CA_MISC_RG_ARPI_8PHASE_XLATCH_FORCE_CA)
			   | P_Fld(0x0, CA_MISC_RG_ARPI_BUFGP_XLATCH_FORCE_CLK)
			   | P_Fld(0x0, CA_MISC_RG_ARPI_BUFGP_XLATCH_FORCE_CMD)
			   | P_Fld(0x0, CA_MISC_RG_ARPI_PSMUX_XLATCH_FORCE_CLK)
			   | P_Fld(0x0, CA_MISC_RG_ARPI_PSMUX_XLATCH_FORCE_CMD)
			   | P_Fld(0x0, CA_MISC_RG_ARPI_SMT_XLATCH_FORCE_CLK)
			   | P_Fld(0x0, CA_MISC_RG_ARPI_SMT_XLATCH_FORCE_CMD)); 			   
   vIO32WriteFldMulti(DDRPHY_B01_MISC, P_Fld(0x1, B01_MISC_RG_ARPI_BYPASS_SR_B01)
			   | P_Fld(0x1, B01_MISC_RG_ARPI_BYPASS_SR_DQS_B0)
			   | P_Fld(0x1, B01_MISC_RG_ARPI_BYPASS_SR_DQS_B1)
			   | P_Fld(0x0, B01_MISC_RG_ARPI_8PHASE_XLATCH_FORCE_B01)
			   | P_Fld(0x0, B01_MISC_RG_ARPI_BUFGP_XLATCH_FORCE_B01)
			   | P_Fld(0x0, B01_MISC_RG_ARPI_BUFGP_XLATCH_FORCE_DQS_B0)
			   | P_Fld(0x0, B01_MISC_RG_ARPI_BUFGP_XLATCH_FORCE_DQS_B1)
			   | P_Fld(0x0, B01_MISC_RG_ARPI_PSMUX_XLATCH_FORCE_B01)
			   | P_Fld(0x0, B01_MISC_RG_ARPI_PSMUX_XLATCH_FORCE_DQS_B0)
			   | P_Fld(0x0, B01_MISC_RG_ARPI_PSMUX_XLATCH_FORCE_DQS_B1)
			   | P_Fld(0x0, B01_MISC_RG_ARPI_SMT_XLATCH_FORCE_B01)
			   | P_Fld(0x0, B01_MISC_RG_ARPI_SMT_XLATCH_FORCE_DQS_B0)
			   | P_Fld(0x0, B01_MISC_RG_ARPI_SMT_XLATCH_FORCE_DQS_B1));
   //vIO32WriteFldAlign(DDRPHY_DDRPHY_CONFIG_CA, 0x0, DDRPHY_CONFIG_CA_RG_ARPI_PSMUX_DRV_SEL_CA);
   vIO32WriteFldAlign(DDRPHY_B0_DQ5, 0x0, B0_DQ5_RG_ARPI_PSMUX_DRV_SEL_B01);   
   /* cc add end */
   
   vIO32WriteFldMulti(DDRPHY_SHU1_CA_CMD4, P_Fld(0x0, SHU1_CA_CMD4_RG_ARPI_DA_MCK_FB_DL_CA)
			   | P_Fld(0x0, SHU1_CA_CMD4_RG_ARPI_AA_MCK_FB_DL_CA)
			   | P_Fld(0x0, SHU1_CA_CMD4_RG_ARPI_AA_MCK_DL_CA));
   vIO32WriteFldMulti(DDRPHY_SHU1_B0_DQ4, P_Fld(0x0, SHU1_B0_DQ4_RG_ARPI_DA_MCK_FB_DL_B0)
			   | P_Fld(0x0, SHU1_B0_DQ4_RG_ARPI_AA_MCK_FB_DL_B0)
			   | P_Fld(0x0, SHU1_B0_DQ4_RG_ARPI_AA_MCK_DL_B0));
   vIO32WriteFldMulti(DDRPHY_SHU1_B1_DQ4, P_Fld(0x0, SHU1_B1_DQ4_RG_ARPI_DA_MCK_FB_DL_B1)
			   | P_Fld(0x0, SHU1_B1_DQ4_RG_ARPI_AA_MCK_FB_DL_B1)
			   | P_Fld(0x0, SHU1_B1_DQ4_RG_ARPI_AA_MCK_DL_B1));

   vIO32WriteFldMulti(DDRPHY_CA_DLL_ARPI0, P_Fld(0x0, CA_DLL_ARPI0_RG_ARPI_LS_SEL_CA)
			   | P_Fld(0x0, CA_DLL_ARPI0_RG_ARPI_LS_EN_CA));
   vIO32WriteFldMulti(DDRPHY_B0_DLL_ARPI0, P_Fld(0x0, B0_DLL_ARPI0_RG_ARPI_LS_SEL_B0)
			   | P_Fld(0x0, B0_DLL_ARPI0_RG_ARPI_LS_EN_B0));
   vIO32WriteFldMulti(DDRPHY_B1_DLL_ARPI0, P_Fld(0x0, B1_DLL_ARPI0_RG_ARPI_LS_SEL_B1)
			   | P_Fld(0x0, B1_DLL_ARPI0_RG_ARPI_LS_EN_B1));
	/* end of 8167 code */

	//zj mark vIO32WriteFldMulti(DDRPHY_B1_DQ8, P_Fld(0x1, B1_DQ8_RG_ARDQM_PMX_SEL_B1)
	//zj mark		| P_Fld(0xff, B1_DQ8_RG_ARDQ_PMX_SEL_B1));
	vIO32WriteFldMulti(DDRPHY_B0_DQ3, P_Fld(0x1, B0_DQ3_RG_RX_ARDQ_STBENCMP_EN_B0)
			| P_Fld(0x1, B0_DQ3_RG_ARDQ_RESETB_B0)
			| P_Fld(0x1, B0_DQ3_RG_TX_ARDQ_EN_B0)
			| P_Fld(0x0, B0_DQ3_RG_RX_ARDQ_SMT_EN_B0)); //cc change from 1->0 according to DE flow 17.10.23 version
	vIO32WriteFldMulti(DDRPHY_B1_DQ3, P_Fld(0x1, B1_DQ3_RG_RX_ARDQ_STBENCMP_EN_B1)
			| P_Fld(0x1, B1_DQ3_RG_ARDQ_RESETB_B1)
			| P_Fld(0x1, B1_DQ3_RG_TX_ARDQ_EN_B1)
			| P_Fld(0x0, B1_DQ3_RG_RX_ARDQ_SMT_EN_B1)); //cc change from 1->0 according to DE flow 17.10.23 version
	vIO32WriteFldMulti(DDRPHY_CA_CMD8, P_Fld(0x1, CA_CMD8_RG_TX_RRESETB_DDR4_SEL) //cc to be comfirmed
			| P_Fld(0x1, CA_CMD8_RG_RRESETB_DRVN)
			| P_Fld(0x1, CA_CMD8_RG_RRESETB_DRVP));
	vIO32WriteFldAlign(DDRPHY_MISC_CG_CTRL4, 0x13400000, MISC_CG_CTRL4_R_PHY_MCK_CG_CTRL);
	vIO32WriteFldMulti(DDRPHY_MISC_SHU_OPT, P_Fld(0x1, MISC_SHU_OPT_R_CA_SHU_PHDET_SPM_EN)
			| P_Fld(0x1, MISC_SHU_OPT_R_CA_SHU_PHY_GATING_RESETB_SPM_EN)
			| P_Fld(0x2, MISC_SHU_OPT_R_DQB1_SHU_PHDET_SPM_EN)
			| P_Fld(0x1, MISC_SHU_OPT_R_DQB1_SHU_PHY_GATING_RESETB_SPM_EN)
			| P_Fld(0x2, MISC_SHU_OPT_R_DQB0_SHU_PHDET_SPM_EN)
			| P_Fld(0x1, MISC_SHU_OPT_R_DQB0_SHU_PHY_GATING_RESETB_SPM_EN));

	//zj mark vIO32WriteFldMulti(DDRPHY_SHU1_PLL9, P_Fld(0x1, SHU1_PLL9_RG_RPHYPLL_LVROD_EN)
	//zj mark		| P_Fld(0x1, SHU1_PLL9_RG_RPHYPLL_RST_DLY));
	//zj mark vIO32WriteFldMulti(DDRPHY_SHU1_PLL11, P_Fld(0x1, SHU1_PLL11_RG_RCLRPLL_LVROD_EN)
	//zj mark		| P_Fld(0x1, SHU1_PLL11_RG_RCLRPLL_RST_DLY));

	vIO32WriteFldMulti(DDRPHY_SHU1_PLL9, P_Fld(0x0, SHU1_PLL9_RG_RPHYPLL_LVROD_EN)
				//cc move to front | P_Fld(0x2, SHU1_PLL9_RG_RPHYPLL_RST_DLY) //cc change from 0->2 from DE golden
				| P_Fld(0x0, SHU1_PLL9_RG_RPHYPLL_MONCK_EN)//cc modified from 1->0
				| P_Fld(0x0, SHU1_PLL9_RG_RPHYPLL_MONREF_EN) // add by cc
				| P_Fld(0x0, SHU1_PLL9_RG_RPHYPLL_MONVC_EN)); // add by cc
	vIO32WriteFldMulti(DDRPHY_SHU1_PLL11, P_Fld(0x0, SHU1_PLL11_RG_RCLRPLL_LVROD_EN)
				//cc move to front | P_Fld(0x2, SHU1_PLL11_RG_RCLRPLL_RST_DLY)//cc change from 0->2 from DE golden
				| P_Fld(0x0, SHU1_PLL11_RG_RCLRPLL_MONCK_EN)//cc modified from 1->0
				| P_Fld(0x0, SHU1_PLL11_RG_RCLRPLL_MONREF_EN) // add by cc
				| P_Fld(0x0, SHU1_PLL11_RG_RCLRPLL_MONVC_EN)); // add by cc

	vIO32WriteFldAlign(DDRPHY_SHU1_PLL7, 0x1, SHU1_PLL7_RG_RCLRPLL_SDM_PCW_CHG); // cc add
	vIO32WriteFldAlign(DDRPHY_SHU1_PLL5, 0x1, SHU1_PLL5_RG_RPHYPLL_SDM_PCW_CHG); // cc add
	
	mcDELAY_US(1);
			
	vIO32WriteFldAlign(DDRPHY_CA_DLL_ARPI0, 0x1, CA_DLL_ARPI0_RG_ARDLL_PD_CK_SEL_CA);
	vIO32WriteFldAlign(DDRPHY_B0_DLL_ARPI0, 0x1, B0_DLL_ARPI0_RG_ARDLL_FASTPJ_CK_SEL_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DLL_ARPI0, 0x1, B1_DLL_ARPI0_RG_ARDLL_FASTPJ_CK_SEL_B1);

	mcDELAY_US(1);

	/* From 8167 */
	//solution for DA_ARPI_CG_FB
    vIO32WriteFldAlign(DDRPHY_MISC_CTRL1, 0x1, MISC_CTRL1_R_DMPHYRST);
    vIO32WriteFldAlign(DRAMC_REG_DDRCONF0, 0x1, DDRCONF0_DMSW_RST);

#if 1 //cc mark. registers not exist any more
    vIO32WriteFldMulti(DDRPHY_SHU1_CA_CMD6, P_Fld(0x0, SHU1_CA_CMD6_RG_ARPI_MIDPI_EN_CA)
    			| P_Fld(0x1, SHU1_CA_CMD6_RG_ARPI_MIDPI_CKDIV4_EN_CA));
    vIO32WriteFldMulti(DDRPHY_SHU1_B0_DQ6, P_Fld(0x0, SHU1_B0_DQ6_RG_ARPI_MIDPI_EN_B0)
    			| P_Fld(0x1, SHU1_B0_DQ6_RG_ARPI_MIDPI_CKDIV4_EN_B0));
    vIO32WriteFldMulti(DDRPHY_SHU1_B1_DQ6, P_Fld(0x0, SHU1_B1_DQ6_RG_ARPI_MIDPI_EN_B1)
    			| P_Fld(0x1, SHU1_B1_DQ6_RG_ARPI_MIDPI_CKDIV4_EN_B1));
#endif

    //>200ns
    mcDELAY_US(1);

    //CG for DLL release
    vIO32WriteFldAlign(DDRPHY_CA_DLL_ARPI2, 0x0, CA_DLL_ARPI2_RG_ARPI_MPDIV_CG_CA);
    vIO32WriteFldMulti(DDRPHY_B0_DLL_ARPI2, P_Fld(0x0, B0_DLL_ARPI2_RG_ARPI_CG_FB_B0)
    			| P_Fld(0x0, B0_DLL_ARPI2_RG_ARPI_MPDIV_CG_B0));
    //cc mark-> _B0 will do both bytes.vIO32WriteFldMulti(DDRPHY_B1_DLL_ARPI2, P_Fld(0x0, B1_DLL_ARPI2_RG_ARPI_CG_FB_B1)
    //			| P_Fld(0x0, B1_DLL_ARPI2_RG_ARPI_MPDIV_CG_B1));

    //solution for DA_ARPI_CG_FB
    vIO32WriteFldAlign(DDRPHY_MISC_CTRL1, 0, MISC_CTRL1_R_DMPHYRST);
    vIO32WriteFldAlign(DRAMC_REG_DDRCONF0, 0, DDRCONF0_DMSW_RST);
    vIO32WriteFldAlign(DDRPHY_PLL1, 0, PLL1_R_DMPHYCLKP0ENB);
    
	/* 8167 code end */

	vIO32WriteFldMulti(DDRPHY_CA_DLL_ARPI2, P_Fld(0x0, CA_DLL_ARPI2_RG_ARPI_CG_MCK_CA)
			| P_Fld(0x0, CA_DLL_ARPI2_RG_ARPI_CG_MCK_FB2DLL_CA)
			| P_Fld(0x0, CA_DLL_ARPI2_RG_ARPI_CG_MCTL_CA)
			| P_Fld(0x0, CA_DLL_ARPI2_RG_ARRPI_CG_FB_CA)
			| P_Fld(0x0, CA_DLL_ARPI2_RG_ARPI_CG_CS)
			| P_Fld(0x0, CA_DLL_ARPI2_RG_ARPI_CG_CLK)
			| P_Fld(0x0, CA_DLL_ARPI2_RG_ARPI_CG_CMD)
			| P_Fld(0x0, CA_DLL_ARPI2_RG_ARPI_CG_CLKIEN)
			| P_Fld(0x0, CA_DLL_ARPI2_RG_ARPI_MPDIV_CG_CA));
	vIO32WriteFldMulti(DDRPHY_B0_DLL_ARPI2, P_Fld(0x0, B0_DLL_ARPI2_RG_ARPI_CG_MCK_B0)
			| P_Fld(0x0, B0_DLL_ARPI2_RG_ARPI_CG_MCK_FB2DLL_B0)
			| P_Fld(0x0, B0_DLL_ARPI2_RG_ARPI_CG_MCTL_B0)
			| P_Fld(0x0, B0_DLL_ARPI2_RG_ARPI_CG_FB_B0)
			| P_Fld(0x0, B0_DLL_ARPI2_RG_ARPI_CG_DQS_B0)
			| P_Fld(0x0, B0_DLL_ARPI2_RG_ARPI_CG_DQM_B0)
			| P_Fld(0x0, B0_DLL_ARPI2_RG_ARPI_CG_DQ_B0)
			| P_Fld(0x0, B0_DLL_ARPI2_RG_ARPI_CG_DQSIEN_B0)
			| P_Fld(0x0, B0_DLL_ARPI2_RG_ARPI_MPDIV_CG_B0));
	vIO32WriteFldMulti(DDRPHY_B1_DLL_ARPI2, P_Fld(0x0, B1_DLL_ARPI2_RG_ARPI_CG_MCK_B1)
			| P_Fld(0x0, B1_DLL_ARPI2_RG_ARPI_CG_MCK_FB2DLL_B1)
			| P_Fld(0x0, B1_DLL_ARPI2_RG_ARPI_CG_MCTL_B1)
			//cc mark _B1 not used. | P_Fld(0x0, B1_DLL_ARPI2_RG_ARPI_CG_FB_B1)
			| P_Fld(0x0, B1_DLL_ARPI2_RG_ARPI_CG_DQS_B1)
			| P_Fld(0x0, B1_DLL_ARPI2_RG_ARPI_CG_DQM_B1)
			| P_Fld(0x0, B1_DLL_ARPI2_RG_ARPI_CG_DQ_B1)
			| P_Fld(0x0, B1_DLL_ARPI2_RG_ARPI_CG_DQSIEN_B1));
			//cc mark _B1 not used. | P_Fld(0x0, B1_DLL_ARPI2_RG_ARPI_MPDIV_CG_B1));
	
	mcDELAY_US(1);

	vIO32WriteFldAlign(DDRPHY_MISC_CG_CTRL0, 0x21f, MISC_CG_CTRL0_CLK_MEM_DFS_CFG);

	/* Master DLL enable */
	vIO32WriteFldMulti(DDRPHY_CA_DLL_ARPI5, P_Fld(0x1, CA_DLL_ARPI5_RG_ARDLL_PHDET_EN_CA)
			| P_Fld(0, CA_DLL_ARPI5_RG_ARDLL_MON_SEL_CA));

	mcDELAY_US(1);

	/* Slave DLL enable */
	vIO32WriteFldMulti(DDRPHY_B0_DLL_ARPI5, P_Fld(0x1, B0_DLL_ARPI5_RG_ARDLL_PHDET_EN_B0)
			| P_Fld(0, B0_DLL_ARPI5_RG_ARDLL_MON_SEL_B0));	
	vIO32WriteFldMulti(DDRPHY_B1_DLL_ARPI5, P_Fld(0x1, B1_DLL_ARPI5_RG_ARDLL_PHDET_EN_B1)
			| P_Fld(0, B1_DLL_ARPI5_RG_ARDLL_MON_SEL_B1));
	
	mcDELAY_US(1);	

#if LJPLL_FREQ_DEBUG_LOG
	DDRPhyFreqMeter();
#endif
}

/* cc notes: From 8167, only settings that set by 8167 && not_set by 7580 are kept */
static DRAM_STATUS_T UpdateInitialSettings_EN7523(DRAMC_CTX_T *p)
{	
#if COMPILE_PCDDR3
	vIO32WriteFldAlign(DDRPHY_B0_DQ3, 0x0, B0_DQ3_RG_BURST_TRACK_EN_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ3, 0x0, B1_DQ3_RG_BURST_TRACK_EN_B1);
	
	vIO32WriteFldMulti(DDRPHY_B0_DQ6, P_Fld(0x1, B0_DQ6_RG_TX_ARDQ_DDR3_SEL_B0)
		| P_Fld(0x0, B0_DQ6_RG_TX_ARDQ_DDR4_SEL_B0));
	vIO32WriteFldMulti(DDRPHY_B1_DQ6, P_Fld(0x1, B1_DQ6_RG_TX_ARDQ_DDR3_SEL_B1)
		| P_Fld(0x0, B1_DQ6_RG_TX_ARDQ_DDR4_SEL_B1));

	vIO32WriteFldMulti(DDRPHY_CA_CMD6, P_Fld(0x1, CA_CMD6_RG_RX_ARCMD_DDR3_SEL)
        | P_Fld(0x0, CA_CMD6_RG_RX_ARCMD_DDR4_SEL)          
        | P_Fld(0x1, CA_CMD6_RG_TX_ARCMD_ODTEN_EXT_DIS));
#endif
	
    vIO32WriteFldAlign(DDRPHY_B0_DQ2, 0x0, B0_DQ2_RG_TX_ARDQM0_ODTEN_DIS);
    vIO32WriteFldAlign(DDRPHY_B1_DQ2, 0x0, B1_DQ2_RG_TX_ARDQM1_ODTEN_DIS);

    vIO32WriteFldAlign(DRAMC_REG_SHU_SCINTV, 0x1f, SHU_SCINTV_SCINTV);
    vIO32WriteFldMulti(DRAMC_REG_CLKAR, P_Fld(0x1, CLKAR_SELPH_CMD_CG_DIS) | P_Fld(0x7FFF, CLKAR_REQQUE_PACG_DIS));
    vIO32WriteFldAlign(DRAMC_REG_SPCMDCTRL, 0x1, SPCMDCTRL_REFR_BLOCKEN);
    vIO32WriteFldAlign(DRAMC_REG_PERFCTL0, 0x0, PERFCTL0_WRFFIFO_OPT);
    vIO32WriteFldMulti(DRAMC_REG_PERFCTL0, P_Fld(0x1, PERFCTL0_REORDEREN)
                | P_Fld(0x0, PERFCTL0_RWHPRICTL));
	
    return DRAM_OK;
}

static DRAM_STATUS_T UpdateInitialSettings_MT7523_PC2(DRAMC_CTX_T *p)
{
	mcSHOW_DBG_MSG2(("UpdateInitialSettings_MT7523_PC2\n"));
	
	vIO32WriteFldMulti(DDRPHY_B0_DQ3, P_Fld(0x0, B0_DQ3_RG_BURST_TRACK_EN_B0)
	                        | P_Fld(0x0, B0_DQ3_RG_RX_ARDQS0_RPRE_TOG_EN_B0)
	                        | P_Fld(0x1, B0_DQ3_RG_RX_ARDQ_STBENCMP_EN_B0)
	                        | P_Fld(0x0, B0_DQ3_RG_RX_ARDQS0_DQSIENMODE)
	                        | P_Fld(0x1, B0_DQ3_RG_RX_ARDQ_IN_BUFF_EN_B0)
	                        | P_Fld(0x1, B0_DQ3_RG_RX_ARDQM0_IN_BUFF_EN)
	                        | P_Fld(0x1, B0_DQ3_RG_RX_ARDQS0_IN_BUFF_EN_B0)
	                        | P_Fld(0x0, B0_DQ3_RG_RX_ARDQ_SMT_EN_B0));
	vIO32WriteFldMulti(DDRPHY_B1_DQ3, P_Fld(0x0, B1_DQ3_RG_BURST_TRACK_EN_B1)
	                        | P_Fld(0x0, B1_DQ3_RG_RX_ARDQS0_RPRE_TOG_EN_B1)
	                        | P_Fld(0x1, B1_DQ3_RG_RX_ARDQ_STBENCMP_EN_B1)
	                        | P_Fld(0x0, B1_DQ3_RG_RX_ARDQS1_DQSIENMODE)
	                        | P_Fld(0x1, B1_DQ3_RG_RX_ARDQ_IN_BUFF_EN_B1)
	                        | P_Fld(0x1, B1_DQ3_RG_RX_ARDQM1_IN_BUFF_EN)
	                        | P_Fld(0x1, B1_DQ3_RG_RX_ARDQS1_IN_BUFF_EN)
	                        | P_Fld(0x0, B1_DQ3_RG_RX_ARDQ_SMT_EN_B1));
	vIO32WriteFldMulti(DDRPHY_B0_DQ6, P_Fld(0x0, B0_DQ6_RG_RX_ARDQ_DMRANK_OUTSEL_B0)
	                        | P_Fld(0x1, B0_DQ6_RG_RX_ARDQ_DDR3_SEL_B0)
	                        | P_Fld(0x0, B0_DQ6_RG_RX_ARDQ_DDR4_SEL_B0)
	                        | P_Fld(0x1, B0_DQ6_RG_RX_ARDQ_O1_SEL_B0)
	                        | P_Fld(0x1, B0_DQ6_RG_RX_RES_BIAS_EN_B0)
	                        | P_Fld(0x0, B0_DQ6_RG_RX_ARDQ_RPRE_TOG_EN_B0)
	                        | P_Fld(0x1, B0_DQ6_RG_TX_ARDQ_SER_MODE_B0)
	                        | P_Fld(0x1, B0_DQ6_RG_TX_ARDQ_ODTEN_EXT_DIS_B0)
	                        //| P_Fld(0x1, B0_DQ6_RG_TX_ARDQ_OE_EXT_DIS_B0) //byzj
	                        | P_Fld(0x0, B0_DQ6_RG_TX_ARDQ_OE_EXT_DIS_B0)
	                        | P_Fld(0x1, B0_DQ6_RG_RX_ARDQ_BIAS_PS_B0));
	vIO32WriteFldMulti(DDRPHY_B1_DQ6, P_Fld(0x0, B1_DQ6_RG_RX_ARDQ_DMRANK_OUTSEL_B1)
	                        | P_Fld(0x1, B1_DQ6_RG_RX_ARDQ_DDR3_SEL_B1)
	                        | P_Fld(0x0, B1_DQ6_RG_RX_ARDQ_DDR4_SEL_B1)
	                        | P_Fld(0x1, B1_DQ6_RG_RX_ARDQ_O1_SEL_B1)
	                        | P_Fld(0x1, B1_DQ6_RG_RX_RES_BIAS_EN_B1)
	                        | P_Fld(0x0, B1_DQ6_RG_RX_ARDQ_RPRE_TOG_EN_B1)
	                        | P_Fld(0x1, B1_DQ6_RG_TX_ARDQ_SER_MODE_B1)
	                        | P_Fld(0x1, B1_DQ6_RG_TX_ARDQ_ODTEN_EXT_DIS_B1)
	                        //| P_Fld(0x1, B1_DQ6_RG_TX_ARDQ_OE_EXT_DIS_B1) //byzj
	                        | P_Fld(0x0, B1_DQ6_RG_TX_ARDQ_OE_EXT_DIS_B1)
	                        | P_Fld(0x1, B1_DQ6_RG_RX_ARDQ_BIAS_PS_B1));
	vIO32WriteFldMulti(DDRPHY_CA_CMD6, P_Fld(0x0, CA_CMD6_RG_RX_ARCMD_DMRANK_OUTSEL)
	                        | P_Fld(0x1, CA_CMD6_RG_RX_ARCMD_DDR3_SEL)
	                        | P_Fld(0x0, CA_CMD6_RG_RX_ARCMD_DDR4_SEL)
	                        | P_Fld(0x1, CA_CMD6_RG_RX_ARCMD_O1_SEL)
	                        | P_Fld(0x1, CA_CMD6_RG_RX_ARCMD_RES_BIAS_EN)
	                        | P_Fld(0x0, CA_CMD6_RG_RX_ARCMD_RPRE_TOG_EN)
	                        | P_Fld(0x1, CA_CMD6_RG_TX_ARCMD_ODTEN_EXT_DIS)
	                        //| P_Fld(0x1, CA_CMD6_RG_TX_ARCMD_OE_EXT_DIS) //byzj
	                        | P_Fld(0x0, CA_CMD6_RG_TX_ARCMD_OE_EXT_DIS)
	                        | P_Fld(0x1, CA_CMD6_RG_RX_ARCMD_BIAS_PS));
	
	vIO32WriteFldMulti(DDRPHY_B0_DQ2, P_Fld(0x0, B0_DQ2_RG_TX_ARDQ_ODTEN_DIS_B0)
	                        | P_Fld(0x0, B0_DQ2_RG_TX_ARDQM0_ODTEN_DIS)
	                        | P_Fld(0x0, B0_DQ2_RG_TX_ARDQS0_ODTEN_DIS)
	                        | P_Fld(0x1, B0_DQ2_RG_TX_ARDQS0_OE_EXT_DIS) //byzj
	                        | P_Fld(0x1, B0_DQ2_RG_TX_ARDQS0_ODTEN_EXT_DIS));
	vIO32WriteFldMulti(DDRPHY_B1_DQ2, P_Fld(0x0, B1_DQ2_RG_TX_ARDQ_ODTEN_DIS_B1)
	                        | P_Fld(0x0, B1_DQ2_RG_TX_ARDQM1_ODTEN_DIS)
	                        | P_Fld(0x0, B1_DQ2_RG_TX_ARDQS1_ODTEN_DIS)
	                        | P_Fld(0x1, B1_DQ2_RG_TX_ARDQS0_OE_EXT_DIS) //byzj
	                        | P_Fld(0x1, B1_DQ2_RG_TX_ARDQS0_ODTEN_EXT_DIS));

	vIO32WriteFldAlign(DDRPHY_B0_DQ8, 0, B0_DQ8_RG_T2RLPBK_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ8, 0, B1_DQ8_RG_T2RLPBK_B1);

	vIO32WriteFldAlign(DDRPHY_B0_DQ5, 1, B0_DQ5_RG_RX_ARDQ_VREF_EN_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ5, 1, B1_DQ5_RG_RX_ARDQ_VREF_EN_B1);

#if 0 //byzj
	if (platform_chip_ver() == CHIP_VER_E1)
	{
	vIO32WriteFldMulti(DDRPHY_SHU1_B0_DQ5, P_Fld(5, SHU1_B0_DQ5_RG_RX_ARDQS0_DQSIEN_DLY_B0)
	                        | P_Fld(0x0, SHU1_B0_DQ5_RG_RX_ARDQ_VREF_BYPASS_B0)
	                        | P_Fld(0xe, SHU1_B0_DQ5_RG_RX_ARDQ_VREF_SEL_B0));
	vIO32WriteFldMulti(DDRPHY_SHU1_B1_DQ5, P_Fld(5, SHU1_B1_DQ5_RG_RX_ARDQS0_DQSIEN_DLY_B1)
	                        | P_Fld(0x0, SHU1_B1_DQ5_RG_RX_ARDQ_VREF_BYPASS_B1)
	                        | P_Fld(0xe, SHU1_B1_DQ5_RG_RX_ARDQ_VREF_SEL_B1));
	}
	else
#endif
	{
	vIO32WriteFldMulti(DDRPHY_SHU1_B0_DQ5, P_Fld(1, SHU1_B0_DQ5_RG_RX_ARDQS0_DQSIEN_DLY_B0)
	                        | P_Fld(0x0, SHU1_B0_DQ5_RX_RX_ARDQ_VREF_BYPASS_B0)
	                        | P_Fld(0xe, SHU1_B0_DQ5_RG_RX_ARDQ_VREF_SEL_B0));
	vIO32WriteFldMulti(DDRPHY_SHU1_B1_DQ5, P_Fld(1, SHU1_B1_DQ5_RG_RX_ARDQS0_DQSIEN_DLY_B1)
	                        | P_Fld(0x0, SHU1_B1_DQ5_RG_RX_ARDQ_VREF_BYPASS_B1)
	                        | P_Fld(0xe, SHU1_B1_DQ5_RG_RX_ARDQ_VREF_SEL_B1));
	}
	vIO32WriteFldAlign(DRAMC_REG_SHU_SCINTV, 0x1f, SHU_SCINTV_SCINTV);
	vIO32WriteFldMulti(DRAMC_REG_CLKAR, P_Fld(0x1, CLKAR_SELPH_CMD_CG_DIS) | P_Fld(0x7FFF, CLKAR_REQQUE_PACG_DIS));
	vIO32WriteFldAlign(DRAMC_REG_SPCMDCTRL, 0x1, SPCMDCTRL_REFR_BLOCKEN);
	vIO32WriteFldAlign(DRAMC_REG_PERFCTL0, 0x0, PERFCTL0_WRFFIFO_OPT);
	vIO32WriteFldMulti(DRAMC_REG_PERFCTL0, P_Fld(0x1, PERFCTL0_REORDEREN)
	        | P_Fld(0x0, PERFCTL0_RWHPRICTL));
	return DRAM_OK;
}

/* Porting from Azalea & 8167 */
static DRAM_STATUS_T DDRPHYSetting_EN7523(DRAMC_CTX_T *p)
{
	U32 u4SDM_PCW,u4PREDIV,u4POSDIV,u4EXTFBDIV;
	U32 u4Result;
	U32 u4clock;
	U32 u4value;

	mcDELAY_US(1);

	//	vIO32WriteFldAlign(DRAM_DMARB_BASE, 0x1b02,RG_FLD_FULL);//dmarb 6000='he0001b02
	mcSHOW_DBG_MSG(("[DDRPHYInit] ====DDRPHYSetting_EN7523====\n"));
	vIO32WriteFldAlign(DRAMC_REG_DDRCONF0, 1, DDRCONF0_DRAMC_SW_RST);
	mcDELAY_US(1);
	vIO32WriteFldAlign(DRAMC_REG_DDRCONF0, 0, DDRCONF0_DRAMC_SW_RST);

	//COSIM ADD
	vIO32WriteFldMulti(DRAMC_REG_CLKCTRL, P_Fld(0x1, CLKCTRL_CLK_EN_1)
				| P_Fld(0x1, CLKCTRL_CLK_EN_0)
				| P_Fld(0xc, CLKCTRL_PSEL_CNT));

	mcSHOW_DBG_MSG(("Start init phy...\n"));

	//ddrphy vref setting
	vIO32WriteFldMulti(DDRPHY_CA_CMD5, P_Fld(0xe, CA_CMD5_RG_RX_ARCMD_EYE_VREF_SEL));
	//vIO32WriteFldMulti(DDRPHY_CA_CMD5+(1<<POS_BANK_NUM), P_Fld(0xe, CA_CMD5_RG_RX_ARCMD_EYE_VREF_SEL));
	vIO32WriteFldMulti(DDRPHY_SHU1_CA_CMD5, P_Fld(0x0, SHU1_CA_CMD5_RG_ARPI_FB_CA)//cc notes: change from 0x6->0x1 from DE golden.
				| P_Fld(0xe, SHU1_CA_CMD5_RG_RX_ARCMD_VREF_SEL));

	vIO32WriteFldAlign(DDRPHY_MISC_SPM_CTRL1, 0x0, MISC_SPM_CTRL1_PHY_SPM_CTL1);
	vIO32WriteFldAlign(DDRPHY_MISC_SPM_CTRL0, 0xffffffff, MISC_SPM_CTRL0_PHY_SPM_CTL0);
	vIO32WriteFldAlign(DDRPHY_MISC_SPM_CTRL2, 0xffffffff, MISC_SPM_CTRL2_PHY_SPM_CTL2);
	vIO32WriteFldAlign(DDRPHY_MISC_CG_CTRL2, 0x6003bf, MISC_CG_CTRL2_RG_MEM_DCM_CTL);
	vIO32WriteFldAlign(DDRPHY_MISC_CG_CTRL4, 0x13300000, MISC_CG_CTRL4_R_PHY_MCK_CG_CTRL);
	//vIO32WriteFldAlign(DDRPHY_MISC_CG_CTRL2+(1<<POS_BANK_NUM), 0x6003bf, MISC_CG_CTRL2_RG_MEM_DCM_CTL);
	//vIO32WriteFldAlign(DDRPHY_MISC_CG_CTRL4+(1<<POS_BANK_NUM), 0x222a2a00, MISC_CG_CTRL4_R_PHY_MCK_CG_CTRL);


	if(p->dram_type == TYPE_PCDDR3)
	{
	#if COMPILE_PCDDR3
		mcSHOW_DBG_MSG(("PCDDR3 Pinmux %d\n", p->pinmux));
		vIO32WriteFldMulti(DDRPHY_SHU1_CA_CMD7, P_Fld(0x0, SHU1_CA_CMD7_RG_ARCMD_REV)
					 | P_Fld(0x1, SHU1_CA_CMD7_R_DMRANKRXDVS_CA));
	#endif
	}
	
	vIO32WriteFldMulti(DDRPHY_SHU1_B0_DQ7, P_Fld(0x1, SHU1_B0_DQ7_MIDPI_DIV4_ENABLE)
				| P_Fld(0x1, SHU1_B0_DQ7_R_DMRANKRXDVS_B0));

	vIO32WriteFldMulti(DDRPHY_SHU1_B1_DQ7, P_Fld(0x1, SHU1_B1_DQ7_R_DMRANKRXDVS_B1));
	
	vIO32WriteFldAlign(DDRPHY_MISC_RXDVS1, 0x7, MISC_RXDVS1_R_IN_GATE_EN_LOW_OPT);
	//vIO32WriteFldAlign(DDRPHY_MISC_RXDVS1+(1<<POS_BANK_NUM), 0x7, MISC_RXDVS1_R_IN_GATE_EN_LOW_OPT);
	vIO32WriteFldAlign(DDRPHY_PLL3, 0x0, PLL3_RG_RPHYPLL_TSTOP_EN);
	vIO32WriteFldMulti(DDRPHY_MISC_VREF_CTRL, P_Fld(0x1, MISC_VREF_CTRL_RG_RVREF_VREF_EN)
				| P_Fld(0x12, MISC_VREF_CTRL_RG_RVREF_SEL_CMD)
				| P_Fld(0x1, MISC_VREF_CTRL_RG_RVREF_DDR3_SEL)
				| P_Fld(0x0, MISC_VREF_CTRL_RG_RVREF_DDR4_SEL)
				| P_Fld(0x12, MISC_VREF_CTRL_RG_RVREF_SEL_DQ));
	vIO32WriteFldMulti(DDRPHY_MISC_IMP_CTRL0, P_Fld(0x12, MISC_IMP_CTRL0_RG_RIMP_VREF_SEL)//cc change from 0x2e->0x12
				| P_Fld(0x1, MISC_IMP_CTRL0_RG_RIMP_DDR3_SEL)
				| P_Fld(0x0, MISC_IMP_CTRL0_RG_RIMP_DDR4_SEL));
	vIO32WriteFldMulti(DDRPHY_SHU1_B0_DQ5, P_Fld(0x5, SHU1_B0_DQ5_RG_RX_ARDQS0_DVS_DLY_B0)
				| P_Fld(0x0, SHU1_B0_DQ5_RG_ARPI_FB_B0));//cc change form 0x6->0x0 from DE golden.
	vIO32WriteFldMulti(DDRPHY_SHU1_B1_DQ5, P_Fld(0x5, SHU1_B1_DQ5_RG_RX_ARDQS0_DVS_DLY_B1)
				| P_Fld(0x0, SHU1_B1_DQ5_RG_ARPI_FB_B1));//cc change form 0x6->0x0 from DE golden.

	mcDELAY_US(1);

	vIO32WriteFldAlign(DDRPHY_B0_DQ3, 0x1, B0_DQ3_RG_RX_ARDQ_STBEN_RESETB_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ3, 0x1, B1_DQ3_RG_RX_ARDQ_STBEN_RESETB_B1);
	vIO32WriteFldAlign(DDRPHY_MISC_CG_CTRL1, 0x3f600, MISC_CG_CTRL1_R_DVS_DIV4_CG_CTRL);
	//vIO32WriteFldAlign(DDRPHY_MISC_CG_CTRL1+(1<<POS_BANK_NUM), 0xff7befdf, MISC_CG_CTRL1_R_DVS_DIV4_CG_CTRL);

	vIO32WriteFldMulti(DDRPHY_B0_DQ4, P_Fld(0x10, B0_DQ4_RG_RX_ARDQS_EYE_R_DLY_B0)
				| P_Fld(0x10, B0_DQ4_RG_RX_ARDQS_EYE_F_DLY_B0));
	vIO32WriteFldMulti(DDRPHY_B0_DQ5, P_Fld(0x0, B0_DQ5_RG_RX_ARDQ_EYE_EN_B0) //cc change from 1 -> 0
				| P_Fld(0x1, B0_DQ5_RG_RX_ARDQ_EYE_SEL_B0)
				| P_Fld(0xe, B0_DQ5_RG_RX_ARDQ_EYE_VREF_SEL_B0));
	vIO32WriteFldMulti(DDRPHY_B0_DQ6, P_Fld(0x1, B0_DQ6_RG_RX_ARDQ_EYE_DLY_DQS_BYPASS_B0)
				| P_Fld(0x0, B0_DQ6_RG_RX_ARDQ_BIAS_VREF_SEL_B0)
				| P_Fld(0x1, B0_DQ6_RG_RX_ARDQ_BIAS_EN_B0)
				| P_Fld(0x1, B0_DQ6_RG_RX_ARDQ_OP_BIAS_SW_EN_B0));
	vIO32WriteFldAlign(DDRPHY_B0_DQ5, 0x1, B0_DQ5_RG_RX_ARDQ_EYE_STBEN_RESETB_B0);
	vIO32WriteFldMulti(DDRPHY_B1_DQ4, P_Fld(0x10, B1_DQ4_RG_RX_ARDQS_EYE_R_DLY_B1)
				| P_Fld(0x10, B1_DQ4_RG_RX_ARDQS_EYE_F_DLY_B1));
	vIO32WriteFldMulti(DDRPHY_B1_DQ5, P_Fld(0x0, B1_DQ5_RG_RX_ARDQ_EYE_EN_B1) //cc change from 1 ->0
				| P_Fld(0x1, B1_DQ5_RG_RX_ARDQ_EYE_SEL_B1)
				| P_Fld(0xe, B1_DQ5_RG_RX_ARDQ_EYE_VREF_SEL_B1));
	vIO32WriteFldMulti(DDRPHY_B1_DQ6, P_Fld(0x1, B1_DQ6_RG_RX_ARDQ_EYE_DLY_DQS_BYPASS_B1)
				| P_Fld(0x0, B1_DQ6_RG_RX_ARDQ_BIAS_VREF_SEL_B1)
				| P_Fld(0x1, B1_DQ6_RG_RX_ARDQ_BIAS_EN_B1)
				| P_Fld(0x1, B1_DQ6_RG_RX_ARDQ_OP_BIAS_SW_EN_B1));
	vIO32WriteFldAlign(DDRPHY_B1_DQ5, 0x1, B1_DQ5_RG_RX_ARDQ_EYE_STBEN_RESETB_B1);

#if 0// cc mark. DE confirms CMD not need
	vIO32WriteFldMulti(DDRPHY_CA_CMD6, P_Fld(0x1, CA_CMD6_RG_TX_ARCMD_DDR3_SEL)
				| P_Fld(0x1, CA_CMD6_RG_RX_ARCMD_DDR3_SEL)
				| P_Fld(0x0, CA_CMD6_RG_TX_ARCMD_DDR4_SEL)
				| P_Fld(0x0, CA_CMD6_RG_RX_ARCMD_DDR4_SEL)
				| P_Fld(0x0, CA_CMD6_RG_RX_ARCMD_BIAS_VREF_SEL));
	vIO32WriteFldMulti(DDRPHY_CA_CMD6+(1<<POS_BANK_NUM), P_Fld(0x1, CA_CMD6_RG_TX_ARCMD_DDR3_SEL)
				| P_Fld(0x1, CA_CMD6_RG_RX_ARCMD_DDR3_SEL)
				| P_Fld(0x0, CA_CMD6_RG_TX_ARCMD_DDR4_SEL)
				| P_Fld(0x0, CA_CMD6_RG_RX_ARCMD_DDR4_SEL)
				| P_Fld(0x0, CA_CMD6_RG_RX_ARCMD_BIAS_VREF_SEL));
#endif

	vIO32WriteFldAlign(DDRPHY_B0_DQ3, 0x1, B0_DQ3_RG_ARDQ_RESETB_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ3, 0x1, B1_DQ3_RG_ARDQ_RESETB_B1);
	vIO32WriteFldAlign(DDRPHY_CA_CMD3, 0x1, CA_CMD3_RG_ARCMD_RESETB); //cc add
	
	vIO32WriteFldMulti(DDRPHY_CA_CMD8, P_Fld(0x1, CA_CMD8_RG_TX_RRESETB_DDR3_SEL)
				| P_Fld(0x1, CA_CMD8_RG_RRESETB_DRVN)
				| P_Fld(0x1, CA_CMD8_RG_RRESETB_DRVP));
	vIO32WriteFldMulti(DDRPHY_MISC_IMP_CTRL1, P_Fld(0x0, MISC_IMP_CTRL1_RG_RIMP_REV)
				| P_Fld(0x0, MISC_IMP_CTRL1_RG_RIMP_PRE_EN));
	vIO32WriteFldAlign(DDRPHY_MISC_CG_CTRL4, 0x13400000, MISC_CG_CTRL4_R_PHY_MCK_CG_CTRL);
	vIO32WriteFldMulti(DDRPHY_MISC_SHU_OPT, 0xfff0f0f0);
	//cc mark vIO32WriteFldAlign(DDRPHY_MISC_CG_CTRL0+(1<<POS_BANK_NUM), 0x100000, MISC_CG_CTRL0_CLK_MEM_DFS_CFG);
	//cc mark vIO32WriteFldAlign(DDRPHY_MISC_CG_CTRL0, 0x1f, MISC_CG_CTRL0_CLK_MEM_DFS_CFG);
	/* 8167 code end */
	
	/* PLL, CKGEN, DLL, PI init */
	mcSHOW_DBG_MSG(("Init PLL, CKGEN, DLL, PI\n"));
	DDRPhyPLLSetting_EN7523(p);
	
	mcSHOW_DBG_MSG(("Tx setting init\n"));

	/* cc add */
	//Tx Init setting
	vIO32WriteFldMulti(DDRPHY_B0_DQ7, P_Fld(0x0, B0_DQ7_RG_TX_ARDQM0_PULL_UP_B0)
		| P_Fld(0x0, B0_DQ7_RG_TX_ARDQM0_PULL_DN_B0)
		| P_Fld(0x0, B0_DQ7_RG_TX_ARDQ_PULL_DN_B0)
		| P_Fld(0x0, B0_DQ7_RG_TX_ARDQ_PULL_UP_B0)
		| P_Fld(0x0, B0_DQ7_RG_TX_ARDQS0_PULL_DN_B0)
		| P_Fld(0x0, B0_DQ7_RG_TX_ARDQS0_PULL_UP_B0)
		| P_Fld(0x0, B0_DQ7_RG_TX_ARDQS0B_PULL_DN_B0)
		| P_Fld(0x0, B0_DQ7_RG_TX_ARDQS0B_PULL_UP_B0));

	vIO32WriteFldMulti(DDRPHY_B1_DQ7, P_Fld(0x0, B1_DQ7_RG_TX_ARDQM0_PULL_UP_B1)
		| P_Fld(0x0, B1_DQ7_RG_TX_ARDQM0_PULL_DN_B1)
		| P_Fld(0x0, B1_DQ7_RG_TX_ARDQ_PULL_DN_B1)
		| P_Fld(0x0, B1_DQ7_RG_TX_ARDQ_PULL_UP_B1)
		| P_Fld(0x0, B1_DQ7_RG_TX_ARDQS0_PULL_DN_B1)
		| P_Fld(0x0, B1_DQ7_RG_TX_ARDQS0_PULL_UP_B1)
		| P_Fld(0x0, B1_DQ7_RG_TX_ARDQS0B_PULL_DN_B1)
		| P_Fld(0x0, B1_DQ7_RG_TX_ARDQS0B_PULL_UP_B1));

	if (p->dram_type == TYPE_PCDDR3) {
	#if COMPILE_PCDDR3
		vIO32WriteFldMulti(DDRPHY_SHU1_B0_DQ1, P_Fld(0x5, SHU1_B0_DQ1_RG_TX_ARDQ_ODTN_B0)
			| P_Fld(0x5, SHU1_B0_DQ1_RG_TX_ARDQ_ODTP_B0)
			| P_Fld(0xc, SHU1_B0_DQ1_RG_TX_ARDQ_DRVN_B_B0) //cc notes: DRVN_B?
			| P_Fld(0xc, SHU1_B0_DQ1_RG_TX_ARDQ_DRVP_B_B0)); //cc notes: DRVP_B??
		vIO32WriteFldMulti(DDRPHY_SHU1_B0_DQ2, P_Fld(0x5, SHU1_B0_DQ2_RG_TX_ARDQS0_ODTN_B0)
			| P_Fld(0x5, SHU1_B0_DQ2_RG_TX_ARDQS0_ODTP_B0)
			| P_Fld(0xc, SHU1_B0_DQ2_RG_TX_ARDQS0_DRVN_B0)
			| P_Fld(0xc, SHU1_B0_DQ2_RG_TX_ARDQS0_DRVP_B0));

		vIO32WriteFldMulti(DDRPHY_SHU1_B1_DQ1, P_Fld(0x5, SHU1_B1_DQ1_RG_TX_ARDQ_ODTN_B1)
			| P_Fld(0x5, SHU1_B1_DQ1_RG_TX_ARDQ_ODTP_B1)
			| P_Fld(0xc, SHU1_B1_DQ1_RG_TX_ARDQ_DRVN_B1) //cc notes: DRVN_B?
			| P_Fld(0xc, SHU1_B1_DQ1_RG_TX_ARDQ_DRVP_B1)); //cc notes: DRVP_B??
		vIO32WriteFldMulti(DDRPHY_SHU1_B1_DQ2, P_Fld(0x5, SHU1_B1_DQ2_RG_TX_ARDQS0_ODTN_B1)
			| P_Fld(0x5, SHU1_B1_DQ2_RG_TX_ARDQS0_ODTP_B1)
			| P_Fld(0xc, SHU1_B1_DQ2_RG_TX_ARDQS0_DRVN_B1)
			| P_Fld(0xc, SHU1_B1_DQ2_RG_TX_ARDQS0_DRVP_B1));
	#endif
	}

	vIO32WriteFldAlign(DDRPHY_B0_DQ3, 0x1, B0_DQ3_RG_TX_ARDQ_EN_B0);
	vIO32WriteFldMulti(DDRPHY_B0_DQ6, P_Fld(0x1, B0_DQ6_RG_TX_ARDQ_SER_MODE_B0) //cc notes: 8UI mode
		| P_Fld(0x0, B0_DQ6_RG_TX_ARDQ_ODTEN_EXT_DIS_B0)
		| P_Fld(0x0, B0_DQ6_RG_TX_ARDQ_OE_EXT_DIS_B0));
	
	vIO32WriteFldAlign(DDRPHY_B1_DQ3, 0x1, B1_DQ3_RG_TX_ARDQ_EN_B1);
	vIO32WriteFldMulti(DDRPHY_B1_DQ6, P_Fld(0x1, B1_DQ6_RG_TX_ARDQ_SER_MODE_B1) //cc notes: 8UI mode
		| P_Fld(0x0, B1_DQ6_RG_TX_ARDQ_ODTEN_EXT_DIS_B1)
		| P_Fld(0x0, B1_DQ6_RG_TX_ARDQ_OE_EXT_DIS_B1));

	vIO32WriteFldMulti(DDRPHY_B0_DQ2, P_Fld(0x0, B0_DQ2_RG_TX_ARDQ_OE_DIS_B0)
		| P_Fld(0x0, B0_DQ2_RG_TX_ARDQ_ODTEN_DIS_B0)
		| P_Fld(0x0, B0_DQ2_RG_TX_ARDQS0_OE_DIS)
		| P_Fld(0x0, B0_DQ2_RG_TX_ARDQS0_ODTEN_DIS)
		| P_Fld(0x0, B0_DQ2_RG_TX_ARDQS0_OE_EXT_DIS)
		| P_Fld(0x0, B0_DQ2_RG_TX_ARDQS0_ODTEN_EXT_DIS));	
	vIO32WriteFldMulti(DDRPHY_B1_DQ2, P_Fld(0x0, B1_DQ2_RG_TX_ARDQ_OE_DIS_B1)
		| P_Fld(0x0, B1_DQ2_RG_TX_ARDQ_ODTEN_DIS_B1)		
		| P_Fld(0x0, B1_DQ2_RG_TX_ARDQS1_OE_DIS)
		| P_Fld(0x0, B1_DQ2_RG_TX_ARDQS1_ODTEN_DIS)
		| P_Fld(0x0, B1_DQ2_RG_TX_ARDQS0_OE_EXT_DIS) //cc notes: coda may name it incorrect
		| P_Fld(0x0, B1_DQ2_RG_TX_ARDQS0_ODTEN_EXT_DIS));	

	vIO32WriteFldAlign(DDRPHY_CA_CMD3, 0x1, CA_CMD3_RG_TX_ARCMD_EN);
	vIO32WriteFldMulti(DDRPHY_CA_CMD6, P_Fld(0x1, CA_CMD6_RG_TX_ARCMD_SER_MODE)
		| P_Fld(0x0, CA_CMD6_RG_TX_ARCMD_OE_EXT_DIS));

	vIO32WriteFldMulti(DDRPHY_CA_CMD2, P_Fld(0x0, CA_CMD2_RG_TX_ARCLK_OE_DIS)
		| P_Fld(0x0, CA_CMD2_RG_TX_ARCMD_OE_DIS)
		| P_Fld(0x0, CA_CMD2_RG_TX_ARCLK_OE_EXT_DIS)
		| P_Fld(0x0, CA_CMD2_RG_TX_ARCLK_ODTEN_DIS)
		| P_Fld(0x0, CA_CMD2_RG_TX_ARCMD_ODTEN_DIS));

	vIO32WriteFldMulti(DDRPHY_CA_CMD7, P_Fld(0x0, CA_CMD7_RG_TX_ARCMD_D0_PULL_DN)
		| P_Fld(0x0, CA_CMD7_RG_TX_ARCMD_D0_PULL_UP)
		| P_Fld(0x0, CA_CMD7_RG_TX_ARCMD_D1_PULL_DN)
		| P_Fld(0x0, CA_CMD7_RG_TX_ARCMD_D1_PULL_UP)
		| P_Fld(0x0, CA_CMD7_RG_TX_ARCLK_PULL_DN)
		| P_Fld(0x0, CA_CMD7_RG_TX_ARCLK_PULL_UP)
		| P_Fld(0x0, CA_CMD7_RG_TX_ARCLKB_PULL_UP)
		| P_Fld(0x0, CA_CMD7_RG_TX_ARCLKB_PULL_DN)
		| P_Fld(0x0, CA_CMD7_RG_TX_ARRESETB_PULL_UP)
		| P_Fld(0x0, CA_CMD7_RG_TX_ARRESETB_PULL_DN));
	
	if (p->dram_type == TYPE_PCDDR3) {
	#if COMPILE_PCDDR3
		vIO32WriteFldMulti(DDRPHY_SHU1_CA_CMD2, P_Fld(0xc, SHU1_CA_CMD2_RG_TX_ARCLK_DRVN)
			| P_Fld(0xc, SHU1_CA_CMD2_RG_TX_ARCLK_DRVP));
		vIO32WriteFldMulti(DDRPHY_SHU1_CA_CMD1, P_Fld(0xc, SHU1_CA_CMD1_RG_TX_ARCMD_DRVN_B) //cc notes: DRVN_B
			| P_Fld(0xc, SHU1_CA_CMD1_RG_TX_ARCMD_DRVP_B)); //cc notes: DRVP_B??
	#endif
	} 
	
	mcSHOW_DBG_MSG(("Rx setting init\n"));
	//Rx init setting
	vIO32WriteFldMulti(DDRPHY_B0_DQ3, P_Fld(0x0, B0_DQ3_RG_RX_ARDQM0_DQSI_SEL_B0)
		//cc mark after DE review. Will be set later as 0x00 | P_Fld(0x0, B0_DQ3_RG_RX_ARDQ_DQSI_SEL_B0)
		| P_Fld(0x0, B0_DQ3_RG_RX_ARDQS0_SWAP_EN_B0)
		| P_Fld(0x1, B0_DQ3_RG_RX_ARDQ_STBENCMP_EN_B0) //cc note: affect PC4 gating R0_R/R0_F
		| P_Fld(0x1, B0_DQ3_RG_RX_ARDQM0_IN_BUFF_EN)
		| P_Fld(0x1, B0_DQ3_RG_RX_ARDQ_IN_BUFF_EN_B0)
		| P_Fld(0x1, B0_DQ3_RG_RX_ARDQS0_IN_BUFF_EN_B0)
		| P_Fld(0x0, B0_DQ3_RG_RX_ARDQ_SMT_EN_B0));//cc change from 1->0 according to DE flow 10.23 version
	vIO32WriteFldMulti(DDRPHY_B1_DQ3, P_Fld(0x0, B1_DQ3_RG_RX_ARDQM0_DQSI_SEL_B1)
		//cc mark after DE review. Will be set later as 0xff | P_Fld(0x0, B1_DQ3_RG_RX_ARDQ_DQSI_SEL_B1)
		| P_Fld(0x0, B1_DQ3_RG_RX_ARDQS0_SWAP_EN_B1)
		| P_Fld(0x1, B1_DQ3_RG_RX_ARDQ_STBENCMP_EN_B1) //cc note: affect PC4 gating R0_R/R0_F		
		| P_Fld(0x1, B1_DQ3_RG_RX_ARDQM1_IN_BUFF_EN)
		| P_Fld(0x1, B1_DQ3_RG_RX_ARDQ_IN_BUFF_EN_B1)
		| P_Fld(0x1, B1_DQ3_RG_RX_ARDQS1_IN_BUFF_EN)
		| P_Fld(0x0, B1_DQ3_RG_RX_ARDQ_SMT_EN_B1));//cc change from 1->0 according to DE flow 10.23 version

	if (p->dram_type == TYPE_PCDDR3) {
		vIO32WriteFldMulti(DDRPHY_B01_MISC, P_Fld(0x0, B01_MISC_RG_RX_ARDQS0_BURST_EN_B01)
			| P_Fld(0x1, B01_MISC_RG_RX_ARDQS0_LP4_BURSTMODE_SEL_B01)); //cc change from 0->1 from DE golden
			//cc mark. will be set during CKGEN init | P_Fld(0x1, B01_MISC_RG_ARPI_SMT_EN_B01)); //cc change from 0->1 from DE golden
		vIO32WriteFldMulti(DDRPHY_B0_DQ3, P_Fld(0x0, B0_DQ3_RG_RX_ARDQS0_DQSIENMODE)
			| P_Fld(0x0, B0_DQ3_RG_RX_ARDQS0_RPRE_TOG_EN_B0));
		vIO32WriteFldAlign(DDRPHY_B0_DQ6, 0x0, B0_DQ6_RG_RX_ARDQ_RPRE_TOG_EN_B0);
		vIO32WriteFldAlign(DDRPHY_B1_DQ6, 0x0, B1_DQ6_RG_RX_ARDQ_RPRE_TOG_EN_B1);
		vIO32WriteFldMulti(DDRPHY_B1_DQ3, P_Fld(0x0, B1_DQ3_RG_RX_ARDQS1_DQSIENMODE)			
			| P_Fld(0x0, B1_DQ3_RG_RX_ARDQS0_RPRE_TOG_EN_B1));
		vIO32WriteFldAlign(DDRPHY_CA_CMD6, 0x0, CA_CMD6_RG_RX_ARCMD_RPRE_TOG_EN);
	}

	vIO32WriteFldAlign(DDRPHY_SHU1_B0_DQ5, 0x0, SHU1_B0_DQ5_RG_RX_ARDQS0_DQSIEN_DLY_B0);
	vIO32WriteFldAlign(DDRPHY_SHU1_B1_DQ5, 0x0, SHU1_B1_DQ5_RG_RX_ARDQS0_DQSIEN_DLY_B1);
	
	vIO32WriteFldAlign(DDRPHY_CA_MISC, 0x1, CA_MISC_RG_ARPI_SMT_EN_CA);//cc change from 0->1 from DE golden
#if ((CA_TWOTCMD_EN == 1) && (DUAL_RANK_ENABLE == 1))	//cc add for DUAL rank better timing
	vIO32WriteFldAlign(DDRPHY_CA_MISC, 0x1, CA_MISC_RG_TX_ARCS1_MCK4X_SEL);
#else
	vIO32WriteFldAlign(DDRPHY_CA_MISC, 0x0, CA_MISC_RG_TX_ARCS1_MCK4X_SEL);
#endif
	vIO32WriteFldAlign(DDRPHY_CA_CMD3, 0x0, CA_CMD3_RG_RX_ARCMD_SMT_EN);
	vIO32WriteFldAlign(DDRPHY_CA_CMD8, 0x0, CA_CMD8_RG_RX_RRESETB_SMT_EN);

	vIO32WriteFldMulti(DDRPHY_B0_DQ6, P_Fld(0x1, B0_DQ6_RG_RX_ARDQ_O1_SEL_B0)
		| P_Fld(0x0, B0_DQ6_RG_RX_ARDQ_BIAS_PS_B0)
		| P_Fld(0x1, B0_DQ6_RG_RX_RES_BIAS_EN_B0)
		| P_Fld(0x0, B0_DQ6_RG_RX_ARDQ_DMRANK_OUTSEL_B0));
	vIO32WriteFldMulti(DDRPHY_B1_DQ6, P_Fld(0x1, B1_DQ6_RG_RX_ARDQ_O1_SEL_B1)
		| P_Fld(0x0, B1_DQ6_RG_RX_ARDQ_BIAS_PS_B1)
		| P_Fld(0x1, B1_DQ6_RG_RX_RES_BIAS_EN_B1)
		| P_Fld(0x0, B1_DQ6_RG_RX_ARDQ_DMRANK_OUTSEL_B1));
	vIO32WriteFldMulti(DDRPHY_CA_CMD6, P_Fld(0x1, CA_CMD6_RG_RX_ARCMD_O1_SEL)
		| P_Fld(0x0, CA_CMD6_RG_RX_ARCMD_BIAS_PS)
		| P_Fld(0x1, CA_CMD6_RG_RX_ARCMD_RES_BIAS_EN)
		| P_Fld(0x0, CA_CMD6_RG_RX_ARCMD_DMRANK_OUTSEL));

	vIO32WriteFldAlign(DDRPHY_B0_DQ8, 0x0, B0_DQ8_RG_T2RLPBK_B0);
	vIO32WriteFldAlign(DDRPHY_B1_DQ8, 0x0, B1_DQ8_RG_T2RLPBK_B1);

	vIO32WriteFldAlign(DDRPHY_B0_DQ5, 0x1, B0_DQ5_RG_RX_ARDQ_RES_BIAS_EN_B01);
	vIO32WriteFldAlign(DDRPHY_B1_DQ5, 0x1, B0_DQ5_RG_RX_ARDQ_RES_BIAS_EN_B01);
	vIO32WriteFldAlign(DDRPHY_B1_DQ5, 0x1, B0_DQ5_RG_RX_ARDQ_RES_BIAS_EN_B01);	
	vIO32WriteFldAlign(DDRPHY_B0_DQ5, 0x1, B0_DQ5_RG_RX_ARDQ_RES_BIAS_EN_B01);
	
	vIO32WriteFldMulti(DDRPHY_B01_MISC, P_Fld(0x0, B01_MISC_RG_RX_ARDQS0_GATE_EN_MODE_B01)
		| P_Fld(0x0, B01_MISC_RG_RX_ARDQ_DMRANK_OUTSEL_B0)
		| P_Fld(0x0, B01_MISC_RG_RX_ARDQ_DMRANK_OUTSEL_B1)
		| P_Fld(0x1, B01_MISC_RG_RX_ARDQS0_SER_RB_EXT_EN_B01)
		| P_Fld(0x0, B01_MISC_RG_RX_ARDQS0_SER_RST_EXT_EN_B01) //cc notes: SER_RST_MODE, DE says = SER_RST_EXT_EN
		| P_Fld(0x0, B01_MISC_RG_RX_ARDQS0_DQSIEN_RB_DLY_B01));

	mcSHOW_DBG_MSG(("Rx Vref init\n"));
	/* RX VREF */
	vIO32WriteFldAlign(DDRPHY_SHU1_B0_DQ5, 0x0, SHU1_B0_DQ5_RX_RX_ARDQ_VREF_BYPASS_B0);
	vIO32WriteFldAlign(DDRPHY_SHU1_B1_DQ5, 0x0, SHU1_B1_DQ5_RG_RX_ARDQ_VREF_BYPASS_B1);
	vIO32WriteFldAlign(DDRPHY_SHU1_CA_CMD5, 0x0, SHU1_CA_CMD5_RG_RX_ARCMD_VREF_BYPASS);
	if (p->dram_type == TYPE_PCDDR3) {
		vIO32WriteFldAlign(DDRPHY_SHU1_B0_DQ5, 0x12, SHU1_B0_DQ5_RG_RX_ARDQ_VREF_SEL_B0);//cc change from DE 17.10.23 rev.
		vIO32WriteFldAlign(DDRPHY_SHU1_B1_DQ5, 0x12, SHU1_B1_DQ5_RG_RX_ARDQ_VREF_SEL_B1);//cc change from DE 17.10.23 rev.
		//cc mark CMD not need?? vIO32WriteFldAlign(DDRPHY_SHU1_CA_CMD5, 0xa, SHU1_CA_CMD5_RG_RX_ARCMD_VREF_SEL);

		vIO32WriteFldMulti(DDRPHY_B0_DQ6, P_Fld(0x1, B0_DQ6_RG_RX_ARDQ_DDR3_SEL_B0)
			| P_Fld(0x0, B0_DQ6_RG_RX_ARDQ_DDR4_SEL_B0));
		vIO32WriteFldMulti(DDRPHY_B1_DQ6, P_Fld(0x1, B1_DQ6_RG_RX_ARDQ_DDR3_SEL_B1)
			| P_Fld(0x0, B1_DQ6_RG_RX_ARDQ_DDR4_SEL_B1));
	} else {
		vIO32WriteFldAlign(DDRPHY_SHU1_B0_DQ5, 0x13, SHU1_B0_DQ5_RG_RX_ARDQ_VREF_SEL_B0);//cc change from DE 17.10.23 rev.
		vIO32WriteFldAlign(DDRPHY_SHU1_B1_DQ5, 0x13, SHU1_B1_DQ5_RG_RX_ARDQ_VREF_SEL_B1);//cc change from DE 17.10.23 rev.
		//cc mark vIO32WriteFldAlign(DDRPHY_SHU1_CA_CMD5, 0xb, SHU1_CA_CMD5_RG_RX_ARCMD_VREF_SEL);
		
		vIO32WriteFldMulti(DDRPHY_B0_DQ6, P_Fld(0x0, B0_DQ6_RG_RX_ARDQ_DDR3_SEL_B0)
			| P_Fld(0x1, B0_DQ6_RG_RX_ARDQ_DDR4_SEL_B0));
		vIO32WriteFldMulti(DDRPHY_B1_DQ6, P_Fld(0x0, B1_DQ6_RG_RX_ARDQ_DDR3_SEL_B1)
			| P_Fld(0x1, B1_DQ6_RG_RX_ARDQ_DDR4_SEL_B1));
	}

	vIO32WriteFldAlign(DDRPHY_B0_DQ5, 0x1, B0_DQ5_RG_RX_ARDQ_VREF_EN_B0);	
	vIO32WriteFldAlign(DDRPHY_B1_DQ5, 0x1, B1_DQ5_RG_RX_ARDQ_VREF_EN_B1);
	//cc add end

	//cc add for Tx VREF setting (DDR3 only. Since DDR4 uses mode reg to adjust) 
#if COMPILE_PCDDR3
	vIO32WriteFldMulti(DDRPHY_MISC_VREF_CTRL, P_Fld(0x1, MISC_VREF_CTRL_RG_RVREF_DDR3_SEL) |
		P_Fld(0x0, MISC_VREF_CTRL_RG_RVREF_DDR4_SEL) |
		P_Fld(0x12, MISC_VREF_CTRL_RG_RVREF_SEL_DQ) |
		P_Fld(0x1, MISC_VREF_CTRL_RG_RVREF_DDR3_DQ_SEL) |
		P_Fld(0x0, MISC_VREF_CTRL_RG_RVREF_DDR4_DQ_SEL));
	vIO32WriteFldMulti(DDRPHY_MISC_VREF_CTRL, P_Fld(0x1, MISC_VREF_CTRL_RG_RVREF_VREF_DQ_EN) |
		P_Fld(0x1, MISC_VREF_CTRL_RG_RVREF_VREF_EN)); //cc notes: this is for CA, need??
#endif
	//cc add end
	
	/* From 8167 */
	vIO32WriteFldAlign(DDRPHY_B0_DQ3, 0x1, B0_DQ3_RG_RX_ARDQS0_STBEN_RESETB);
	vIO32WriteFldAlign(DDRPHY_B1_DQ3, 0x1, B1_DQ3_RG_RX_ARDQS1_STBEN_RESETB);
	/* 8167 code end */
	

	/* From 8167 */
	vIO32WriteFldMulti(DDRPHY_MISC_CTRL1, P_Fld(0x1, MISC_CTRL1_R_DMDA_RRESETB_E)
				| P_Fld(0x1, MISC_CTRL1_R_DMDQSIENCG_EN)
				| P_Fld(0x1, MISC_CTRL1_R_DMARPIDQ_SW)
				| P_Fld(0x1, MISC_CTRL1_R_DM_TX_ARCMD_OE)
				| P_Fld(0x1, MISC_CTRL1_R_DM_TX_ARCLK_OE));
    #if 0
	vIO32WriteFldMulti(DDRPHY_MISC_CTRL1+(1<<POS_BANK_NUM), 
				P_Fld(0x0, MISC_CTRL1_R_DMDA_RRESETB_E)//cc changed from 1->0 for Model Error will occur (repeated RESET). Confirm??
				| P_Fld(0x1, MISC_CTRL1_R_DMDQSIENCG_EN)
				| P_Fld(0x1, MISC_CTRL1_R_DMARPIDQ_SW)
				| P_Fld(0x1, MISC_CTRL1_R_DM_TX_ARCMD_OE)
				| P_Fld(0x1, MISC_CTRL1_R_DM_TX_ARCLK_OE));
    #endif
	vIO32WriteFldAlign(DDRPHY_CA_CMD8, 0x0, CA_CMD8_RG_TX_RRESETB_PULL_DN);
	vIO32WriteFldAlign(DDRPHY_CA_CMD7, 0x0, CA_CMD7_RG_TX_ARRESETB_PULL_DN);
	//cc remove vIO32WriteFldAlign(DDRPHY_CA_CMD7+(1<<POS_BANK_NUM), 0x0, CA_CMD7_RG_TX_ARRESETB_PULL_DN);

#if 1 //cc notes -- from 8167. need confirm if en_4bitMux, how to set??
	if (p->en_4bitMux == ENABLE)
	{
	#if COMPILE_THIS_PART
		vIO32WriteFldAlign(DRAMC_REG_DDRCONF0, 0x1, DDRCONF0_DQ4BMUX);
		vIO32WriteFldAlign(DDRPHY_B0_DQ3, 0xf0, B0_DQ3_RG_RX_ARDQ_DQSI_SEL_B0);
		vIO32WriteFldAlign(DDRPHY_B1_DQ3, 0xf0, B1_DQ3_RG_RX_ARDQ_DQSI_SEL_B1);
		vIO32WriteFldMulti(DDRPHY_B0_DQ2, P_Fld(0, B0_DQ2_RG_TX_ARDQM0_MCK4X_SEL_B0)
					| P_Fld(0xf0, B0_DQ2_RG_TX_ARDQ_MCK4X_SEL_B0));
		vIO32WriteFldMulti(DDRPHY_B1_DQ2, P_Fld(1, B1_DQ2_RG_TX_ARDQM0_MCK4X_SEL_B1)
					| P_Fld(0xf0, B1_DQ2_RG_TX_ARDQ_MCK4X_SEL_B1));
	#endif
	}
	else
	{
		vIO32WriteFldAlign(DDRPHY_B0_DQ3, 0x0, B0_DQ3_RG_RX_ARDQ_DQSI_SEL_B0);
		vIO32WriteFldAlign(DDRPHY_B1_DQ3, 0x0, B1_DQ3_RG_RX_ARDQ_DQSI_SEL_B1); //cc change from 0xff->0x0 from DE golden
		vIO32WriteFldMulti(DDRPHY_B0_DQ2, P_Fld(0, B0_DQ2_RG_TX_ARDQM0_MCK4X_SEL_B0)
					| P_Fld(0, B0_DQ2_RG_TX_ARDQ_MCK4X_SEL_B0));
		vIO32WriteFldMulti(DDRPHY_B1_DQ2, P_Fld(1, B1_DQ2_RG_TX_ARDQM0_MCK4X_SEL_B1)
					| P_Fld(0xff, B1_DQ2_RG_TX_ARDQ_MCK4X_SEL_B1));
	}
#endif
	if (p->dram_type == TYPE_PCDDR2)
		UpdateInitialSettings_MT7523_PC2(p);
	else
	UpdateInitialSettings_EN7523(p);
	/* 8167 code end */

	
#if 0 // cc remove. To be confirmed. PIMMUX setting??
	if (p->pinmux < PIN_MUX_TYPE_LPDDR3_211BALL)
	    vIO32WriteFldAlign(DRAMC_REG_PINMUX_TYPE, (u8)(p->pinmux) & 3, PINMUX_TYPE_SEL);
	else if(p->pinmux == PIN_MUX_TYPE_LPDDR3_211BALL)
	    vIO32WriteFldAlign(DRAMC_REG_PINMUX_TYPE, (u8) PIN_MUX_TYPE_LPDDR3_168BALL & 3, PINMUX_TYPE_SEL);
	else //if(p->pinmux == PIN_MUX_TYPE_DDR3X4)
	    vIO32WriteFldAlign(DRAMC_REG_PINMUX_TYPE, (u8) PIN_MUX_TYPE_DDR3X16X2 & 3, PINMUX_TYPE_SEL);
#endif

	//cc add
	switch (p->pinmux) {
#if FOR_DV_SIMULATION //YMC add for 7523 ASIC pinmux
	case PIN_MUX_TYPE_DDR3KGD:		
		vIO32WriteFldAlign(DDRPHY_MISC_CTRL1, 0x0, MISC_CTRL1_R_DMPINMUX);
		break;

	case PIN_MUX_TYPE_DDR3X16:
		vIO32WriteFldAlign(DDRPHY_MISC_CTRL1, 0x1, MISC_CTRL1_R_DMPINMUX);
		break;
	
	case PIN_MUX_TYPE_DDR3X16x2:		
		vIO32WriteFldAlign(DDRPHY_MISC_CTRL1, 0x2, MISC_CTRL1_R_DMPINMUX);
		break;

	case PIN_MUX_TYPE_DDR4X16:
		vIO32WriteFldAlign(DDRPHY_MISC_CTRL1, 0x3, MISC_CTRL1_R_DMPINMUX);
		break;
#else //YMC add for 7523 ASIC pinmux
	case PIN_MUX_TYPE_DDR3KGD:		
		vIO32WriteFldAlign_All(DDRPHY_MISC_CTRL1, 0x0, MISC_CTRL1_R_DMPINMUX);
		break;

	case PIN_MUX_TYPE_DDR2KGD:
		vIO32WriteFldAlign_All(DDRPHY_MISC_CTRL1, 0x1, MISC_CTRL1_R_DMPINMUX);
		break;
		
	case PIN_MUX_TYPE_DDR2DDR3X16:
		vIO32WriteFldAlign_All(DDRPHY_MISC_CTRL1, 0x2, MISC_CTRL1_R_DMPINMUX);
		break;
#endif
	}
	//cc add end
	mcSHOW_DBG_MSG(("DDRPHY_Init end\n"));

	return DRAM_OK;
}

static DRAM_STATUS_T DramcSetting_Default_PC2(DRAMC_CTX_T *p)
{
	mcSHOW_DBG_MSG2(("[Dramc] PCDDR2 default setting update \n"));

	//TX CMD 2T
	vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_CA1, P_Fld(0x0, SHU_SELPH_CA1_TXDLY_CS1)
    				| P_Fld(0x0, SHU_SELPH_CA1_TXDLY_RAS)
    				| P_Fld(0x0, SHU_SELPH_CA1_TXDLY_CAS)
    				| P_Fld(0x0, SHU_SELPH_CA1_TXDLY_WE)
    				| P_Fld(0x0, SHU_SELPH_CA1_TXDLY_RESET)
    				| P_Fld(0x0, SHU_SELPH_CA1_TXDLY_ODT)
    				| P_Fld(0x0, SHU_SELPH_CA1_TXDLY_CKE)
    				| P_Fld(0x0, SHU_SELPH_CA1_TXDLY_CS));

	vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_CA2, P_Fld(0x0, SHU_SELPH_CA2_TXDLY_CKE1)
    				| P_Fld(0x0, SHU_SELPH_CA2_TXDLY_CMD)
    				| P_Fld(0x0, SHU_SELPH_CA2_TXDLY_BA2)
    				| P_Fld(0x0, SHU_SELPH_CA2_TXDLY_BA1)
    				| P_Fld(0x0, SHU_SELPH_CA2_TXDLY_BA0));

	vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_CA3, P_Fld(0x0, SHU_SELPH_CA3_TXDLY_RA7)
    				| P_Fld(0x0, SHU_SELPH_CA3_TXDLY_RA6)
    				| P_Fld(0x0, SHU_SELPH_CA3_TXDLY_RA5)
    				| P_Fld(0x0, SHU_SELPH_CA3_TXDLY_RA4)
    				| P_Fld(0x0, SHU_SELPH_CA3_TXDLY_RA3)
    				| P_Fld(0x0, SHU_SELPH_CA3_TXDLY_RA2)
    				| P_Fld(0x0, SHU_SELPH_CA3_TXDLY_RA1)
    				| P_Fld(0x0, SHU_SELPH_CA3_TXDLY_RA0));

	vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_CA4, P_Fld(0x0, SHU_SELPH_CA4_TXDLY_RA15)
    				| P_Fld(0x0, SHU_SELPH_CA4_TXDLY_RA14)
    				| P_Fld(0x0, SHU_SELPH_CA4_TXDLY_RA13)
    				| P_Fld(0x0, SHU_SELPH_CA4_TXDLY_RA12)
    				| P_Fld(0x0, SHU_SELPH_CA4_TXDLY_RA11)
    				| P_Fld(0x0, SHU_SELPH_CA4_TXDLY_RA10)
    				| P_Fld(0x0, SHU_SELPH_CA4_TXDLY_RA9)
    				| P_Fld(0x0, SHU_SELPH_CA4_TXDLY_RA8));

	//TX CMD 05T
#if 0
	//1T mode
	vIO32Write4B(DRAMC_REG_SHU_SELPH_CA5, 0x11111111);
	vIO32Write4B(DRAMC_REG_SHU_SELPH_CA6, 0x01101111);
	vIO32Write4B(DRAMC_REG_SHU_SELPH_CA7, 0x11111111);
	vIO32Write4B(DRAMC_REG_SHU_SELPH_CA8, 0x11111111);
#else
	//2T mode
	vIO32Write4B(DRAMC_REG_SHU_SELPH_CA5, 0x11111011); //ldy modify
	vIO32Write4B(DRAMC_REG_SHU_SELPH_CA6, 0x01101111); //ldy modify
	vIO32Write4B(DRAMC_REG_SHU_SELPH_CA7, 0x11111111); //ldy modify
	vIO32Write4B(DRAMC_REG_SHU_SELPH_CA8, 0x11111111); //ldy modify
#endif

	//CMD PI
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_CA_CMD9, P_Fld(0x0, SHU1_R0_CA_CMD9_RG_RK0_ARPI_CLK)
						| P_Fld(0xC, SHU1_R0_CA_CMD9_RG_RK0_ARPI_CMD) //ldy modify from A to C
						| P_Fld(0x0, SHU1_R0_CA_CMD9_RG_RK0_ARPI_CS));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_CA_CMD9, P_Fld(0x0, SHU1_R1_CA_CMD9_RG_RK1_ARPI_CLK)
						| P_Fld(0x0, SHU1_R1_CA_CMD9_RG_RK1_ARPI_CMD)
						| P_Fld(0x0, SHU1_R1_CA_CMD9_RG_RK1_ARPI_CS));

	//TX DQS 2T ldy modify
	vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_DQS0, P_Fld(0x2, SHU_SELPH_DQS0_TXDLY_OEN_DQS3)
					| P_Fld(0x2, SHU_SELPH_DQS0_TXDLY_OEN_DQS2)
					| P_Fld(0x2, SHU_SELPH_DQS0_TXDLY_OEN_DQS1)
					| P_Fld(0x2, SHU_SELPH_DQS0_TXDLY_OEN_DQS0)
					| P_Fld(0x2, SHU_SELPH_DQS0_TXDLY_DQS3)
					| P_Fld(0x2, SHU_SELPH_DQS0_TXDLY_DQS2)
					| P_Fld(0x2, SHU_SELPH_DQS0_TXDLY_DQS1)
					| P_Fld(0x2, SHU_SELPH_DQS0_TXDLY_DQS0));
	//TX DQS 05T ldy modify
	vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_DQS1, P_Fld(0x1, SHU_SELPH_DQS1_DLY_OEN_DQS3)
					| P_Fld(0x1, SHU_SELPH_DQS1_DLY_OEN_DQS2)
					| P_Fld(0x1, SHU_SELPH_DQS1_DLY_OEN_DQS1)
					| P_Fld(0x1, SHU_SELPH_DQS1_DLY_OEN_DQS0)
					| P_Fld(0x1, SHU_SELPH_DQS1_DLY_DQS3)
					| P_Fld(0x1, SHU_SELPH_DQS1_DLY_DQS2)
					| P_Fld(0x1, SHU_SELPH_DQS1_DLY_DQS1)
					| P_Fld(0x1, SHU_SELPH_DQS1_DLY_DQS0));
	//TX DQ 2T ldy modify
	vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ0, P_Fld(0x2, SHURK0_SELPH_DQ0_TXDLY_OEN_DQ3)
					| P_Fld(0x2, SHURK0_SELPH_DQ0_TXDLY_OEN_DQ2)
					| P_Fld(0x2, SHURK0_SELPH_DQ0_TXDLY_OEN_DQ1)
					| P_Fld(0x2, SHURK0_SELPH_DQ0_TXDLY_OEN_DQ0)
					| P_Fld(0x2, SHURK0_SELPH_DQ0_TXDLY_DQ3)
					| P_Fld(0x2, SHURK0_SELPH_DQ0_TXDLY_DQ2)
					| P_Fld(0x2, SHURK0_SELPH_DQ0_TXDLY_DQ1)
					| P_Fld(0x2, SHURK0_SELPH_DQ0_TXDLY_DQ0));
	//TX DQM 2T ldy modify
	vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ1, P_Fld(0x2, SHURK0_SELPH_DQ1_TXDLY_OEN_DQM3)
					| P_Fld(0x2, SHURK0_SELPH_DQ1_TXDLY_OEN_DQM2)
					| P_Fld(0x2, SHURK0_SELPH_DQ1_TXDLY_OEN_DQM1)
					| P_Fld(0x2, SHURK0_SELPH_DQ1_TXDLY_OEN_DQM0)
					| P_Fld(0x2, SHURK0_SELPH_DQ1_TXDLY_DQM3)
					| P_Fld(0x2, SHURK0_SELPH_DQ1_TXDLY_DQM2)
					| P_Fld(0x2, SHURK0_SELPH_DQ1_TXDLY_DQM1)
					| P_Fld(0x2, SHURK0_SELPH_DQ1_TXDLY_DQM0));
	//TX DQ 05T ldy modify
	vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ2, P_Fld(0x1, SHURK0_SELPH_DQ2_DLY_OEN_DQ3)
					| P_Fld(0x1, SHURK0_SELPH_DQ2_DLY_OEN_DQ2)
					| P_Fld(0x1, SHURK0_SELPH_DQ2_DLY_OEN_DQ1)
					| P_Fld(0x1, SHURK0_SELPH_DQ2_DLY_OEN_DQ0)
					| P_Fld(0x1, SHURK0_SELPH_DQ2_DLY_DQ3)
					| P_Fld(0x1, SHURK0_SELPH_DQ2_DLY_DQ2)
					| P_Fld(0x1, SHURK0_SELPH_DQ2_DLY_DQ1)
					| P_Fld(0x1, SHURK0_SELPH_DQ2_DLY_DQ0));
	//TX DQM 05T ldy modify
	vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ3, P_Fld(0x1, SHURK0_SELPH_DQ3_DLY_OEN_DQM3)
					| P_Fld(0x1, SHURK0_SELPH_DQ3_DLY_OEN_DQM2)
					| P_Fld(0x1, SHURK0_SELPH_DQ3_DLY_OEN_DQM1)
					| P_Fld(0x1, SHURK0_SELPH_DQ3_DLY_OEN_DQM0)
					| P_Fld(0x1, SHURK0_SELPH_DQ3_DLY_DQM3)
					| P_Fld(0x1, SHURK0_SELPH_DQ3_DLY_DQM2)
					| P_Fld(0x1, SHURK0_SELPH_DQ3_DLY_DQM1)
					| P_Fld(0x1, SHURK0_SELPH_DQ3_DLY_DQM0));

	//TX RANK1 DQ 2T ldy modify
	vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_DQ0, P_Fld(0x2, SHURK1_SELPH_DQ0_TX_DLY_R1OEN_DQ3)
					| P_Fld(0x2, SHURK1_SELPH_DQ0_TX_DLY_R1OEN_DQ2)
					| P_Fld(0x2, SHURK1_SELPH_DQ0_TX_DLY_R1OEN_DQ1)
					| P_Fld(0x2, SHURK1_SELPH_DQ0_TX_DLY_R1OEN_DQ0)
					| P_Fld(0x2, SHURK1_SELPH_DQ0_TX_DLY_R1DQ3)
					| P_Fld(0x2, SHURK1_SELPH_DQ0_TX_DLY_R1DQ2)
					| P_Fld(0x2, SHURK1_SELPH_DQ0_TX_DLY_R1DQ1)
					| P_Fld(0x2, SHURK1_SELPH_DQ0_TX_DLY_R1DQ0));
	//TX RANK1 DQM 2T ldy modify
	vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_DQ1, P_Fld(0x2, SHURK1_SELPH_DQ1_TX_DLY_R1OEN_DQM3)
					| P_Fld(0x2, SHURK1_SELPH_DQ1_TX_DLY_R1OEN_DQM2)
					| P_Fld(0x2, SHURK1_SELPH_DQ1_TX_DLY_R1OEN_DQM1)
					| P_Fld(0x2, SHURK1_SELPH_DQ1_TX_DLY_R1OEN_DQM0)
					| P_Fld(0x2, SHURK1_SELPH_DQ1_TX_DLY_R1DQM3)
					| P_Fld(0x2, SHURK1_SELPH_DQ1_TX_DLY_R1DQM2)
					| P_Fld(0x2, SHURK1_SELPH_DQ1_TX_DLY_R1DQM1)
					| P_Fld(0x2, SHURK1_SELPH_DQ1_TX_DLY_R1DQM0));
	//TX RANK1 DQ 05T ldy modify
	vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_DQ2, P_Fld(0x3, SHURK1_SELPH_DQ2_DLY_R1OEN_DQ3)
					| P_Fld(0x3, SHURK1_SELPH_DQ2_DLY_R1OEN_DQ2)
					| P_Fld(0x3, SHURK1_SELPH_DQ2_DLY_R1OEN_DQ1)
					| P_Fld(0x3, SHURK1_SELPH_DQ2_DLY_R1OEN_DQ0)
					| P_Fld(0x3, SHURK1_SELPH_DQ2_DLY_R1DQ3)
					| P_Fld(0x3, SHURK1_SELPH_DQ2_DLY_R1DQ2)
					| P_Fld(0x3, SHURK1_SELPH_DQ2_DLY_R1DQ1)
					| P_Fld(0x3, SHURK1_SELPH_DQ2_DLY_R1DQ0));

	//TX RANK1 DQM 05T ldy modify
	vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_DQ3, P_Fld(0x3, SHURK1_SELPH_DQ3_DLY_R1OEN_DQM3)
					| P_Fld(0x3, SHURK1_SELPH_DQ3_DLY_R1OEN_DQM2)
					| P_Fld(0x3, SHURK1_SELPH_DQ3_DLY_R1OEN_DQM1)
					| P_Fld(0x3, SHURK1_SELPH_DQ3_DLY_R1OEN_DQM0)
					| P_Fld(0x3, SHURK1_SELPH_DQ3_DLY_R1DQM3)
					| P_Fld(0x3, SHURK1_SELPH_DQ3_DLY_R1DQM2)
					| P_Fld(0x3, SHURK1_SELPH_DQ3_DLY_R1DQM1)
					| P_Fld(0x3, SHURK1_SELPH_DQ3_DLY_R1DQM0));

	//TX DQS/DQM/DQ PI
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ7, P_Fld(0x0, SHU1_R0_B0_DQ7_RK0_ARPI_PBYTE_B0)
							| P_Fld(0xf, SHU1_R0_B0_DQ7_RK0_ARPI_DQM_B0)
							| P_Fld(0xf, SHU1_R0_B0_DQ7_RK0_ARPI_DQ_B0));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B0_DQ7, P_Fld(0x0, SHU1_R1_B0_DQ7_RK1_ARPI_PBYTE_B0)
							| P_Fld(0xf, SHU1_R1_B0_DQ7_RK1_ARPI_DQM_B0)
							| P_Fld(0xf, SHU1_R1_B0_DQ7_RK1_ARPI_DQ_B0));
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ7, P_Fld(0x0, SHU1_R0_B1_DQ7_RK0_ARPI_PBYTE_B1)
							| P_Fld(0xf, SHU1_R0_B1_DQ7_RK0_ARPI_DQM_B1)
							| P_Fld(0xf, SHU1_R0_B1_DQ7_RK0_ARPI_DQ_B1));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B1_DQ7, P_Fld(0x0, SHU1_R1_B1_DQ7_RK1_ARPI_PBYTE_B1)
							| P_Fld(0xf, SHU1_R1_B1_DQ7_RK1_ARPI_DQM_B1)
							| P_Fld(0xf, SHU1_R1_B1_DQ7_RK1_ARPI_DQ_B1));
    //ODT EN ldy modify
    vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_ODTEN0, P_Fld(0x0, SHURK0_SELPH_ODTEN0_TXDLY_B3_RODTEN_P1)
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN0_TXDLY_B3_RODTEN)
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN0_TXDLY_B2_RODTEN_P1)
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN0_TXDLY_B2_RODTEN)
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN0_TXDLY_B1_RODTEN_P1)
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN0_TXDLY_B1_RODTEN)
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN0_TXDLY_B0_RODTEN_P1)
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN0_TXDLY_B0_RODTEN));

    vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_ODTEN1, P_Fld(0x0, SHURK0_SELPH_ODTEN1_DLY_B3_RODTEN_P1)
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN1_DLY_B3_RODTEN)
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN1_DLY_B2_RODTEN_P1)
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN1_DLY_B2_RODTEN)
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN1_DLY_B1_RODTEN_P1)
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN1_DLY_B1_RODTEN)
    				/*byzj | P_Fld(0x0, SHURK0_SELPH_ODTEN1_DLY_B0_RODTEN_P1) */
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN1_DLY_B0_RODTEN_P1)/*add byzj*/
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN1_DLY_B0_RODTEN));

    vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_ODTEN0, P_Fld(0x0, SHURK1_SELPH_ODTEN0_TXDLY_B3_R1RODTEN_P1)
    				| P_Fld(0x0, SHURK1_SELPH_ODTEN0_TXDLY_B3_R1RODTEN)
    				| P_Fld(0x0, SHURK1_SELPH_ODTEN0_TXDLY_B2_R1RODTEN_P1)
    				| P_Fld(0x0, SHURK1_SELPH_ODTEN0_TXDLY_B2_R1RODTEN)
    				| P_Fld(0x0, SHURK1_SELPH_ODTEN0_TXDLY_B1_R1RODTEN_P1)
    				| P_Fld(0x0, SHURK1_SELPH_ODTEN0_TXDLY_B1_R1RODTEN)
    				| P_Fld(0x0, SHURK1_SELPH_ODTEN0_TXDLY_B0_R1RODTEN_P1)
    				| P_Fld(0x0, SHURK1_SELPH_ODTEN0_TXDLY_B0_R1RODTEN));

    vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_ODTEN1, P_Fld(0x0, SHURK1_SELPH_ODTEN1_DLY_B3_R1RODTEN_P1)
    				| P_Fld(0x0, SHURK1_SELPH_ODTEN1_DLY_B3_R1RODTEN)
    				| P_Fld(0x0, SHURK1_SELPH_ODTEN1_DLY_B2_R1RODTEN_P1)
    				| P_Fld(0x0, SHURK1_SELPH_ODTEN1_DLY_B2_R1RODTEN)
    				| P_Fld(0x0, SHURK1_SELPH_ODTEN1_DLY_B1_R1RODTEN_P1)
    				| P_Fld(0x0, SHURK1_SELPH_ODTEN1_DLY_B1_R1RODTEN)
    				| P_Fld(0x0, SHURK1_SELPH_ODTEN1_DLY_B0_R1RODTEN_P1)
    				| P_Fld(0x0, SHURK1_SELPH_ODTEN1_DLY_B0_R1RODTEN));

    vIO32WriteFldMulti(DRAMC_REG_SHURK2_SELPH_ODTEN0, P_Fld(0x0, SHURK2_SELPH_ODTEN0_TXDLY_B3_R2RODTEN_P1)
    				| P_Fld(0x0, SHURK2_SELPH_ODTEN0_TXDLY_B3_R2RODTEN)
    				| P_Fld(0x0, SHURK2_SELPH_ODTEN0_TXDLY_B2_R2RODTEN_P1)
    				| P_Fld(0x0, SHURK2_SELPH_ODTEN0_TXDLY_B2_R2RODTEN)
    				| P_Fld(0x0, SHURK2_SELPH_ODTEN0_TXDLY_B1_R2RODTEN_P1)
    				| P_Fld(0x0, SHURK2_SELPH_ODTEN0_TXDLY_B1_R2RODTEN)
    				| P_Fld(0x0, SHURK2_SELPH_ODTEN0_TXDLY_B0_R2RODTEN_P1)
    				| P_Fld(0x0, SHURK2_SELPH_ODTEN0_TXDLY_B0_R2RODTEN));

    vIO32WriteFldMulti(DRAMC_REG_SHURK2_SELPH_ODTEN1, P_Fld(0x6, SHURK2_SELPH_ODTEN1_DLY_B3_R2RODTEN_P1)
    				| P_Fld(0x6, SHURK2_SELPH_ODTEN1_DLY_B3_R2RODTEN)
    				| P_Fld(0x6, SHURK2_SELPH_ODTEN1_DLY_B2_R2RODTEN_P1)
    				| P_Fld(0x6, SHURK2_SELPH_ODTEN1_DLY_B2_R2RODTEN)
    				| P_Fld(0x6, SHURK2_SELPH_ODTEN1_DLY_B1_R2RODTEN_P1)
    				| P_Fld(0x6, SHURK2_SELPH_ODTEN1_DLY_B1_R2RODTEN)
    				| P_Fld(0x6, SHURK2_SELPH_ODTEN1_DLY_B0_R2RODTEN_P1)
    				| P_Fld(0x6, SHURK2_SELPH_ODTEN1_DLY_B0_R2RODTEN));


	//DLE ldy modify
   vIO32WriteFldMulti(DRAMC_REG_SHU_CONF1, P_Fld(0xb, SHU_CONF1_DATLAT_DSEL_PHY)
    				| P_Fld(0xa, SHU_CONF1_DATLAT_DSEL)
    				| P_Fld(0xc, SHU_CONF1_DATLAT));

    //GW 2T ldy modify
	vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQSG0, P_Fld(0x1, SHURK0_SELPH_DQSG0_TX_DLY_DQS3_GATED_P1)
	                | P_Fld(0x0, SHURK0_SELPH_DQSG0_TX_DLY_DQS3_GATED)
	                | P_Fld(0x1, SHURK0_SELPH_DQSG0_TX_DLY_DQS2_GATED_P1)
	                | P_Fld(0x0, SHURK0_SELPH_DQSG0_TX_DLY_DQS2_GATED)
	                | P_Fld(0x1, SHURK0_SELPH_DQSG0_TX_DLY_DQS1_GATED_P1)
	                | P_Fld(0x0, SHURK0_SELPH_DQSG0_TX_DLY_DQS1_GATED)
	                | P_Fld(0x1, SHURK0_SELPH_DQSG0_TX_DLY_DQS0_GATED_P1)
	                | P_Fld(0x0, SHURK0_SELPH_DQSG0_TX_DLY_DQS0_GATED));

	vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQSG1, P_Fld(0x1, SHURK0_SELPH_DQSG1_REG_DLY_DQS3_GATED_P1)
	                | P_Fld(0x7, SHURK0_SELPH_DQSG1_REG_DLY_DQS3_GATED)
	                | P_Fld(0x1, SHURK0_SELPH_DQSG1_REG_DLY_DQS2_GATED_P1)
	                | P_Fld(0x7, SHURK0_SELPH_DQSG1_REG_DLY_DQS2_GATED)
	                | P_Fld(0x1, SHURK0_SELPH_DQSG1_REG_DLY_DQS1_GATED_P1)
	                | P_Fld(0x7, SHURK0_SELPH_DQSG1_REG_DLY_DQS1_GATED)
	                | P_Fld(0x1, SHURK0_SELPH_DQSG1_REG_DLY_DQS0_GATED_P1)
	                | P_Fld(0x7, SHURK0_SELPH_DQSG1_REG_DLY_DQS0_GATED));

	vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_DQSG0, P_Fld(0x1, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS3_GATED_P1)
	                | P_Fld(0x0, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS3_GATED)
	                | P_Fld(0x1, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS2_GATED_P1)
	                | P_Fld(0x0, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS2_GATED)
	                | P_Fld(0x1, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS1_GATED_P1)
	                | P_Fld(0x0, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS1_GATED)
	                | P_Fld(0x1, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS0_GATED_P1)
	                | P_Fld(0x0, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS0_GATED));

	vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_DQSG1, P_Fld(0x3, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS3_GATED_P1)
	                | P_Fld(0x7, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS3_GATED)
	                | P_Fld(0x3, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS2_GATED_P1)
	                | P_Fld(0x7, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS2_GATED)
	                | P_Fld(0x3, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS1_GATED_P1)
	                | P_Fld(0x7, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS1_GATED)
	                | P_Fld(0x3, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS0_GATED_P1)
	                | P_Fld(0x7, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS0_GATED));


	//RX delay ldy modify ???
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ2, P_Fld(0x8, SHU1_R0_B0_DQ2_RK0_RX_ARDQ1_F_DLY_B0) //cc change from 12->0
        				| P_Fld(0x0, SHU1_R0_B0_DQ2_RK0_RX_ARDQ1_R_DLY_B0)//cc change from 12->0
        				| P_Fld(0x8, SHU1_R0_B0_DQ2_RK0_RX_ARDQ0_F_DLY_B0)//cc change from 12->0
        				| P_Fld(0x0, SHU1_R0_B0_DQ2_RK0_RX_ARDQ0_R_DLY_B0));//cc change from 12->0
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ3, P_Fld(0x8, SHU1_R0_B0_DQ3_RK0_RX_ARDQ3_F_DLY_B0)//cc change from 12->0
        				| P_Fld(0x0, SHU1_R0_B0_DQ3_RK0_RX_ARDQ3_R_DLY_B0)//cc change from 12->0
        				| P_Fld(0x8, SHU1_R0_B0_DQ3_RK0_RX_ARDQ2_F_DLY_B0)//cc change from 12->0
        				| P_Fld(0x0, SHU1_R0_B0_DQ3_RK0_RX_ARDQ2_R_DLY_B0));//cc change from 12->0
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ4, P_Fld(0x8, SHU1_R0_B0_DQ4_RK0_RX_ARDQ5_F_DLY_B0)//cc change from 12->0
        				| P_Fld(0x0, SHU1_R0_B0_DQ4_RK0_RX_ARDQ5_R_DLY_B0)//cc change from 12->0
        				| P_Fld(0x8, SHU1_R0_B0_DQ4_RK0_RX_ARDQ4_F_DLY_B0)//cc change from 12->0
        				| P_Fld(0x0, SHU1_R0_B0_DQ4_RK0_RX_ARDQ4_R_DLY_B0));//cc change from 12->0
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ5, P_Fld(0x8, SHU1_R0_B0_DQ5_RK0_RX_ARDQ7_F_DLY_B0)//cc change from 12->0
        				| P_Fld(0x0, SHU1_R0_B0_DQ5_RK0_RX_ARDQ7_R_DLY_B0)//cc change from 12->0
        				| P_Fld(0x8, SHU1_R0_B0_DQ5_RK0_RX_ARDQ6_F_DLY_B0)//cc change from 12->0
        				| P_Fld(0x0, SHU1_R0_B0_DQ5_RK0_RX_ARDQ6_R_DLY_B0));//cc change from 12->0
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ6, P_Fld(0x8, SHU1_R0_B0_DQ6_RK0_RX_ARDQS0_F_DLY_B0)
            			| P_Fld(0x0, SHU1_R0_B0_DQ6_RK0_RX_ARDQS0_R_DLY_B0)
            			| P_Fld(0x8, SHU1_R0_B0_DQ6_RK0_RX_ARDQM0_F_DLY_B0)
            			| P_Fld(0x0, SHU1_R0_B0_DQ6_RK0_RX_ARDQM0_R_DLY_B0));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B0_DQ2, P_Fld(0x9, SHU1_R1_B0_DQ2_RK1_RX_ARDQ1_F_DLY_B0)
        				| P_Fld(0x1, SHU1_R1_B0_DQ2_RK1_RX_ARDQ1_R_DLY_B0)
        				| P_Fld(0x9, SHU1_R1_B0_DQ2_RK1_RX_ARDQ0_F_DLY_B0)
        				| P_Fld(0x1, SHU1_R1_B0_DQ2_RK1_RX_ARDQ0_R_DLY_B0));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B0_DQ3, P_Fld(0x9, SHU1_R1_B0_DQ3_RK1_RX_ARDQ3_F_DLY_B0)
        				| P_Fld(0x1, SHU1_R1_B0_DQ3_RK1_RX_ARDQ3_R_DLY_B0)
        				| P_Fld(0x9, SHU1_R1_B0_DQ3_RK1_RX_ARDQ2_F_DLY_B0)
        				| P_Fld(0x1, SHU1_R1_B0_DQ3_RK1_RX_ARDQ2_R_DLY_B0));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B0_DQ4, P_Fld(0x9, SHU1_R1_B0_DQ4_RK1_RX_ARDQ5_F_DLY_B0)
        				| P_Fld(0x1, SHU1_R1_B0_DQ4_RK1_RX_ARDQ5_R_DLY_B0)
        				| P_Fld(0x9, SHU1_R1_B0_DQ4_RK1_RX_ARDQ4_F_DLY_B0)
        				| P_Fld(0x1, SHU1_R1_B0_DQ4_RK1_RX_ARDQ4_R_DLY_B0));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B0_DQ5, P_Fld(0x9, SHU1_R1_B0_DQ5_RK1_RX_ARDQ7_F_DLY_B0)
        				| P_Fld(0x1, SHU1_R1_B0_DQ5_RK1_RX_ARDQ7_R_DLY_B0)
        				| P_Fld(0x9, SHU1_R1_B0_DQ5_RK1_RX_ARDQ6_F_DLY_B0)
        				| P_Fld(0x1, SHU1_R1_B0_DQ5_RK1_RX_ARDQ6_R_DLY_B0));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B0_DQ6, P_Fld(0x9, SHU1_R1_B0_DQ6_RK1_RX_ARDQS0_F_DLY_B0)
        				| P_Fld(0x1, SHU1_R1_B0_DQ6_RK1_RX_ARDQS0_R_DLY_B0)
        				| P_Fld(0x9, SHU1_R1_B0_DQ6_RK1_RX_ARDQM0_F_DLY_B0)
        				| P_Fld(0x1, SHU1_R1_B0_DQ6_RK1_RX_ARDQM0_R_DLY_B0));
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ2, P_Fld(0x8, SHU1_R0_B1_DQ2_RK0_RX_ARDQ1_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ2_RK0_RX_ARDQ1_R_DLY_B1)
        				| P_Fld(0x8, SHU1_R0_B1_DQ2_RK0_RX_ARDQ0_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ2_RK0_RX_ARDQ0_R_DLY_B1));
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ3, P_Fld(0x8, SHU1_R0_B1_DQ3_RK0_RX_ARDQ3_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ3_RK0_RX_ARDQ3_R_DLY_B1)
        				| P_Fld(0x8, SHU1_R0_B1_DQ3_RK0_RX_ARDQ2_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ3_RK0_RX_ARDQ2_R_DLY_B1));
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ4, P_Fld(0x8, SHU1_R0_B1_DQ4_RK0_RX_ARDQ5_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ4_RK0_RX_ARDQ5_R_DLY_B1)
        				| P_Fld(0x8, SHU1_R0_B1_DQ4_RK0_RX_ARDQ4_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ4_RK0_RX_ARDQ4_R_DLY_B1));
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ5, P_Fld(0x8, SHU1_R0_B1_DQ5_RK0_RX_ARDQ7_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ5_RK0_RX_ARDQ7_R_DLY_B1)
        				| P_Fld(0x8, SHU1_R0_B1_DQ5_RK0_RX_ARDQ6_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ5_RK0_RX_ARDQ6_R_DLY_B1));
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ6, P_Fld(0x8, SHU1_R0_B1_DQ6_RK0_RX_ARDQS0_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ6_RK0_RX_ARDQS0_R_DLY_B1)
        				| P_Fld(0x8, SHU1_R0_B1_DQ6_RK0_RX_ARDQM0_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ6_RK0_RX_ARDQM0_R_DLY_B1));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B1_DQ2, P_Fld(0x9, SHU1_R1_B1_DQ2_RK1_RX_ARDQ1_F_DLY_B1)
        				| P_Fld(0x1, SHU1_R1_B1_DQ2_RK1_RX_ARDQ1_R_DLY_B1)
        				| P_Fld(0x9, SHU1_R1_B1_DQ2_RK1_RX_ARDQ0_F_DLY_B1)
        				| P_Fld(0x1, SHU1_R1_B1_DQ2_RK1_RX_ARDQ0_R_DLY_B1));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B1_DQ3, P_Fld(0x9, SHU1_R1_B1_DQ3_RK1_RX_ARDQ3_F_DLY_B1)
        				| P_Fld(0x1, SHU1_R1_B1_DQ3_RK1_RX_ARDQ3_R_DLY_B1)
        				| P_Fld(0x9, SHU1_R1_B1_DQ3_RK1_RX_ARDQ2_F_DLY_B1)
        				| P_Fld(0x1, SHU1_R1_B1_DQ3_RK1_RX_ARDQ2_R_DLY_B1));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B1_DQ4, P_Fld(0x9, SHU1_R1_B1_DQ4_RK1_RX_ARDQ5_F_DLY_B1)
        				| P_Fld(0x1, SHU1_R1_B1_DQ4_RK1_RX_ARDQ5_R_DLY_B1)
        				| P_Fld(0x9, SHU1_R1_B1_DQ4_RK1_RX_ARDQ4_F_DLY_B1)
        				| P_Fld(0x1, SHU1_R1_B1_DQ4_RK1_RX_ARDQ4_R_DLY_B1));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B1_DQ5, P_Fld(0x9, SHU1_R1_B1_DQ5_RK1_RX_ARDQ7_F_DLY_B1)
        				| P_Fld(0x1, SHU1_R1_B1_DQ5_RK1_RX_ARDQ7_R_DLY_B1)
        				| P_Fld(0x9, SHU1_R1_B1_DQ5_RK1_RX_ARDQ6_F_DLY_B1)
        				| P_Fld(0x1, SHU1_R1_B1_DQ5_RK1_RX_ARDQ6_R_DLY_B1));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B1_DQ6, P_Fld(0x9, SHU1_R1_B1_DQ6_RK1_RX_ARDQS0_F_DLY_B1)
        				| P_Fld(0x1, SHU1_R1_B1_DQ6_RK1_RX_ARDQS0_R_DLY_B1)
        				| P_Fld(0x9, SHU1_R1_B1_DQ6_RK1_RX_ARDQM0_F_DLY_B1)
        				| P_Fld(0x1, SHU1_R1_B1_DQ6_RK1_RX_ARDQM0_R_DLY_B1));


    //Rank0 and Rank1
    vIO32WriteFldMulti(DRAMC_REG_SHU_RANKCTL, P_Fld(0x2, SHU_RANKCTL_RANKINCTL_PHY)
    				| P_Fld(0x1, SHU_RANKCTL_DMSTBLAT));

	vIO32WriteFldAlign(DRAMC_REG_SHURK0_DQSCTL, 0x1, SHURK0_DQSCTL_DQSINCTL);
	vIO32WriteFldAlign(DRAMC_REG_SHURK1_DQSCTL, 0x1, SHURK1_DQSCTL_R1DQSINCTL);
	return DRAM_OK;

}

static DRAM_STATUS_T DramcSetting_ACTiming_PC2(DRAMC_CTX_T *p)
{
	mcSHOW_DBG_MSG2(("[Dramc] PCDDR2 AC Timing update \n"));

	vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM0, P_Fld(0x3, SHU_ACTIM0_TRCD)  //YMC chg for 1066 optimal
                    | P_Fld(0x2, SHU_ACTIM0_TRRD)
                    | P_Fld(0x8, SHU_ACTIM0_TWR)						//YMC chg for 1066 optimal
                    | P_Fld(0x5, SHU_ACTIM0_TWTR));						//YMC chg for 1066 optimal

	vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM1, P_Fld(0x8, SHU_ACTIM1_TRC)	//YMC chg for 1066 optimal
                    | P_Fld(0x4, SHU_ACTIM1_TRAS)						//YMC chg for 1066 optimal
                    | P_Fld(0x3, SHU_ACTIM1_TRP)						//YMC chg for 1066 optimal
                    | P_Fld(0x0, SHU_ACTIM1_TRPAB));

	vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM2, P_Fld(0x4, SHU_ACTIM2_TFAW)	//YMC chg for 1066 optimal
                    | P_Fld(0x1, SHU_ACTIM2_TR2W)						//YMC chg for 1066 optimal
                    | P_Fld(0x1, SHU_ACTIM2_TRTP)						//YMC chg for 1066 optimal
                    | P_Fld(0x0, SHU_ACTIM2_TXP));

	#if 0
	vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM3, P_Fld(0x6D, SHU_ACTIM3_TRFC)
					| P_Fld(0x18, SHU_ACTIM3_TRFCPB));
	#else
	vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM3, P_Fld(0x4b, SHU_ACTIM3_TRFC)	//YMC chg for 1066 optimal
					| P_Fld(0x18, SHU_ACTIM3_TRFCPB));
	#endif
	if(p->clk_src == PLL_Fin_25M)
	{
		vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM4, P_Fld(0x22, SHU_ACTIM4_TZQCS)
		    			| P_Fld(0x60, SHU_ACTIM4_REFCNT_FR_CLK)					//YMC chg for 1066 optimal
		    			| P_Fld(0xff, SHU_ACTIM4_TXREFCNT));
	}
	else
	{
		vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM4, P_Fld(0x22, SHU_ACTIM4_TZQCS)
		    			| P_Fld(0x4d, SHU_ACTIM4_REFCNT_FR_CLK)					//YMC chg for 1066 optimal
		    			| P_Fld(0xff, SHU_ACTIM4_TXREFCNT));
	}

	vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM5, P_Fld(0x1, SHU_ACTIM5_TMRR2W)
	                | P_Fld(0x9, SHU_ACTIM5_TWTPD)
	                | P_Fld(0x8, SHU_ACTIM5_TR2PD));

	vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM_XRT, P_Fld(0x8, SHU_ACTIM_XRT_XRTW2W)
	                | P_Fld(0x1, SHU_ACTIM_XRT_XRTW2R)
	                | P_Fld(0x4, SHU_ACTIM_XRT_XRTR2W)
	                | P_Fld(0x9, SHU_ACTIM_XRT_XRTR2R));

	vIO32WriteFldMulti(DRAMC_REG_SHU_AC_TIME_05T, P_Fld(0x1, SHU_AC_TIME_05T_TWTR_M05T)
	                | P_Fld(0x1, SHU_AC_TIME_05T_TRP_05T)
	                | P_Fld(0x1, SHU_AC_TIME_05T_TRCD_05T)
	                | P_Fld(0x1, SHU_AC_TIME_05T_TXP_05T)
	                | P_Fld(0x1, SHU_AC_TIME_05T_TRFC_05T));

	return DRAM_OK;

}

static DRAM_STATUS_T DramcSetting_EN7523_PC2(DRAMC_CTX_T *p)
{
    vIO32WriteFldMulti(DRAMC_REG_REFCTRL0, P_Fld(0x1, REFCTRL0_REFFRERUN)
            		| P_Fld(0x1, REFCTRL0_REFDIS)
            		| P_Fld(0x5, REFCTRL0_REF_PREGATE_CNT)
            		| P_Fld(0x7, REFCTRL0_ADVREF_CNT)
            		| P_Fld(0x1, REFCTRL0_PBREF_DISBYREFNUM)
            		| P_Fld(0x4, REFCTRL0_DISBYREFNUM)
            		| P_Fld(0x1, REFCTRL0_DLLFRZ));

    vIO32WriteFldMulti(DDRPHY_MISC_CTRL3, P_Fld(0x0, MISC_CTRL3_ARPI_CG_CLK_OPT));
    //byzj vIO32WriteFldMulti(DDRPHY_MISC_CTRL3+(1<<POS_BANK_NUM), P_Fld(0x0, MISC_CTRL3_ARPI_CG_CLK_OPT));

    vIO32WriteFldMulti(DRAMC_REG_SHU_DQSG_RETRY, P_Fld(0x1, SHU_DQSG_RETRY_R_RDY_SEL_DLE)
    		        | P_Fld(0x6, SHU_DQSG_RETRY_R_DQSIENLAT));

    vIO32WriteFldMulti(DRAMC_REG_SHUCTRL2, P_Fld(0x1, SHUCTRL2_SHU_PERIOD_GO_ZERO_CNT)
            		| P_Fld(0x1, SHUCTRL2_R_DVFS_OPTION)
            		| P_Fld(0x1, SHUCTRL2_R_DVFS_PARK_N)
            		| P_Fld(0x1, SHUCTRL2_R_DVFS_DLL_CHA)
            		| P_Fld(0x7, SHUCTRL2_R_DLL_IDLE));

    vIO32WriteFldMulti(DRAMC_REG_DVFSDLL, P_Fld(0xe, DVFSDLL_DLL_IDLE_SHU4)
            		| P_Fld(0x9, DVFSDLL_DLL_IDLE_SHU3)
            		| P_Fld(0x6, DVFSDLL_DLL_IDLE_SHU2)
            		| P_Fld(0x1, DVFSDLL_DLL_LOCK_SHU_EN));

    if (p->data_width == DATA_WIDTH_16BIT)
    {
        vIO32WriteFldMulti(DRAMC_REG_DDRCONF0, P_Fld(0x1, DDRCONF0_DDR2EN)
            		| P_Fld(0x1, DDRCONF0_BC4OTF_OPT)
            		| P_Fld(0x1, DDRCONF0_BK8EN));
    }
    else
    {
        vIO32WriteFldMulti(DRAMC_REG_DDRCONF0, P_Fld(0x1, DDRCONF0_DDR2EN)
            		| P_Fld(0x1, DDRCONF0_DM64BITEN)
            		| P_Fld(0x1, DDRCONF0_BC4OTF_OPT)
            		| P_Fld(0x1, DDRCONF0_BK8EN));
    }

    vIO32WriteFldMulti(DDRPHY_MISC_CTRL0, P_Fld(0x1, MISC_CTRL0_R_DMDQSIEN_FIFO_EN)
            		| P_Fld(0x1, MISC_CTRL0_R_DMDQSIEN_DEPTH_HALF) //ldy change from 0->1
            		| P_Fld(0x1, MISC_CTRL0_R_DMSTBEN_OUTSEL)
            		| P_Fld(0xf, MISC_CTRL0_R_DMDQSIEN_SYNCOPT));

	#if 0 //byzj
    vIO32WriteFldMulti(DDRPHY_MISC_CTRL0+(1<<POS_BANK_NUM), P_Fld(0x1, MISC_CTRL0_R_DMDQSIEN_FIFO_EN)
            		| P_Fld(0x0, MISC_CTRL0_R_DMDQSIEN_DEPTH_HALF)
            		| P_Fld(0x1, MISC_CTRL0_R_DMSTBEN_OUTSEL)
            		| P_Fld(0xf, MISC_CTRL0_R_DMDQSIEN_SYNCOPT));
	#endif

    vIO32WriteFldMulti(DRAMC_REG_SHU_CONF0, P_Fld(0x2, SHU_CONF0_MATYPE)
            		| P_Fld(0x1, SHU_CONF0_BL4)
            		| P_Fld(0x1, SHU_CONF0_FDIV2)
            		| P_Fld(0x1, SHU_CONF0_REFTHD)
            		| P_Fld(0x1, SHU_CONF0_ADVPREEN)
            		| P_Fld(0x3f, SHU_CONF0_DMPGTIM));

    vIO32WriteFldMulti(DRAMC_REG_SHU_ODTCTRL, P_Fld(0x3, SHU_ODTCTRL_TWODT)
            		| P_Fld(0x5, SHU_ODTCTRL_RODT)
            		| P_Fld(0x1, SHU_ODTCTRL_WOEN)
            		| P_Fld(0x0, SHU_ODTCTRL_ROEN)); //ldy add

    vIO32WriteFldMulti(DRAMC_REG_REFCTRL0, P_Fld(0x0, REFCTRL0_REFFRERUN)
            		| P_Fld(0x2, REFCTRL0_DISBYREFNUM)
            		| P_Fld(0x0, REFCTRL0_DLLFRZ));

    vIO32WriteFldMulti(DRAMC_REG_STBCAL, P_Fld(0x1, STBCAL_DQSIENMODE)
            	    | P_Fld(0x0, STBCAL_DQSIENMODE_SELPH));

    vIO32WriteFldMulti(DRAMC_REG_SREFCTRL, P_Fld(0x0, SREFCTRL_SREF_HW_EN)
                	| P_Fld(0x8, SREFCTRL_SREFDLY)
                	| P_Fld(0x0, SREFCTRL_SREF2_OPTION));

    vIO32WriteFldMulti(DRAMC_REG_SHU_PIPE, P_Fld(0x1, SHU_PIPE_READ_START_EXTEND1)
            	    | P_Fld(0x1, SHU_PIPE_DLE_LAST_EXTEND1));

    vIO32WriteFldAlign(DRAMC_REG_RKCFG, 0x1, RKCFG_CKE2RANK_OPT2);

    vIO32WriteFldMulti(DRAMC_REG_SHU_CONF2, P_Fld(0x7, SHU_CONF2_DCMDLYREF)
        			| P_Fld(0x32, SHU_CONF2_FSPCHG_PRDCNT)
        			| P_Fld(0xf, SHU_CONF2_TCMDO1LAT));

    vIO32WriteFldMulti(DRAMC_REG_SHU_SCINTV, P_Fld(0x0, SHU_SCINTV_DQS2DQ_SHU_PITHRD)
        			| P_Fld(0x15, SHU_SCINTV_MRW_INTV)
        			| P_Fld(0x0, SHU_SCINTV_RDDQC_INTV)
        			| P_Fld(0x0, SHU_SCINTV_TZQLAT));

    vIO32WriteFldAlign(DRAMC_REG_SHUCTRL, 0x0, SHUCTRL_R_SHUFFLE_BLOCK_OPT);

    vIO32WriteFldAlign(DRAMC_REG_REFCTRL1, 0x0, REFCTRL1_SREF_PRD_OPT);

    vIO32WriteFldAlign(DRAMC_REG_REFRATRE_FILTER, 0x1, REFRATRE_FILTER_PB2AB_OPT);
	//----------------------------------------------------------------------------
	DramcSetting_Default_PC2(p);

	DramcSetting_ACTiming_PC2(p);

	//----------------------------------------------------------------------------
    vIO32WriteFldMulti(DDRPHY_PLL2, P_Fld(0x1, PLL2_GDDR3RST));
	mcDELAY_US(500);//Reset ON

    vIO32WriteFldMulti(DRAMC_REG_MISCTL0, P_Fld(0x1, MISCTL0_REFP_ARB_EN2)
    				| P_Fld(0x1, MISCTL0_PBC_ARB_EN)
    				| P_Fld(0x1, MISCTL0_REFA_ARB_EN2));
    vIO32WriteFldMulti(DRAMC_REG_PERFCTL0, P_Fld(0x1, PERFCTL0_MWHPRIEN)
    				| P_Fld(0x1, PERFCTL0_RWSPLIT) //ldy change from 0 -> 1
    				| P_Fld(0x1, PERFCTL0_WFLUSHEN)
    				| P_Fld(0x1, PERFCTL0_EMILLATEN)
    				| P_Fld(0x1, PERFCTL0_RWAGEEN)
    				| P_Fld(0x1, PERFCTL0_RWLLATEN)
    				| P_Fld(0x1, PERFCTL0_RWHPRIEN)
    				| P_Fld(0x0, PERFCTL0_RWOFOEN) //ldy change fron 1->0
    				| P_Fld(0x1, PERFCTL0_DISRDPHASE1)
    				| P_Fld(0x1, PERFCTL0_DUALSCHEN));
    vIO32WriteFldAlign(DRAMC_REG_PERFCTL0, 0x0, PERFCTL0_DUALSCHEN);

    vIO32WriteFldAlign(DRAMC_REG_ARBCTL, 0x80, ARBCTL_MAXPENDCNT);

    vIO32WriteFldMulti(DRAMC_REG_PADCTRL, P_Fld(0x1, PADCTRL_DQIENLATEBEGIN)
    				| P_Fld(0x1, PADCTRL_DQIENQKEND));

    vIO32WriteFldMulti(DRAMC_REG_DRAMC_PD_CTRL, P_Fld(0x1, DRAMC_PD_CTRL_DCMREF_OPT)
    				| P_Fld(0x1, DRAMC_PD_CTRL_DCMEN));

    vIO32WriteFldAlign(DRAMC_REG_CLKCTRL, 0x1, CLKCTRL_CLK_EN_1);

    vIO32WriteFldMulti(DRAMC_REG_REFCTRL0, P_Fld(0x5, REFCTRL0_REF_PREGATE_CNT)
    				| P_Fld(0x4, REFCTRL0_DISBYREFNUM)
    				| P_Fld(0x1, REFCTRL0_DLLFRZ));

    vIO32WriteFldAlign(DRAMC_REG_SPCMDCTRL, 0x1, SPCMDCTRL_CLR_EN);

    vIO32WriteFldMulti(DRAMC_REG_CATRAINING1, P_Fld(0x4, CATRAINING1_CATRAIN_INTV)
    				| P_Fld(0x3, CATRAINING1_CATRAINLAT));

    vIO32WriteFldMulti(DRAMC_REG_STBCAL, P_Fld(0x1, STBCAL_DQSIENMODE)
    				| P_Fld(0x0, STBCAL_DQSIENMODE_SELPH)
    				| P_Fld(0x1, STBCAL_REFUICHG));

    vIO32WriteFldMulti(DRAMC_REG_RKCFG, P_Fld(0x1, RKCFG_MRS2RK)
                    | P_Fld(0x1, RKCFG_RKMODE)
                    | P_Fld(0x1, RKCFG_CKE2RANK_OPT2));
     vIO32WriteFldAlign(DRAMC_REG_RKCFG, 1, RKCFG_CS2RANK);
	 mcDELAY_US(20);
     vIO32WriteFldMulti(DRAMC_REG_CKECTRL, P_Fld(0x1, CKECTRL_CKEON)
                    | P_Fld(0x1, CKECTRL_CKETIMER_SEL)
                    | P_Fld(0x1, CKECTRL_CKEFIXON)
                    | P_Fld(0x1, CKECTRL_CKE1FIXON));
    mcDELAY_US(100);//CKE ON

	//------------------------------------------------------
	// precharger all
	vIO32WriteFldAlign(DRAMC_REG_SPCMD, 1, SPCMD_PREAEN);
	vIO32WriteFldAlign(DRAMC_REG_SPCMD, 0, SPCMD_PREAEN);
	mcDELAY_US(1);

	//Mode register init
	//EMR2
	DramcModeRegWrite_PC3 (p, 0x2, 0x0);
	mcDELAY_US(100);

	//EMR3
	DramcModeRegWrite_PC3 (p, 0x3, 0x0);
	mcDELAY_US(100);

	//EMR1
	//DramcModeRegWrite_PC3 (p, 0x1, 0x40);
	DramcModeRegWrite_PC3 (p, 0x1, 0x0);
	mcDELAY_US(100);

	//MRS
	DramcModeRegWrite_PC3 (p, 0x0, 0xf73);
	mcDELAY_US(100);

	// precharger all
	vIO32WriteFldAlign(DRAMC_REG_SPCMD, 1, SPCMD_PREAEN);
	vIO32WriteFldAlign(DRAMC_REG_SPCMD, 0, SPCMD_PREAEN);
	mcDELAY_US(1);

	//auto-refresh
	vIO32WriteFldAlign(DRAMC_REG_SPCMD, 1, SPCMD_AREFEN);
	vIO32WriteFldAlign(DRAMC_REG_SPCMD, 0, SPCMD_AREFEN);
	mcDELAY_US(1);

	//auto-refresh again
	vIO32WriteFldAlign(DRAMC_REG_SPCMD, 1, SPCMD_AREFEN);
	vIO32WriteFldAlign(DRAMC_REG_SPCMD, 0, SPCMD_AREFEN);
	mcDELAY_US(1);

	//MRS: A8 low
	DramcModeRegWrite_PC3 (p, 0x0, 0xe73);
	mcDELAY_US(100);

	//EMR1
	// OCD calibration default
	//Rtt 150ohm
	//DramcModeRegWrite_PC3 (p, 0x1, 0x3c0);
	DramcModeRegWrite_PC3 (p, 0x1, 0x380);
	mcDELAY_US(100);

	//EMR1
	//DramcModeRegWrite_PC3 (p, 0x1, 0x50);
	DramcModeRegWrite_PC3 (p, 0x1, 0x10);
	mcDELAY_US(100);


	//MR end, enable MRR
	vIO32WriteFldMulti(DRAMC_REG_MRS, P_Fld(0x7, MRS_MRSBA) |
									P_Fld(0x1FFF, MRS_MRSMA));
	vIO32WriteFldAlign(DRAMC_REG_SPCMD, 1, SPCMD_MRREN);
	vIO32WriteFldAlign(DRAMC_REG_SPCMD, 0, SPCMD_MRREN);
	mcDELAY_US(100);

	// precharger all
	vIO32WriteFldAlign(DRAMC_REG_SPCMD, 1, SPCMD_PREAEN);
	vIO32WriteFldAlign(DRAMC_REG_SPCMD, 0, SPCMD_PREAEN);
	mcDELAY_US(10);
	//------------------------------------------------------
	//ldy add: after ModeReg init, set CS0/CS1 as independent.
	vIO32WriteFldMulti(DRAMC_REG_RKCFG, P_Fld(0x0, RKCFG_CS2RANK)
		| P_Fld(0x0, RKCFG_MRS2RK));
	//ldy add end

    vIO32WriteFldMulti(DRAMC_REG_DRAMCTRL, P_Fld(0x7, DRAMCTRL_TCMD));

    //byzj vIO32WriteFldAlign(DRAMC_REG_SPCMD, 1, SPCMD_ZQCEN);

    //byzj vIO32WriteFldAlign(DRAMC_REG_SPCMD, 0, SPCMD_ZQCEN);

    vIO32WriteFldAlign(DRAMC_REG_SPCMD, 1, SPCMD_TCMDEN);

    vIO32WriteFldAlign(DRAMC_REG_SPCMD, 0, SPCMD_TCMDEN);

    vIO32WriteFldAlign(DRAMC_REG_HW_MRR_FUN, 0x1, HW_MRR_FUN_TMRR_ENA);

    vIO32WriteFldAlign(DRAMC_REG_DRAMCTRL, 0x1, DRAMCTRL_WDATRGO);

    vIO32WriteFldAlign(DRAMC_REG_DRAMCTRL, 0x1, DRAMCTRL_CLKWITRFC);

	vIO32WriteFldAlign(DRAMC_REG_SHU_ODTCTRL, 0x1, SHU_ODTCTRL_WOEN); /* ldy add to enable Write ODT */

    vIO32WriteFldAlign(DRAMC_REG_ZQCS, 0x56, ZQCS_ZQCSOP);

    vIO32WriteFldAlign(DRAMC_REG_SHU_CONF3, 0xff, SHU_CONF3_REFRCNT);

    vIO32WriteFldMulti(DRAMC_REG_SPCMDCTRL, P_Fld(0x0, SPCMDCTRL_REFRDIS)
    				| P_Fld(0x1, SPCMDCTRL_CLR_EN));

    vIO32WriteFldMulti(DRAMC_REG_SHU_CONF1, P_Fld(0xb0, SHU_CONF1_REFBW_FR));

    vIO32WriteFldAlign(DRAMC_REG_REFCTRL0, 0x1, REFCTRL0_REFFRERUN);

    vIO32WriteFldAlign(DRAMC_REG_SREFCTRL, 0x1, SREFCTRL_SREF_HW_EN);

    vIO32WriteFldMulti(DRAMC_REG_DRAMC_PD_CTRL, P_Fld(0x1, DRAMC_PD_CTRL_PHYCLKDYNGEN)
        			| P_Fld(0x0, DRAMC_PD_CTRL_MIOCKCTRLOFF)
        			| P_Fld(0x1, DRAMC_PD_CTRL_DCMEN));

    vIO32WriteFldMulti(DRAMC_REG_EYESCAN, P_Fld(0x1, EYESCAN_RX_DQ_EYE_SEL)
    			    | P_Fld(0x1, EYESCAN_RG_EX_EYE_SCAN_EN));

    vIO32WriteFldMulti(DRAMC_REG_STBCAL1, P_Fld(0x1, STBCAL1_STBCNT_LATCH_EN)
    		    | P_Fld(0x1, STBCAL1_STBENCMPEN));

    vIO32WriteFldAlign(DRAMC_REG_TEST2_1, 0x10000, TEST2_1_TEST2_BASE);

    vIO32WriteFldAlign(DRAMC_REG_TEST2_2, 0x400, TEST2_2_TEST2_OFF);

    vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0x1, TEST2_3_TEST2WREN2_HW_EN)
        		| P_Fld(0x4, TEST2_3_DQSICALSTP)
        		| P_Fld(0x1, TEST2_3_TESTAUDPAT));

    vIO32WriteFldAlign(DRAMC_REG_SHUCTRL2, 0x1, SHUCTRL2_MR13_SHU_EN);

    vIO32WriteFldMulti(DRAMC_REG_DRAMCTRL, P_Fld(0x1, DRAMCTRL_REQQUE_THD_EN)
        		| P_Fld(0x1, DRAMCTRL_PREALL_OPTION)
        		| P_Fld(0x1, DRAMCTRL_DPDRK_OPT));

    vIO32WriteFldAlign(DRAMC_REG_SHU_CKECTRL, 0x3, SHU_CKECTRL_SREF_CK_DLY);

    vIO32WriteFldAlign(DRAMC_REG_DUMMY_RD, 0x2, DUMMY_RD_RANK_NUM);

    vIO32WriteFldAlign(DRAMC_REG_TEST2_4, 0x4, TEST2_4_TESTAGENTRKSEL);

    vIO32WriteFldMulti(DRAMC_REG_REFCTRL1, P_Fld(0x1, REFCTRL1_REF_QUE_AUTOSAVE_EN)
    		    | P_Fld(0x1, REFCTRL1_SLEFREF_AUTOSAVE_EN));

    vIO32WriteFldMulti(DRAMC_REG_RSTMASK, P_Fld(0x0, RSTMASK_PHY_SYNC_MASK)
        		| P_Fld(0x0, RSTMASK_DAT_SYNC_MASK)
        		| P_Fld(0x0, RSTMASK_GT_SYNC_MASK)
        		| P_Fld(0x0, RSTMASK_DVFS_SYNC_MASK)
        		| P_Fld(0x0, RSTMASK_GT_SYNC_MASK_FOR_PHY)
        		| P_Fld(0x0, RSTMASK_DVFS_SYNC_MASK_FOR_PHY));

    vIO32WriteFldAlign(DRAMC_REG_DRAMCTRL, 0x0, DRAMCTRL_CTOREQ_HPRI_OPT);

#if 0
    vIO32WriteFldAlign(DDRPHY_PLL1, 0x1, PLL1_DDR3_2X8_OPEN);
#endif

    vIO32WriteFldMulti(DRAMC_REG_CKECTRL, P_Fld(0x0, CKECTRL_CKEFIXON)
    		    | P_Fld(0x0, CKECTRL_CKE1FIXON));

    vIO32WriteFldAlign(DRAMC_REG_REFCTRL0, 0x0, REFCTRL0_REFDIS);

    return DRAM_OK;
}

#if COMPILE_PCDDR3
static DRAM_STATUS_T DramcSetting_Default_PC3(DRAMC_CTX_T *p)
{
	mcSHOW_DBG_MSG2(("[Dramc] PCDDR3 default setting update \n"));

	//TX CMD 2T
	vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_CA1, P_Fld(0x0, SHU_SELPH_CA1_TXDLY_CS1)
    				| P_Fld(0x0, SHU_SELPH_CA1_TXDLY_RAS)
    				| P_Fld(0x0, SHU_SELPH_CA1_TXDLY_CAS)
    				| P_Fld(0x0, SHU_SELPH_CA1_TXDLY_WE)
    				| P_Fld(0x0, SHU_SELPH_CA1_TXDLY_RESET)
    				| P_Fld(0x0, SHU_SELPH_CA1_TXDLY_ODT)
    				| P_Fld(0x0, SHU_SELPH_CA1_TXDLY_CKE)
    				| P_Fld(0x0, SHU_SELPH_CA1_TXDLY_CS));

	vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_CA2, P_Fld(0x0, SHU_SELPH_CA2_TXDLY_CKE1)
    				| P_Fld(0x0, SHU_SELPH_CA2_TXDLY_CMD)
    				| P_Fld(0x0, SHU_SELPH_CA2_TXDLY_BA2)
    				| P_Fld(0x0, SHU_SELPH_CA2_TXDLY_BA1)
    				| P_Fld(0x0, SHU_SELPH_CA2_TXDLY_BA0));

	vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_CA3, P_Fld(0x0, SHU_SELPH_CA3_TXDLY_RA7)
    				| P_Fld(0x0, SHU_SELPH_CA3_TXDLY_RA6)
    				| P_Fld(0x0, SHU_SELPH_CA3_TXDLY_RA5)
    				| P_Fld(0x0, SHU_SELPH_CA3_TXDLY_RA4)
    				| P_Fld(0x0, SHU_SELPH_CA3_TXDLY_RA3)
    				| P_Fld(0x0, SHU_SELPH_CA3_TXDLY_RA2)
    				| P_Fld(0x0, SHU_SELPH_CA3_TXDLY_RA1)
    				| P_Fld(0x0, SHU_SELPH_CA3_TXDLY_RA0));

	vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_CA4, P_Fld(0x0, SHU_SELPH_CA4_TXDLY_RA15)
    				| P_Fld(0x0, SHU_SELPH_CA4_TXDLY_RA14)
    				| P_Fld(0x0, SHU_SELPH_CA4_TXDLY_RA13)
    				| P_Fld(0x0, SHU_SELPH_CA4_TXDLY_RA12)
    				| P_Fld(0x0, SHU_SELPH_CA4_TXDLY_RA11)
    				| P_Fld(0x0, SHU_SELPH_CA4_TXDLY_RA10)
    				| P_Fld(0x0, SHU_SELPH_CA4_TXDLY_RA9)
    				| P_Fld(0x0, SHU_SELPH_CA4_TXDLY_RA8));

	//TX CMD 05T
#if 0
	//1T mode
	vIO32Write4B(DRAMC_REG_SHU_SELPH_CA5, 0x11111111);
	vIO32Write4B(DRAMC_REG_SHU_SELPH_CA6, 0x01101111);
	vIO32Write4B(DRAMC_REG_SHU_SELPH_CA7, 0x11111111);
	vIO32Write4B(DRAMC_REG_SHU_SELPH_CA8, 0x11111111);
#else
	//2T mode
#if 0// cc mark for coding style change
	vIO32Write4B(DRAMC_REG_SHU_SELPH_CA5, 0x10000111);
	vIO32Write4B(DRAMC_REG_SHU_SELPH_CA6, 0x01000000);
	vIO32Write4B(DRAMC_REG_SHU_SELPH_CA7, 0x00000000);
	vIO32Write4B(DRAMC_REG_SHU_SELPH_CA8, 0x00000000);
#else
	//cc add
#if CA_TWOTCMD_EN
	vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_CA5, P_Fld(0x1, SHU_SELPH_CA5_DLY_CS)
		| P_Fld(0x1, SHU_SELPH_CA5_DLY_CKE)
		| P_Fld(0x1, SHU_SELPH_CA5_DLY_ODT) /* cc change from 0->1 to resolve ODT tIH violation */
		| P_Fld(0x0, SHU_SELPH_CA5_DLY_WE)
		| P_Fld(0x0, SHU_SELPH_CA5_DLY_CAS)
		| P_Fld(0x0, SHU_SELPH_CA5_DLY_RAS)
		| P_Fld(0x1, SHU_SELPH_CA5_DLY_CS1));
	vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_CA6, P_Fld(0x0, SHU_SELPH_CA6_DLY_BA0)
		| P_Fld(0x0, SHU_SELPH_CA6_DLY_BA1)
		| P_Fld(0x0, SHU_SELPH_CA6_DLY_BA2)
		| P_Fld(0x0, SHU_SELPH_CA6_DLY_CMD)
		| P_Fld(0x1, SHU_SELPH_CA6_DLY_CKE1));	
	vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_CA7, P_Fld(0x0, SHU_SELPH_CA7_DLY_RA0)
		| P_Fld(0x0, SHU_SELPH_CA7_DLY_RA1)
		| P_Fld(0x0, SHU_SELPH_CA7_DLY_RA2)
		| P_Fld(0x0, SHU_SELPH_CA7_DLY_RA3)
		| P_Fld(0x0, SHU_SELPH_CA7_DLY_RA4)
		| P_Fld(0x0, SHU_SELPH_CA7_DLY_RA5)
		| P_Fld(0x0, SHU_SELPH_CA7_DLY_RA6)
		| P_Fld(0x0, SHU_SELPH_CA7_DLY_RA7));	
	vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_CA8, P_Fld(0x0, SHU_SELPH_CA8_DLY_RA8)
		| P_Fld(0x0, SHU_SELPH_CA8_DLY_RA9)
		| P_Fld(0x0, SHU_SELPH_CA8_DLY_RA10)
		| P_Fld(0x0, SHU_SELPH_CA8_DLY_RA11)
		| P_Fld(0x0, SHU_SELPH_CA8_DLY_RA12)
		| P_Fld(0x0, SHU_SELPH_CA8_DLY_RA13)
		| P_Fld(0x0, SHU_SELPH_CA8_DLY_RA14)
		| P_Fld(0x0, SHU_SELPH_CA8_DLY_RA15));
#else
	vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_CA5, P_Fld(0x1, SHU_SELPH_CA5_DLY_CS)
		| P_Fld(0x1, SHU_SELPH_CA5_DLY_CKE)
		| P_Fld(0x1, SHU_SELPH_CA5_DLY_ODT)
		| P_Fld(0x1, SHU_SELPH_CA5_DLY_WE)
		| P_Fld(0x1, SHU_SELPH_CA5_DLY_CAS)
		| P_Fld(0x1, SHU_SELPH_CA5_DLY_RAS)
		| P_Fld(0x1, SHU_SELPH_CA5_DLY_CS1)
		| P_Fld(0x1, SHU_SELPH_CA5_DLY_RESET));
	vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_CA6, P_Fld(0x1, SHU_SELPH_CA6_DLY_BA0)
		| P_Fld(0x1, SHU_SELPH_CA6_DLY_BA1)
		| P_Fld(0x1, SHU_SELPH_CA6_DLY_BA2)
		| P_Fld(0x0, SHU_SELPH_CA6_DLY_CMD)
		| P_Fld(0x1, SHU_SELPH_CA6_DLY_CKE1));	
	vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_CA7, P_Fld(0x1, SHU_SELPH_CA7_DLY_RA0)
		| P_Fld(0x1, SHU_SELPH_CA7_DLY_RA1)
		| P_Fld(0x1, SHU_SELPH_CA7_DLY_RA2)
		| P_Fld(0x1, SHU_SELPH_CA7_DLY_RA3)
		| P_Fld(0x1, SHU_SELPH_CA7_DLY_RA4)
		| P_Fld(0x1, SHU_SELPH_CA7_DLY_RA5)
		| P_Fld(0x1, SHU_SELPH_CA7_DLY_RA6)
		| P_Fld(0x1, SHU_SELPH_CA7_DLY_RA7));	
	vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_CA8, P_Fld(0x1, SHU_SELPH_CA8_DLY_RA8)
		| P_Fld(0x1, SHU_SELPH_CA8_DLY_RA9)
		| P_Fld(0x1, SHU_SELPH_CA8_DLY_RA10)
		| P_Fld(0x1, SHU_SELPH_CA8_DLY_RA11)
		| P_Fld(0x1, SHU_SELPH_CA8_DLY_RA12)
		| P_Fld(0x1, SHU_SELPH_CA8_DLY_RA13)
		| P_Fld(0x1, SHU_SELPH_CA8_DLY_RA14)
		| P_Fld(0x1, SHU_SELPH_CA8_DLY_RA15));

#endif
	//cc add end
#endif
#endif

	//CMD PI
    vIO32WriteFldMulti(DDRPHY_SHU1_R0_CA_CMD9, P_Fld(0x0, SHU1_R0_CA_CMD9_RG_RK0_ARPI_CLK)
    				| P_Fld(0xA, SHU1_R0_CA_CMD9_RG_RK0_ARPI_CMD)	//YMC modify_20180425
    				| P_Fld(0x0, SHU1_R0_CA_CMD9_RG_RK0_ARPI_CS));
    vIO32WriteFldMulti(DDRPHY_SHU1_R1_CA_CMD9, P_Fld(0x0, SHU1_R1_CA_CMD9_RG_RK1_ARPI_CLK)
    				| P_Fld(0x0, SHU1_R1_CA_CMD9_RG_RK1_ARPI_CMD)
    				| P_Fld(0x0, SHU1_R1_CA_CMD9_RG_RK1_ARPI_CS));

	if (p->frequency <= DDR_DDR1600) {
		//TX DQS 2T
		vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_DQS0, P_Fld(0x1, SHU_SELPH_DQS0_TXDLY_OEN_DQS3)
						| P_Fld(0x1, SHU_SELPH_DQS0_TXDLY_OEN_DQS2)
						| P_Fld(0x1, SHU_SELPH_DQS0_TXDLY_OEN_DQS1)
						| P_Fld(0x1, SHU_SELPH_DQS0_TXDLY_OEN_DQS0)
						| P_Fld(0x2, SHU_SELPH_DQS0_TXDLY_DQS3)
						| P_Fld(0x2, SHU_SELPH_DQS0_TXDLY_DQS2)
						| P_Fld(0x2, SHU_SELPH_DQS0_TXDLY_DQS1)
						| P_Fld(0x2, SHU_SELPH_DQS0_TXDLY_DQS0));
		//TX DQS 05T
		vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_DQS1, P_Fld(0x3, SHU_SELPH_DQS1_DLY_OEN_DQS3)
						| P_Fld(0x2, SHU_SELPH_DQS1_DLY_OEN_DQS2)
						| P_Fld(0x2, SHU_SELPH_DQS1_DLY_OEN_DQS1)
						| P_Fld(0x2, SHU_SELPH_DQS1_DLY_OEN_DQS0)
						| P_Fld(0x0, SHU_SELPH_DQS1_DLY_DQS3)
						| P_Fld(0x0, SHU_SELPH_DQS1_DLY_DQS2)
						| P_Fld(0x0, SHU_SELPH_DQS1_DLY_DQS1)
						| P_Fld(0x0, SHU_SELPH_DQS1_DLY_DQS0));
		//TX DQ 2T
		vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ0, P_Fld(0x1, SHURK0_SELPH_DQ0_TXDLY_OEN_DQ3)
						| P_Fld(0x1, SHURK0_SELPH_DQ0_TXDLY_OEN_DQ2)
						| P_Fld(0x1, SHURK0_SELPH_DQ0_TXDLY_OEN_DQ1)
						| P_Fld(0x1, SHURK0_SELPH_DQ0_TXDLY_OEN_DQ0)
						| P_Fld(0x2, SHURK0_SELPH_DQ0_TXDLY_DQ3)
						| P_Fld(0x2, SHURK0_SELPH_DQ0_TXDLY_DQ2)
						| P_Fld(0x2, SHURK0_SELPH_DQ0_TXDLY_DQ1)
						| P_Fld(0x2, SHURK0_SELPH_DQ0_TXDLY_DQ0));
		//TX DQM 2T
		vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ1, P_Fld(0x1, SHURK0_SELPH_DQ1_TXDLY_OEN_DQM3)
						| P_Fld(0x1, SHURK0_SELPH_DQ1_TXDLY_OEN_DQM2)
						| P_Fld(0x1, SHURK0_SELPH_DQ1_TXDLY_OEN_DQM1)
						| P_Fld(0x1, SHURK0_SELPH_DQ1_TXDLY_OEN_DQM0)
						| P_Fld(0x2, SHURK0_SELPH_DQ1_TXDLY_DQM3)
						| P_Fld(0x2, SHURK0_SELPH_DQ1_TXDLY_DQM2)
						| P_Fld(0x2, SHURK0_SELPH_DQ1_TXDLY_DQM1)
						| P_Fld(0x2, SHURK0_SELPH_DQ1_TXDLY_DQM0));
		//TX DQ 05T
		vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ2, P_Fld(0x3, SHURK0_SELPH_DQ2_DLY_OEN_DQ3)
						| P_Fld(0x2, SHURK0_SELPH_DQ2_DLY_OEN_DQ2)
						| P_Fld(0x2, SHURK0_SELPH_DQ2_DLY_OEN_DQ1)
						| P_Fld(0x2, SHURK0_SELPH_DQ2_DLY_OEN_DQ0)
						| P_Fld(0x0, SHURK0_SELPH_DQ2_DLY_DQ3)
						| P_Fld(0x0, SHURK0_SELPH_DQ2_DLY_DQ2)
						| P_Fld(0x0, SHURK0_SELPH_DQ2_DLY_DQ1)
						| P_Fld(0x0, SHURK0_SELPH_DQ2_DLY_DQ0));
		//TX DQM 05T
		vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ3, P_Fld(0x3, SHURK0_SELPH_DQ3_DLY_OEN_DQM3)
						| P_Fld(0x2, SHURK0_SELPH_DQ3_DLY_OEN_DQM2)
						| P_Fld(0x2, SHURK0_SELPH_DQ3_DLY_OEN_DQM1)
						| P_Fld(0x2, SHURK0_SELPH_DQ3_DLY_OEN_DQM0)
						| P_Fld(0x0, SHURK0_SELPH_DQ3_DLY_DQM3)
						| P_Fld(0x0, SHURK0_SELPH_DQ3_DLY_DQM2)
						| P_Fld(0x0, SHURK0_SELPH_DQ3_DLY_DQM1)
						| P_Fld(0x0, SHURK0_SELPH_DQ3_DLY_DQM0));

		//TX RANK1 DQ 2T
		vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_DQ0, P_Fld(0x1, SHURK1_SELPH_DQ0_TX_DLY_R1OEN_DQ3)
						| P_Fld(0x1, SHURK1_SELPH_DQ0_TX_DLY_R1OEN_DQ2)
						| P_Fld(0x1, SHURK1_SELPH_DQ0_TX_DLY_R1OEN_DQ1)
						| P_Fld(0x1, SHURK1_SELPH_DQ0_TX_DLY_R1OEN_DQ0)
						| P_Fld(0x2, SHURK1_SELPH_DQ0_TX_DLY_R1DQ3)
						| P_Fld(0x2, SHURK1_SELPH_DQ0_TX_DLY_R1DQ2)
						| P_Fld(0x2, SHURK1_SELPH_DQ0_TX_DLY_R1DQ1)
						| P_Fld(0x2, SHURK1_SELPH_DQ0_TX_DLY_R1DQ0));
		//TX RANK1 DQM 2T
		vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_DQ1, P_Fld(0x1, SHURK1_SELPH_DQ1_TX_DLY_R1OEN_DQM3)
						| P_Fld(0x1, SHURK1_SELPH_DQ1_TX_DLY_R1OEN_DQM2)
						| P_Fld(0x1, SHURK1_SELPH_DQ1_TX_DLY_R1OEN_DQM1)
						| P_Fld(0x1, SHURK1_SELPH_DQ1_TX_DLY_R1OEN_DQM0)
						| P_Fld(0x2, SHURK1_SELPH_DQ1_TX_DLY_R1DQM3)
						| P_Fld(0x2, SHURK1_SELPH_DQ1_TX_DLY_R1DQM2)
						| P_Fld(0x2, SHURK1_SELPH_DQ1_TX_DLY_R1DQM1)
						| P_Fld(0x2, SHURK1_SELPH_DQ1_TX_DLY_R1DQM0));
		//TX RANK1 DQ 05T
		vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_DQ2, P_Fld(0x3, SHURK1_SELPH_DQ2_DLY_R1OEN_DQ3)
						| P_Fld(0x2, SHURK1_SELPH_DQ2_DLY_R1OEN_DQ2)
						| P_Fld(0x2, SHURK1_SELPH_DQ2_DLY_R1OEN_DQ1)
						| P_Fld(0x2, SHURK1_SELPH_DQ2_DLY_R1OEN_DQ0)
						| P_Fld(0x0, SHURK1_SELPH_DQ2_DLY_R1DQ3)
						| P_Fld(0x0, SHURK1_SELPH_DQ2_DLY_R1DQ2)
						| P_Fld(0x0, SHURK1_SELPH_DQ2_DLY_R1DQ1)
						| P_Fld(0x0, SHURK1_SELPH_DQ2_DLY_R1DQ0));

		//TX RANK1 DQM 05T
		vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_DQ3, P_Fld(0x3, SHURK1_SELPH_DQ3_DLY_R1OEN_DQM3)
						| P_Fld(0x2, SHURK1_SELPH_DQ3_DLY_R1OEN_DQM2)
						| P_Fld(0x2, SHURK1_SELPH_DQ3_DLY_R1OEN_DQM1)
						| P_Fld(0x2, SHURK1_SELPH_DQ3_DLY_R1OEN_DQM0)
						| P_Fld(0x0, SHURK1_SELPH_DQ3_DLY_R1DQM3)
						| P_Fld(0x0, SHURK1_SELPH_DQ3_DLY_R1DQM2)
						| P_Fld(0x0, SHURK1_SELPH_DQ3_DLY_R1DQM1)
						| P_Fld(0x0, SHURK1_SELPH_DQ3_DLY_R1DQM0));
	} else if (p->frequency == DDR_DDR1866) {
		//TX DQS 2T
		vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_DQS0, P_Fld(0x2, SHU_SELPH_DQS0_TXDLY_OEN_DQS3)
						| P_Fld(0x2, SHU_SELPH_DQS0_TXDLY_OEN_DQS2)
						| P_Fld(0x2, SHU_SELPH_DQS0_TXDLY_OEN_DQS1)
						| P_Fld(0x2, SHU_SELPH_DQS0_TXDLY_OEN_DQS0)
						| P_Fld(0x2, SHU_SELPH_DQS0_TXDLY_DQS3)
						| P_Fld(0x2, SHU_SELPH_DQS0_TXDLY_DQS2)
						| P_Fld(0x2, SHU_SELPH_DQS0_TXDLY_DQS1)
						| P_Fld(0x2, SHU_SELPH_DQS0_TXDLY_DQS0));
		//TX DQS 05T
		vIO32WriteFldMulti(DRAMC_REG_SHU_SELPH_DQS1, P_Fld(0x5, SHU_SELPH_DQS1_DLY_OEN_DQS3)
						| P_Fld(0x5, SHU_SELPH_DQS1_DLY_OEN_DQS2)
						| P_Fld(0x5, SHU_SELPH_DQS1_DLY_OEN_DQS1)
						| P_Fld(0x5, SHU_SELPH_DQS1_DLY_OEN_DQS0)
						| P_Fld(0x3, SHU_SELPH_DQS1_DLY_DQS3)
						| P_Fld(0x3, SHU_SELPH_DQS1_DLY_DQS2)
						| P_Fld(0x3, SHU_SELPH_DQS1_DLY_DQS1)
						| P_Fld(0x3, SHU_SELPH_DQS1_DLY_DQS0));
		//TX DQ 2T
		vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ0, P_Fld(0x2, SHURK0_SELPH_DQ0_TXDLY_OEN_DQ3)
						| P_Fld(0x2, SHURK0_SELPH_DQ0_TXDLY_OEN_DQ2)
						| P_Fld(0x2, SHURK0_SELPH_DQ0_TXDLY_OEN_DQ1)
						| P_Fld(0x2, SHURK0_SELPH_DQ0_TXDLY_OEN_DQ0)
						| P_Fld(0x2, SHURK0_SELPH_DQ0_TXDLY_DQ3)
						| P_Fld(0x2, SHURK0_SELPH_DQ0_TXDLY_DQ2)
						| P_Fld(0x2, SHURK0_SELPH_DQ0_TXDLY_DQ1)
						| P_Fld(0x2, SHURK0_SELPH_DQ0_TXDLY_DQ0));
		//TX DQM 2T
		vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ1, P_Fld(0x2, SHURK0_SELPH_DQ1_TXDLY_OEN_DQM3)
						| P_Fld(0x2, SHURK0_SELPH_DQ1_TXDLY_OEN_DQM2)
						| P_Fld(0x2, SHURK0_SELPH_DQ1_TXDLY_OEN_DQM1)
						| P_Fld(0x2, SHURK0_SELPH_DQ1_TXDLY_OEN_DQM0)
						| P_Fld(0x2, SHURK0_SELPH_DQ1_TXDLY_DQM3)
						| P_Fld(0x2, SHURK0_SELPH_DQ1_TXDLY_DQM2)
						| P_Fld(0x2, SHURK0_SELPH_DQ1_TXDLY_DQM1)
						| P_Fld(0x2, SHURK0_SELPH_DQ1_TXDLY_DQM0));
		//TX DQ 05T
		vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ2, P_Fld(0x5, SHURK0_SELPH_DQ2_DLY_OEN_DQ3)
						| P_Fld(0x5, SHURK0_SELPH_DQ2_DLY_OEN_DQ2)
						| P_Fld(0x5, SHURK0_SELPH_DQ2_DLY_OEN_DQ1)
						| P_Fld(0x5, SHURK0_SELPH_DQ2_DLY_OEN_DQ0)
						| P_Fld(0x3, SHURK0_SELPH_DQ2_DLY_DQ3)
						| P_Fld(0x3, SHURK0_SELPH_DQ2_DLY_DQ2)
						| P_Fld(0x3, SHURK0_SELPH_DQ2_DLY_DQ1)
						| P_Fld(0x3, SHURK0_SELPH_DQ2_DLY_DQ0));
		//TX DQM 05T
		vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQ3, P_Fld(0x5, SHURK0_SELPH_DQ3_DLY_OEN_DQM3)
						| P_Fld(0x5, SHURK0_SELPH_DQ3_DLY_OEN_DQM2)
						| P_Fld(0x5, SHURK0_SELPH_DQ3_DLY_OEN_DQM1)
						| P_Fld(0x5, SHURK0_SELPH_DQ3_DLY_OEN_DQM0)
						| P_Fld(0x3, SHURK0_SELPH_DQ3_DLY_DQM3)
						| P_Fld(0x3, SHURK0_SELPH_DQ3_DLY_DQM2)
						| P_Fld(0x3, SHURK0_SELPH_DQ3_DLY_DQM1)
						| P_Fld(0x3, SHURK0_SELPH_DQ3_DLY_DQM0));

		//TX RANK1 DQ 2T
		vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_DQ0, P_Fld(0x2, SHURK1_SELPH_DQ0_TX_DLY_R1OEN_DQ3)
						| P_Fld(0x2, SHURK1_SELPH_DQ0_TX_DLY_R1OEN_DQ2)
						| P_Fld(0x2, SHURK1_SELPH_DQ0_TX_DLY_R1OEN_DQ1)
						| P_Fld(0x2, SHURK1_SELPH_DQ0_TX_DLY_R1OEN_DQ0)
						| P_Fld(0x2, SHURK1_SELPH_DQ0_TX_DLY_R1DQ3)
						| P_Fld(0x2, SHURK1_SELPH_DQ0_TX_DLY_R1DQ2)
						| P_Fld(0x2, SHURK1_SELPH_DQ0_TX_DLY_R1DQ1)
						| P_Fld(0x2, SHURK1_SELPH_DQ0_TX_DLY_R1DQ0));
		//TX RANK1 DQM 2T
		vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_DQ1, P_Fld(0x2, SHURK1_SELPH_DQ1_TX_DLY_R1OEN_DQM3)
						| P_Fld(0x2, SHURK1_SELPH_DQ1_TX_DLY_R1OEN_DQM2)
						| P_Fld(0x2, SHURK1_SELPH_DQ1_TX_DLY_R1OEN_DQM1)
						| P_Fld(0x2, SHURK1_SELPH_DQ1_TX_DLY_R1OEN_DQM0)
						| P_Fld(0x2, SHURK1_SELPH_DQ1_TX_DLY_R1DQM3)
						| P_Fld(0x2, SHURK1_SELPH_DQ1_TX_DLY_R1DQM2)
						| P_Fld(0x2, SHURK1_SELPH_DQ1_TX_DLY_R1DQM1)
						| P_Fld(0x2, SHURK1_SELPH_DQ1_TX_DLY_R1DQM0));
		//TX RANK1 DQ 05T
		vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_DQ2, P_Fld(0x5, SHURK1_SELPH_DQ2_DLY_R1OEN_DQ3)
						| P_Fld(0x5, SHURK1_SELPH_DQ2_DLY_R1OEN_DQ2)
						| P_Fld(0x5, SHURK1_SELPH_DQ2_DLY_R1OEN_DQ1)
						| P_Fld(0x5, SHURK1_SELPH_DQ2_DLY_R1OEN_DQ0)
						| P_Fld(0x3, SHURK1_SELPH_DQ2_DLY_R1DQ3)
						| P_Fld(0x3, SHURK1_SELPH_DQ2_DLY_R1DQ2)
						| P_Fld(0x3, SHURK1_SELPH_DQ2_DLY_R1DQ1)
						| P_Fld(0x3, SHURK1_SELPH_DQ2_DLY_R1DQ0));

		//TX RANK1 DQM 05T
		vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_DQ3, P_Fld(0x5, SHURK1_SELPH_DQ3_DLY_R1OEN_DQM3)
						| P_Fld(0x5, SHURK1_SELPH_DQ3_DLY_R1OEN_DQM2)
						| P_Fld(0x5, SHURK1_SELPH_DQ3_DLY_R1OEN_DQM1)
						| P_Fld(0x5, SHURK1_SELPH_DQ3_DLY_R1OEN_DQM0)
						| P_Fld(0x3, SHURK1_SELPH_DQ3_DLY_R1DQM3)
						| P_Fld(0x3, SHURK1_SELPH_DQ3_DLY_R1DQM2)
						| P_Fld(0x3, SHURK1_SELPH_DQ3_DLY_R1DQM1)
						| P_Fld(0x3, SHURK1_SELPH_DQ3_DLY_R1DQM0));
	}

	//TX DQS/DQM/DQ PI
    vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ7, P_Fld(0x0, SHU1_R0_B0_DQ7_RK0_ARPI_PBYTE_B0)
    				| P_Fld(0xf, SHU1_R0_B0_DQ7_RK0_ARPI_DQM_B0)
    				| P_Fld(0xf, SHU1_R0_B0_DQ7_RK0_ARPI_DQ_B0));
    vIO32WriteFldMulti(DDRPHY_SHU1_R1_B0_DQ7, P_Fld(0x0, SHU1_R1_B0_DQ7_RK1_ARPI_PBYTE_B0)
    				| P_Fld(0xf, SHU1_R1_B0_DQ7_RK1_ARPI_DQM_B0)
    				| P_Fld(0xf, SHU1_R1_B0_DQ7_RK1_ARPI_DQ_B0));
    vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ7, P_Fld(0x0, SHU1_R0_B1_DQ7_RK0_ARPI_PBYTE_B1)
    				| P_Fld(0xf, SHU1_R0_B1_DQ7_RK0_ARPI_DQM_B1)
    				| P_Fld(0xf, SHU1_R0_B1_DQ7_RK0_ARPI_DQ_B1));
    vIO32WriteFldMulti(DDRPHY_SHU1_R1_B1_DQ7, P_Fld(0x0, SHU1_R1_B1_DQ7_RK1_ARPI_PBYTE_B1)
    				| P_Fld(0xf, SHU1_R1_B1_DQ7_RK1_ARPI_DQM_B1)
    				| P_Fld(0xf, SHU1_R1_B1_DQ7_RK1_ARPI_DQ_B1));

    //ODT EN
    vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_ODTEN0, P_Fld(0x1, SHURK0_SELPH_ODTEN0_TXDLY_B3_RODTEN_P1)
    				| P_Fld(0x1, SHURK0_SELPH_ODTEN0_TXDLY_B3_RODTEN)
    				| P_Fld(0x1, SHURK0_SELPH_ODTEN0_TXDLY_B2_RODTEN_P1)
    				| P_Fld(0x1, SHURK0_SELPH_ODTEN0_TXDLY_B2_RODTEN)
    				| P_Fld(0x1, SHURK0_SELPH_ODTEN0_TXDLY_B1_RODTEN_P1) //cc change from 0->1
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN0_TXDLY_B1_RODTEN) //cc change from 0->1
    				| P_Fld(0x1, SHURK0_SELPH_ODTEN0_TXDLY_B0_RODTEN_P1) //cc change from 0->1
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN0_TXDLY_B0_RODTEN)); //cc change from 0->1

    vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_ODTEN1, P_Fld(0x0, SHURK0_SELPH_ODTEN1_DLY_B3_RODTEN_P1)
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN1_DLY_B3_RODTEN)
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN1_DLY_B2_RODTEN_P1)
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN1_DLY_B2_RODTEN)
    				| P_Fld(0x3, SHURK0_SELPH_ODTEN1_DLY_B1_RODTEN_P1) //cc change from 0->3
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN1_DLY_B1_RODTEN) //cc change from 0->3
    				| P_Fld(0x3, SHURK0_SELPH_ODTEN1_DLY_B0_RODTEN_P1) //cc change from 0->3
    				| P_Fld(0x0, SHURK0_SELPH_ODTEN1_DLY_B0_RODTEN)); //cc change from 0->3

    vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_ODTEN0, P_Fld(0x0, SHURK1_SELPH_ODTEN0_TXDLY_B3_R1RODTEN_P1)
    				| P_Fld(0x0, SHURK1_SELPH_ODTEN0_TXDLY_B3_R1RODTEN)
    				| P_Fld(0x0, SHURK1_SELPH_ODTEN0_TXDLY_B2_R1RODTEN_P1)
    				| P_Fld(0x0, SHURK1_SELPH_ODTEN0_TXDLY_B2_R1RODTEN)
    				| P_Fld(0x1, SHURK1_SELPH_ODTEN0_TXDLY_B1_R1RODTEN_P1) //cc change from 0->1
    				| P_Fld(0x1, SHURK1_SELPH_ODTEN0_TXDLY_B1_R1RODTEN) //cc change from 0->1
    				| P_Fld(0x1, SHURK1_SELPH_ODTEN0_TXDLY_B0_R1RODTEN_P1) //cc change from 0->1
    				| P_Fld(0x1, SHURK1_SELPH_ODTEN0_TXDLY_B0_R1RODTEN)); //cc change from 0->1

    vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_ODTEN1, P_Fld(0x0, SHURK1_SELPH_ODTEN1_DLY_B3_R1RODTEN_P1)
    				| P_Fld(0x0, SHURK1_SELPH_ODTEN1_DLY_B3_R1RODTEN)
    				| P_Fld(0x0, SHURK1_SELPH_ODTEN1_DLY_B2_R1RODTEN_P1)
    				| P_Fld(0x0, SHURK1_SELPH_ODTEN1_DLY_B2_R1RODTEN)
    				| P_Fld(0x3, SHURK1_SELPH_ODTEN1_DLY_B1_R1RODTEN_P1) //cc change from 0->3
    				| P_Fld(0x3, SHURK1_SELPH_ODTEN1_DLY_B1_R1RODTEN) //cc change from 0->3
    				| P_Fld(0x3, SHURK1_SELPH_ODTEN1_DLY_B0_R1RODTEN_P1) //cc change from 0->3
    				| P_Fld(0x3, SHURK1_SELPH_ODTEN1_DLY_B0_R1RODTEN)); //cc change from 0->3
	
#if 0 //cc mark for RK2 not supported
    vIO32WriteFldMulti(DRAMC_REG_SHURK2_SELPH_ODTEN0, P_Fld(0x0, SHURK2_SELPH_ODTEN0_TXDLY_B3_R2RODTEN_P1)
    				| P_Fld(0x0, SHURK2_SELPH_ODTEN0_TXDLY_B3_R2RODTEN)
    				| P_Fld(0x0, SHURK2_SELPH_ODTEN0_TXDLY_B2_R2RODTEN_P1)
    				| P_Fld(0x0, SHURK2_SELPH_ODTEN0_TXDLY_B2_R2RODTEN)
    				| P_Fld(0x0, SHURK2_SELPH_ODTEN0_TXDLY_B1_R2RODTEN_P1)
    				| P_Fld(0x0, SHURK2_SELPH_ODTEN0_TXDLY_B1_R2RODTEN)
    				| P_Fld(0x0, SHURK2_SELPH_ODTEN0_TXDLY_B0_R2RODTEN_P1)
    				| P_Fld(0x0, SHURK2_SELPH_ODTEN0_TXDLY_B0_R2RODTEN));

    vIO32WriteFldMulti(DRAMC_REG_SHURK2_SELPH_ODTEN1, P_Fld(0x6, SHURK2_SELPH_ODTEN1_DLY_B3_R2RODTEN_P1)
    				| P_Fld(0x6, SHURK2_SELPH_ODTEN1_DLY_B3_R2RODTEN)
    				| P_Fld(0x6, SHURK2_SELPH_ODTEN1_DLY_B2_R2RODTEN_P1)
    				| P_Fld(0x6, SHURK2_SELPH_ODTEN1_DLY_B2_R2RODTEN)
    				| P_Fld(0x6, SHURK2_SELPH_ODTEN1_DLY_B1_R2RODTEN_P1)
    				| P_Fld(0x6, SHURK2_SELPH_ODTEN1_DLY_B1_R2RODTEN)
    				| P_Fld(0x6, SHURK2_SELPH_ODTEN1_DLY_B0_R2RODTEN_P1)
    				| P_Fld(0x6, SHURK2_SELPH_ODTEN1_DLY_B0_R2RODTEN));
#endif

	//DLE
    vIO32WriteFldMulti(DRAMC_REG_SHU_CONF1, P_Fld(0xb, SHU_CONF1_DATLAT_DSEL_PHY)
    				| P_Fld(0xc, SHU_CONF1_DATLAT_DSEL)//cc change from 0xb->0xc
    				| P_Fld(0xe, SHU_CONF1_DATLAT));

    //GW 2T
#if TEST_ODT_ON
	vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQSG0, P_Fld(0x2, SHURK0_SELPH_DQSG0_TX_DLY_DQS3_GATED_P1)
					| P_Fld(0x2, SHURK0_SELPH_DQSG0_TX_DLY_DQS3_GATED)
					| P_Fld(0x2, SHURK0_SELPH_DQSG0_TX_DLY_DQS2_GATED_P1)
					| P_Fld(0x2, SHURK0_SELPH_DQSG0_TX_DLY_DQS2_GATED)
					| P_Fld(0x2, SHURK0_SELPH_DQSG0_TX_DLY_DQS1_GATED_P1)
					| P_Fld(0x2, SHURK0_SELPH_DQSG0_TX_DLY_DQS1_GATED)
					| P_Fld(0x2, SHURK0_SELPH_DQSG0_TX_DLY_DQS0_GATED_P1)
					| P_Fld(0x2, SHURK0_SELPH_DQSG0_TX_DLY_DQS0_GATED));

	vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQSG1, P_Fld(0x2, SHURK0_SELPH_DQSG1_REG_DLY_DQS3_GATED_P1)
					| P_Fld(0x0, SHURK0_SELPH_DQSG1_REG_DLY_DQS3_GATED)
					| P_Fld(0x2, SHURK0_SELPH_DQSG1_REG_DLY_DQS2_GATED_P1)
					| P_Fld(0x0, SHURK0_SELPH_DQSG1_REG_DLY_DQS2_GATED)
					| P_Fld(0x2, SHURK0_SELPH_DQSG1_REG_DLY_DQS1_GATED_P1) //cc change from 6->4
					| P_Fld(0x0, SHURK0_SELPH_DQSG1_REG_DLY_DQS1_GATED) //cc change from 4->2
					| P_Fld(0x2, SHURK0_SELPH_DQSG1_REG_DLY_DQS0_GATED_P1)//cc change from 6->4
					| P_Fld(0x0, SHURK0_SELPH_DQSG1_REG_DLY_DQS0_GATED)); //cc change from 4->2

	vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_DQSG0, P_Fld(0x2, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS3_GATED_P1)
					| P_Fld(0x2, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS3_GATED)
					| P_Fld(0x2, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS2_GATED_P1)
					| P_Fld(0x2, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS2_GATED)
					| P_Fld(0x2, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS1_GATED_P1)
					| P_Fld(0x2, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS1_GATED)
					| P_Fld(0x2, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS0_GATED_P1)
					| P_Fld(0x2, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS0_GATED));

	vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_DQSG1, P_Fld(0x2, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS3_GATED_P1)
					| P_Fld(0x0, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS3_GATED)
					| P_Fld(0x2, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS2_GATED_P1)
					| P_Fld(0x0, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS2_GATED)
					| P_Fld(0x2, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS1_GATED_P1) //cc change. Same as RANK0
					| P_Fld(0x0, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS1_GATED)//cc change. Same as RANK0
					| P_Fld(0x2, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS0_GATED_P1)//cc change. Same as RANK0
					| P_Fld(0x0, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS0_GATED));//cc change. Same as RANK0

#else
    vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQSG0, P_Fld(0x1, SHURK0_SELPH_DQSG0_TX_DLY_DQS3_GATED_P1)
                    | P_Fld(0x1, SHURK0_SELPH_DQSG0_TX_DLY_DQS3_GATED)
                    | P_Fld(0x1, SHURK0_SELPH_DQSG0_TX_DLY_DQS2_GATED_P1)
                    | P_Fld(0x1, SHURK0_SELPH_DQSG0_TX_DLY_DQS2_GATED)
                    | P_Fld(0x1, SHURK0_SELPH_DQSG0_TX_DLY_DQS1_GATED_P1)
                    | P_Fld(0x1, SHURK0_SELPH_DQSG0_TX_DLY_DQS1_GATED)
                    | P_Fld(0x1, SHURK0_SELPH_DQSG0_TX_DLY_DQS0_GATED_P1)
                    | P_Fld(0x1, SHURK0_SELPH_DQSG0_TX_DLY_DQS0_GATED));

    vIO32WriteFldMulti(DRAMC_REG_SHURK0_SELPH_DQSG1, P_Fld(0x6, SHURK0_SELPH_DQSG1_REG_DLY_DQS3_GATED_P1)
                    | P_Fld(0x4, SHURK0_SELPH_DQSG1_REG_DLY_DQS3_GATED)
                    | P_Fld(0x6, SHURK0_SELPH_DQSG1_REG_DLY_DQS2_GATED_P1)
                    | P_Fld(0x4, SHURK0_SELPH_DQSG1_REG_DLY_DQS2_GATED)
                    | P_Fld(0x4, SHURK0_SELPH_DQSG1_REG_DLY_DQS1_GATED_P1) //cc change from 6->4
                    | P_Fld(0x2, SHURK0_SELPH_DQSG1_REG_DLY_DQS1_GATED) //cc change from 4->2
                    | P_Fld(0x4, SHURK0_SELPH_DQSG1_REG_DLY_DQS0_GATED_P1)//cc change from 6->4
                    | P_Fld(0x2, SHURK0_SELPH_DQSG1_REG_DLY_DQS0_GATED)); //cc change from 4->2

    vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_DQSG0, P_Fld(0x1, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS3_GATED_P1)
                    | P_Fld(0x1, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS3_GATED)
                    | P_Fld(0x1, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS2_GATED_P1)
                    | P_Fld(0x1, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS2_GATED)
                    | P_Fld(0x1, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS1_GATED_P1)
                    | P_Fld(0x1, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS1_GATED)
                    | P_Fld(0x1, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS0_GATED_P1)
                    | P_Fld(0x1, SHURK1_SELPH_DQSG0_TX_DLY_R1DQS0_GATED));

    vIO32WriteFldMulti(DRAMC_REG_SHURK1_SELPH_DQSG1, P_Fld(0x6, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS3_GATED_P1)
                    | P_Fld(0x4, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS3_GATED)
                    | P_Fld(0x6, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS2_GATED_P1)
                    | P_Fld(0x4, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS2_GATED)
                    | P_Fld(0x4, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS1_GATED_P1) //cc change. Same as RANK0
                    | P_Fld(0x2, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS1_GATED)//cc change. Same as RANK0
                    | P_Fld(0x4, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS0_GATED_P1)//cc change. Same as RANK0
                    | P_Fld(0x2, SHURK1_SELPH_DQSG1_REG_DLY_R1DQS0_GATED));//cc change. Same as RANK0
#endif

	//RX delay
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ2, P_Fld(0x0, SHU1_R0_B0_DQ2_RK0_RX_ARDQ1_F_DLY_B0) //cc change from 12->0
        				| P_Fld(0x0, SHU1_R0_B0_DQ2_RK0_RX_ARDQ1_R_DLY_B0)//cc change from 12->0
        				| P_Fld(0x0, SHU1_R0_B0_DQ2_RK0_RX_ARDQ0_F_DLY_B0)//cc change from 12->0
        				| P_Fld(0x0, SHU1_R0_B0_DQ2_RK0_RX_ARDQ0_R_DLY_B0));//cc change from 12->0
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ3, P_Fld(0x0, SHU1_R0_B0_DQ3_RK0_RX_ARDQ3_F_DLY_B0)//cc change from 12->0
        				| P_Fld(0x0, SHU1_R0_B0_DQ3_RK0_RX_ARDQ3_R_DLY_B0)//cc change from 12->0
        				| P_Fld(0x0, SHU1_R0_B0_DQ3_RK0_RX_ARDQ2_F_DLY_B0)//cc change from 12->0
        				| P_Fld(0x0, SHU1_R0_B0_DQ3_RK0_RX_ARDQ2_R_DLY_B0));//cc change from 12->0
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ4, P_Fld(0x0, SHU1_R0_B0_DQ4_RK0_RX_ARDQ5_F_DLY_B0)//cc change from 12->0
        				| P_Fld(0x0, SHU1_R0_B0_DQ4_RK0_RX_ARDQ5_R_DLY_B0)//cc change from 12->0
        				| P_Fld(0x0, SHU1_R0_B0_DQ4_RK0_RX_ARDQ4_F_DLY_B0)//cc change from 12->0
        				| P_Fld(0x0, SHU1_R0_B0_DQ4_RK0_RX_ARDQ4_R_DLY_B0));//cc change from 12->0
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ5, P_Fld(0x0, SHU1_R0_B0_DQ5_RK0_RX_ARDQ7_F_DLY_B0)//cc change from 12->0
        				| P_Fld(0x0, SHU1_R0_B0_DQ5_RK0_RX_ARDQ7_R_DLY_B0)//cc change from 12->0
        				| P_Fld(0x0, SHU1_R0_B0_DQ5_RK0_RX_ARDQ6_F_DLY_B0)//cc change from 12->0
        				| P_Fld(0x0, SHU1_R0_B0_DQ5_RK0_RX_ARDQ6_R_DLY_B0));//cc change from 12->0
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B0_DQ6, P_Fld(0x0, SHU1_R0_B0_DQ6_RK0_RX_ARDQS0_F_DLY_B0)
            			| P_Fld(0x0, SHU1_R0_B0_DQ6_RK0_RX_ARDQS0_R_DLY_B0)
            			| P_Fld(0x0, SHU1_R0_B0_DQ6_RK0_RX_ARDQM0_F_DLY_B0)
            			| P_Fld(0x0, SHU1_R0_B0_DQ6_RK0_RX_ARDQM0_R_DLY_B0));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B0_DQ2, P_Fld(0x0, SHU1_R1_B0_DQ2_RK1_RX_ARDQ1_F_DLY_B0)
        				| P_Fld(0x0, SHU1_R1_B0_DQ2_RK1_RX_ARDQ1_R_DLY_B0)
        				| P_Fld(0x0, SHU1_R1_B0_DQ2_RK1_RX_ARDQ0_F_DLY_B0)
        				| P_Fld(0x0, SHU1_R1_B0_DQ2_RK1_RX_ARDQ0_R_DLY_B0));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B0_DQ3, P_Fld(0x0, SHU1_R1_B0_DQ3_RK1_RX_ARDQ3_F_DLY_B0)
        				| P_Fld(0x0, SHU1_R1_B0_DQ3_RK1_RX_ARDQ3_R_DLY_B0)
        				| P_Fld(0x0, SHU1_R1_B0_DQ3_RK1_RX_ARDQ2_F_DLY_B0)
        				| P_Fld(0x0, SHU1_R1_B0_DQ3_RK1_RX_ARDQ2_R_DLY_B0));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B0_DQ4, P_Fld(0x0, SHU1_R1_B0_DQ4_RK1_RX_ARDQ5_F_DLY_B0)
        				| P_Fld(0x0, SHU1_R1_B0_DQ4_RK1_RX_ARDQ5_R_DLY_B0)
        				| P_Fld(0x0, SHU1_R1_B0_DQ4_RK1_RX_ARDQ4_F_DLY_B0)
        				| P_Fld(0x0, SHU1_R1_B0_DQ4_RK1_RX_ARDQ4_R_DLY_B0));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B0_DQ5, P_Fld(0x0, SHU1_R1_B0_DQ5_RK1_RX_ARDQ7_F_DLY_B0)
        				| P_Fld(0x0, SHU1_R1_B0_DQ5_RK1_RX_ARDQ7_R_DLY_B0)
        				| P_Fld(0x0, SHU1_R1_B0_DQ5_RK1_RX_ARDQ6_F_DLY_B0)
        				| P_Fld(0x0, SHU1_R1_B0_DQ5_RK1_RX_ARDQ6_R_DLY_B0));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B0_DQ6, P_Fld(0x0, SHU1_R1_B0_DQ6_RK1_RX_ARDQS0_F_DLY_B0)
        				| P_Fld(0x0, SHU1_R1_B0_DQ6_RK1_RX_ARDQS0_R_DLY_B0)
        				| P_Fld(0x0, SHU1_R1_B0_DQ6_RK1_RX_ARDQM0_F_DLY_B0)
        				| P_Fld(0x0, SHU1_R1_B0_DQ6_RK1_RX_ARDQM0_R_DLY_B0));
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ2, P_Fld(0x0, SHU1_R0_B1_DQ2_RK0_RX_ARDQ1_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ2_RK0_RX_ARDQ1_R_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ2_RK0_RX_ARDQ0_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ2_RK0_RX_ARDQ0_R_DLY_B1));
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ3, P_Fld(0x0, SHU1_R0_B1_DQ3_RK0_RX_ARDQ3_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ3_RK0_RX_ARDQ3_R_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ3_RK0_RX_ARDQ2_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ3_RK0_RX_ARDQ2_R_DLY_B1));
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ4, P_Fld(0x0, SHU1_R0_B1_DQ4_RK0_RX_ARDQ5_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ4_RK0_RX_ARDQ5_R_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ4_RK0_RX_ARDQ4_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ4_RK0_RX_ARDQ4_R_DLY_B1));
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ5, P_Fld(0x0, SHU1_R0_B1_DQ5_RK0_RX_ARDQ7_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ5_RK0_RX_ARDQ7_R_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ5_RK0_RX_ARDQ6_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ5_RK0_RX_ARDQ6_R_DLY_B1));
	vIO32WriteFldMulti(DDRPHY_SHU1_R0_B1_DQ6, P_Fld(0x0, SHU1_R0_B1_DQ6_RK0_RX_ARDQS0_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ6_RK0_RX_ARDQS0_R_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ6_RK0_RX_ARDQM0_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R0_B1_DQ6_RK0_RX_ARDQM0_R_DLY_B1));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B1_DQ2, P_Fld(0x0, SHU1_R1_B1_DQ2_RK1_RX_ARDQ1_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R1_B1_DQ2_RK1_RX_ARDQ1_R_DLY_B1)
        				| P_Fld(0x0, SHU1_R1_B1_DQ2_RK1_RX_ARDQ0_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R1_B1_DQ2_RK1_RX_ARDQ0_R_DLY_B1));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B1_DQ3, P_Fld(0x0, SHU1_R1_B1_DQ3_RK1_RX_ARDQ3_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R1_B1_DQ3_RK1_RX_ARDQ3_R_DLY_B1)
        				| P_Fld(0x0, SHU1_R1_B1_DQ3_RK1_RX_ARDQ2_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R1_B1_DQ3_RK1_RX_ARDQ2_R_DLY_B1));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B1_DQ4, P_Fld(0x0, SHU1_R1_B1_DQ4_RK1_RX_ARDQ5_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R1_B1_DQ4_RK1_RX_ARDQ5_R_DLY_B1)
        				| P_Fld(0x0, SHU1_R1_B1_DQ4_RK1_RX_ARDQ4_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R1_B1_DQ4_RK1_RX_ARDQ4_R_DLY_B1));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B1_DQ5, P_Fld(0x0, SHU1_R1_B1_DQ5_RK1_RX_ARDQ7_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R1_B1_DQ5_RK1_RX_ARDQ7_R_DLY_B1)
        				| P_Fld(0x0, SHU1_R1_B1_DQ5_RK1_RX_ARDQ6_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R1_B1_DQ5_RK1_RX_ARDQ6_R_DLY_B1));
	vIO32WriteFldMulti(DDRPHY_SHU1_R1_B1_DQ6, P_Fld(0x0, SHU1_R1_B1_DQ6_RK1_RX_ARDQS0_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R1_B1_DQ6_RK1_RX_ARDQS0_R_DLY_B1)
        				| P_Fld(0x0, SHU1_R1_B1_DQ6_RK1_RX_ARDQM0_F_DLY_B1)
        				| P_Fld(0x0, SHU1_R1_B1_DQ6_RK1_RX_ARDQM0_R_DLY_B1));

    //Rank0 and Rank1
    vIO32WriteFldMulti(DRAMC_REG_SHU_RANKCTL, P_Fld(0x2, SHU_RANKCTL_RANKINCTL_PHY)
    				| P_Fld(0x1, SHU_RANKCTL_DMSTBLAT));

    vIO32WriteFldAlign(DRAMC_REG_SHURK0_DQSCTL, 0x2, SHURK0_DQSCTL_DQSINCTL);
    vIO32WriteFldAlign(DRAMC_REG_SHURK1_DQSCTL, 0x2, SHURK1_DQSCTL_R1DQSINCTL);

	return DRAM_OK;
}


static DRAM_STATUS_T DramcSetting_ACTiming_PC3(DRAMC_CTX_T *p)
{
	const U16 *ACTable;
	U8 i;
	
	mcSHOW_DBG_MSG2(("[Dramc] PCDDR3 AC Timing update \n"));
	#if 1//dbg_print
	prom_puts("[Dramc] PCDDR3 AC Timing update \n");
	#endif
	i = 0;

	if (p->frequency < DDR_DDR1600) {
		/* cc: For Lower than 1600 speed, use 1600 ACTiming setting */
		ACTable = ACTimingTable[0];
		mcSHOW_DBG_MSG(("Speed < 1600, use 1600 timing\n", i));
	}else {
	for (i = 0; i < TOTAL_AC_TIMING_NUMBER; i++) {
		if ((ACTimingTable[i][AC_TIMING_DRAM_TYPE] == p->dram_type) &&
			(ACTimingTable[i][AC_TIMING_FREQUENCY] == p->frequency)) {			
			mcSHOW_DBG_MSG(("ACTiming match item %d\n", i));
			ACTable = ACTimingTable[i];
			break;
		}
	}
	}

	if (i == TOTAL_AC_TIMING_NUMBER) {
		mcSHOW_DBG_MSG(("Error: No match ACTiming found\n"));
		#if 1//dbg_print
		prom_puts("Error: No match ACTiming found\n");
		#endif
		return DRAM_FAIL;
	}
	
	vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM0, P_Fld(ACTable[AC_TIMING_TRCD], SHU_ACTIM0_TRCD)
                    | P_Fld(ACTable[AC_TIMING_TRRD], SHU_ACTIM0_TRRD)
                    | P_Fld(ACTable[AC_TIMING_TWR], SHU_ACTIM0_TWR)
                    | P_Fld(ACTable[AC_TIMING_TWTR], SHU_ACTIM0_TWTR));

	vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM1, P_Fld(ACTable[AC_TIMING_TRC], SHU_ACTIM1_TRC)
                    | P_Fld(ACTable[AC_TIMING_TRAS], SHU_ACTIM1_TRAS)
                    | P_Fld(ACTable[AC_TIMING_TRP], SHU_ACTIM1_TRP)
                    | P_Fld(ACTable[AC_TIMING_TRPAB], SHU_ACTIM1_TRPAB));

	vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM2, P_Fld(ACTable[AC_TIMING_TFAW], SHU_ACTIM2_TFAW)
                    | P_Fld(ACTable[AC_TIMING_TRTW_ODT_OFF + p->odt_onoff], SHU_ACTIM2_TR2W)
                    | P_Fld(ACTable[AC_TIMING_TRTP], SHU_ACTIM2_TRTP)
                    | P_Fld(ACTable[AC_TIMING_TXP], SHU_ACTIM2_TXP));

	#if 0
	vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM3, P_Fld(0x6D, SHU_ACTIM3_TRFC)
					| P_Fld(0x18, SHU_ACTIM3_TRFCPB));
	#else
	vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM3, P_Fld(ACTable[AC_TIMING_TRFC], SHU_ACTIM3_TRFC)
					| P_Fld(ACTable[AC_TIMING_TRFCPB], SHU_ACTIM3_TRFCPB));
	#endif
	if(p->clk_src == PLL_Fin_25M)
	{
	    vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM4, P_Fld(ACTable[AC_TIMING_TZQCS], SHU_ACTIM4_TZQCS)
	        			| P_Fld(ACTable[AC_TIMING_REFCNT_FR_CLK], SHU_ACTIM4_REFCNT_FR_CLK)
	        			| P_Fld(ACTable[AC_TIMING_TXREFCNT], SHU_ACTIM4_TXREFCNT));
	}
	else
	{
		vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM4, P_Fld(ACTable[AC_TIMING_TZQCS], SHU_ACTIM4_TZQCS)
	        			| P_Fld(ACTable[AC_TIMING_REFCNT_FR_CLK_20M], SHU_ACTIM4_REFCNT_FR_CLK)
	        			| P_Fld(ACTable[AC_TIMING_TXREFCNT], SHU_ACTIM4_TXREFCNT));
	}

	vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM5, P_Fld(ACTable[AC_TIMING_TMRR2W_ODT_OFF + p->odt_onoff], SHU_ACTIM5_TMRR2W)
                    | P_Fld(ACTable[AC_TIMING_TWTPD], SHU_ACTIM5_TWTPD)
                    | P_Fld(ACTable[AC_TIMING_TRTPD], SHU_ACTIM5_TR2PD));

    vIO32WriteFldMulti(DRAMC_REG_SHU_ACTIM_XRT, P_Fld(ACTable[AC_TIMING_XRTW2W], SHU_ACTIM_XRT_XRTW2W)
                    | P_Fld(ACTable[AC_TIMING_XRTR2W], SHU_ACTIM_XRT_XRTR2W)
					| P_Fld(ACTable[AC_TIMING_XRTW2R], SHU_ACTIM_XRT_XRTW2R)
                    | P_Fld(ACTable[AC_TIMING_XRTR2R], SHU_ACTIM_XRT_XRTR2R));

	vIO32WriteFldMulti(DRAMC_REG_SHU_AC_TIME_05T, P_Fld(ACTable[AC_TIMING_TWTR_05T], SHU_AC_TIME_05T_TWTR_M05T)
                    | P_Fld(ACTable[AC_TIMING_TRP_05T], SHU_AC_TIME_05T_TRP_05T)
                    | P_Fld(ACTable[AC_TIMING_TRCD_05T], SHU_AC_TIME_05T_TRCD_05T)
                    | P_Fld(ACTable[AC_TIMING_TXP_05T], SHU_AC_TIME_05T_TXP_05T)
                    | P_Fld(ACTable[AC_TIMING_TRC_05T], SHU_AC_TIME_05T_TRC_05T));
	
	vIO32WriteFldAlign(DRAMC_REG_SHURK0_DQSCTL, ACTable[AC_TIMING_DQSINCTL_FOR_GATING], SHURK0_DQSCTL_DQSINCTL);
	vIO32WriteFldAlign(DRAMC_REG_SHURK1_DQSCTL, ACTable[AC_TIMING_DQSINCTL_FOR_GATING], SHURK1_DQSCTL_R1DQSINCTL);

	return DRAM_OK;
}

static DRAM_STATUS_T DramcSetting_EN7523_PC3(DRAMC_CTX_T *p)
{
	mcSHOW_DBG_MSG(("[DRAMCInit]========== DramcSetting_EN7523_PC3 =============\n"));
	
    vIO32WriteFldMulti(DRAMC_REG_REFCTRL0, P_Fld(0x1, REFCTRL0_REFFRERUN)
            		| P_Fld(0x1, REFCTRL0_REFDIS)
            		| P_Fld(0x5, REFCTRL0_REF_PREGATE_CNT)
            		| P_Fld(0x7, REFCTRL0_ADVREF_CNT)
            		| P_Fld(0x1, REFCTRL0_PBREF_DISBYREFNUM)
            		| P_Fld(0x4, REFCTRL0_DISBYREFNUM)
            		| P_Fld(0x1, REFCTRL0_DLLFRZ));

    vIO32WriteFldMulti(DDRPHY_MISC_CTRL3, P_Fld(0x0, MISC_CTRL3_ARPI_CG_CLK_OPT));
    //vIO32WriteFldMulti(DDRPHY_MISC_CTRL3+(1<<POS_BANK_NUM), P_Fld(0x0, MISC_CTRL3_ARPI_CG_CLK_OPT));

    vIO32WriteFldMulti(DRAMC_REG_SHU_DQSG_RETRY, P_Fld(0x1, SHU_DQSG_RETRY_R_RDY_SEL_DLE)
    		        | P_Fld(0x6, SHU_DQSG_RETRY_R_DQSIENLAT));

    vIO32WriteFldMulti(DRAMC_REG_SHUCTRL2, P_Fld(0x1, SHUCTRL2_SHU_PERIOD_GO_ZERO_CNT)
            		| P_Fld(0x1, SHUCTRL2_R_DVFS_OPTION)
            		| P_Fld(0x1, SHUCTRL2_R_DVFS_PARK_N)
            		| P_Fld(0x1, SHUCTRL2_R_DVFS_DLL_CHA)
            		| P_Fld(0x7, SHUCTRL2_R_DLL_IDLE));

    vIO32WriteFldMulti(DRAMC_REG_DVFSDLL, P_Fld(0xe, DVFSDLL_DLL_IDLE_SHU4)
            		| P_Fld(0x9, DVFSDLL_DLL_IDLE_SHU3)
            		| P_Fld(0x6, DVFSDLL_DLL_IDLE_SHU2)
            		| P_Fld(0x1, DVFSDLL_DLL_LOCK_SHU_EN));

    if (p->data_width == DATA_WIDTH_16BIT)
    {
	    vIO32WriteFldMulti(DRAMC_REG_DDRCONF0, P_Fld(0x1, DDRCONF0_DDR3EN)
	            		| P_Fld(0x1, DDRCONF0_BC4OTF_OPT)
	            		| P_Fld(0x1, DDRCONF0_BK8EN));
    }
    else
    {
	    vIO32WriteFldMulti(DRAMC_REG_DDRCONF0, P_Fld(0x1, DDRCONF0_DDR3EN)
	            		| P_Fld(0x1, DDRCONF0_DM64BITEN)
	            		| P_Fld(0x1, DDRCONF0_BC4OTF_OPT)
	            		| P_Fld(0x1, DDRCONF0_BK8EN));
    }

    vIO32WriteFldMulti(DDRPHY_MISC_CTRL0, P_Fld(0x1, MISC_CTRL0_R_DMDQSIEN_FIFO_EN)
            		| P_Fld(0x1, MISC_CTRL0_R_DMDQSIEN_DEPTH_HALF) //cc change from 0->1
            		| P_Fld(0x1, MISC_CTRL0_R_DMSTBEN_OUTSEL)
            		| P_Fld(0xf, MISC_CTRL0_R_DMDQSIEN_SYNCOPT));

    #if 0
    vIO32WriteFldMulti(DDRPHY_MISC_CTRL0+(1<<POS_BANK_NUM), P_Fld(0x1, MISC_CTRL0_R_DMDQSIEN_FIFO_EN)
            		| P_Fld(0x1, MISC_CTRL0_R_DMDQSIEN_DEPTH_HALF) //cc change from 0->1
            		| P_Fld(0x1, MISC_CTRL0_R_DMSTBEN_OUTSEL)
            		| P_Fld(0xf, MISC_CTRL0_R_DMDQSIEN_SYNCOPT));
    #endif
    
    vIO32WriteFldMulti(DRAMC_REG_SHU_CONF0, P_Fld(0x2, SHU_CONF0_MATYPE)
            		| P_Fld(0x1, SHU_CONF0_BL4)
            		| P_Fld(0x1, SHU_CONF0_FDIV2)
            		| P_Fld(0x1, SHU_CONF0_REFTHD)
            		| P_Fld(0x1, SHU_CONF0_ADVPREEN)
            		| P_Fld(0x3f, SHU_CONF0_DMPGTIM));

    vIO32WriteFldMulti(DRAMC_REG_SHU_ODTCTRL, P_Fld(0x3, SHU_ODTCTRL_TWODT)
            		| P_Fld(0x5, SHU_ODTCTRL_RODT)
            		| P_Fld(0x0, SHU_ODTCTRL_WOEN)
            		| P_Fld(0x1, SHU_ODTCTRL_RODTE) /* cc add */
            		| P_Fld(0x1, SHU_ODTCTRL_RODTE2) /* cc add */
            		| P_Fld(p->odt_onoff, SHU_ODTCTRL_ROEN)); /* cc change from 0->1 to enable Read ODT */

    vIO32WriteFldMulti(DRAMC_REG_REFCTRL0, P_Fld(0x0, REFCTRL0_REFFRERUN)
            		| P_Fld(0x2, REFCTRL0_DISBYREFNUM)
            		| P_Fld(0x0, REFCTRL0_DLLFRZ));

    vIO32WriteFldMulti(DRAMC_REG_STBCAL, P_Fld(0x1, STBCAL_DQSIENMODE)
            	    | P_Fld(0x0, STBCAL_DQSIENMODE_SELPH));

    vIO32WriteFldMulti(DRAMC_REG_SREFCTRL, P_Fld(0x0, SREFCTRL_SREF_HW_EN)
                	| P_Fld(0x8, SREFCTRL_SREFDLY)
                	| P_Fld(0x0, SREFCTRL_SREF2_OPTION));

    vIO32WriteFldMulti(DRAMC_REG_SHU_PIPE, P_Fld(0x1, SHU_PIPE_READ_START_EXTEND1)
            	    | P_Fld(0x1, SHU_PIPE_DLE_LAST_EXTEND1));

    vIO32WriteFldAlign(DRAMC_REG_RKCFG, 0x1, RKCFG_CKE2RANK_OPT2);

    vIO32WriteFldMulti(DRAMC_REG_SHU_CONF2, P_Fld(0x7, SHU_CONF2_DCMDLYREF)
        			| P_Fld(0x32, SHU_CONF2_FSPCHG_PRDCNT)
        			| P_Fld(0xf, SHU_CONF2_TCMDO1LAT));

    vIO32WriteFldMulti(DRAMC_REG_SHU_SCINTV, P_Fld(0x0, SHU_SCINTV_DQS2DQ_SHU_PITHRD)
        			| P_Fld(0x15, SHU_SCINTV_MRW_INTV)
        			| P_Fld(0x0, SHU_SCINTV_RDDQC_INTV)
        			| P_Fld(0x0, SHU_SCINTV_TZQLAT));

    vIO32WriteFldAlign(DRAMC_REG_SHUCTRL, 0x0, SHUCTRL_R_SHUFFLE_BLOCK_OPT);

    vIO32WriteFldAlign(DRAMC_REG_REFCTRL1, 0x0, REFCTRL1_SREF_PRD_OPT);

    vIO32WriteFldAlign(DRAMC_REG_REFRATRE_FILTER, 0x1, REFRATRE_FILTER_PB2AB_OPT);
	//----------------------------------------------------------------------------
	DramcSetting_Default_PC3(p);

	DramcSetting_ACTiming_PC3(p);

	//----------------------------------------------------------------------------
    vIO32WriteFldMulti(DDRPHY_PLL2, P_Fld(0x1, PLL2_GDDR3RST));
	mcDELAY_US(500);//Reset ON

    vIO32WriteFldMulti(DRAMC_REG_MISCTL0, P_Fld(0x1, MISCTL0_REFP_ARB_EN2)
    				| P_Fld(0x1, MISCTL0_PBC_ARB_EN)
    				| P_Fld(0x1, MISCTL0_REFA_ARB_EN2));
    vIO32WriteFldMulti(DRAMC_REG_PERFCTL0, P_Fld(0x1, PERFCTL0_MWHPRIEN)
    				| P_Fld(0x0, PERFCTL0_RWSPLIT) //cc change from 1->0
    				| P_Fld(0x1, PERFCTL0_WFLUSHEN)
    				| P_Fld(0x1, PERFCTL0_EMILLATEN)
    				| P_Fld(0x1, PERFCTL0_RWAGEEN)
    				| P_Fld(0x1, PERFCTL0_RWLLATEN)
    				| P_Fld(0x1, PERFCTL0_RWHPRIEN)
    				| P_Fld(0x0, PERFCTL0_RWOFOEN) //cc change from 1->0
    				| P_Fld(0x1, PERFCTL0_DISRDPHASE1)
    				| P_Fld(0x1, PERFCTL0_DUALSCHEN));
    vIO32WriteFldAlign(DRAMC_REG_PERFCTL0, 0x0, PERFCTL0_DUALSCHEN);

    vIO32WriteFldAlign(DRAMC_REG_ARBCTL, 0x80, ARBCTL_MAXPENDCNT);

    vIO32WriteFldMulti(DRAMC_REG_PADCTRL, P_Fld(0x1, PADCTRL_DQIENLATEBEGIN)
    				| P_Fld(0x1, PADCTRL_DQIENQKEND));

    vIO32WriteFldMulti(DRAMC_REG_DRAMC_PD_CTRL, P_Fld(0x1, DRAMC_PD_CTRL_DCMREF_OPT)
    				| P_Fld(0x1, DRAMC_PD_CTRL_DCMEN));

    vIO32WriteFldAlign(DRAMC_REG_CLKCTRL, 0x1, CLKCTRL_CLK_EN_1);

    vIO32WriteFldMulti(DRAMC_REG_REFCTRL0, P_Fld(0x5, REFCTRL0_REF_PREGATE_CNT)
    				| P_Fld(0x4, REFCTRL0_DISBYREFNUM)
    				| P_Fld(0x1, REFCTRL0_DLLFRZ));
	vIO32WriteFldAlign(DRAMC_REG_DDRCONF0, 0x1, DDRCONF0_GDDR3RST);

    vIO32WriteFldMulti(DRAMC_REG_SPCMDCTRL, P_Fld(0x1, SPCMDCTRL_CLR_EN)
		| P_Fld(0x0, SPCMDCTRL_R_DMDVFSMRW_EN)); //cc add

    vIO32WriteFldMulti(DRAMC_REG_CATRAINING1, P_Fld(0x4, CATRAINING1_CATRAIN_INTV)
    				| P_Fld(0x3, CATRAINING1_CATRAINLAT));

    vIO32WriteFldMulti(DRAMC_REG_STBCAL, P_Fld(0x1, STBCAL_DQSIENMODE)
    				| P_Fld(0x0, STBCAL_DQSIENMODE_SELPH)
    				| P_Fld(0x1, STBCAL_REFUICHG));

    vIO32WriteFldMulti(DRAMC_REG_RKCFG, P_Fld(0x1, RKCFG_MRS2RK)
                    | P_Fld(0x1, RKCFG_RKMODE)
                    | P_Fld(0x1, RKCFG_CKE2RANK_OPT2));
     vIO32WriteFldAlign(DRAMC_REG_RKCFG, 1, RKCFG_CS2RANK);
	 mcDELAY_US(500);	//YMC chg to follow spec.
     vIO32WriteFldMulti(DRAMC_REG_CKECTRL, P_Fld(0x1, CKECTRL_CKEON)
                    | P_Fld(0x1, CKECTRL_CKETIMER_SEL)
                    | P_Fld(0x1, CKECTRL_CKEFIXON)
                    | P_Fld(0x1, CKECTRL_CKE1FIXON));
    mcDELAY_US(100);//CKE ON

	switch (p->frequency) {
	case DDR_DDR1600:		
		DramcModeRegWrite_PC3 (p, 0x2, 0x18); //cc notes: CWL = 8
		mcDELAY_US(100);
		
		//MR3
		DramcModeRegWrite_PC3 (p, 0x3, 0x0);
		mcDELAY_US(100);
		
		//MR1
		DramcModeRegWrite_PC3 (p, 0x1, DDR_PC3_MR1); //cc change: RTT_NOM = RZQ/4 = 60ohm
		mcDELAY_US(100);
		
		DramcModeRegWrite_PC3 (p, 0x0, 0x1d70); //cc notes: CL = 11
		mcDELAY_US(100);
		break;

	case DDR_DDR1866:
		DramcModeRegWrite_PC3(p, 0x2, 0x20); //cc notes: CWL = 9
		mcDELAY_US(100);
		
		//MR3
		DramcModeRegWrite_PC3 (p, 0x3, 0x0);
		mcDELAY_US(100);
		
		//MR1
		DramcModeRegWrite_PC3 (p, 0x1, DDR_PC3_MR1); //cc change: RTT_NOM = RZQ/4 = 60ohm
		mcDELAY_US(100);

		DramcModeRegWrite_PC3(p, 0x0, 0x1114); //cc notes: CL = 13	//YMC chg CL
		mcDELAY_US(100);
		break;
		 
	case DDR_DDR1333:
		DramcModeRegWrite_PC3(p, 0x2, 0x10); //cc notes: CWL = 7
		mcDELAY_US(100);
		
		//MR3
		DramcModeRegWrite_PC3 (p, 0x3, 0x0);
		mcDELAY_US(100);
		
		//MR1
		DramcModeRegWrite_PC3 (p, 0x1, DDR_PC3_MR1); //cc change: RTT_NOM = RZQ/4 = 60ohm
		mcDELAY_US(100);
		
		DramcModeRegWrite_PC3(p, 0x0, 0x1b50); //cc notes: CL = 9	//YMC chg WR
		mcDELAY_US(100);
		break;	
	default:
	case DDR_DDR1200:
		DramcModeRegWrite_PC3(p, 0x2, 0x10); //cc notes: CWL = 7
		mcDELAY_US(100);
		
		//MR3
		DramcModeRegWrite_PC3 (p, 0x3, 0x0);
		mcDELAY_US(100);
		
		//MR1
		DramcModeRegWrite_PC3 (p, 0x1, DDR_PC3_MR1); //cc change: RTT_NOM = RZQ/4 = 60ohm
		mcDELAY_US(100);

		DramcModeRegWrite_PC3(p, 0x0, 0x1d60); //cc notes: CL = 10
		mcDELAY_US(100);
		break;
	}
#if 0
	//------------------------------------------------------
	//Mode register init
	//MR2
	if (p->frequency <= DDR_DDR1600)
		DramcModeRegWrite_PC3 (p, 0x2, 0x18); //cc notes: CWL = 8
	else if (p->frequency == DDR_DDR1866)
		DramcModeRegWrite_PC3(p, 0x2, 0x20); //cc notes: CWL = 9
	mcDELAY_US(100);

	//MR3
	DramcModeRegWrite_PC3 (p, 0x3, 0x0);
	mcDELAY_US(100);

	//MR1
	DramcModeRegWrite_PC3 (p, 0x1, DDR_PC3_MR1); //cc change: RTT_NOM = RZQ/4 = 60ohm
	mcDELAY_US(100);

	//MR0
	if (p->frequency <= DDR_DDR1600)
		DramcModeRegWrite_PC3 (p, 0x0, 0x1d70);
	else if (p->frequency == DDR_DDR1866)
		DramcModeRegWrite_PC3(p, 0x0, 0x1170);
	mcDELAY_US(100);
	//------------------------------------------------------
#endif
	
	//cc add: after ModeReg init, set CS0/CS1 as independent.
	vIO32WriteFldMulti(DRAMC_REG_RKCFG, P_Fld(0x0, RKCFG_CS2RANK)
		| P_Fld(0x0, RKCFG_MRS2RK));
	//cc add end

    vIO32WriteFldMulti(DRAMC_REG_DRAMCTRL, P_Fld(0x7, DRAMCTRL_TCMD));

	vIO32WriteFldAlign(DRAMC_REG_MRS, 0x400, MRS_MRSMA); //cc add to do ZQCL. Original code only does ZQCS
    vIO32WriteFldAlign(DRAMC_REG_SPCMD, 1, SPCMD_ZQCEN);
	mcDELAY_US(100);
    vIO32WriteFldAlign(DRAMC_REG_SPCMD, 0, SPCMD_ZQCEN);

    vIO32WriteFldAlign(DRAMC_REG_SPCMD, 1, SPCMD_TCMDEN);

    vIO32WriteFldAlign(DRAMC_REG_SPCMD, 0, SPCMD_TCMDEN);

    vIO32WriteFldAlign(DRAMC_REG_HW_MRR_FUN, 0x1, HW_MRR_FUN_TMRR_ENA);

    vIO32WriteFldAlign(DRAMC_REG_DRAMCTRL, 0x1, DRAMCTRL_WDATRGO);

    vIO32WriteFldAlign(DRAMC_REG_DRAMCTRL, 0x1, DRAMCTRL_CLKWITRFC);
	
    vIO32WriteFldAlign(DRAMC_REG_SHU_ODTCTRL, p->odt_onoff, SHU_ODTCTRL_WOEN); /* cc add to enable Write ODT */

    vIO32WriteFldAlign(DRAMC_REG_ZQCS, 0x56, ZQCS_ZQCSOP);

    vIO32WriteFldAlign(DRAMC_REG_SHU_CONF3, 0xff, SHU_CONF3_REFRCNT);

    vIO32WriteFldMulti(DRAMC_REG_SPCMDCTRL, P_Fld(0x0, SPCMDCTRL_REFRDIS)
    			    | P_Fld(0x1, SPCMDCTRL_CLR_EN));

    vIO32WriteFldMulti(DRAMC_REG_SHU_CONF1, P_Fld(0xb0, SHU_CONF1_REFBW_FR));

    vIO32WriteFldAlign(DRAMC_REG_REFCTRL0, 0x1, REFCTRL0_REFFRERUN);

    vIO32WriteFldAlign(DRAMC_REG_SREFCTRL, 0x1, SREFCTRL_SREF_HW_EN);

    vIO32WriteFldMulti(DRAMC_REG_DRAMC_PD_CTRL, P_Fld(0x1, DRAMC_PD_CTRL_PHYCLKDYNGEN)
        			| P_Fld(0x0, DRAMC_PD_CTRL_MIOCKCTRLOFF)
        			| P_Fld(0x1, DRAMC_PD_CTRL_DCMEN));

    vIO32WriteFldMulti(DRAMC_REG_EYESCAN, P_Fld(0x1, EYESCAN_RX_DQ_EYE_SEL)
    			    | P_Fld(0x1, EYESCAN_RG_EX_EYE_SCAN_EN));

    vIO32WriteFldMulti(DRAMC_REG_STBCAL1, P_Fld(0x1, STBCAL1_STBCNT_LATCH_EN)
    		    | P_Fld(0x1, STBCAL1_STBENCMPEN));

    vIO32WriteFldAlign(DRAMC_REG_TEST2_1, 0x10000, TEST2_1_TEST2_BASE);

    vIO32WriteFldAlign(DRAMC_REG_TEST2_2, 0x400, TEST2_2_TEST2_OFF);

    vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0x1, TEST2_3_TEST2WREN2_HW_EN)
        		| P_Fld(0x4, TEST2_3_DQSICALSTP)
        		| P_Fld(0x1, TEST2_3_TESTAUDPAT));

    vIO32WriteFldAlign(DRAMC_REG_SHUCTRL2, 0x1, SHUCTRL2_MR13_SHU_EN);

    vIO32WriteFldMulti(DRAMC_REG_DRAMCTRL, P_Fld(0x1, DRAMCTRL_REQQUE_THD_EN)
        		| P_Fld(0x1, DRAMCTRL_PREALL_OPTION)
        		| P_Fld(0x1, DRAMCTRL_DPDRK_OPT));

    vIO32WriteFldAlign(DRAMC_REG_SHU_CKECTRL, 0x3, SHU_CKECTRL_SREF_CK_DLY);

    vIO32WriteFldAlign(DRAMC_REG_DUMMY_RD, 0x2, DUMMY_RD_RANK_NUM);

    vIO32WriteFldAlign(DRAMC_REG_TEST2_4, 0x4, TEST2_4_TESTAGENTRKSEL);

    vIO32WriteFldMulti(DRAMC_REG_REFCTRL1, P_Fld(0x1, REFCTRL1_REF_QUE_AUTOSAVE_EN)
    		    | P_Fld(0x1, REFCTRL1_SLEFREF_AUTOSAVE_EN));

    vIO32WriteFldMulti(DRAMC_REG_RSTMASK, P_Fld(0x0, RSTMASK_PHY_SYNC_MASK)
        		| P_Fld(0x0, RSTMASK_DAT_SYNC_MASK)
        		| P_Fld(0x0, RSTMASK_GT_SYNC_MASK)
        		| P_Fld(0x0, RSTMASK_DVFS_SYNC_MASK)
        		| P_Fld(0x0, RSTMASK_GT_SYNC_MASK_FOR_PHY)
        		| P_Fld(0x0, RSTMASK_DVFS_SYNC_MASK_FOR_PHY));

    vIO32WriteFldAlign(DRAMC_REG_DRAMCTRL, 0x0, DRAMCTRL_CTOREQ_HPRI_OPT);

#if DRAM_8BIT_X2
    vIO32WriteFldAlign(DDRPHY_PLL1, 0x1, PLL1_DDR3_2X8_OPEN);
#endif

    vIO32WriteFldMulti(DRAMC_REG_CKECTRL, P_Fld(0x0, CKECTRL_CKEFIXON)
    		    | P_Fld(0x0, CKECTRL_CKE1FIXON));

    vIO32WriteFldAlign(DRAMC_REG_REFCTRL0, 0x0, REFCTRL0_REFDIS);

    return DRAM_OK;
}
#endif //COMPILE_PCDDR3

#define DUMP_REGISTERS		0

#if DUMP_REGISTERS
//-------------------------------------------------------------------------
/** DramcRegDump
 *  Dump all registers (DDRPHY and DRAMC)
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 */
//-------------------------------------------------------------------------
void DramcRegDumpRange(U32 u4StartAddr, U32 u4EndAddr)
{
    U32 ii;

    for(ii=u4StartAddr; ii<u4EndAddr; ii+=4)
    {
        mcSHOW_DBG_MSG(("Reg(0x%xh) Address 0x%X = 0x%X\n", (ii&0xfff)>>2, ii, (*(volatile unsigned int *)(ii))));
		//print("Reg(0x%xh) Address 0x%X = 0x%X\n", (ii&0xfff)>>2, ii, (*(volatile unsigned int *)(ii)));
    }
}

DRAM_STATUS_T DramcRegDump(DRAMC_CTX_T *p)
{
    mcSHOW_DBG_MSG(("\n=================DRAMC_NAO_BASE================================\n"));
    DramcRegDumpRange(DRAMC_NAO_BASE_ADDRESS, DRAMC_NAO_BASE_ADDRESS+0x1000);
    mcSHOW_DBG_MSG(("\n=================DRAMC_AO_BASE================================\n"));
    DramcRegDumpRange(DRAMC_AO_BASE_ADDRESS, DRAMC_AO_BASE_ADDRESS+0x1000);

    mcSHOW_DBG_MSG(("\n=================PHY_A_BASE================================\n"));
    DramcRegDumpRange(DDRPHY_B01_BASE_ADDRESS, DDRPHY_B01_BASE_ADDRESS+0x1000);
    mcSHOW_DBG_MSG(("\n=================PHY_B_BASE================================\n"));
    DramcRegDumpRange(DDRPHY_CMD_BASE_ADDRESS, DDRPHY_CMD_BASE_ADDRESS+0x1000);

    return DRAM_OK;
}

void Dump_Debug_Registers(DRAMC_CTX_T *p, int index)
{
	mcSHOW_DBG_MSG(("Dump registers. Stage.%d\n", index));
	DramcRegDump(p);
}
#endif //DUMP_REGISTER

DRAM_STATUS_T DramcInit(DRAMC_CTX_T *p)
{
    mcSHOW_DBG_MSG2(("[DramcInit] ====Begin====\n"));
    DDRPHYSetting_EN7523(p);

	if (p->dram_type == TYPE_PCDDR3)
    {    
	#if COMPILE_PCDDR3
        DramcSetting_EN7523_PC3(p);
	#endif
    }
	else if (p->dram_type == TYPE_PCDDR2)
    {
    #if COMPILE_PCDDR2
	    mcSHOW_DBG_MSG(("PCDDR2 type with %dMHz.\n", p->frequency));
        DramcSetting_EN7523_PC2(p);
	#endif
    }
    else
    {
        mcSHOW_DBG_MSG(("Not support this DDR type.\n"));
		#if 1//dbg_print
		prom_puts("Not support this DDR type.\n");
		#endif
		return DRAM_FAIL;
    }

    mcSHOW_DBG_MSG2(("[DramcInit] ====Done====\n"));

    return DRAM_OK;
}

#if COMPILE_THIS_PART
void DramcEnterSelfRefresh(DRAMC_CTX_T *p, U8 op)
{
    U8 ucstatus = 0;
    U32 uiTemp;
    U32 u4TimeCnt;

    u4TimeCnt = TIME_OUT_CNT;

    mcSHOW_DBG_MSG(("[DramcEnterSelfRefresh]  op:%d (0:exit, 1:enter)\n", op));

    if (op == 1) // enter self refresh
    {
        vIO32WriteFldAlign(DRAMC_REG_SREFCTRL, 1, SREFCTRL_SELFREF);
        mcDELAY_US(2);
        uiTemp = u4IO32ReadFldAlign(DRAMC_REG_MISC_STATUSA, MISC_STATUSA_SREF_STATE);
        while((uiTemp==0) &&(u4TimeCnt>0))
        {
            mcSHOW_DBG_MSG2(("Still not enter self refresh(%d)\n",u4TimeCnt));
            mcDELAY_US(1);
            uiTemp = u4IO32ReadFldAlign(DRAMC_REG_MISC_STATUSA, MISC_STATUSA_SREF_STATE);
            u4TimeCnt --;
        }
    }
    else // exit self refresh
    {
        vIO32WriteFldAlign(DRAMC_REG_SREFCTRL, 0, SREFCTRL_SELFREF);

        mcDELAY_US(2);
        uiTemp = u4IO32ReadFldAlign(DRAMC_REG_MISC_STATUSA, MISC_STATUSA_SREF_STATE);
        while ((uiTemp!=0) &&(u4TimeCnt>0))
        {
            mcSHOW_DBG_MSG2(("Still not exit self refresh(%d)\n", u4TimeCnt));
            mcDELAY_US(1);
            uiTemp = u4IO32ReadFldAlign(DRAMC_REG_MISC_STATUSA, MISC_STATUSA_SREF_STATE);
            u4TimeCnt--;
        }
    }

   if(u4TimeCnt ==0)
   {
        mcSHOW_DBG_MSG(("[DramcEnterSelfRefresh]  Self Rresh operation fail\n"));
   }
   else
   {
        mcSHOW_DBG_MSG(("[DramcEnterSelfRefresh]  Self Rresh operation done\n"));
   }
}
#endif


#ifdef DUMMY_READ_FOR_TRACKING
void DramcDummyReadForTrackingEnable(DRAMC_CTX_T *p)
{
    vIO32WriteFldAlign(DRAMC_REG_DUMMY_RD, p->support_rank_num, DUMMY_RD_RANK_NUM);

    vIO32WriteFldAlign(DRAMC_REG_TEST2_4, 4, TEST2_4_TESTAGENTRKSEL);//Dummy Read rank selection is controlled by Test Agent
    vIO32WriteFldAlign(DRAMC_REG_RK1_DUMMY_RD_ADR, 0, RK1_DUMMY_RD_ADR_DMY_RD_RK1_LEN);
    vIO32WriteFldAlign(DRAMC_REG_RK0_DUMMY_RD_ADR, 0, RK0_DUMMY_RD_ADR_DMY_RD_RK0_LEN);
    vIO32WriteFldMulti(DRAMC_REG_DUMMY_RD, P_Fld(1, DUMMY_RD_DUMMY_RD_CNT7) | P_Fld(1, DUMMY_RD_DUMMY_RD_EN));
}
#endif

#if GATING_ONLY_FOR_DEBUG
void DramcGatingDebugRankSel(DRAMC_CTX_T *p, U8 u1Rank)
{
    #if DUAL_RANK_ENABLE
    if (p->support_rank_num==RANK_SINGLE)
    #endif
    {
        u1Rank =0;
    }

    vIO32WriteFldAlign(DDRPHY_MISC_CTRL1, u1Rank, MISC_CTRL1_R_DMSTBENCMP_RK_OPT);
}

void DramcGatingDebugInit(DRAMC_CTX_T *p)
{
    DramPhyReset(p);

    //enable &reset DQS counter
    vIO32WriteFldAlign(DRAMC_REG_SPCMD, 1, SPCMD_DQSGCNTEN);
    mcDELAY_US(4);//wait 1 auto refresh after DQS Counter enable

    vIO32WriteFldAlign(DRAMC_REG_SPCMD, 1, SPCMD_DQSGCNTRST);
    mcDELAY_US(1);//delay 2T
    vIO32WriteFldAlign(DRAMC_REG_SPCMD, 0, SPCMD_DQSGCNTRST);
    //mcSHOW_DBG_MSG(("DramcGatingDebugInit done\n" ));
}

void DramcGatingDebugExit(DRAMC_CTX_T *p)
{
    //enable &reset DQS counter
    vIO32WriteFldAlign(DRAMC_REG_SPCMD, 0, SPCMD_DQSGCNTEN);
    vIO32WriteFldAlign(DDRPHY_MISC_CTRL1, 0, MISC_CTRL1_R_DMSTBENCMP_RK_OPT);
}

void DramcGatingDebug(DRAMC_CTX_T *p)
{
    U32 LP3_DataPerByte[DQS_NUMBER];
    U32 u4DebugCnt[DQS_NUMBER];
    U16 u2DebugCntPerByte;

    U32 u4value, u4all_result_R, u4all_result_F, u4err_value;


    mcDELAY_MS(10);

    LP3_DataPerByte[0] = (u4IO32ReadFldAlign(DDRPHY_MISC_STBERR_RK0_R, MISC_STBERR_RK0_R_STBERR_RK0_R));
    LP3_DataPerByte[2] = (LP3_DataPerByte[0] >>8) & 0xff;
    LP3_DataPerByte[0] &= 0xff;

    u4all_result_R = LP3_DataPerByte[0] | (LP3_DataPerByte[2] <<8);

    // falling
    LP3_DataPerByte[0] = (u4IO32ReadFldAlign(DDRPHY_MISC_STBERR_RK0_F, MISC_STBERR_RK0_F_STBERR_RK0_F));
    LP3_DataPerByte[2] = (LP3_DataPerByte[0] >>8) & 0xff;
    LP3_DataPerByte[0] &= 0xff;

    u4all_result_F = LP3_DataPerByte[0] | (LP3_DataPerByte[2] <<8);

     //read DQS counter
    u4DebugCnt[0] = u4IO32Read4B(DRAMC_REG_DQSGNWCNT0);
    u4DebugCnt[1] = (u4DebugCnt[0] >> 16) & 0xffff;
    u4DebugCnt[0] &= 0xffff;

    u4DebugCnt[2] = u4IO32Read4B(DRAMC_REG_DQSGNWCNT1);
    u4DebugCnt[3] = (u4DebugCnt[2] >> 16) & 0xffff;
    u4DebugCnt[2] &= 0xffff;

    mcSHOW_DBG_MSG(("\n[DramcGatingDebug] DQS count (B3->B0) 0x%H, 0x%H, 0x%H, 0x%H \nError flag Rank0 (B3->B0) %B %B  %B %B  %B %B  %B %B\n", \
                                   u4DebugCnt[3], u4DebugCnt[2], u4DebugCnt[1], u4DebugCnt[0], \
                                   (u4all_result_F>>24)&0xff, (u4all_result_R>>24)&0xff, \
                                   (u4all_result_F>>16)&0xff, (u4all_result_R>>16)&0xff, \
                                   (u4all_result_F>>8)&0xff,   (u4all_result_R>>8)&0xff, \
                                   (u4all_result_F)&0xff,         (u4all_result_R)&0xff));

    #if DUAL_RANK_ENABLE
    if (p->support_rank_num==RANK_DUAL)
    {
        LP3_DataPerByte[0] = (u4IO32ReadFldAlign(DDRPHY_MISC_STBERR_RK1_R, MISC_STBERR_RK1_R_STBERR_RK1_R));//PHY_B
        LP3_DataPerByte[2] = (LP3_DataPerByte[0] >>8) & 0xff;
        LP3_DataPerByte[0] &= 0xff;

        u4all_result_R = LP3_DataPerByte[0] | (LP3_DataPerByte[2] <<8);

        // falling
        LP3_DataPerByte[0] = (u4IO32ReadFldAlign(DDRPHY_MISC_STBERR_RK1_F, MISC_STBERR_RK1_F_STBERR_RK1_F));//PHY_B
        LP3_DataPerByte[2] = (LP3_DataPerByte[0] >>8) & 0xff;
        LP3_DataPerByte[0] &= 0xff;

        u4all_result_F = LP3_DataPerByte[0];

        mcSHOW_DBG_MSG(("Error flag Rank1 (B3->B0) %B %B  %B %B  %B %B  %B %B\n", \
                                       (u4all_result_F>>24)&0xff, (u4all_result_R>>24)&0xff, \
                                       (u4all_result_F>>16)&0xff, (u4all_result_R>>16)&0xff, \
                                       (u4all_result_F>>8)&0xff,   (u4all_result_R>>8)&0xff, \
                                       (u4all_result_F)&0xff,         (u4all_result_R)&0xff));
    }
    #else
    mcSHOW_DBG_MSG((" \n" ));
    #endif

    //vIO32WriteFldAlign(DRAMC_REG_SPCMD, 1, SPCMD_DQSGCNTRST);
    //mcDELAY_US(1);//delay 2T
    //vIO32WriteFldAlign(DRAMC_REG_SPCMD, 0, SPCMD_DQSGCNTRST);

}
#endif

#if  CPU_RW_TEST_AFTER_K
void DramcDumpDebugInfo(DRAMC_CTX_T *p)
{
    U8 u1RefreshRate = 0;
    mcSHOW_DBG_MSG(("=========================\n"));


    #if GATING_ONLY_FOR_DEBUG
    // Read gating error flag
    //DramcGatingDebugInit(p);
    DramcGatingDebug(p);
    #endif

    // Read HW gating tracking
#ifdef HW_GATING
        DramcPrintHWGatingStatus(p);
#endif

#ifdef TEMP_SENSOR_ENABLE
        u1RefreshRate = u1GetMR4RefreshRate(p);
        mcSHOW_ERR_MSG(("[CHA] MRR(MR4) Reg.3B8h[10:8]=%x\n", u1RefreshRate));
#endif

    mcSHOW_DBG_MSG(("=========================\n"));
}
#endif

void TransferToSPMControl(DRAMC_CTX_T *p)
{
#if COMPILE_THIS_PART
    U32 u4WbrBackup = 0;

    vIO32WriteFldAlign(DDRPHY_MISC_SPM_CTRL0, 0xffffffff, MISC_SPM_CTRL0_PHY_SPM_CTL0);//Lewis@20160505: PLL control is by SW RG no matter Low power
    vIO32WriteFldAlign(DDRPHY_MISC_SPM_CTRL2, 0xffffffff, MISC_SPM_CTRL2_PHY_SPM_CTL2);
    //cc mark vIO32WriteFldAlign(DDRPHY_CA_CMD2, 0x1fff, SEL_MUX0);
    //cc mark vIO32WriteFldAlign(DDRPHY_CA_CMD3, 0x7fffff, SEL_MUX1);
    return;
#endif
}

void Switch26MHzDisableDummyReadRefreshAllBank(DRAMC_CTX_T *p)
{
#if COMPILE_THIS_PART

    vIO32WriteFldAlign(DRAMC_REG_REFCTRL0, 0, REFCTRL0_PBREFEN);//Switch to all bank refresh


    vIO32WriteFldMulti(DRAMC_REG_DUMMY_RD, P_Fld(0x0, DUMMY_RD_DQSG_DMYWR_EN)//Disable Dummy Read
                | P_Fld(0x0, DUMMY_RD_DQSG_DMYRD_EN) | P_Fld(0x0, DUMMY_RD_SREF_DMYRD_EN)
                | P_Fld(0x0, DUMMY_RD_DUMMY_RD_EN) | P_Fld(0x0, DUMMY_RD_DMY_RD_DBG)
                | P_Fld(0x0, DUMMY_RD_DMY_WR_DBG));
    return;
#endif
}

void EnableDramcPhyDCM(DRAMC_CTX_T *p, bool bEn)
{
    if(bEn)
    {
        vIO32WriteFldMulti( DRAMC_REG_DRAMC_PD_CTRL, P_Fld(0x1, DRAMC_PD_CTRL_DCMENNOTRFC)
                | P_Fld(0x1, DRAMC_PD_CTRL_COMBCLKCTRL)
                | P_Fld(0x1, DRAMC_PD_CTRL_PHYCLKDYNGEN)
                | P_Fld(0x1, DRAMC_PD_CTRL_DCMEN) | P_Fld(0x0, DRAMC_PD_CTRL_COMBPHY_CLKENSAME)
                | P_Fld(0x1, DRAMC_PD_CTRL_DCMEN2));

        vIO32WriteFldMulti(DDRPHY_MISC_CTRL3, P_Fld(0x1, MISC_CTRL3_ARPI_CG_MCTL_DQ_OPT)
				| P_Fld(0x0, MISC_CTRL3_ARPI_CG_MCTL_CA_OPT));
        //vIO32WriteFldAlign(DDRPHY_MISC_CTRL3+(1<<POS_BANK_NUM), 0x1, MISC_CTRL3_ARPI_CG_MCTL_DQ_OPT);

        //cc mark for clock will be switched vIO32WriteFldAlign(DDRPHY_MISC_CG_CTRL0, 0x0, MISC_CG_CTRL0_CLK_MEM_DFS_CFG);
    }
    else
    {
        vIO32WriteFldMulti(DRAMC_REG_DRAMC_PD_CTRL, P_Fld(0x0, DRAMC_PD_CTRL_DCMENNOTRFC)
                | P_Fld(0x0, DRAMC_PD_CTRL_COMBCLKCTRL)
                | P_Fld(0x0, DRAMC_PD_CTRL_PHYCLKDYNGEN)
                | P_Fld(0x0, DRAMC_PD_CTRL_DCMEN) | P_Fld(0x1, DRAMC_PD_CTRL_COMBPHY_CLKENSAME)
                | P_Fld(0x0, DRAMC_PD_CTRL_DCMEN2));

        vIO32WriteFldMulti(DDRPHY_MISC_CTRL3, P_Fld(0x0, MISC_CTRL3_ARPI_CG_MCTL_DQ_OPT)
				| P_Fld(0x0, MISC_CTRL3_ARPI_CG_MCTL_CA_OPT));
        //vIO32WriteFldAlign(DDRPHY_MISC_CTRL3+(1<<POS_BANK_NUM), 0x0, MISC_CTRL3_ARPI_CG_MCTL_DQ_OPT);

        //cc mark for hang will occur vIO32WriteFldAlign(DDRPHY_MISC_CG_CTRL0, 0xffffffff, MISC_CG_CTRL0_CLK_MEM_DFS_CFG);

    }
    return;
}

void HwSaveForSR(DRAMC_CTX_T *p)
{
#if COMPILE_THIS_PART

        vIO32WriteFldMulti(DRAMC_REG_RSTMASK, P_Fld(0, RSTMASK_GT_SYNC_MASK)
                    | P_Fld(0, RSTMASK_GT_SYNC_MASK_FOR_PHY));
        vIO32WriteFldAlign(DRAMC_REG_REFCTRL1, 1, REFCTRL1_SLEFREF_AUTOSAVE_EN);
        vIO32WriteFldMulti(DRAMC_REG_SREFCTRL, P_Fld(0, SREFCTRL_SREF2_OPTION)
                    | P_Fld(0, SREFCTRL_SREF3_OPTION));
#endif
}

void DramcRunTimeConfig(DRAMC_CTX_T *p)
{
    mcSHOW_DBG_MSG(("=== [DramcRunTimeConfig] ===\n"));

	if(p->dram_type == TYPE_PCDDR4)
	{
		//vIO32WriteFldAlign(DRAMC_REG_CKECTRL, 1, CKECTRL_CKEFIXON);
		//return;
	}

    //TransferPLLToSPMControl(p);

#if COMPILE_THIS_PART
	if (p->dram_type == TYPE_LPDDR3 || p->dram_type == TYPE_LPDDR2)
	{
	#ifdef HW_GATING
	     DramcHWGatingInit(p);                                   // HW gating initial before RunTime config.
	     DramcHWGatingOnOff((DRAMC_CTX_T *) p, 1); // Enable HW gating tracking
	     mcSHOW_DBG_MSG(("HW_GATING: ON\n"));
	#else
	     DramcHWGatingOnOff((DRAMC_CTX_T *) p, 0);
	     mcSHOW_DBG_MSG(("HW_GATING: OFF\n"));
	#endif

	#ifdef DUMMY_READ_FOR_TRACKING
	     DramcDummyReadForTrackingEnable(p);
	     mcSHOW_DBG_MSG(("DUMMY_READ_FOR_TRACKING: ON\n"));
	#else
	     mcSHOW_DBG_MSG(("DUMMY_READ_FOR_TRACKING: OFF\n"));
	#endif

	//HW sync gating tracking
	     mcSHOW_DBG_MSG(("DFS_HW_SYNC_GATING_TRACKING: OFF\n"));
	}
#endif

#ifdef ZQCS_ENABLE_LP3
    vIO32WriteFldAlign(DRAMC_REG_SPCMDCTRL, 1, SPCMDCTRL_ZQCSDISB);
    mcSHOW_DBG_MSG(("ZQCS_ENABLE: ON\n"));
#else
    vIO32WriteFldAlign(DRAMC_REG_SPCMDCTRL, 0, SPCMDCTRL_ZQCSDISB);
    mcSHOW_DBG_MSG(("ZQCS_ENABLE: OFF\n"));
#endif

#if APPLY_LOWPOWER_GOLDEN_SETTINGS
    EnableDramcPhyDCM(p, 1);
    mcSHOW_DBG_MSG(("LOWPOWER_GOLDEN_SETTINGS(DCM): ON\n"));
#else
    EnableDramcPhyDCM(p, 0);
    mcSHOW_DBG_MSG(("LOWPOWER_GOLDEN_SETTINGS(DCM): OFF\n"));
#endif

#ifdef SPM_CONTROL_AFTERK
    TransferToSPMControl(p);  //don't enable in ETT
    mcSHOW_DBG_MSG(("SPM_CONTROL_AFTERK: ON\n"));
#else
    mcSHOW_DBG_MSG(("SPM_CONTROL_AFTERK: OFF\n"));
#endif

#ifdef TEMP_SENSOR_ENABLE
    // enable MR4 refresh rate reference, interval = 0x10
    //vIO32WriteFldAlign(DRAMC_REG_SHU_CONF3, 0x10, SHU_CONF3_REFRCNT);
    //lp3 only has CHA
    vIO32WriteFldAlign(DRAMC_REG_SPCMDCTRL, 0, SPCMDCTRL_REFRDIS);

    mcSHOW_DBG_MSG(("TEMP_SENSOR_ENABLE: ON\n"));
#else
    vIO32WriteFldAlign(DRAMC_REG_SPCMDCTRL, 1, SPCMDCTRL_REFRDIS);
    mcSHOW_DBG_MSG(("TEMP_SENSOR_ENABLE: OFF\n"));
#endif

#if ENABLE_PER_BANK_REFRESH
    vIO32WriteFldAlign(DRAMC_REG_REFCTRL0, 1, REFCTRL0_PBREFEN);
    mcSHOW_DBG_MSG(("ENABLE_PER_BANK_REFRESH: ON\n"));
#else
    vIO32WriteFldAlign(DRAMC_REG_REFCTRL0, 0, REFCTRL0_PBREFEN);
    mcSHOW_DBG_MSG(("ENABLE_PER_BANK_REFRESH: OFF\n"));
#endif
#ifdef HW_SAVE_FOR_SR
    HwSaveForSR(p);
    mcSHOW_DBG_MSG(("HW_SAVE_FOR_SR: ON\n"));
#else
    mcSHOW_DBG_MSG(("HW_SAVE_FOR_SR: OFF\n"));
#endif
    mcSHOW_DBG_MSG(("=========================\n"));
}


void DramcSetRankEngine2(DRAMC_CTX_T *p, U8 u1RankSel)
{
    //LPDDR2_3_ADRDECEN_TARKMODE =0, always rank0
    vIO32WriteFldAlign(DRAMC_REG_DRAMCTRL, 1, DRAMCTRL_ADRDECEN_TARKMODE);

    // DUMMY_TESTAGENTRKSEL =0, select rank according to CATRAIN_TESTAGENTRK
    vIO32WriteFldAlign(DRAMC_REG_TEST2_4, 0, TEST2_4_TESTAGENTRKSEL);

    //CATRAIN_TESTAGENTRK = u1RankSel
    vIO32WriteFldAlign(DRAMC_REG_TEST2_4, u1RankSel, TEST2_4_TESTAGENTRK);
}

U32 DramcEngine1(DRAMC_CTX_T *p, U32 test2_1, U32 test2_2, S16 loopforever, U8 period)
{
#if COMPILE_THIS_PART //cc notes: currently TE1 is not used. 
    U32 value;
    test2_1 = 0x55000000;
    test2_2 = 0xaa000400;

    DramcSetRankEngine2(p, p->rank);

    vIO32WriteFldMulti(DRAMC_REG_TEST2_0, P_Fld( 0x55, TEST2_0_TEST2_PAT0)
												|  P_Fld(0xaa, TEST2_0_TEST2_PAT1));
    vIO32Write4B(DRAMC_REG_TEST2_1, 0);
    vIO32Write4B(DRAMC_REG_TEST2_2, 0x01007fff);

    vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(1, TEST2_3_TEST1));
	value = 0;
	while((u4IO32ReadFldAlign(DRAMC_REG_TESTRPT, TESTRPT_DM_CMP_CPT))==0)
	{
		//ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT), &u4value);
		mcDELAY_US(1);
		value++;
		if(value > 10000)
		{
		   /*TINFO="fcWAVEFORM_MEASURE_A %d: time out\n", u4loop_count*/
		   mcSHOW_DBG_MSG(("fcWAVEFORM_MEASURE_A %d :time out\n", value));
		   break;
		}
	}
	mcDELAY_US(1);
	value = (u4IO32Read4B(DRAMC_REG_TESTRPT) >> 4) & 1;
	vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TEST1));

	return value ? 0xffffffff : 0;
#else
	return 0;
#endif

}

/* cc porting from Azalea */
static U32 uiReg0D0h;
DRAM_STATUS_T DramcEngine2Init(DRAMC_CTX_T *p, U32 test2_1, U32 test2_2, U8 testaudpat, U8 log2loopcount)
{
    // error handling
    if (!p)
    {
        mcSHOW_ERR_MSG(("context is NULL\n"));
        return DRAM_FAIL;
    }

    // check loop number validness
//    if ((log2loopcount > 15) || (log2loopcount < 0))		// U8 >=0 always.
    if (log2loopcount > 15)
    {
        mcSHOW_ERR_MSG(("wrong parameter log2loopcount:    log2loopcount just 0 to 15 !\n"));
        return DRAM_FAIL;
    }

 //   DramcSetRankEngine2(p, p->rank);

    uiReg0D0h=u4IO32Read4B(DRAMC_REG_DUMMY_RD);
    vIO32WriteFldMulti(DRAMC_REG_DUMMY_RD, P_Fld(0, DUMMY_RD_DQSG_DMYRD_EN) | P_Fld(0, DUMMY_RD_DQSG_DMYWR_EN) | P_Fld(0, DUMMY_RD_DUMMY_RD_EN) | P_Fld(0, DUMMY_RD_SREF_DMYRD_EN) | P_Fld(0, DUMMY_RD_DMY_RD_DBG) | P_Fld(0, DUMMY_RD_DMY_WR_DBG));  //must close dummy read when do test agent

    vIO32WriteFldAlign(DRAMC_REG_TESTCHIP_DMA1, 0, TESTCHIP_DMA1_DMA_LP4MATAB_OPT);

    // disable self test engine1 and self test engine2
    vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TEST2W) | P_Fld(0, TEST2_3_TEST2R) | P_Fld(0, TEST2_3_TEST1));


    // 1.set pattern ,base address ,offset address
    // 2.select  ISI pattern or audio pattern or xtalk pattern
    // 3.set loop number
    // 4.enable read or write
    // 5.loop to check DM_CMP_CPT
    // 6.return CMP_ERR
    // currently only implement ucengine_status = 1, others are left for future extension

    // 1
    vIO32WriteFldMulti(DRAMC_REG_TEST2_0, P_Fld((test2_1>>24)&0xff,TEST2_0_TEST2_PAT0)|P_Fld((test2_2>>24)&0xff,TEST2_0_TEST2_PAT1));
#if (FOR_DV_SIMULATION == 1/* cc mark || SW_CHANGE_FOR_SIMULATION==1*/)
    //vIO32WriteFldAlign(DRAMC_REG_TEST2_1, ((test2_1+0x100)<<4)&0x00ffffff,RG_FLD_FULL);    //DV sim memory 0~0x100 has values, can't used
    vIO32WriteFldAlign(DRAMC_REG_TEST2_1, (test2_1+0x100000)&0x00ffffff,RG_FLD_FULL);    //DV sim memory 0~0x100 has values, can't used
#else
    vIO32WriteFldAlign(DRAMC_REG_TEST2_1, (test2_1<<4)&0x00ffffff,RG_FLD_FULL);
#endif
    vIO32WriteFldAlign(DRAMC_REG_TEST2_2, (test2_2<<4)&0x00ffffff,RG_FLD_FULL);

    // 2 & 3
    // (TESTXTALKPAT, TESTAUDPAT) = 00 (ISI), 01 (AUD), 10 (XTALK), 11 (UNKNOWN)
    if (testaudpat == 2)   // xtalk
    {
        //TEST_REQ_LEN1=1 is new feature, hope to make dq bus continously.
        //but DV simulation will got problem of compare err
        //so go back to use old way
        //TEST_REQ_LEN1=0, R_DMRWOFOEN=1
        vIO32WriteFldMulti(DRAMC_REG_TEST2_4, P_Fld(0, TEST2_4_TEST_REQ_LEN1));   //test agent 2 with cmd length = 0
        vIO32WriteFldMulti(DRAMC_REG_PERFCTL0, P_Fld(0, PERFCTL0_RWOFOEN)); //cc change from 1->0

        // select XTALK pattern
        // set addr 0x044 [7] to 0
        vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TESTAUDPAT)|P_Fld(log2loopcount,TEST2_3_TESTCNT)); //dont use audio pattern

        // set addr 0x48[16] to 1, TESTXTALKPAT = 1
        vIO32WriteFldMulti(DRAMC_REG_TEST2_4, P_Fld(1, TEST2_4_TESTXTALKPAT)|P_Fld(0,TEST2_4_TESTAUDMODE)|P_Fld(0,TEST2_4_TESTAUDBITINV));  //use XTALK pattern, dont use audio pattern

        // R_DMTESTSSOPAT=0, R_DMTESTSSOXTALKPAT=0
        vIO32WriteFldMulti(DRAMC_REG_TEST2_4, P_Fld(0, TEST2_4_TESTSSOPAT)|P_Fld(0,TEST2_4_TESTSSOXTALKPAT));   //dont use sso, sso+xtalk pattern
    }
    else if (testaudpat == 1)   // audio
    {
        // set AUDINIT=0x11 AUDINC=0x0d AUDBITINV=1 AUDMODE=1
        vIO32WriteFldMulti(DRAMC_REG_TEST2_4, \
            P_Fld(0x00000011, TEST2_4_TESTAUDINIT)|P_Fld(0x0000000d, TEST2_4_TESTAUDINC)| \
            P_Fld(0, TEST2_4_TESTXTALKPAT)|P_Fld(1,TEST2_4_TESTAUDMODE)|P_Fld(1,TEST2_4_TESTAUDBITINV));

        // set addr 0x044 [7] to 1 ,select audio pattern
        vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(1, TEST2_3_TESTAUDPAT)|P_Fld(log2loopcount,TEST2_3_TESTCNT));
    }
    else   // ISI
    {
        // select ISI pattern
        vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TESTAUDPAT)|P_Fld(log2loopcount,TEST2_3_TESTCNT));
        vIO32WriteFldAlign(DRAMC_REG_TEST2_4, 0, TEST2_4_TESTXTALKPAT);
    }
    return DRAM_OK;
}


U32 DramcEngine2Run(DRAMC_CTX_T *p, DRAM_TE_OP_T wr, U8 testaudpat)
{
    U32 u4loop_count = 0;
    U32 u4result = 0xffffffff;

    // 4
    if (wr == TE_OP_READ_CHECK)
    {
        if ((testaudpat == 1) || (testaudpat == 2))
        {
            //if audio pattern, enable read only (disable write after read), AUDMODE=0x48[15]=0
            vIO32WriteFldAlign(DRAMC_REG_TEST2_4, 0, TEST2_4_TESTAUDMODE);
        }

        // enable read, 0x008[31:29]
        vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TEST2W) | P_Fld(1, TEST2_3_TEST2R) | P_Fld(0, TEST2_3_TEST1));
    }
    else if (wr == TE_OP_WRITE_READ_CHECK)
    {
        // enable write
        vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(1, TEST2_3_TEST2W) | P_Fld(0, TEST2_3_TEST2R) | P_Fld(0, TEST2_3_TEST1));

        // read data compare ready check
        u4loop_count = 0;
        while((u4IO32ReadFldAlign(DRAMC_REG_TESTRPT, TESTRPT_DM_CMP_CPT))==0)
        {
            //ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT), &u4value);
            mcDELAY_US(CMP_CPT_POLLING_PERIOD);
            u4loop_count++;
            if ((u4loop_count > 3) &&(u4loop_count <= MAX_CMP_CPT_WAIT_LOOP))
            {
                //mcSHOW_ERR_MSG(("TESTRPT_DM_CMP_CPT (Write): %d\n", u4loop_count));
            }
            else if(u4loop_count > MAX_CMP_CPT_WAIT_LOOP)
            {
               /*TINFO="fcWAVEFORM_MEASURE_A %d: time out\n", u4loop_count*/
               mcSHOW_DBG_MSG(("fcWAVEFORM_MEASURE_A %d :time out\n", u4loop_count));
                break;
            }
        }

        // disable write
        vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TEST2W) | P_Fld(0, TEST2_3_TEST2R) | P_Fld(0, TEST2_3_TEST1));

        mcDELAY_US(1);

        // enable read
        vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TEST2W) | P_Fld(1, TEST2_3_TEST2R) | P_Fld(0, TEST2_3_TEST1));
    }

    // 5
    // read data compare ready check
    u4loop_count = 0;
    while((u4IO32ReadFldAlign(DRAMC_REG_TESTRPT, TESTRPT_DM_CMP_CPT))==0)
    {
            mcDELAY_US(CMP_CPT_POLLING_PERIOD);
            u4loop_count++;
            if ((u4loop_count > 3)&&(u4loop_count <= MAX_CMP_CPT_WAIT_LOOP))
            {
                //mcSHOW_ERR_MSG(("TESTRPT_DM_CMP_CPT (Read): %d\n", u4loop_count));
            }
            else if(u4loop_count > MAX_CMP_CPT_WAIT_LOOP)
            {
               /*TINFO="fcWAVEFORM_MEASURE_B %d: time out\n", u4loop_count*/
               mcSHOW_DBG_MSG(("fcWAVEFORM_MEASURE_B %d: time out\n", u4loop_count));
               break;
            }
    }

    // delay 10ns after ready check from DE suggestion (1ms here)
    mcDELAY_US(1);

    // 6
    // return CMP_ERR, 0 is ok ,others are fail,diable test2w or test2r
    // get result
    // or all result
    u4result = (u4IO32Read4B(DRAMC_REG_CMP_ERR));
    // disable read
    vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TEST2W) | P_Fld(0, TEST2_3_TEST2R) | P_Fld(0, TEST2_3_TEST1)| P_Fld(0, TEST2_3_TEST2WREN2_HW_EN));

    return u4result;
}

void DramcEngine2End(DRAMC_CTX_T *p)
{
    vIO32WriteFldAlign(DRAMC_REG_DUMMY_RD, uiReg0D0h,RG_FLD_FULL);
}

/* end of Azaea code */

//-------------------------------------------------------------------------
/** DramcEngine2
 *  start the self test engine 2 inside dramc to test dram w/r.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  wr              (DRAM_TE_OP_T): TE operation
 *  @param  test2_1         (U32): 28bits,base address[27:0].
 *  @param  test2_2         (U32): 28bits,offset address[27:0]. (unit is 16-byte, i.e: 0x100 is 0x1000).
 *  @param  loopforever     (S16): 0    read\write one time ,then exit
 *                                >0 enable eingie2, after "loopforever" second ,write log and exit
 *                                -1 loop forever to read\write, every "period" seconds ,check result ,only when we find error,write log and exit
 *                                -2 loop forever to read\write, every "period" seconds ,write log ,only when we find error,write log and exit
 *                                -3 just enable loop forever ,then exit
 *  @param period           (U8):  it is valid only when loopforever <0; period should greater than 0
 *  @param log2loopcount    (U8): test loop number of test agent2 loop number =2^(log2loopcount) ,0 one time
 *  @retval status          (U32): return the value of DM_CMP_ERR  ,0  is ok ,others mean  error
 */
//-------------------------------------------------------------------------
U32 DramcEngine2(DRAMC_CTX_T *p, DRAM_TE_OP_T wr, U32 test2_1, U32 test2_2, U8 testaudpat, S16 loopforever, U8 period, U8 log2loopcount)
{
    U8 ucengine_status;
    U32 uiReg0D0h;
    U32 u4loop_count = 0;
    U32 u4result = 0xffffffff;
    U32 u4log2loopcount = (U32) log2loopcount;

    // error handling
    if (!p)
    {
        mcSHOW_ERR_MSG(("context is NULL\n"));
        return u4result;
    }

    // check loop number validness
//    if ((log2loopcount > 15) || (log2loopcount < 0))		// U8 >=0 always.
    if (log2loopcount > 15)
    {
        mcSHOW_ERR_MSG(("wrong parameter log2loopcount:    log2loopcount just 0 to 15 !\n"));
        return u4result;
    }

    DramcSetRankEngine2(p, p->rank);

    uiReg0D0h=u4IO32Read4B(DRAMC_REG_DUMMY_RD);
    vIO32WriteFldMulti(DRAMC_REG_DUMMY_RD, P_Fld(0, DUMMY_RD_DQSG_DMYRD_EN) | P_Fld(0, DUMMY_RD_DQSG_DMYWR_EN) | P_Fld(0, DUMMY_RD_DUMMY_RD_EN) | P_Fld(0, DUMMY_RD_SREF_DMYRD_EN) | P_Fld(0, DUMMY_RD_DMY_RD_DBG) | P_Fld(0, DUMMY_RD_DMY_WR_DBG));  //must close dummy read when do test agent

    vIO32WriteFldAlign(DRAMC_REG_TESTCHIP_DMA1, 0, TESTCHIP_DMA1_DMA_LP4MATAB_OPT);

    // disable self test engine1 and self test engine2
    vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TEST2W) | P_Fld(0, TEST2_3_TEST2R) | P_Fld(0, TEST2_3_TEST1));

    // we get the status
    // loopforever    period    status    mean
    //     0             x         1       read\write one time ,then exit ,don't write log
    //    >0             x         2       read\write in a loop,after "loopforever" seconds ,disable it ,return the R\W status
    //    -1            >0         3       read\write in a loop,every "period" seconds ,check result ,only when we find error,write log and exit
    //    -2            >0         4       read\write in a loop,every "period" seconds ,write log ,only when we find error,write log and exit
    //    -3             x         5       just enable loop forever , then exit (so we should disable engine1 outside the function)
    if (loopforever == 0)
    {
        ucengine_status = 1;
    }
    else if (loopforever > 0)
    {
        ucengine_status = 2;
    }
    else if (loopforever == -1)
    {
        if (period > 0)
        {
            ucengine_status = 3;
        }
        else
        {
            mcSHOW_ERR_MSG(("parameter 'period' should be equal or greater than 0\n"));
            return u4result;
        }
    }
    else if (loopforever == -2)
    {
        if (period > 0)
        {
            ucengine_status = 4;
        }
        else
        {
            mcSHOW_ERR_MSG(("parameter 'period' should be equal or greater than 0\n"));
            return u4result;
        }
    }
    else if (loopforever == -3)
    {
        if (period > 0)
        {
            ucengine_status = 5;
        }
        else
        {
            mcSHOW_ERR_MSG(("parameter 'period' should be equal or greater than 0\n"));
            return u4result;
        }
    }
    else
    {
        mcSHOW_ERR_MSG(("parameter 'loopforever' should be 0 -1 -2 -3 or greater than 0\n"));
        return u4result;
    }

    // set ADRDECEN=0, address decode not by DRAMC
    //2012/10/03, the same as A60806, for TA&UART b'31=1; for TE b'31=0
    //2013/7/9, for A60501, always set to 1
#ifdef fcFOR_A60806_TEST
        vIO32WriteFldAlign(DRAMC_REG_DRAMCTRL,0, DRAMCTRL_ADRDECEN);
#endif

    // 1.set pattern ,base address ,offset address
    // 2.select  ISI pattern or audio pattern or xtalk pattern
    // 3.set loop number
    // 4.enable read or write
    // 5.loop to check DM_CMP_CPT
    // 6.return CMP_ERR
    // currently only implement ucengine_status = 1, others are left for future extension
    /*if (ucengine_status == 4)
    {
        mcSHOW_DBG_MSG(("============================================\n"));
        mcSHOW_DBG_MSG(("enable test egine2 loop forever\n"));
        mcSHOW_DBG_MSG(("============================================\n"));
    }*/
    u4result = 0;
    while(1)
    {
        // 1
        vIO32WriteFldMulti(DRAMC_REG_TEST2_0, P_Fld((test2_1>>24)&0xff,TEST2_0_TEST2_PAT0)|P_Fld((test2_2>>24)&0xff,TEST2_0_TEST2_PAT1));

        vIO32Write4B(DRAMC_REG_TEST2_1, (test2_1<<4)&0x00ffffff);

        vIO32Write4B(DRAMC_REG_TEST2_2, (test2_2<<4)&0x00ffffff);

        // 2 & 3
        // (TESTXTALKPAT, TESTAUDPAT) = 00 (ISI), 01 (AUD), 10 (XTALK), 11 (UNKNOWN)
        if (testaudpat == 2)   // xtalk
        {
            //TEST_REQ_LEN1=1 is new feature, hope to make dq bus continously.
            //but DV simulation will got problem of compare err
            //so go back to use old way
            //TEST_REQ_LEN1=0, R_DMRWOFOEN=1
            vIO32WriteFldMulti(DRAMC_REG_TEST2_4, P_Fld(0, TEST2_4_TEST_REQ_LEN1));   //test agent 2 with cmd length = 0
            vIO32WriteFldMulti(DRAMC_REG_PERFCTL0, P_Fld(0, PERFCTL0_RWOFOEN)); //cc change from 1->0

            // select XTALK pattern
            // set addr 0x044 [7] to 0
            vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TESTAUDPAT)|P_Fld(u4log2loopcount,TEST2_3_TESTCNT)); //don¡¦t use audio pattern

            // set addr 0x48[16] to 1, TESTXTALKPAT = 1
            vIO32WriteFldMulti(DRAMC_REG_TEST2_4, P_Fld(1, TEST2_4_TESTXTALKPAT)|P_Fld(0,TEST2_4_TESTAUDMODE)|P_Fld(0,TEST2_4_TESTAUDBITINV));  //use XTALK pattern, don¡¦t use audio pattern

            // R_DMTESTSSOPAT=0, R_DMTESTSSOXTALKPAT=0
            vIO32WriteFldMulti(DRAMC_REG_TEST2_4, P_Fld(0, TEST2_4_TESTSSOPAT)|P_Fld(0,TEST2_4_TESTSSOXTALKPAT));   //don¡¦t use sso, sso+xtalk pattern
        }
        else if (testaudpat == 1)   // audio
        {
            // set AUDINIT=0x11 AUDINC=0x0d AUDBITINV=1 AUDMODE=1
            vIO32WriteFldMulti(DRAMC_REG_TEST2_4, \
                P_Fld(0x00000011, TEST2_4_TESTAUDINIT)|P_Fld(0x0000000d, TEST2_4_TESTAUDINC)| \
                P_Fld(0, TEST2_4_TESTXTALKPAT)|P_Fld(1,TEST2_4_TESTAUDMODE)|P_Fld(1,TEST2_4_TESTAUDBITINV));

            // set addr 0x044 [7] to 1 ,select audio pattern
            vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(1, TEST2_3_TESTAUDPAT)|P_Fld(u4log2loopcount,TEST2_3_TESTCNT));
        }
        else   // ISI
        {
            // select ISI pattern
            vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TESTAUDPAT)|P_Fld(u4log2loopcount,TEST2_3_TESTCNT));
            vIO32WriteFldAlign(DRAMC_REG_TEST2_4, 0, TEST2_4_TESTXTALKPAT);
        }

        // 4
        if (wr == TE_OP_READ_CHECK)
        {
            if ((testaudpat == 1) || (testaudpat == 2))
            {
                //if audio pattern, enable read only (disable write after read), AUDMODE=0x48[15]=0
                vIO32WriteFldAlign(DRAMC_REG_TEST2_4, 0, TEST2_4_TESTAUDMODE);
            }

            // enable read, 0x008[31:29]
            vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TEST2W) | P_Fld(1, TEST2_3_TEST2R) | P_Fld(0, TEST2_3_TEST1));
        }
        else if (wr == TE_OP_WRITE_READ_CHECK)
        {
            // enable write
            vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(1, TEST2_3_TEST2W) | P_Fld(0, TEST2_3_TEST2R) | P_Fld(0, TEST2_3_TEST1));

            // read data compare ready check
            u4loop_count = 0;
            while((u4IO32ReadFldAlign(DRAMC_REG_TESTRPT, TESTRPT_DM_CMP_CPT))==0)
            {
                //ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT), &u4value);
                mcDELAY_US(CMP_CPT_POLLING_PERIOD);
                u4loop_count++;
                if ((u4loop_count > 3) &&(u4loop_count <= MAX_CMP_CPT_WAIT_LOOP))
                {
                    //mcSHOW_ERR_MSG(("TESTRPT_DM_CMP_CPT (Write): %d\n", u4loop_count));
                }
                else if(u4loop_count > MAX_CMP_CPT_WAIT_LOOP)
                {
                   /*TINFO="fcWAVEFORM_MEASURE_A %d: time out\n", u4loop_count*/
                   mcSHOW_DBG_MSG(("fcWAVEFORM_MEASURE_A %d :time out\n", u4loop_count));
                   break;
                }
            }

            // disable write
            vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TEST2W) | P_Fld(0, TEST2_3_TEST2R) | P_Fld(0, TEST2_3_TEST1));

            mcDELAY_US(1);

            // enable read
            vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TEST2W) | P_Fld(1, TEST2_3_TEST2R) | P_Fld(0, TEST2_3_TEST1));
        }

        // 5
        // read data compare ready check
        u4loop_count = 0;
        while((u4IO32ReadFldAlign(DRAMC_REG_TESTRPT, TESTRPT_DM_CMP_CPT))==0)
        {
                mcDELAY_US(CMP_CPT_POLLING_PERIOD);
                u4loop_count++;
                if ((u4loop_count > 3)&&(u4loop_count <= MAX_CMP_CPT_WAIT_LOOP))
                {
                    //mcSHOW_ERR_MSG(("TESTRPT_DM_CMP_CPT (Read): %d\n", u4loop_count));
                }
                else if(u4loop_count > MAX_CMP_CPT_WAIT_LOOP)
                {
                   /*TINFO="fcWAVEFORM_MEASURE_B %d: time out\n", u4loop_count*/
                   mcSHOW_DBG_MSG(("fcWAVEFORM_MEASURE_B %d: time out\n", u4loop_count));
                   break;
                }
        }

        // delay 10ns after ready check from DE suggestion (1ms here)
        mcDELAY_US(1);

        // 6
        // return CMP_ERR, 0 is ok ,others are fail,diable test2w or test2r
        // get result
        // or all result
        u4result |= (u4IO32Read4B(DRAMC_REG_CMP_ERR));
		
        // disable read
        vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TEST2W) | P_Fld(0, TEST2_3_TEST2R) | P_Fld(0, TEST2_3_TEST1));

        // handle status
        if (ucengine_status == 1)
        {
            // set ADRDECEN to 1
            //2013/7/9, for A60808, always set to 1
            #ifdef fcFOR_A60806_TEST
            vIO32WriteFldAlign(DRAMC_REG_DRAMCTRL,1, DRAMCTRL_ADRDECEN);
            #endif
            break;
        }
        else if (ucengine_status == 2)
        {
            mcSHOW_ERR_MSG(("not support for now\n"));
            break;
        }
        else if (ucengine_status == 3)
        {
            mcSHOW_ERR_MSG(("not support for now\n"));
            break;
        }
        else if (ucengine_status == 4)
        {
            mcSHOW_ERR_MSG(("not support for now\n"));
            break;
        }
        else if (ucengine_status == 5)
        {
            mcSHOW_ERR_MSG(("not support for now\n"));
            break;
        }
        else
        {
            mcSHOW_ERR_MSG(("not support for now\n"));
            break;
        }
    }


    vIO32Write4B(DRAMC_REG_DUMMY_RD, uiReg0D0h);
	

    return u4result;
}

U32 TestEngineCompare(DRAMC_CTX_T *p)
{
    U8 jj;
    U32 u4err_value;

    if (p->test_pattern== TEST_AUDIO_PATTERN)
    {
        // enable TE2, audio pattern
        if (p->fglow_freq_write_en == ENABLE)
        {
            u4err_value = DramcEngine2(p, TE_OP_READ_CHECK, p->test2_1, p->test2_2, 1, 0, 0, 0);
        }
        else
        {
            u4err_value = 0;
            for (jj = 0; jj < 1; jj++)
            {
                u4err_value |= DramcEngine2(p, TE_OP_WRITE_READ_CHECK, p->test2_1, p->test2_2, 1, 0, 0, 0);
            }
        }
    }
    else if (p->test_pattern == TEST_ISI_PATTERN)
    {
        // enable TE2, ISI pattern
        u4err_value = 0;
        for (jj = 0; jj < 1; jj++)
        {
            u4err_value |= DramcEngine2(p, TE_OP_WRITE_READ_CHECK, p->test2_1, p->test2_2, 0, 0, 0, 0);
        }
    }
    else if (p->test_pattern == TEST_XTALK_PATTERN)
    {
        if (p->fglow_freq_write_en == ENABLE)
        {
            u4err_value = DramcEngine2(p, TE_OP_READ_CHECK, p->test2_1, p->test2_2, 2, 0, 0, 0);
        }
        else
        {
            u4err_value = DramcEngine2(p, TE_OP_WRITE_READ_CHECK, p->test2_1, p->test2_2, 2, 0, 0, 0);
        }
    }
    else if (p->test_pattern == TEST_MIX_PATTERN)
    {
        u4err_value = DramcEngine2(p, TE_OP_WRITE_READ_CHECK, p->test2_1, p->test2_2, 1, 0, 0, 0);
        u4err_value |= DramcEngine2(p, TE_OP_WRITE_READ_CHECK, p->test2_1, p->test2_2, 2, 0, 0, 0);
    }
    else
    {
        mcSHOW_ERR_MSG(("Not support test pattern!! Use audio pattern by default.\n"));
        u4err_value = DramcEngine2(p, TE_OP_WRITE_READ_CHECK, p->test2_1, p->test2_2, 1, 0, 0, 0);
    }
	
	return u4err_value;
}

#if COMPILE_THIS_PART
U32 DramcEngine1New(DRAMC_CTX_T *p, U32 u4TestBaseAddr, U32 u4TestLen)
{
	U32 uiReg0D0h;
	U32 u4loop_count = 0;
	U32 u4result = 0xffffffff;

    // error handling
    if(!p)
    {
        mcSHOW_ERR_MSG(("context is NULL\n"));
        return u4result;
    }

	if(u4TestBaseAddr < 0x40000000) u4TestBaseAddr = 0x00000000;
	else							u4TestBaseAddr = u4TestBaseAddr - 0x40000000;
	u4TestLen = (u4TestLen & 0xFFFF8000) | 0x7FFF;//test agent1 min end address is 0x7FFF
    mcSHOW_DBG_MSG(("test agent 1 : addr = 0x%x, len = 0x%x\n", u4TestBaseAddr, u4TestLen));

	// DramcSetRankEngine2(p, p->rank);
	//must close dummy read when do test agent
	uiReg0D0h=u4IO32Read4B(DRAMC_REG_DUMMY_RD);
	vIO32WriteFldMulti(DRAMC_REG_DUMMY_RD, P_Fld(0, DUMMY_RD_DQSG_DMYRD_EN) | P_Fld(0, DUMMY_RD_DQSG_DMYWR_EN) | P_Fld(0, DUMMY_RD_DUMMY_RD_EN) | P_Fld(0, DUMMY_RD_SREF_DMYRD_EN) | P_Fld(0, DUMMY_RD_DMY_RD_DBG) | P_Fld(0, DUMMY_RD_DMY_WR_DBG));
	vIO32WriteFldAlign(DRAMC_REG_TESTCHIP_DMA1, 0, TESTCHIP_DMA1_DMA_LP4MATAB_OPT);

	// disable self test engine1 and self test engine2
	vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TEST2W) | P_Fld(0, TEST2_3_TEST2R) | P_Fld(0, TEST2_3_TEST1));

	//---------------------------------------------------------------------------------------------------
	u4result = 0;
	// 1.set pattern ,base address ,offset address
	vIO32WriteFldMulti(DRAMC_REG_TEST2_0, P_Fld(0x5a,TEST2_0_TEST2_PAT0)|P_Fld(0x5a,TEST2_0_TEST2_PAT1));
	vIO32Write4B(DRAMC_REG_TEST2_1, u4TestBaseAddr & 0xfffffff0);
	vIO32Write4B(DRAMC_REG_TEST2_2, u4TestLen);

	//2.Trigger test agent 1
	vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(1, TEST2_3_TEST1));

	u4loop_count = 0;
	while((u4IO32ReadFldAlign(DRAMC_REG_TESTRPT, TESTRPT_DM_CMP_CPT))==0)
	{
			mcDELAY_US(CMP_CPT_POLLING_PERIOD);
			u4loop_count++;
			if ((u4loop_count > 3)&&(u4loop_count <= MAX_CMP_CPT_WAIT_LOOP))
			{
				//mcSHOW_ERR_MSG(("TESTRPT_DM_CMP_CPT (Read): %d\n", u4loop_count));
			}
			else if(u4loop_count > MAX_CMP_CPT_WAIT_LOOP)
			{
			   /*TINFO="fcWAVEFORM_MEASURE_B %d: time out\n", u4loop_count*/
			   mcSHOW_DBG_MSG(("test agent read time out : 0x%x \n", u4loop_count));
			   break;
			}
	}
	// delay 10ns after ready check from DE suggestion (1ms here)
	mcDELAY_US(1);

 	//3.get result
	// or all result
	#if 0
	u4result = (u4IO32Read4B(DRAMC_REG_CMP_ERR));
	#else
	u4result = u4IO32Read4B(DRAMC_REG_TESTRPT) & 0xFFF;
	#endif

	//---------------------------------------------------------------------------------------------------
	vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TEST1));
	vIO32Write4B(DRAMC_REG_DUMMY_RD, uiReg0D0h);
	mcSHOW_DBG_MSG(("cmp_err = 0x%x\n", u4result));

	return (u4result == 0x101)? 0 : 0xffffffff;

}


U32 DramcEngine2New(DRAMC_CTX_T *p, U32 u4TestBaseAddr, U32 u4TestLen, U8 u4TestPattern, DRAM_TE_OP_T wr)
{
    U8 ucengine_status;
    U32 uiReg0D0h;
    U32 u4loop_count = 0;
    U32 u4result = 0xffffffff;
    U32 u4log2loopcount = 0x01;

    // error handling
    if(!p)
    {
        mcSHOW_ERR_MSG(("context is NULL\n"));
        return u4result;
    }

    if(u4log2loopcount > 15)
    {
        mcSHOW_ERR_MSG(("wrong parameter log2loopcount:    log2loopcount just 0 to 15 !\n"));
        return u4result;
    }

    if(u4TestBaseAddr < 0x40000000) u4TestBaseAddr = 0x00000000;
	else							u4TestBaseAddr = u4TestBaseAddr - 0x40000000;
    mcSHOW_DBG_MSG(("test agent 2 : addr = 0x%x, len = 0x%x, pattern = 0x%x, rw = 0x%x\n", u4TestBaseAddr, u4TestLen, u4TestPattern, (U32)wr));

    // DramcSetRankEngine2(p, p->rank);
    //must close dummy read when do test agent
    uiReg0D0h=u4IO32Read4B(DRAMC_REG_DUMMY_RD);
    vIO32WriteFldMulti(DRAMC_REG_DUMMY_RD, P_Fld(0, DUMMY_RD_DQSG_DMYRD_EN) | P_Fld(0, DUMMY_RD_DQSG_DMYWR_EN) | P_Fld(0, DUMMY_RD_DUMMY_RD_EN) | P_Fld(0, DUMMY_RD_SREF_DMYRD_EN) | P_Fld(0, DUMMY_RD_DMY_RD_DBG) | P_Fld(0, DUMMY_RD_DMY_WR_DBG));
    vIO32WriteFldAlign(DRAMC_REG_TESTCHIP_DMA1, 0, TESTCHIP_DMA1_DMA_LP4MATAB_OPT);

    // disable self test engine1 and self test engine2
    vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TEST2W) | P_Fld(0, TEST2_3_TEST2R) | P_Fld(0, TEST2_3_TEST1));

    // 1.set pattern ,base address ,offset address
    // 2.select  ISI pattern or audio pattern or xtalk pattern
    // 3.set loop number
    // 4.enable read or write
    // 5.loop to check DM_CMP_CPT
    // 6.return CMP_ERR
    // currently only implement ucengine_status = 1, others are left for future extension
    u4result = 0;
    // 1.set pattern ,base address ,offset address
    vIO32WriteFldMulti(DRAMC_REG_TEST2_0, P_Fld(0x5a,TEST2_0_TEST2_PAT0)|P_Fld(0x5a,TEST2_0_TEST2_PAT1));
    vIO32Write4B(DRAMC_REG_TEST2_1, u4TestBaseAddr & 0xfffffff0);
    vIO32Write4B(DRAMC_REG_TEST2_2, u4TestLen & 0xfffffff0);

    // 2 & 3 select test pattern and loopnumber
    if (u4TestPattern == TEST_XTALK_PATTERN)
    {//xtalk
        //TEST_REQ_LEN1=1 is new feature, hope to make dq bus continously.
        //but DV simulation will got problem of compare err
        //so go back to use old way
        //TEST_REQ_LEN1=0, R_DMRWOFOEN=1
        vIO32WriteFldMulti(DRAMC_REG_TEST2_4, P_Fld(0, TEST2_4_TEST_REQ_LEN1));   //test agent 2 with cmd length = 0
        vIO32WriteFldMulti(DRAMC_REG_PERFCTL0, P_Fld(0, PERFCTL0_RWOFOEN));//cc change from 1->0

        //select XTALK pattern
        vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TESTAUDPAT)|P_Fld(u4log2loopcount,TEST2_3_TESTCNT)); //don't use audio pattern
        vIO32WriteFldMulti(DRAMC_REG_TEST2_4, P_Fld(1, TEST2_4_TESTXTALKPAT)|P_Fld(0,TEST2_4_TESTAUDMODE)|P_Fld(0,TEST2_4_TESTAUDBITINV));  //use XTALK pattern, don't use audio pattern
        vIO32WriteFldMulti(DRAMC_REG_TEST2_4, P_Fld(0, TEST2_4_TESTSSOPAT)|P_Fld(0,TEST2_4_TESTSSOXTALKPAT));   //don't use sso, sso+xtalk pattern
    }
    else if (u4TestPattern == TEST_AUDIO_PATTERN)
    {//audio
        //set AUDINIT=0x11 AUDINC=0x0d AUDBITINV=1 AUDMODE=1
        vIO32WriteFldMulti(DRAMC_REG_TEST2_4, \
            P_Fld(0x00000011, TEST2_4_TESTAUDINIT)|P_Fld(0x0000000d, TEST2_4_TESTAUDINC)| \
            P_Fld(0, TEST2_4_TESTXTALKPAT)|P_Fld(1,TEST2_4_TESTAUDMODE)|P_Fld(1,TEST2_4_TESTAUDBITINV));
        //select audio pattern
        vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(1, TEST2_3_TESTAUDPAT)|P_Fld(u4log2loopcount,TEST2_3_TESTCNT));
    }
    else
    {// ISI
        // select ISI pattern
        vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TESTAUDPAT)|P_Fld(u4log2loopcount,TEST2_3_TESTCNT));
        vIO32WriteFldAlign(DRAMC_REG_TEST2_4, 0, TEST2_4_TESTXTALKPAT);
    }

    // 4 enable read or write
    if (wr == TE_OP_READ_CHECK)
    {
        if ((u4TestPattern == 1) || (u4TestPattern == 2))
        {
            //if audio pattern, enable read only (disable write after read), AUDMODE=0x48[15]=0
            vIO32WriteFldAlign(DRAMC_REG_TEST2_4, 0, TEST2_4_TESTAUDMODE);
        }

        // enable read, 0x008[31:29]
        vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TEST2W) | P_Fld(1, TEST2_3_TEST2R) | P_Fld(0, TEST2_3_TEST1));
    }
    else if (wr == TE_OP_WRITE_READ_CHECK)
    {
        // enable write
        vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(1, TEST2_3_TEST2W) | P_Fld(0, TEST2_3_TEST2R) | P_Fld(0, TEST2_3_TEST1));

        // read data compare ready check
        u4loop_count = 0;
        while((u4IO32ReadFldAlign(DRAMC_REG_TESTRPT, TESTRPT_DM_CMP_CPT))==0)
        {
            //ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT), &u4value);
            mcDELAY_US(CMP_CPT_POLLING_PERIOD);
            u4loop_count++;
            if ((u4loop_count > 3) &&(u4loop_count <= MAX_CMP_CPT_WAIT_LOOP))
            {
                //mcSHOW_ERR_MSG(("TESTRPT_DM_CMP_CPT (Write): %d\n", u4loop_count));
            }
            else if(u4loop_count > MAX_CMP_CPT_WAIT_LOOP)
            {
               /*TINFO="fcWAVEFORM_MEASURE_A %d: time out\n", u4loop_count*/
               mcSHOW_DBG_MSG(("test agent write time out : 0x%x \n", u4loop_count));
               break;
            }
        }

        // disable write
        vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TEST2W) | P_Fld(0, TEST2_3_TEST2R) | P_Fld(0, TEST2_3_TEST1));

        mcDELAY_US(1);

        // enable read
        vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TEST2W) | P_Fld(1, TEST2_3_TEST2R) | P_Fld(0, TEST2_3_TEST1));
    }

    // 5
    // read data compare ready check
    u4loop_count = 0;
    while((u4IO32ReadFldAlign(DRAMC_REG_TESTRPT, TESTRPT_DM_CMP_CPT))==0)
    {
            mcDELAY_US(CMP_CPT_POLLING_PERIOD);
            u4loop_count++;
            if ((u4loop_count > 3)&&(u4loop_count <= MAX_CMP_CPT_WAIT_LOOP))
            {
                //mcSHOW_ERR_MSG(("TESTRPT_DM_CMP_CPT (Read): %d\n", u4loop_count));
            }
            else if(u4loop_count > MAX_CMP_CPT_WAIT_LOOP)
            {
               /*TINFO="fcWAVEFORM_MEASURE_B %d: time out\n", u4loop_count*/
               mcSHOW_DBG_MSG(("test agent read time out : 0x%x \n", u4loop_count));
               break;
            }
    }

    // delay 10ns after ready check from DE suggestion (1ms here)
    mcDELAY_US(1);

    // 6
    // return CMP_ERR, 0 is ok ,others are fail,diable test2w or test2r
    // get result
    // or all result
    u4result |= (u4IO32Read4B(DRAMC_REG_CMP_ERR));
    // disable read
    vIO32WriteFldMulti(DRAMC_REG_TEST2_3, P_Fld(0, TEST2_3_TEST2W) | P_Fld(0, TEST2_3_TEST2R) | P_Fld(0, TEST2_3_TEST1));

    vIO32Write4B(DRAMC_REG_DUMMY_RD, uiReg0D0h);

    mcSHOW_DBG_MSG(("cmp_err = 0x%x\n", u4result));

    return u4result;
}

U32 TestEngineCompareNew(DRAMC_CTX_T *p, U32 u4TestBaseAddr, U32 u4TestLen)
{
    U8 jj;
    U32 u4err_value;

	u4err_value = DramcEngine2New(p,  u4TestBaseAddr, u4TestLen, TEST_ISI_PATTERN, TE_OP_WRITE_READ_CHECK);
	//u4err_value |= DramcEngine2New(p, u4TestBaseAddr, u4TestLen, TEST_AUDIO_PATTERN, TE_OP_WRITE_READ_CHECK); //test always fail, mark it
	u4err_value |= DramcEngine2New(p, u4TestBaseAddr, u4TestLen, TEST_XTALK_PATTERN, TE_OP_WRITE_READ_CHECK);

	//mcSHOW_DBG_MSG(("Test agent 2 R/W : base addr = 0x%x, len = 0x%x, cmp_err = 0x%x\n", u4TestBaseAddr, u4TestLen, u4err_value));


	if(u4err_value)
	{
		//mcSHOW_DBG_MSG(("Test agent 2 R/W test FAIL: 0x%x\n", u4err_value));
	}
	else
	{
		//mcSHOW_DBG_MSG(("Test agent 2 R/W test PASS: 0x%x\n", u4err_value));
	}


	#if 1
	u4err_value |= DramcEngine1New(p,  u4TestBaseAddr, u4TestLen);
	//mcSHOW_DBG_MSG(("Test agent 1 R/W : base addr = 0x%x, len = 0x%x, cmp_err = 0x%x\n", u4TestBaseAddr, u4TestLen, u4err_value));
	if(u4err_value)
	{
		//mcSHOW_DBG_MSG(("Test agent 1 R/W test FAIL: 0x%x\n", u4err_value));
	}
	else
	{
		//mcSHOW_DBG_MSG(("Test agent 1 R/W test PASS: 0x%x\n", u4err_value));
	}
	#endif

	return u4err_value;
}
#endif

#if ENABLE_AUTO_DETECTION
void DramcModeRegRead(DRAMC_CTX_T *p, U8 u1MRIdx, U16 *u2pValue)
{
    U32 u4MRValue;
    //vIO32WriteFldAlign(DRAMC_REG_MRS, p->rank, MRS_MRRRK);
    vIO32WriteFldAlign(DRAMC_REG_MRS, u1MRIdx, MRS_MRSMA);

    // MRR command will be fired when MRREN 0->1
    vIO32WriteFldAlign(DRAMC_REG_SPCMD, 1, SPCMD_MRREN);

    // wait MRR command fired.
    while(u4IO32ReadFldAlign(DRAMC_REG_SPCMDRESP, SPCMDRESP_MRR_RESPONSE) ==0)
    {
        mcDELAY_US(1);
    }

    // Read out mode register value
    u4MRValue = u4IO32ReadFldAlign(DRAMC_REG_MRR_STATUS, MRR_STATUS_MRR_REG);
    *u2pValue = (U16)u4MRValue;

    // Set MRREN =0 for next time MRR.
    vIO32WriteFldAlign(DRAMC_REG_SPCMD, 0, SPCMD_MRREN);

    mcSHOW_DBG_MSG3(("Read MR%d =0x%x\n", u1MRIdx, u4MRValue));
}


void DramcModeRegReadByRank(DRAMC_CTX_T *p, U8 u1Rank, U8 u1MRIdx, U16 *u2pValue)
{
    U16 u2Value = 0;
    vIO32WriteFldAlign(DRAMC_REG_MRS, u1Rank, MRS_MRRRK);
    DramcModeRegRead(p,u1MRIdx, &u2Value);
    vIO32WriteFldAlign(DRAMC_REG_MRS, 0, MRS_MRRRK);
    *u2pValue = u2Value;
}
#endif

#if COMPILE_THIS_PART
void DramcModeRegWrite(DRAMC_CTX_T *p, U8 u1MRIdx, U8 u1Value)
{
    U32 u4Rank = 0;
    //vIO32WriteFldAlign(DRAMC_REG_MRS, p->rank, MRS_MRSRK);
    vIO32WriteFldAlign(DRAMC_REG_MRS, u1MRIdx, MRS_MRSMA);
    vIO32WriteFldAlign(DRAMC_REG_MRS, u1Value, MRS_MRSOP);

    // MRW command will be fired when MRWEN 0->1
    vIO32WriteFldAlign(DRAMC_REG_SPCMD, 1, SPCMD_MRWEN);
	
	mcSHOW_DBG_MSG2(("Fire MRW command...\n"));
    // wait MRW command fired.
    while(u4IO32ReadFldAlign(DRAMC_REG_SPCMDRESP, SPCMDRESP_MRW_RESPONSE) ==0)
    {
        mcDELAY_US(1);
    }
	mcSHOW_DBG_MSG2(("ModeReg.%d, value.%x done\n", u1MRIdx, u1Value));
	
    // Set MRWEN =0 for next time MRW.
    vIO32WriteFldAlign(DRAMC_REG_SPCMD, 0, SPCMD_MRWEN);

    u4Rank = u4IO32ReadFldAlign(DRAMC_REG_MRS, MRS_MRSRK);
    mcSHOW_DBG_MSG2(("Write Rank%d MR%d =0x%x\n", u4Rank, u1MRIdx, u1Value));
}

void DramcModeRegWriteByRank(DRAMC_CTX_T *p, U8 u1Rank, U8 u1MRIdx, U8 u1Value)
{
    vIO32WriteFldAlign(DRAMC_REG_MRS, u1Rank, MRS_MRSRK);
    DramcModeRegWrite(p,u1MRIdx, u1Value);
    vIO32WriteFldAlign(DRAMC_REG_MRS, 0, MRS_MRSRK);
}
#endif

#if 1 //COMPILE_PCDDR3
void DramcModeRegWrite_PC3(DRAMC_CTX_T *p, U8 u1MRIdx, U32 u4Value)
{
    //U32 u4Rank = 0;
    //vIO32WriteFldAlign(DRAMC_REG_MRS, p->rank, MRS_MRSRK);
    //mcSHOW_DBG_MSG(("MR:%d, OP:%x\n", u1MRIdx, u1Value));
    vIO32WriteFldAlign(DRAMC_REG_MRS, u1MRIdx, MRS_MRSBA);
    vIO32WriteFldAlign(DRAMC_REG_MRS, u4Value, MRS_MRSMA);

    // MRW command will be fired when MRWEN 0->1
    vIO32WriteFldAlign(DRAMC_REG_SPCMD, 1, SPCMD_MRWEN);

	
	mcSHOW_DBG_MSG2(("Fire MRW command...\n"));
	#if 1//dbg_print
	prom_puts("Fire MRW command...\n");
	#endif
    // wait MRW command fired.
#if 1  //marked for FPGA test
    while(u4IO32ReadFldAlign(DRAMC_REG_SPCMDRESP, SPCMDRESP_MRW_RESPONSE) ==0)
    {
        //cc mark in case too many logs. mcSHOW_DBG_MSG2(("wait MRW command fired\n"));
        mcDELAY_US(1);
    }
#endif
	mcSHOW_DBG_MSG2(("ModeReg.%d, value.%x done\n", u1MRIdx, u4Value));
	#if 1//dbg_print
	prom_puts("ModeReg.");
	prom_print_dec(u1MRIdx);
	prom_puts(", value.0x");
	prom_print_hex(u4Value,8);
	prom_puts(" done\n");
	#endif
    // Set MRWEN =0 for next time MRW.
    vIO32WriteFldAlign(DRAMC_REG_SPCMD, 0, SPCMD_MRWEN);

    //u4Rank = u4IO32ReadFldAlign(DRAMC_REG_MRS, MRS_MRSRK);
    //mcSHOW_DBG_MSG2(("Write Rank%d MR%d =0x%x\n", u4Rank, u1MRIdx, u1Value));
}
#endif

void DramReInitForCalibration(DRAMC_CTX_T *p)
{
#if COMPILE_THIS_PART
    #if WRITE_LEVELING_MOVE_DQS_INSTEAD_OF_CLK
    memset(p->arfgWriteLevelingInitShif, FALSE, sizeof(p->arfgWriteLevelingInitShif));
    //>fgWriteLevelingInitShif= FALSE;
    #endif
    #if TX_PERBIT_INIT_FLOW_CONTROL
    memset(p->fgTXPerbifInit, FALSE, sizeof(p->fgTXPerbifInit));
    #endif
    DramcInit(p);
    vApplyConfigBeforeCalibration(p);
#endif
}

void DramcHWDQSGatingTracking_DVT(DRAMC_CTX_T *p)
{
#ifdef HW_GATING
#if DramcHWDQSGatingTracking_DVT_JADE_TRACKING_MODE
        vIO32WriteFldMulti(DRAMC_REG_STBCAL, P_Fld(0, STBCAL_STB_DQIEN_IG) | P_Fld(0, STBCAL_PICHGBLOCK_NORD) | \
                                                P_Fld(1, STBCAL_REFUICHG) | P_Fld(0, STBCAL_PHYVALID_IG) | \
                                                P_Fld(1, STBCAL_STBSTATE_OPT)| P_Fld(0, STBCAL_STBDLELAST_FILTER) | \
                                                P_Fld(0, STBCAL_STBDLELAST_PULSE) | \
                                                P_Fld(0, STBCAL_STBDLELAST_OPT)|  \
                                                P_Fld(0, STBCAL_PIMASK_RKCHG_OPT));

        vIO32WriteFldMulti(DRAMC_REG_STBCAL1, \
                                                P_Fld(0, STBCAL1_STBCAL_FILTER)| \
                                                P_Fld(1, STBCAL1_STB_FLAGCLR) | \
                                                P_Fld(0, STBCAL1_STB_SHIFT_DTCOUT_IG));

        vIO32WriteFldMulti(DRAMC_REG_SHU1_DQSG, \
                                                P_Fld(0, SHU1_DQSG_STB_UPDMASKCYC) | \
                                                P_Fld(0, SHU1_DQSG_STB_UPDMASK_EN));

        vIO32WriteFldMulti(DDRPHY_MISC_CTRL0, \
                                                P_Fld(0, MISC_CTRL0_R_DMDQSIEN_FIFO_EN) | \
                                                P_Fld(0, MISC_CTRL0_R_DMVALID_DLY)|  \
                                                P_Fld(0, MISC_CTRL0_R_DMVALID_DLY_OPT) | \
                                                P_Fld(0, MISC_CTRL0_R_DMDQSIEN_SYNCOPT));

#else
        vIO32WriteFldMulti(DRAMC_REG_STBCAL, P_Fld(1, STBCAL_STB_DQIEN_IG) | P_Fld(1, STBCAL_PICHGBLOCK_NORD) | \
                                                P_Fld(0, STBCAL_REFUICHG) | P_Fld(0, STBCAL_PHYVALID_IG) | \
                                                P_Fld(0, STBCAL_STBSTATE_OPT)| P_Fld(0, STBCAL_STBDLELAST_FILTER) | \
                                                P_Fld(0, STBCAL_STBDLELAST_PULSE) | \
                                                P_Fld(0, STBCAL_STBDLELAST_OPT)|  \
                                                P_Fld(1, STBCAL_PIMASK_RKCHG_OPT));

        vIO32WriteFldMulti(DRAMC_REG_STBCAL1, \
                                                P_Fld(1, STBCAL1_STBCAL_FILTER)| \
                                                P_Fld(1, STBCAL1_STB_FLAGCLR) | \
                                                P_Fld(1, STBCAL1_STB_SHIFT_DTCOUT_IG));

        vIO32WriteFldMulti(DRAMC_REG_SHU1_DQSG, \
                                                P_Fld(9, SHU1_DQSG_STB_UPDMASKCYC) | \
                                                P_Fld(1, SHU1_DQSG_STB_UPDMASK_EN));

        vIO32WriteFldMulti(DDRPHY_MISC_CTRL0, \
                                                P_Fld(0, MISC_CTRL0_R_DMDQSIEN_FIFO_EN) | \
                                                P_Fld(2, MISC_CTRL0_R_DMVALID_DLY)|  \
                                                P_Fld(1, MISC_CTRL0_R_DMVALID_DLY_OPT) | \
                                                P_Fld(0, MISC_CTRL0_R_DMDQSIEN_SYNCOPT));
#endif

    vIO32WriteFldAlign(DDRPHY_B0_DQ6, 1, B0_DQ6_RG_RX_ARDQ_DMRANK_OUTSEL_B0);
    vIO32WriteFldAlign(DDRPHY_B1_DQ6, 1, B1_DQ6_RG_RX_ARDQ_DMRANK_OUTSEL_B1);
    vIO32WriteFldAlign(DDRPHY_CA_CMD6, 1, CA_CMD6_RG_RX_ARCMD_DMRANK_OUTSEL);
#endif
}

void DramcHWGatingInit(DRAMC_CTX_T *p)
{
#ifdef HW_GATING
    vIO32WriteFldMulti(DRAMC_REG_STBCAL, P_Fld(0, STBCAL_STBCALEN) | \
                                            P_Fld(0, STBCAL_STBCAL2R)| P_Fld(0, STBCAL_STB_SELPHYCALEN) | \
                                            P_Fld(0, STBCAL_STBSTATE_OPT) | \
                                            P_Fld(0, STBCAL_RKCHGMASKDIS)| P_Fld(0, STBCAL_REFUICHG) | \
                                            P_Fld(1, STBCAL_PICGEN));

    vIO32WriteFldMulti(DRAMC_REG_SHURK0_DQSCAL, P_Fld(0, SHURK0_DQSCAL_R0DQSIENLLMTEN) | P_Fld(0, SHURK0_DQSCAL_R0DQSIENHLMTEN));
    vIO32WriteFldMulti(DRAMC_REG_SHURK1_DQSCAL, P_Fld(0, SHURK1_DQSCAL_R1DQSIENLLMTEN) | P_Fld(0, SHURK1_DQSCAL_R1DQSIENHLMTEN));
    vIO32WriteFldMulti(DRAMC_REG_SHU_RANKCTL, P_Fld(1, SHU_RANKCTL_DQSG_MODE));
    vIO32WriteFldMulti(DRAMC_REG_SHU_RANKCTL, P_Fld(1, SHU_RANKCTL_PICGLAT));
    vIO32WriteFldMulti(DRAMC_REG_STBCAL, P_Fld(1, STBCAL_DQSIENCG_CHG_EN));
    vIO32WriteFldAlign(DRAMC_REG_STBCAL, 0, STBCAL_CG_RKEN);
    vIO32WriteFldAlign(DRAMC_REG_STBCAL, 1, STBCAL_DQSIENCG_NORMAL_EN);
    vIO32WriteFldAlign(DDRPHY_MISC_CTRL1, 1, MISC_CTRL1_R_DMDQSIENCG_EN);

    DramcHWDQSGatingTracking_DVT(p);
#endif
}

void DramcHWGatingOnOff(DRAMC_CTX_T *p, U8 u1OnOff)
{
    #ifdef HW_GATING
    vIO32WriteFldAlign(DRAMC_REG_STBCAL, u1OnOff, STBCAL_STBCALEN);
    vIO32WriteFldAlign(DRAMC_REG_STBCAL, u1OnOff, STBCAL_STB_SELPHYCALEN);
    #else
    vIO32WriteFldAlign(DRAMC_REG_STBCAL, 0, STBCAL_STBCALEN);   // PI tracking off = HW gating tracking off
    vIO32WriteFldAlign(DRAMC_REG_STBCAL, 0, STBCAL_STB_SELPHYCALEN);
    #endif
}


#if CPU_RW_TEST_AFTER_K
void DramcPrintHWGatingStatus(DRAMC_CTX_T *p)
{
#ifdef HW_GATING
    U8 u1RankIdx, u1RankMax, u1ByteIdx, ii;
    U32 u4ResultDQS_PI, u4ResultDQS_UI, u4ResultDQS_UI_P1;
    U8 u1Dqs_pi[DQS_BIT_NUMBER], u1Dqs_ui[DQS_BIT_NUMBER],u1Dqs_ui_P1[DQS_BIT_NUMBER];
    U16 u2TmpValue, u2TmpUI[DQS_NUMBER], u2TmpPI[DQS_NUMBER];
    U32 MANUDLLFRZ_bak, STBSTATE_OPT_bak;
    U32 backup_rank;

    backup_rank = u1GetRank(p);

    #if DUAL_RANK_ENABLE
    if (p->support_rank_num==RANK_DUAL)
        u1RankMax = RANK_MAX;
    else
    #endif
         u1RankMax =RANK_1;

    MANUDLLFRZ_bak = u4IO32ReadFldAlign(DRAMC_REG_TEST2_3, TEST2_3_MANUDLLFRZ);
    STBSTATE_OPT_bak = u4IO32ReadFldAlign(DRAMC_REG_STBCAL, STBCAL_STBSTATE_OPT);

    vIO32WriteFldAlign(DRAMC_REG_TEST2_3, 1, TEST2_3_MANUDLLFRZ);
    vIO32WriteFldAlign(DRAMC_REG_STBCAL, 1, STBCAL_STBSTATE_OPT);

    for(u1RankIdx=0; u1RankIdx<u1RankMax; u1RankIdx++)
    {
        vSetRank(p, u1RankIdx);
        mcSHOW_DBG_MSG(("[DramcHWGatingStatus] Rank=%d\n", u1RankIdx));
        u4ResultDQS_PI = u4IO32Read4B(DRAMC_REG_RK0_DQSIENDLY);
        u1Dqs_pi[0]  = u4ResultDQS_PI & 0xff;
        u1Dqs_pi[1]  = (u4ResultDQS_PI >>8) & 0xff;
        u1Dqs_pi[2]  = (u4ResultDQS_PI >>16) & 0xff;
        u1Dqs_pi[3]  = (u4ResultDQS_PI >>24) & 0xff;
        //mcSHOW_DBG_MSG(("[DramcHWGatingStatus] PI status (DQS0, DQS1, DQS2, DQS3) =(%d, %d, %d, %d)\n", u1Dqs_pi[0], u1Dqs_pi[1], u1Dqs_pi[2], u1Dqs_pi[3]));

        u4ResultDQS_UI = u4IO32Read4B(DRAMC_REG_RK0_DQSIENUIDLY);
        u1Dqs_ui[0]  = u4ResultDQS_UI & 0xff;
        u1Dqs_ui[1]  = (u4ResultDQS_UI >>8) & 0xff;
        u1Dqs_ui[2]  = (u4ResultDQS_UI >>16) & 0xff;
        u1Dqs_ui[3]  = (u4ResultDQS_UI >>24) & 0xff;
        //mcSHOW_DBG_MSG(("[DramcHWGatingStatus] UI status (DQS0, DQS1, DQS2, DQS3) =(%d, %d, %d, %d)\n", u1Dqs_ui[0], u1Dqs_ui[1], u1Dqs_ui[2], u1Dqs_ui[3]));
        mcSHOW_DBG_MSG((" Byte0(2T, 0.5T, PI) =(%d, %d, %d)\n Byte1(2T, 0.5T, PI) =(%d, %d, %d)\n Byte2(2T, 0.5T, PI) =(%d, %d, %d)\n Byte3(2T, 0.5T, PI) =(%d, %d, %d)\n", \
                                        u1Dqs_ui[0]/8, u1Dqs_ui[0]%8, u1Dqs_pi[0], u1Dqs_ui[1]/8, u1Dqs_ui[1]%8, u1Dqs_pi[1], \
                                        u1Dqs_ui[2]/8, u1Dqs_ui[2]%8, u1Dqs_pi[2], u1Dqs_ui[3]/8, u1Dqs_ui[3]%8, u1Dqs_pi[3]));


        u4ResultDQS_UI_P1 = u4IO32Read4B(DRAMC_REG_RK0_DQSIENUIDLY_P1);
        u1Dqs_ui_P1[0]  = u4ResultDQS_UI_P1 & 0xff;
        u1Dqs_ui_P1[1]  = (u4ResultDQS_UI_P1 >>8) & 0xff;
        u1Dqs_ui_P1[2]  = (u4ResultDQS_UI_P1 >>16) & 0xff;
        u1Dqs_ui_P1[3]  = (u4ResultDQS_UI_P1 >>24) & 0xff;
        mcSHOW_DBG_MSG((" UI_Phase1 (DQS0~3) =(%d, %d, %d, %d)\n\n", u1Dqs_ui_P1[0], u1Dqs_ui_P1[1], u1Dqs_ui_P1[2], u1Dqs_ui_P1[3]));

    }
    vSetRank(p, backup_rank);
    vIO32WriteFldAlign(DRAMC_REG_STBCAL, STBSTATE_OPT_bak, STBCAL_STBSTATE_OPT);
    vIO32WriteFldAlign(DRAMC_REG_TEST2_3, MANUDLLFRZ_bak, TEST2_3_MANUDLLFRZ);
#endif
}

#endif  //#if (CPU_RW_TEST_AFTER_K)


#if COMPILE_THIS_PART
U8 u1GetMR4RefreshRate(DRAMC_CTX_T *p)
{
    U16 u1Dummy, u1RefreshRate;

    DramcModeRegRead(p, 4, &u1Dummy);
    u1RefreshRate = (U8)u4IO32ReadFldAlign(DRAMC_REG_MISC_STATUSA, MISC_STATUSA_REFRESH_RATE);
    mcSHOW_DBG_MSG(("[u2GetRefreshRate] MR4 0x%x,  u1RefreshRate= 0x%x\n", u1Dummy, u1RefreshRate));

    return u1RefreshRate;
}
#endif

#if COMPILE_THIS_PART
#if ENABLE_DDRPHY_FREQ_METER
void DDRPhyFreqMeter(int fixclk, int monclk_sel)
{
    u32 value = 0;
    if (!((fixclk == 0) || (fixclk == 2)))
    {
        mcSHOW_DBG_MSG(("[DDRPhyFreqMeter] fixclk error\n"));
        ASSERT(0);
    }
    vIO32WriteFldAlign(DDRPHY_SHU1_PLL9, 0x1, SHU1_PLL9_RG_RPHYPLL_MONCK_EN);
    vIO32WriteFldAlign(DDRPHY_SHU1_PLL11, 0x1, SHU1_PLL11_RG_RCLRPLL_MONCK_EN);
    vIO32WriteFldAlign(DDRPHY_SHU1_PLL9, 0x1, SHU1_PLL9_RG_RPHYPLL_MONREF_EN);
    vIO32WriteFldAlign(DDRPHY_SHU1_PLL11, 0x1, SHU1_PLL11_RG_RCLRPLL_MONREF_EN);

#if !FOR_DV_SIMULATION	/* add by cc - line */
    // reset
    DRV_WriteReg32(CKSYS_BASE + 0x10, 1<<14);
    // reset deassert
    DRV_WriteReg32(CKSYS_BASE + 0x10, 0<<14);
    // set window and target
    DRV_WriteReg32(CKSYS_BASE + 0x10, 0x100 | ((monclk_sel & 0x7f) << 16) |
                	((fixclk & 3) << 24) | (1 << 15));
    mcDELAY_US(100);
    value = DRV_Reg32(CKSYS_BASE + 0x14);
    // reset
    DRV_WriteReg32(CKSYS_BASE + 0x10, 1<<14);
    // reset deassert
    DRV_WriteReg32(CKSYS_BASE + 0x10, 0<<14);
    //vIO32WriteFldAlign(DDRPHY_SHU1_PLL9, 0x0, SHU1_PLL9_RG_RPHYPLL_MONCK_EN);
    //vIO32WriteFldAlign(DDRPHY_SHU1_PLL11, 0x0, SHU1_PLL11_RG_RCLRPLL_MONCK_EN);
    //vIO32WriteFldAlign(DDRPHY_SHU1_PLL9, 0x0, SHU1_PLL9_RG_RPHYPLL_MONREF_EN);
    //vIO32WriteFldAlign(DDRPHY_SHU1_PLL11, 0x0, SHU1_PLL11_RG_RCLRPLL_MONREF_EN);
#endif /* add by cc - line */

    if (fixclk == 0)
    {
        mcSHOW_DBG_MSG2(("MEMPLL %d: %dKHz\n", monclk_sel, (26000*value)/(0x100+1)));
    }
    else
    {
        mcSHOW_DBG_MSG2(("MEMPLL %d: %dHz\n", monclk_sel, (32000*value)/(0x100+1)));
    }

    if (fixclk == 0 && monclk_sel == 0x1b)
    {
        mcSHOW_DBG_MSG(("DRAM Clock: %dMHz\n", ((26*value)/(0x100+1)*4)));
    }

}
#endif

#ifdef DDR_INIT_TIME_PROFILING
UINT32 temp;

void TimeProfileBegin(void)
{
    /* enable ARM CPU PMU */
    asm volatile(
        "MRC p15, 0, %0, c9, c12, 0\n"
        "BIC %0, %0, #1 << 0\n"   /* disable */
        "ORR %0, %0, #1 << 2\n"   /* reset cycle count */
        "BIC %0, %0, #1 << 3\n"   /* count every clock cycle */
        "MCR p15, 0, %0, c9, c12, 0\n"
        : "+r"(temp)
        :
        : "cc"
    );
    asm volatile(
        "MRC p15, 0, %0, c9, c12, 0\n"
        "ORR %0, %0, #1 << 0\n"   /* enable */
        "MCR p15, 0, %0, c9, c12, 0\n"
        "MRC p15, 0, %0, c9, c12, 1\n"
        "ORR %0, %0, #1 << 31\n"
        "MCR p15, 0, %0, c9, c12, 1\n"
        : "+r"(temp)
        :
        : "cc"
    );
}

UINT32 TimeProfileEnd(void)
{
    UINT32 div = 897*1024;
    asm volatile(
        "MRC p15, 0, %0, c9, c12, 0\n"
        "BIC %0, %0, #1 << 0\n"   /* disable */
        "MCR p15, 0, %0, c9, c12, 0\n"
        "MRC p15, 0, %0, c9, c13, 0\n"
        : "+r"(temp)
        :
        : "cc"
    );

    //mcSHOW_ERR_MSG((" TimeProfileEnd: %d CPU cycles(%d ms)\n\r", temp, temp/div));
    return (temp/div);
}
#endif

#endif


