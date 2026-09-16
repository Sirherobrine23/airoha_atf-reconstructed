/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Reconstructed from AN7583 BL22 dramc_dvfs.o and cross-checked against
 * the public MediaTek MT8192/MT8195 implementation.
 *
 * These three function sections are byte-identical between AN7581 and AN7583.
 */
#include "dramc_common.h"
#include "dramc_int_global.h"

void vSetDFSTable(DRAMC_CTX_T *p, DRAM_DFS_FREQUENCY_TABLE_T *pFreqTable)
{
	p->pDFSTable = pFreqTable;
}

void vSetDFSFreqSelByTable(DRAMC_CTX_T *p, DRAM_DFS_FREQUENCY_TABLE_T *pFreqTable)
{
	p->pDFSTable = pFreqTable;
	DDRPhyFreqSel(p, pFreqTable->freq_sel);
}

void DVFSSettings(DRAMC_CTX_T *p)
{
	/* Vendor AN7581/AN7583 object is an empty stub. */
	(void)p;
}
