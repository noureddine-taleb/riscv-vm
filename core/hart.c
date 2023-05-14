#include <stdio.h>
#include <types.h>
#include <stdlib.h>
#include <string.h>

#include <types.h>
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
							   bus_exec_access, addr,
							   &hart->instruction, sizeof(uxlen));
}

uxlen hart_decode(struct hart *hart);

static uxlen hart_execute(struct hart *hart)
{
	hart->execute(hart);

	/*
	 * clear x0 if any instruction has written into it
	 */
	hart->x[0] = 0;

	return 0;
}

void hart_run(struct hart *hart)
{
	hart->override_pc = 0;

	if (hart_fetch(hart) == 0)
	{
		hart_decode(hart);
		hart_execute(hart);
	}

	/*
	 * increase program counter here
	 */
	hart->pc = hart->override_pc ? hart->override_pc : hart->pc + 4;

	hart->csr_store.cycle++;
	hart->csr_store.instret++;
}
