/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _DRAMC_H
#define _DRAMC_H
#include "dramc_common.h"
#include "x_hal_io.h"
//#include "dramc_pi_api.h"
#include <asm/io.h>
#include <asm/tc3162.h>


#if 0
typedef struct {
    char *name;
    char **factor_tbl;
    char *curr_val;
    char *opt_val;
    void (*factor_handler) (char *);
} tuning_factor;


#define ETT_TUNING_FACTOR_NUMS(x)	(sizeof(x)/sizeof(tuning_factor))
#endif

#define REDUCE_CODE_SIZE
#define RELEASE


//*****************************************
//Define how many steps we have in coarse tune, fine tune
//check the number of  dqsi_gw_dly_fine_tbl and dqsi_gw_dly_coarse_tbl
//To-be-porting
//Yanbing: Modify DRAM_START macro to NAND flash version, which is also compatible for SPI flash.
#define DRAM_START (0x80080000) //(0x80040000)	//YMC chg to map ARM DDR physical addr.
//#define DRAM_START_OLD (0xa0080000) //(0x80000000)

#define DRAM_BASE_ADDR 0x80000000	//YMC chg to map ARM DDR physical addr.

#define isDDR3 ( ((readl(EFUSE_VERIFY_DATA0) >> 16) & 0x1) ? 0 : 1 )

#define CEIL_A_OVER_B(_A, _B)	(((_A)-(_B)*((_A)/(_B))) > 0? (_A)/(_B)+1:(_A)/(_B))

#define _DRAMC0_BASE            0x1fb20000 //     0x10004000
//#define DDRPHY_BASE                 0x10011000
//#define DRAMC_NAO_BASE              0x1020F000


#define DRAMC_WRITE_REG_2(base_addr,offset,val)	writel(val,(base_addr+(offset*4)))

#define DRAMC_READ_REG_2(base_addr,offset)   	readl(base_addr+(offset*4))							   


#define delay_a_while(count) \
	do{	\
	__udelay(count); \
	}while(0)



typedef struct {
    int (*test_case) (unsigned int, unsigned int, void *);
    unsigned int start;
    unsigned int range;
    void *ext_arg;
} test_case;

#define ETT_TEST_CASE_NUMS(x)	(sizeof(x)/sizeof(test_case))

#if 0
#if defined(MT6589)
#define DRAMC_WRITE_REG(val,offset)  do{ \
                                      (*(volatile unsigned int *)(_DRAMC0_BASE + (offset))) = (unsigned int)(val); \
                                      (*(volatile unsigned int *)(DDRPHY_BASE + (offset))) = (unsigned int)(val); \
                                      (*(volatile unsigned int *)(DRAMC_NAO_BASE + (offset))) = (unsigned int)(val); \
                                      }while(0)

#define DRAMC_WRITE_REG_W(val,offset)     do{ \
                                      (*(volatile unsigned int *)(_DRAMC0_BASE + (offset))) = (unsigned int)(val); \
                                      (*(volatile unsigned int *)(DDRPHY_BASE + (offset))) = (unsigned int)(val); \
                                      (*(volatile unsigned int *)(DRAMC_NAO_BASE + (offset))) = (unsigned int)(val); \
                                      }while(0)

#define DRAMC_WRITE_REG_H(val,offset)     do{ \
                                      (*(volatile unsigned short *)(_DRAMC0_BASE + (offset))) = (unsigned short)(val); \
                                      (*(volatile unsigned short *)(DDRPHY_BASE + (offset))) = (unsigned short)(val); \
                                      (*(volatile unsigned short *)(DDRPHY_BASE + (offset))) = (unsigned short)(val); \
                                      }while(0)
#define DRAMC_WRITE_REG_B(val,offset)     do{ \
                                      (*(volatile unsigned char *)(_DRAMC0_BASE + (offset))) = (unsigned char)(val); \
                                      (*(volatile unsigned char *)(DDRPHY_BASE + (offset))) = (unsigned char)(val); \
                                      (*(volatile unsigned char *)(DDRPHY_BASE + (offset))) = (unsigned char)(val); \
                                      }while(0)
#define DRAMC_READ_REG(offset)         ( \
                                        (*(volatile unsigned int *)(_DRAMC0_BASE + (offset))) |\
                                        (*(volatile unsigned int *)(DDRPHY_BASE + (offset))) |\
                                        (*(volatile unsigned int *)(DRAMC_NAO_BASE + (offset))) \
                                       )
#define DRAMC_WRITE_SET(val,offset)     do{ \
                                      (*(volatile unsigned int *)(_DRAMC0_BASE + (offset))) |= (unsigned int)(val); \
                                      (*(volatile unsigned int *)(DDRPHY_BASE + (offset))) |= (unsigned int)(val); \
                                      (*(volatile unsigned int *)(DRAMC_NAO_BASE + (offset))) |= (unsigned int)(val); \
                                      }while(0)

#define DRAMC_WRITE_CLEAR(val,offset)     do{ \
                                      (*(volatile unsigned int *)(_DRAMC0_BASE + (offset))) &= ~(unsigned int)(val); \
                                      (*(volatile unsigned int *)(DDRPHY_BASE + (offset))) &= ~(unsigned int)(val); \
                                      (*(volatile unsigned int *)(DRAMC_NAO_BASE + (offset))) &= ~(unsigned int)(val); \
                                      }while(0)

#define DDRPHY_WRITE_REG(val,offset)    __raw_writel(val, (DDRPHY_BASE + (offset)))
#define DRAMC0_WRITE_REG(val,offset)    __raw_writel(val, (_DRAMC0_BASE + (offset)))
#define DRAMC_NAO_WRITE_REG(val,offset) __raw_writel(val, (DRAMC_NAO_BASE + (offset)))
#else

#endif


#define ETT_TEST_CASE_NUMS(x)	(sizeof(x)/sizeof(test_case))

#define GRAY_ENCODED(a) (a)

#ifndef NULL
#define NULL    0
#endif
#endif

typedef enum {
	ORI_PAT,
	INCR_PAT,
	ANTI_INCR_PAT,
} patType;

#if 0
/* define supported DRAM types */
enum
{
  TYPE_LPDDR2 = 0,
  TYPE_DDR3,
  TYPE_LPDDR3,
  TYPE_mDDR,
};
#endif
#endif  /* !_DRAMC_H */
