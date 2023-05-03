#include <stdio.h>
#include <types.h>
#include <stdlib.h>
#include <string.h>

#include <types.h>
#include <riscv_helper.h>
#include <riscv_instr.h>
#include <soc.h>

#include <hart.h>

void hart_init(struct hart *hart, struct soc *soc, u64 _start)
{
	memset(hart, 0, sizeof(struct hart));

	hart->curr_priv_mode = machine_mode;
	hart->pc = _start;
	hart->soc = soc;
	hart->access_memory = access_protected_memory;

	hart_init_csr_regs(hart);
}

uxlen hart_fetch(struct hart *hart)
{
	uxlen addr = hart->pc;

	return hart->access_memory(hart, hart->curr_priv_mode,
							   bus_instr_access, addr,
							   &hart->instruction, sizeof(uxlen));
}

uxlen hart_decode(struct hart *hart)
{
	hart->opcode = (hart->instruction & 0x7F);
	hart->rd = 0;
	hart->rs1 = 0;
	hart->rs2 = 0;
	hart->func3 = 0;
	hart->func7 = 0;
	hart->immediate = 0;
	hart->jump_offset = 0;

	call_from_opcode_list(hart, &opcode_list_desc, hart->opcode);

	return 0;
}

static uxlen hart_execute(struct hart *hart)
{
	hart->execute_cb(hart);

	/*
	 * clear x0 if any instruction has written into it
	 */
	hart->x[0] = 0;

	return 0;
}

void hart_run(struct hart *hart)
{
	hart->next_pc = 0;

	if (hart_fetch(hart) == 0)
	{
		hart_decode(hart);
		hart_execute(hart);
	}

	/*
	 * increase program counter here
	 */
	hart->pc = hart->next_pc ? hart->next_pc : hart->pc + 4;

	hart->csr_store.cycle++;
	hart->csr_store.instret++;
	hart->csr_store.time++;
}
