//
//  DRAMC_COMMON.H    
//

#ifndef _DRAMC_COMMON_H_
#define _DRAMC_COMMON_H_

#include "dramc_register.h"
#include "dramc_pi_api.h"

/***********************************************************************/
/*                  Public Types                                       */
/***********************************************************************/

/*------------------------------------------------------------*/
/*                  macros, defines, typedefs, enums          */
/*------------------------------------------------------------*/
/************************** Common Macro *********************/
//#define dsb() __asm__ __volatile__ ("dsb" : : : "memory")

// K2?? : The following needs to be porting.
// choose a proper mcDELAY
//====

/* add by cc - block */
/* To avoid build errors */
#if 0
#define delay_us(x)
#define register_read_c(u4reg_addr)	(0)
#define register_write_c(u4reg_addr, u4val)
#define memset(p, v, n)
#define memcpy(d, s, n)
#endif
#define ASSERT(x)
/* cc add end */

#if __FLASH_TOOL_DA__
    #define mcDELAY_US(x)       gpt4_busy_wait_us(x)
    #define mcDELAY_MS(x)       gpt4_busy_wait_us(x*1000)
#else
#if FOR_DV_SIMULATION
	#define mcDELAY_US(x)		do {if (x > 10) delay_us(10); else delay_us(x);} while (0)
	#define mcDELAY_MS(x)		delay_us(10)
#else
    #define mcDELAY_US(x)       __udelay(x) //pause_polling(x)//gpt_busy_wait_us(x)	//YMC change for BU's delay function
    #define mcDELAY_MS(x)       __udelay(1000*x) //pause_polling(x*1000)//gpt_busy_wait_us(x*1000)	//YMC change for BU's delay function
#endif
#endif


/**********************************************/
/* Priority of debug log                      */
/*--------------------------------------------*/
/* mcSHOW_DBG_MSG: High                       */
/* mcSHOW_DBG_MSG2: Medium High               */
/* mcSHOW_DBG_MSG3: Medium Low                */ 
/* mcSHOW_DBG_MSG4: Low                       */
/**********************************************/
extern U32 u4DRAMdebugLOgEnable;
extern U32 u4DRAMdebugLOgEnable2;

#if __FLASH_TOOL_DA__
  #define printf LOGD
  #define print LOGD
#endif

#define CHIP_VER_E1 1
#define platform_chip_ver()  CHIP_VER_E1

//cc add. If not SIM env, these functions are undefined
#if !FOR_DV_SIMULATION
#define timestamp_show()
#endif

#if FOR_DV_SIMULATION
#define mcSHOW_DBG_MSG(_x_)   {printf _x_;}    
#define mcSHOW_DBG_MSG2(_x_)  {printf _x_;}    
#define mcSHOW_DBG_MSG3(_x_)  {printf _x_;}    
#define mcSHOW_DBG_MSG4(_x_)  {printf _x_;}
#define mcSHOW_USER_MSG(_x_)    
#define mcSHOW_ERR_MSG(_x_)   {printf _x_;}
#define mcFPRINTF(_x_)
#define prom_puts(_x_)        
#define prom_print_dec(_x_)   
#define prom_print_hex(_x_, _y_)
#else
#define mcSHOW_DBG_MSG(_x_)   
#define mcSHOW_DBG_MSG2(_x_)  
#define mcSHOW_DBG_MSG3(_x_)      
#define mcSHOW_DBG_MSG4(_x_)  
#define mcSHOW_USER_MSG(_x_)    
#define mcSHOW_ERR_MSG(_x_)   
#define mcFPRINTF(_x_)
#define prom_puts(_x_)		  printf(_x_)      
#define prom_print_dec(_x_)   printf("%d",_x_)
#define prom_print_hex(_x_, _y_) printf("%x",_x_)
#define BGA 3	//YMC add for EN7523 pkg type
#endif
#define dbg_print 0	//YMC add for BU debug print

extern int dump_log;
#endif   // _DRAMC_COMMON_H_
