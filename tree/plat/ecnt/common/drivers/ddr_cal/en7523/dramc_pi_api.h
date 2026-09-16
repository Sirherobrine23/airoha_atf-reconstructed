#ifndef _PI_API_H
#define _PI_API_H

/***********************************************************************/
/*              Includes                                               */
/***********************************************************************/

/***********************************************************************/
/*              Constant Define                                        */
/***********************************************************************/

#define __FLASH_TOOL_DA__   0

#if __FLASH_TOOL_DA__
  #include "sys/types.h"
#else
#if 0 //mark cc to fix build errors
  #include "typedefs.h"
#endif
#endif

/* add by cc - block */
#if 1
typedef signed char     INT8;
typedef signed short    INT16;
typedef signed int      INT32;
typedef unsigned char     UINT8;
typedef unsigned short    UINT16;
typedef unsigned int      UINT32;
typedef signed char     S8;
typedef signed short    S16;
typedef signed int      S32;
typedef unsigned char       U8;
typedef unsigned short      U16;
typedef unsigned int        U32;
typedef unsigned long long  U64;
typedef unsigned char       BOOL;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;
typedef unsigned char bool;
typedef unsigned int      *UINT32P;

#define NULL	(void *)0
#define FALSE   0
#define TRUE    1
#endif
/* cc add end */

//Bring Up Selection : Do Not open it when normal operation
//#define FIRST_BRING_UP

//DRAMC Chip
#define EN7580        	1
#define EN7523			2
#define fcFOR_CHIP_ID 	EN7523

#define fcR_EXT 45  	// EXT resistor : 45ohm
#define fcR_OCD 45  	// OCD resistor : 45ohm
#define fcR_ODT 120		// ODT resistor : 120ohm

#define FOR_DV_SIMULATION	0	//YMC change for real chip

//#define DRAM_ETT
//#define ENABLE_CALIBRATION_WINDOW_LOG_FOR_FT //FT log for finding out IC with small window size

#define DDR_AUTO_REBOOT 0
#define DDR_PC4_TCCD	1  // cc changed from 0->1 to avoid CCD error in model. 0 : tCCD = 4, 1: tCCD = 5
//PC4_MR1 = 0x003; //OFF : 0x003, 60 : 0x103, 120 : 0x203, 240 : 0x403, 34 : 0x703
//PC4_MR2 = 0x000; //OFF : 0x000, 120 : 0x200, 240 : 0x400
#define DDR_PC3_MR1		(0x1 << 2)	//RTT_NOM to disable YMC_20180423	//(0x1 << 2) //set RTT_NOM to RZQ/4, i.e, 60ohm
#define DDR_PC4_MR1		(0x003 | (1 << 8)) //cc add (1 << 8) to set RTT_NOM to RZQ/4, i.e, 60ohm

#define DUAL_RANK_ENABLE 0 //cc notes: ECN schematic impilies that only one RANK is enabled
#define CA_TWOTCMD_EN	 1	//cc add: if CA uses 2T command or 1T

//Feature option
#define ENABLE_CA_TRAINING  1
#define ENABLE_WRITE_LEVELING 1

//SW option
#define ENABLE_AUTO_DETECTION 0  //only can enable when DUAL_RANK_ENABLE is 1
#define APPLY_POWER_INIT_SEQUENCE   1
#define COMPILE_THIS_PART			0 //cc define for code size optimize
#define COMPILE_PCDDR3				1
#define COMPILE_PCDDR3_1866			1 //dingyang 1333 need set 0

#define COMPILE_PCDDR2				1


// Sw work around options.
#define WRITE_LEVELING_MOVE_DQS_INSTEAD_OF_CLK 1 // work around for clock multi phase problem(cannot move clk or the clk will be bad)
#define CA_TRAIN_RESULT_DO_NOT_MOVE_CLK 1   // work around for clock multi phase problem(cannot move clk or the clk will be bad)
#define TX_PERBIT_INIT_FLOW_CONTROL 1 // work around for clock multi phase problem(cannot move clk or the clk will be bad)
#define DramcHWDQSGatingTracking_DVT_JADE_TRACKING_MODE 1

//init for test engine
#define DEFAULT_TEST2_1_CAL 0x55000000   // pattern0 and base address for test engine when we do calibration
#if FOR_DV_SIMULATION
#define DEFAULT_TEST2_2_CAL 0xaa000020   //tg try speed up sim
#else
#define DEFAULT_TEST2_2_CAL 0xaa000400   // pattern1 and offset address for test engine when we  do calibraion
#endif

#define CMP_CPT_POLLING_PERIOD 1       // timeout for TE2: (CMP_CPT_POLLING_PERIOD X MAX_CMP_CPT_WAIT_LOOP)
#define MAX_CMP_CPT_WAIT_LOOP 0x10000   // max loop

// common
#define DQS_NUMBER   2 //cc modified from 4-->2
#define DQS_BIT_NUMBER  8
#define DQ_DATA_WIDTH   16 //cc modified from 32->16   // define max support bus width in the system (to allocate array size)
#define TIME_OUT_CNT    100 //100us

// WL
#define SUPPORT_CATRAININT_DELAY_LINE_CALI 0    //support CA0~CA9 delay line calibration
#define SUPPORT_TX_DELAY_LINE_CALI 0

// Gating window
#define DQS_GW_1T_RPRE		1 //cc add. 1 for 1nCK read Preamble. 0 for 2nCK. Generally, <2400 uses 1nCK.
#define DQS_GW_COARSE_STEP	1//8 // for testing// 1
#define DQS_GW_FINE_START 0
#define DQS_GW_FINE_END 32
#define DQS_GW_FINE_STEP 4
#define GATING_ADJUST_TXDLY_FOR_TRACKING  0 //cc changed from 1->0 since PC3 & PC4 do not support Gating Tracking

#define TEST_ODT_ON					1

// DATLAT
#define DATLAT_TAP_NUMBER 32   // DATLAT[3:0] = {0x80[20:4]}

// RX DQ/DQS
#define MAX_RX_DQSDLY_TAPS 127   	// 0x018, May set back to 64 if no need.
#define MAX_RX_DQDLY_TAPS 63      // 0x210~0x22c, 0~15 delay tap
#define DDR4_RDDQC_USED		0
#define DDR3_MPR_USED		0

// TX DQ/DQS
#define TX_DQ_DELAY_BEGIN_LP3 0   // first step to DQ delay
#define TX_DQ_DELAY_END_LP3 31   // first step to DQ delay
#define MAX_TX_DQDLY_TAPS 31   // max DQ TAP number
#define MAX_TX_DQSDLY_TAPS 31   // max DQS TAP number

//Run time config
#define RUN_TIME_CONFIG_ENABLE		1 //cc add
#define HW_GATING //DQS strobe calibration enable
#define DUMMY_READ_FOR_TRACKING
#define ZQCS_ENABLE_LP3 //Lewis@20160606: Enable LP3 ZQCS run time since there is no problem.
//#define TEMP_SENSOR_ENABLE //After enable rumtime MR4 read, Get DramC SPCMDRESP_REFRESH_RATE.
#define APPLY_LOWPOWER_GOLDEN_SETTINGS 1 //Low pwer settings
//#define SPM_CONTROL_AFTERK //Disable in bring-up and enable thereafter.

//#define HW_SAVE_FOR_SR
#define ENABLE_PER_BANK_REFRESH 0

//Debug option
#define GATING_ONLY_FOR_DEBUG 0

#if FOR_DV_SIMULATION
#define CPU_RW_TEST_AFTER_K 0
#else
#define CPU_RW_TEST_AFTER_K 0// need to enable GATING_ONLY_FOR_DEBUG at the same time for gating debug log
#endif
#define LJPLL_FREQ_DEBUG_LOG 0
#define ENABLE_DDRPHY_FREQ_METER 1
#define ENABLE_MIOCK_JMETER  1

#define RX_EYE_SCAN			0
#define TX_EYE_SCAN			0
#define DRAMC_SELFTEST_AFTER_CAL	0

//... add new feature compile option here.
//misc feature

//======================== FIRST_BRING_UP Init Definition =====================
#define FIRST_BRING_UP /* Should be defined only for Bring up */

#undef DRAMC_SELFTEST_AFTER_CAL
#if FOR_DV_SIMULATION
#define DRAMC_SELFTEST_AFTER_CAL	0 //dingyang notes: IC bring up need set 1
#else
#define DRAMC_SELFTEST_AFTER_CAL	1
#endif

#ifdef FIRST_BRING_UP
#undef ORIGINAL_PLL_INIT
#define ORIGINAL_PLL_INIT 0

#undef DUAL_RANK_ENABLE
#define DUAL_RANK_ENABLE 0 //cc notes: bring up EVB only use 1 rank.

#undef ENABLE_CA_TRAINING
#define ENABLE_CA_TRAINING  0

#undef ENABLE_WRITE_LEVELING
#define ENABLE_WRITE_LEVELING 1

#undef ENABLE_PER_BANK_REFRESH
#define ENABLE_PER_BANK_REFRESH 0

#undef HW_GATING
#undef DUMMY_READ_FOR_TRACKING

/* add by cc -- block */
#undef ENABLE_AUTO_DETECTION
#define ENABLE_AUTO_DETECTION 0
/* cc add end */

#undef ENABLE_MIOCK_JMETER
#define ENABLE_MIOCK_JMETER  0 //cc notes: for bring up, Tx perbit adjust disabled, so JM K is not neccessary.
/* cc add */
#undef RUN_TIME_CONFIG_ENABLE
#define RUN_TIME_CONFIG_ENABLE		0 //cc notes: for bring up, run-time tracking shall be disabled.
/* cc add end */

#undef RX_EYE_SCAN
#define RX_EYE_SCAN			0 //cc notes: for DVT, normal function shall be disabled.

#undef TX_EYE_SCAN
#define TX_EYE_SCAN			0 //cc notes: for DVT, normal function shall be disabled.

#endif //FIRST_BRING_UP
//=============================================================================

//======================== ETT Definition =====================================
#if defined(DRAM_ETT) || defined(ENABLE_CALIBRATION_WINDOW_LOG_FOR_FT)
//#define DDR_INIT_TIME_PROFILING
#if DUAL_RANK_ENABLE
#define DUAL_RANK_RX_K
#define DUAL_RANK_TX_K
#endif
#endif

//=============================================================================
#if __FLASH_TOOL_DA__
typedef signed char     INT8;
typedef signed short    INT16;
typedef signed int      INT32;
typedef unsigned char     UINT8;
typedef unsigned short    UINT16;
typedef unsigned int      UINT32;
typedef unsigned int      *UINT32P;
typedef signed char     S8;
typedef signed short    S16;
typedef signed int      S32;
typedef unsigned char       U8;
typedef unsigned short      U16;
typedef unsigned int        U32;
#endif

/***********************************************************************/
/*              Defines                                                */
/***********************************************************************/
#define ENABLE  1
#define DISABLE 0

#define CBT_LOW_FREQ   0
#define CBT_HIGH_FREQ   1

typedef enum
{
    DRAM_OK = 0, // OK
    DRAM_FAIL    // FAIL
} DRAM_STATUS_T; // DRAM status type

typedef enum
{
    PLL_Fin_20M = 0,	//YMC chg for Xtal_SEL use
    PLL_Fin_25M,		//YMC chg for Xtal_SEL use
    CLOCK_SRC_T_MAX		//YMC chg for Xtal_SEL use
}CLOCK_SRC_T;

typedef enum
{
    DDR_DDR1066 = 1066,
	DDR_DDR1200 = 1200,
    DDR_DDR1333 = 1333,
    DDR_DDR1600 = 1600,  
    DDR_DDR1866 = 1866,
    PLL_FREQ_SEL_MAX
} DRAM_PLL_FREQ_SEL_T; // DRAM DFS type

typedef enum
{
    DRAM_CALIBRATION_ZQ = 0,
    DRAM_CALIBRATION_SW_IMPEDANCE ,
    DRAM_CALIBRATION_CA_TRAIN ,
    DRAM_CALIBRATION_WRITE_LEVEL,
    DRAM_CALIBRATION_GATING,
    DRAM_CALIBRATION_DATLAT,
    DRAM_CALIBRATION_RX_RDDQC,
    DRAM_CALIBRATION_RX_PERBIT,
    DRAM_CALIBRATION_TX_PERBIT,
    DRAM_CALIBRATION_MAX
} DRAM_CALIBRATION_STATUS_T;

typedef enum
{
    RANK_0= 0,
#if (DUAL_RANK_ENABLE == 1) // cc add: 7580 may support dual-rank DDR3
	RANK_1,
#endif
    RANK_MAX
} DRAM_RANK_T;

typedef enum
{
    RANK_SINGLE = 1,
    RANK_DUAL
} DRAM_RANK_NUMBER_T;

typedef enum
{
    TYPE_DDR1 = 1,
    TYPE_LPDDR2,
    TYPE_LPDDR3,
    TYPE_PCDDR2,
    TYPE_PCDDR3,
    TYPE_PCDDR4
} DRAM_DRAM_TYPE_T;

typedef enum
{
    DRAM_COMSETTING_DEFAULT = 0,
    DRAM_COMSETTING_DDR3_X4,
    DRAM_COMSETTING_DDR3_X8,
    DRAM_COMSETTING_DDR3_X16,
    DRAM_COMSETTING_DDR3_X32,
    DRAM_COMSETTING_DDR4_X16,
    DRAM_COMSETTING_DDR4_X32,
    DRAM_COMSETTING_LP3_178BALL,
    DRAM_COMSETTING_LP3_168BALL,
    DRAM_COMSETTING_DDR3_ASYM,
    DRAM_COMSETTING_DDR4_ASYM,  
    DRAM_COMSETTING_MAX
} DRAM_COMMONSETTING_T;

typedef enum { 
    DRAM_ADC_SIZE_1GB = 0,
    DRAM_ADC_SIZE_2GB,
    DRAM_ADC_SIZE_1_5GB,
    DRAM_ADC_SIZE_512MB,
    DRAM_ADC_SIZE_MAX
}DRAM_ADC_SIZE_T;

typedef enum
{
    CBT_NORMAL_MODE = 0,
    CBT_BYTE_MODE1
} DRAM_CBT_MODE_T;

typedef enum
{
    ODT_OFF = 0,
    ODT_ON
} DRAM_ODT_MODE_T;


typedef enum
{
    DBI_OFF = 0,
    DBI_ON
} DRAM_DBI_MODE_T;

typedef enum
{
    DATA_WIDTH_16BIT = 16,
    DATA_WIDTH_32BIT = 32
} DRAM_DATA_WIDTH_T;

// for A60501 DDR3
typedef enum
{
    PCB_LOC_ASIDE = 0,
    PCB_LOC_BSIDE
} DRAM_PCB_LOC_T;


typedef enum
{
    PACKAGE_SBS = 0,
    PACKAGE_POP
} DRAM_PACKAGE_T;

typedef enum
{
    TE_OP_WRITE_READ_CHECK = 0,
    TE_OP_READ_CHECK
} DRAM_TE_OP_T;

typedef enum
{
    TEST_ISI_PATTERN = 0,
    TEST_AUDIO_PATTERN,
    TEST_TA1_SIMPLE,
    TEST_TESTPAT4,
    TEST_TESTPAT4_3,
    TEST_XTALK_PATTERN,
    TEST_MIX_PATTERN,
    TEST_DMA,
} DRAM_TEST_PATTERN_T;

typedef enum
{
    BL_TYPE_4 = 0,
    BL_TYPE_8
} DRAM_BL_TYPE_T;

// used for record last test pattern in TA
typedef enum
{
    TA_PATTERN_IDLE,
    TA_PATTERN_TA43,
    TA_PATTERN_TA4,
    TA_PATTERN_UNKNOWM,
} DRAM_TA_PATTERN_T;

typedef enum
{
    DMA_OP_PURE_READ,
    DMA_OP_PURE_WRITE,
    DMA_OP_READ_WRITE,
} DRAM_DMA_OP_T;

typedef enum
{
    DMA_PREPARE_DATA_ONLY,
    DMA_CHECK_DATA_ACCESS_ONLY_AND_NO_WAIT,
    DMA_CHECK_COMAPRE_RESULT_ONLY,
    DMA_CHECK_DATA_ACCESS_AND_COMPARE,
} DRAM_DMA_CHECK_RESULT_T;


typedef enum
{
    fcDATLAT_USE_DEFAULT = 0,
    fcDATLAT_USE_RX_SCAN,
    //fcDATLAT_USE_TXRX_SCAN
}DRAM_DATLAT_CALIBRATION_TYTE_T;


typedef enum
{
    TX_DQ_DQS_MOVE_DQ_ONLY = 0,
    TX_DQ_DQS_MOVE_DQM_ONLY,
    TX_DQ_DQS_MOVE_DQ_DQM
}DRAM_TX_PER_BIT_CALIBRATION_TYTE_T;

#if 0//cc mark (fcFOR_CHIP_ID == MT8167)
typedef enum
{ //p->pinmux ID
    PIN_MUX_TYPE_DDR3X16X2 = 0,
    PIN_MUX_TYPE_DDR4X16X2,
    PIN_MUX_TYPE_LPDDR3_178BALL,
    PIN_MUX_TYPE_LPDDR3_168BALL,
    PIN_MUX_TYPE_DDR3X8,
    PIN_MUX_TYPE_LPDDR3_211BALL,
    PIN_MUX_TYPE_DDR3X4,
    PIN_MUX_TYPE_MAX
} PIN_MUX_TYPE_T;
#elif (fcFOR_CHIP_ID == EN7523) // cc add
typedef enum
{
	PIN_MUX_TYPE_DDR3KGD = 0,
	PIN_MUX_TYPE_DDR3X16, // single rank
	PIN_MUX_TYPE_DDR3X16x2, // dual rank
	PIN_MUX_TYPE_DDR2DDR3X16, // single rank, YMC add for EN7523 new type
	PIN_MUX_TYPE_DDR2KGD,	// single rank, YMC add for EN7523 new type
	PIN_MUX_TYPE_DDR4X16,
	//PIN_MUX_TYPE_DDR3X4, //cc notes: 4bit mode?
	PIN_MUX_TYPE_MAX
}PIN_MUX_TYPE_T;
#endif // cc add end

////////////////////////////
typedef struct _DRAMC_CTX_T
{
    DRAM_RANK_NUMBER_T support_rank_num;
    DRAM_RANK_T rank;
    DRAM_DRAM_TYPE_T dram_type;
    DRAM_ODT_MODE_T odt_onoff;
    DRAM_CBT_MODE_T dram_cbt_mode;
    DRAM_DBI_MODE_T DBI_R_onoff;
    DRAM_DBI_MODE_T DBI_W_onoff;
    DRAM_PACKAGE_T package;
    DRAM_DATA_WIDTH_T data_width;
    U32 test2_1;
    U32 test2_2;
    DRAM_TEST_PATTERN_T test_pattern;
    DRAM_PLL_FREQ_SEL_T frequency;
	CLOCK_SRC_T	clk_src;		//YMC add for Xtal_SEL use
    U8 vendor_id;
    U16 density;
    U8 fglow_freq_write_en;
    U8 ssc_en;
    U8 en_4bitMux;
    U8 enable_rx_scan_vref;
    U8 enable_tx_scan_vref;
    PIN_MUX_TYPE_T pinmux;
    U8 asymmetric;
    U16 min_winsize;
    U16 sum_winsize;
    U32 aru4CalResultFlag[RANK_MAX];// record the calibration is fail or success,  0:success, 1: fail
    U32 aru4CalExecuteFlag[RANK_MAX]; // record the calibration is execute or not,  0:no operate, 1: done

    #if WRITE_LEVELING_MOVE_DQS_INSTEAD_OF_CLK
    BOOL arfgWriteLevelingInitShif[RANK_MAX];
    #endif
    #if TX_PERBIT_INIT_FLOW_CONTROL
    BOOL fgTXPerbifInit[RANK_MAX];
    #endif
} DRAMC_CTX_T;

typedef struct _PASS_WIN_DATA_T
{
    S16 first_pass;
    S16 last_pass;
    S16 win_center;
    U16 win_size;
    U16 best_dqdly;
} PASS_WIN_DATA_T;

typedef struct _VrefDQ_WIN_T
{
    U16 VrefValue;
    U16 TxWin;
} VrefDQ_WIN_T;

#if 0
typedef struct _RXDQS_PERBIT_DLY_T
{
    S8 first_dqdly_pass;
    S8 last_dqdly_pass;
    S8 first_dqsdly_pass;
    S8 last_dqsdly_pass;
    U8 best_dqdly;
    U8 best_dqsdly;
} RXDQS_PERBIT_DLY_T;
#endif
typedef struct _TXDQS_PERBIT_DLY_T
{
    S8 first_dqdly_pass;
    S8 last_dqdly_pass;
    S8 first_dqsdly_pass;
    S8 last_dqsdly_pass;
    U8 best_dqdly;
    U8 best_dqsdly;
} TXDQS_PERBIT_DLY_T;

typedef struct _DRAM_INFO_BY_MRR_T
{
    U16 u2MR5VendorID;
    U8 u4MR8DieNumber[RANK_MAX];
    U32 u4MR8Density[RANK_MAX];
} DRAM_INFO_BY_MRR_T;


/////////////////////////////////// SAVE TO SRAM FORMAT ////////////////////////////////////////////
typedef struct _SAVE_TO_SRAM_FORMAT_PASS_WIN_DATA_T
{
    S16 first_pass;
    S16 last_pass;
    S16 win_center;
    U16 win_size;
    U16 left_margin;
    U16 right_margin;
} SAVE_TO_SRAM_FORMAT_PASS_WIN_DATA_T;

typedef struct _SAVE_TO_SRAM_FORMAT_SwImpedanceCal_T
{
    U8  DRVP;
    U8  DRVN;
	U8  ODTP;
    U8  ODTN;
    U8  Reserved_1; //sagy
    U32 Reserved_2; //sagy
} SAVE_TO_SRAM_FORMAT_SwImpedanceCal_T;

typedef struct _SAVE_TO_SRAM_FORMAT_CaTraining_T
{
    U8  CA_delay;
    U8  Clk_delay;
    U8  CS_delay;
    U8  Reserved[4];
    SAVE_TO_SRAM_FORMAT_PASS_WIN_DATA_T WinPerBit[10];
}SAVE_TO_SRAM_FORMAT_CaTraining_T;

typedef struct _SAVE_TO_SRAM_FORMAT_Write_Leveling_T
{
    U8  CA_delay;
    U8  Clk_delay;
    U8  Final_Clk_delay;
    U8  DQS0_delay;
    U8  DQS1_delay;
    U8  DQS2_delay;
    U8  DQS3_delay;
    U8  Reserved[1];
}SAVE_TO_SRAM_FORMAT_Write_Leveling_T;

typedef struct _SAVE_TO_SRAM_FORMAT_GatingWindow_T
{
    U8  DQS0_2T;
    U8  DQS0_05T;
    U8  DQS0_PI;
    U8  DQS1_2T;
    U8  DQS1_05T;
    U8  DQS1_PI;
    U8  Reserved0[2];

    U8  DQS2_2T;
    U8  DQS2_05T;
    U8  DQS2_PI;
    U8  DQS3_2T;
    U8  DQS3_05T;
    U8  DQS3_PI;
    U8  Reserved1[2];

    U8  DQS0_RODT_2T;
    U8  DQS0_RODT_05T;
    U8  DQS1_RODT_2T;
    U8  DQS1_RODT_05T;
    U8  DQS2_RODT_2T;
    U8  DQS2_RODT_05T;
    U8  DQS3_RODT_2T;
    U8  DQS3_RODT_05T;
    u8  Gating_Win[4];
}SAVE_TO_SRAM_FORMAT_GatingWindow_T;

typedef struct _SAVE_TO_SRAM_FORMAT_RXWinDowPerbitCal_T
{
    U8  DQS0_delay;
    U8  DQS1_delay;
    U8  DQS2_delay;
    U8  DQS3_delay;
    U8  DQM0_delay;
    U8  DQM1_delay;
    U8  DQM2_delay;
    U8  DQM3_delay;

    U8  DQ_delay[32];
    SAVE_TO_SRAM_FORMAT_PASS_WIN_DATA_T WinPerBit[32];

}SAVE_TO_SRAM_FORMAT_RXWinDowPerbitCal_T;

typedef struct _SAVE_TO_SRAM_FORMAT_TxWindowPerbitCal_T
{
    U8  PI_DQ_delay[4];
    U8  Large_UI[4];
    U8  Small_UI[4];
    U8  PI[4];
    SAVE_TO_SRAM_FORMAT_PASS_WIN_DATA_T WinPerBit[32];
}SAVE_TO_SRAM_FORMAT_TxWindowPerbitCal_T;

typedef struct _SAVE_TO_SRAM_FORMAT_DATLAT_T
{
    U8  best_step;
    U8  Reserved[7];
}SAVE_TO_SRAM_FORMAT_DATLAT_T;

typedef struct _SAVE_TO_SRAM_FORMAT_RANK_T
{
    SAVE_TO_SRAM_FORMAT_SwImpedanceCal_T    SwImpedanceCal;

    SAVE_TO_SRAM_FORMAT_CaTraining_T    CaTraining;

    SAVE_TO_SRAM_FORMAT_Write_Leveling_T    Write_Leveling;

    SAVE_TO_SRAM_FORMAT_GatingWindow_T  GatingWindow;

    SAVE_TO_SRAM_FORMAT_TxWindowPerbitCal_T TxWindowPerbitCal;

    SAVE_TO_SRAM_FORMAT_DATLAT_T    DATLAT;

    SAVE_TO_SRAM_FORMAT_RXWinDowPerbitCal_T RxWindowPerbitCal;
} SAVE_TO_SRAM_FORMAT_RANK_T;

typedef struct _SAVE_TO_SRAM_FORMAT_RxdqsGatingPostProcess_T
{
    U8  s1ChangeDQSINCTL;
    U8  reg_TX_dly_DQSgated_min;
    U8  u1TXDLY_Cal_min;
    U8  TX_dly_DQSgated_check_min;
    U8  TX_dly_DQSgated_check_max;
    U8  DQSINCTL;
    U8  RANKINCTL;
    U8  u4XRTR2R;
}SAVE_TO_SRAM_FORMAT_RxdqsGatingPostProcess_T;

typedef struct _SAVE_TO_SRAM_FORMAT_DualRankRxdatlatCal_T
{
    U8  Rank0_Datlat;
    U8  Rank1_Datlat;
    U8  Final_Datlat;
    U8  Reserved[5];
}SAVE_TO_SRAM_FORMAT_DualRankRxdatlatCal_T;

typedef struct _SAVE_TO_SRAM_FORMAT_CHANNEL_T
{
    SAVE_TO_SRAM_FORMAT_RANK_T                      RANK[RANK_MAX];
    SAVE_TO_SRAM_FORMAT_RxdqsGatingPostProcess_T    RxdqsGatingPostProcess;
    SAVE_TO_SRAM_FORMAT_DualRankRxdatlatCal_T       DualRankRxdatlatCal;
} SAVE_TO_SRAM_FORMAT_CHANNEL_T;

//For new register access
#define SYS_REG_ADDR(offset)    (offset)

/***********************************************************************/
/*              External declarations                                  */
/***********************************************************************/
extern U8 RXPERBIT_LOG_PRINT;
#if !FOR_DV_SIMULATION	//YMC add for EN7523 pkg type & xtal_sel.
extern U32 pkg_type;
extern U32 xtal_sel;
#endif
/***********************************************************************/
/*              Public Functions                                       */
/***********************************************************************/
// basic function
DRAM_STATUS_T DramcInit(DRAMC_CTX_T *p);
void DramcSetRankEngine2(DRAMC_CTX_T *p, U8 u1RankSel);
U32 DramcEngine1(DRAMC_CTX_T *p, U32 test2_1, U32 test2_2, S16 loopforever, U8 period);
U32 DramcEngine2(DRAMC_CTX_T *p, DRAM_TE_OP_T wr, U32 test2_1, U32 test2_2, U8 testaudpat, S16 loopforever, U8 period, U8 log2loopcount);
DRAM_STATUS_T DramcEngine2Init(DRAMC_CTX_T *p, U32 test2_1, U32 test2_2, U8 testaudpat, U8 log2loopcount);
U32 DramcEngine2Run(DRAMC_CTX_T *p, DRAM_TE_OP_T wr, U8 testaudpat);
void DramcEngine2End(DRAMC_CTX_T *p);

U32 TestEngineCompare(DRAMC_CTX_T *p);
extern U32 DramcEngine2New(DRAMC_CTX_T *p, U32 u4TestBaseAddr, U32 u4TestLen, U8 u4TestPattern, DRAM_TE_OP_T wr);
extern U32 TestEngineCompareNew(DRAMC_CTX_T *p, U32 u4TestBaseAddr, U32 u4TestLen);
void DramcEnterSelfRefresh(DRAMC_CTX_T *p, U8 op);
void DramcRunTimeConfig(DRAMC_CTX_T *p);
void TransferToSPMControl(DRAMC_CTX_T *p);
void Switch26MHzDisableDummyReadRefreshAllBank(DRAMC_CTX_T *p);
void EnableDramcPhyDCM(DRAMC_CTX_T *p, bool bEn);

DRAM_STATUS_T DramcRegDump(DRAMC_CTX_T *p);
void DramcModeRegReadByRank(DRAMC_CTX_T *p, U8 u1Rank, U8 u1MRIdx, U16 *u2pValue);
void DramcModeRegRead(DRAMC_CTX_T *p, U8 u1MRIdx, U16 *u1pValue);
void DramcModeRegWrite(DRAMC_CTX_T *p, U8 u1MRIdx, U8 u1Value);
void DramcModeRegWrite_PC3(DRAMC_CTX_T *p, U8 u1MRIdx, U32 u1Value);
extern void DramcModeRegWrite_PC4(DRAMC_CTX_T *p, U8 u1BGIdx, U8 u1MRIdx, U32 u1Value);

void DramPhyReset(DRAMC_CTX_T *p);
U8 u1GetMR4RefreshRate(DRAMC_CTX_T *p);

// mandatory calibration function
DRAM_STATUS_T DramcSwImpedanceCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcWriteLevelingLP3(DRAMC_CTX_T *p);
#if GATING_ONLY_FOR_DEBUG
void DramcGatingDebugInit(DRAMC_CTX_T *p);
void DramcGatingDebugRankSel(DRAMC_CTX_T *p, U8 u1Rank);
void DramcGatingDebug(DRAMC_CTX_T *p);
void DramcGatingDebugExit(DRAMC_CTX_T *p);
#endif
void Global_Option_Init(DRAMC_CTX_T *p);
#if ENABLE_MIOCK_JMETER
DRAM_STATUS_T DramcMiockJmeter(DRAMC_CTX_T *p);
#endif

DRAM_STATUS_T DramcWriteLeveling4Bit(DRAMC_CTX_T *p, U8 u1DlyLine);
DRAM_STATUS_T DramcWriteLeveling(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcRxdqsGatingCal_PCDDR3(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcRxdqsGatingCal_PCDDR4(DRAMC_CTX_T *p);

DRAM_STATUS_T DramcRxdatlatCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcRxWindowPerbitCal(DRAMC_CTX_T *p, U8 u1UseTestEngine);
DRAM_STATUS_T DramcTxWindowPerbitCal(DRAMC_CTX_T *p, DRAM_TX_PER_BIT_CALIBRATION_TYTE_T calType);
DRAM_STATUS_T DramcRxdqsGatingPreProcess(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcRxdqsGatingPostProcess(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcDualRankRxdatlatCal(DRAMC_CTX_T *p);
void v4bitPinmuxEnableVerify(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcTxVrefDQCalibration(DRAMC_CTX_T *p);
void vApplyConfigBeforeCalibration(DRAMC_CTX_T *p);
void vApplyConfigAfterCalibration(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcRxdatlatCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcCATraining(DRAMC_CTX_T *p);
void DramcHWGatingInit(DRAMC_CTX_T *p);
void DramcHWGatingOnOff(DRAMC_CTX_T *p, U8 u1OnOff);
void DramcPrintHWGatingStatus(DRAMC_CTX_T *p);

#if 0  //no use now, disable for saving code size.
void vGetCalibrationResult(DRAMC_CTX_T *p, U8 ucCalType, U8 *ucCalExecute, U8 *ucCalResult); // get result of specific calibration
#endif
// get result of all calibration of specific rank.
void vGetCalibrationResult_All(DRAMC_CTX_T *p, U8 u1Rank, U32 *u4CalExecute, U32 *u4CalResult);
void vPrintCalibrationResult(DRAMC_CTX_T *p);
extern void Dump_Debug_Registers(DRAMC_CTX_T *p, int index);

// reference function
DRAM_STATUS_T DramcRankSwap(DRAMC_CTX_T *p, U8 u1Rank);

// DDR reserved mode function
void Dramc_DDR_Reserved_Mode_setting(void);

#if ENABLE_DDRPHY_FREQ_METER
extern void DDRPhyFreqMeter(int fixclk, int monclk_sel);
#else
#define DDRPhyFreqMeter(_x_)
#endif

extern void SaveCurrDramCtx(DRAMC_CTX_T *p);

void vSetRankNumber(DRAMC_CTX_T *p);
void vSetRank(DRAMC_CTX_T *p, U8 ucRank);
U8 u1GetRank(DRAMC_CTX_T *p);
void vIO32WriteFldAlign_Phy_All(U32 reg32, U32 val32, U32 fld);
// Global variables

#ifdef DDR_INIT_TIME_PROFILING
void TimeProfileBegin(void);
UINT32 TimeProfileEnd(void);
#endif

extern U8 u1MR2Value;

#endif // _PI_API_H
