#include "dramc.h"
//#include <asm/tc3162.h>
//#include <asm/mipsregs.h>

U32 pkg_type=0,dram_size=0,xtal_sel=PLL_Fin_25M;


/* Description
  *	SW auto detect dram size.
  *	Return actual dram size. Unit:MB
  * Algorithm
  *	- Write a 32-bit data(A) on dram addr:"DRAM_START". 
  *	- Write another 32-bit data(B) on "DRAM_START+check_size". 
  *	- If DRAM size is equal to check_size, addr:"DRAM_START+check_size" is actually not exist.  
  *	- Writing a data on "DRAM_START+check_size" is equivalent to writing a data on "DRAM_START", so A will be replaced by B; otherwise, data on "DRAM_START" is still A. 
  *	- Search for correct DRAM size from minimum supported size to maximum supported size. 
  */
unsigned int _calculate_dram_size(void){
		unsigned int *checkaddr1, *checkaddr2;
		unsigned int check_size	=0x02000000;  //minimum DRAM size=32MB
		unsigned int max_size;
		bool fg_search_done	=0;
		unsigned int val1=0, val2=0, i=0;

		#ifndef RELEASE
		prom_puts("Calculate size.\n");
		#endif
		
		max_size=0x40000000;  //maximum size=1GB
		
		//__asm__ __volatile__("":::"memory");
		while((!fg_search_done)&&(check_size<max_size)){
			checkaddr1=DRAM_START;
			checkaddr2=DRAM_START+check_size;
			writel(0x12345678,checkaddr1);
			__udelay(2);
			writel(0x87654321,checkaddr2);
			__udelay(2);	
			val2=readl(checkaddr2);
			val1=readl(checkaddr1);
			#ifndef RELEASE
			prom_puts("addr=0x");
			prom_print_hex(checkaddr2,8);
			prom_puts("\n");
			prom_print_hex(val2,8);
			prom_puts("\n");
			prom_puts("addr=0x");
			prom_print_hex(checkaddr1,8);
			prom_puts("\n");
			prom_print_hex(val1,8);
			prom_puts("\n");
		    #endif
		
			while(val2!=0x87654321){
				#ifndef RELEASE
				prom_puts("dram r/w error!\n");
				prom_puts("addr=0x");
				prom_print_hex(checkaddr2,8);
				prom_puts("\n");
				prom_print_hex(val2,8);
				prom_puts("\n");
				prom_puts("write again\n");
		   		#endif
				writel(0x87654321,checkaddr2); //write a value in the addr:  (row,bank,column)=(0,0,2^(column_bit))
				val2=readl(checkaddr2);
				val1=readl(checkaddr1);
				#ifndef RELEASE
				prom_puts("addr=0x");
				prom_print_hex(checkaddr2,8);
				prom_puts("\n");
				prom_print_hex(val2,8);
				prom_puts("\n");
				prom_puts("addr=0x");
				prom_print_hex(checkaddr1,8);
				prom_puts("\n");
				prom_print_hex(val1,8);
				prom_puts("\n");
				#endif
				//while(1){};
		}
			if(val1==0x12345678)
				check_size*=2;
			else if(val1==0x87654321)
				fg_search_done=1;
			else{
				prom_puts("dram r/w error!\n");
				prom_puts("addr=0x");
				prom_print_hex(checkaddr1,8);
				prom_puts("\n");
				prom_print_hex(val1,8);
				prom_puts("\n");
				//while(1){};
				return 0;
			}
		}
		//__asm__ __volatile__("":::"memory");
			
		prom_puts("DRAM size=");
		prom_print_dec(check_size>>20);
		prom_puts("MB\n");
		
		return (check_size>>20);
}


void dramc_reg_dump(void)
{
	int i;
	for(i=0;i<256;i++){
		prom_puts("0x");
		prom_print_hex((0x1fb24000+i*4),3);
		prom_puts(":");
		prom_print_hex(DRAMC_READ_REG_2(0x1fb24000,i),8);
		prom_puts("\n");
	}
}

int dramc_main(void) 
{
	U32 val;
#if 0
    unsigned int value, cca;

    /* clear Status.ERL bit, otherwise, if SegCtl.EU bit is set, TLB can't work */
    value = read_c0_status();
    value &= (~(1<<2));
    write_c0_status(value);

    cca = read_c0_config() & 0x7;

	/* SegCtl0 */
	value = /*for virAddr 0xE0000000~0xFFFFFFFF, 3.5~4.0 GB*/
	        ((MIPS_SEGCFG_MK << MIPS_SEGCFG_AM_SHIFT) | 
	        /* When the segment is set as mapped mode, its phyAddr and CCA are decided by TLB,
	         * so the PA and CCA fields can be ignored */
		    (7 << MIPS_SEGCFG_PA_SHIFT) |
		    /* When EU bit is set as 1, make sure that Status.ERL==0, otherwise TLB can't work. 
		     * Because when Status.ERL==1, if EU bit is set, the segment becomes unmapped and uncached.
             * During boot time, Status.ERL==1 is normal */
		    (1 << MIPS_SEGCFG_EU_SHIFT)) |
		    (cca) |
		    /*for virAddr 0xC0000000~0xDFFFFFFF, 3.0~3.5 GB*/
		    (((MIPS_SEGCFG_MSK << MIPS_SEGCFG_AM_SHIFT) | 
		    (6 << MIPS_SEGCFG_PA_SHIFT) |
		    /* When EU bit is set as 1, make sure that Status.ERL==0, otherwise TLB can't work */
		    (1 << MIPS_SEGCFG_EU_SHIFT) |
		    (cca)) << 16);
    write_c0_segCtl0(value);
#endif

	writel( 0x1, 0x1fb00040);
	writel( 0x0, 0x1fb00040);

	//wait for ASIC back to enable
	#if 1
	//prom_print_hex(readl(0x1fb0009c),8);
	//prom_puts("\n");
	val = readl(0x1fb0009c) & 0x1;
	if(val == 1)	//val = 0: FPGA; = 1: ASIC
	{
		xtal_sel = (readl(0x1fa20254)>>19)&0x1;
		pkg_type = (readl(0x1fa20254)>>14)&0x3;
		if(isDDR3)
		{
			DPI_SW_main_PCDDR3();
		}
		else	//isDDR2
		{
			DPI_SW_main_PCDDR2();
		}
	}
	#endif

	#if 0//ndef RELEASE
	dramc_reg_dump();
	#endif
	
	#ifndef RELEASE
	prom_puts("dramc reg setting after calibration:");
	dramc_reg_dump();
	#endif

	//check_column_bank();
	dram_size=_calculate_dram_size();
	SET_DRAM_SIZE(dram_size);
	
	prom_puts("\r\nEN7523DRAMC V0.1\n");
	
	#if 0
	prom_puts("Press HW_RST button a while to bypass dramtest\n");
	__udelay(3000000);
	if(readl(0x1fbf0204)&0x1){
		dramTest();
	}
	#endif
	return 0;
}


