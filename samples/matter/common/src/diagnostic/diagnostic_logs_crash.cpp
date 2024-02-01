/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "diagnostic_logs_crash.h"

#include <cstdlib>
#include <zephyr/kernel.h>
#include <zephyr/retention/retention.h>

using namespace Nrf;

#define Verify(expr, value)                                                                                            \
	do {                                                                                                           \
		if (!(expr)) {                                                                                         \
			return (value);                                                                                \
		}                                                                                                      \
	} while (false)

size_t CrashData::CalculateSize()
{
	bool unusedOut;
	return ProcessConversionToLog(nullptr, 0, unusedOut);
}

size_t CrashData::ProcessConversionToLog(char *outBuffer, size_t bufferSize, bool &end)
{
	/* Allow to enter outBuffer as nullptr to calculate actual size.
	if buffer is available, its size cannot be 0 */
	if (outBuffer && bufferSize == 0) {
		return 0;
	}
	mOutBufferSize = bufferSize;
	mOutBuffer = outBuffer;
	mOffset = 0;
	mLine = 0;
	end = false;

	/* Process all stages one by one until reaching the buffer full or the end of the logs. */
	Verify(BasicDump(), mOffset);
	Verify(FpuDump(), mOffset);
	Verify(ExtraDump(), mOffset);
	Verify(ReasonDump(), mOffset);
	Verify(MultithreadDump(), mOffset);
	end = true;

	return mOffset;
}

bool CrashData::BasicDump()
{
	Verify(Collect("Faulting instruction address (r15/pc): 0x%08x\n", mDescription.Esf.basic.pc), false);
	Verify(Collect("r0/a1:  0x%08x  r1/a2:  0x%08x  r2/a3:  0x%08x\n", mDescription.Esf.basic.a1,
		       mDescription.Esf.basic.a2, mDescription.Esf.basic.a3),
	       false);
	Verify(Collect("r3/a4:  0x%08x r12/ip:  0x%08x r14/lr:  0x%08x\n", mDescription.Esf.basic.a4,
		       mDescription.Esf.basic.ip, mDescription.Esf.basic.lr),
	       false);
	Verify(Collect(" xpsr:  0x%08x\n", mDescription.Esf.basic.xpsr), false);

	return true;
}

bool CrashData::FpuDump()
{
#if defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	for (size_t i = 0; i < ARRAY_SIZE(mDescription.Esf.fpu.s); i += sizeof(uint32_t)) {
		Verify(Collect("s[%2d]:  0x%08x  s[%2d]:  0x%08x"
			       "  s[%2d]:  0x%08x  s[%2d]:  0x%08x\n",
			       i, (uint32_t)mDescription.Esf.fpu.s[i], i + 1, (uint32_t)mDescription.Esf.fpu.s[i + 1],
			       i + 2, (uint32_t)mDescription.Esf.fpu.s[i + 2], i + 3,
			       (uint32_t)mDescription.Esf.fpu.s[i + 3]),
		       false);
	}
#ifdef CONFIG_VFP_FEATURE_REGS_S64_D32
	for (size_t i = 0; i < ARRAY_SIZE(mDescription.Esf.fpu.d); i += sizeof(uint32_t)) {
		Verify(Collect("d[%2d]:  0x%16llx  d[%2d]:  0x%16llx"
			       "  d[%2d]:  0x%16llx  d[%2d]:  0x%16llx\n",
			       i, (uint64_t)mDescription.Esf.fpu.d[i], i + 1, (uint64_t)mDescription.Esf.fpu.d[i + 1],
			       i + 2, (uint64_t)mDescription.Esf.fpu.d[i + 2], i + 3,
			       (uint64_t)mDescription.Esf.fpu.d[i + 3]),
		       false);
	}
#endif
	Verify(Collect("fpscr:  0x%08x\n", mDescription.Esf.fpu.fpscr), false);
#endif

	return true;
}

bool CrashData::ExtraDump()
{
#if defined(CONFIG_EXTRA_EXCEPTION_INFO)
	const struct _callee_saved *callee = mDescription.Esf.extra_info.callee;

	if (callee != NULL) {
		Verify(Collect("r4/v1:  0x%08x  r5/v2:  0x%08x  r6/v3:  0x%08x\n", callee->v1, callee->v2, callee->v3),
		       false);
		Verify(Collect("r7/v4:  0x%08x  r8/v5:  0x%08x  r9/v6:  0x%08x\n", callee->v4, callee->v5, callee->v6),
		       false);
		Verify(Collect("r10/v7: 0x%08x  r11/v8: 0x%08x    psp:  0x%08x\n", callee->v7, callee->v8, callee->psp),
		       false);
	}

	Verify(Collect("EXC_RETURN: 0x%0x\n", mDescription.Esf.extra_info.exc_return), false);

#endif /* CONFIG_EXTRA_EXCEPTION_INFO */

	return true;
}

bool CrashData::ReasonDump()
{
	Verify(Collect("***** %s *****\n", FaultSourceToStr(mDescription.Source)), false);
	Verify(Collect("ZEPHYR FATAL ERROR %d: %s on CPU %d\n", mDescription.Reason, ReasonToStr(mDescription.Reason),
		       _current_cpu->id),
	       false);

	return true;
}

bool CrashData::MultithreadDump()
{
#ifdef CONFIG_MULTITHREADING
	if (mDescription.ThreadName && mDescription.ThreadInt) {
		Verify(Collect("Current thread: %p (%s)\n", mDescription.ThreadInt, mDescription.ThreadName), false);
	}
#endif

	return true;
}

const char *CrashData::ReasonToStr(unsigned int reason)
{
	switch (reason) {
	case K_ERR_CPU_EXCEPTION:
		return "CPU exception";
	case K_ERR_SPURIOUS_IRQ:
		return "Unhandled interrupt";
	case K_ERR_STACK_CHK_FAIL:
		return "Stack overflow";
	case K_ERR_KERNEL_OOPS:
		return "Kernel oops";
	case K_ERR_KERNEL_PANIC:
		return "Kernel panic";
	case K_ERR_ARM_USAGE_NO_COPROCESSOR:
		return "No coprocessor instructions";
	case K_ERR_ARM_USAGE_ILLEGAL_EXC_RETURN:
		return "Illegal load of EXC_RETURN into PC";
	case K_ERR_ARM_USAGE_ILLEGAL_EPSR:
		return "Illegal use of the EPSR";
	case K_ERR_ARM_USAGE_UNDEFINED_INSTRUCTION:
		return "Attempt to execute undefined instruction";
	case K_ERR_ARM_USAGE_STACK_OVERFLOW:
		return "Stack overflow (context area not valid)";
	case K_ERR_ARM_USAGE_UNALIGNED_ACCESS:
		return "Unaligned memory access";
	case K_ERR_ARM_USAGE_DIV_0:
		return "Division by zero";
	case K_ERR_ARM_USAGE_GENERIC:
		return "Generic Usage fault";
	case K_ERR_ARM_SECURE_ENTRY_POINT:
		return "Invalid entry point";
	case K_ERR_ARM_SECURE_INTEGRITY_SIGNATURE:
		return "Invalid integrity signature";
	case K_ERR_ARM_SECURE_EXCEPTION_RETURN:
		return "Invalid exception return";
	case K_ERR_ARM_SECURE_ATTRIBUTION_UNIT:
		return "Attribution unit violation";
	case K_ERR_ARM_SECURE_TRANSITION:
		return "Invalid transition";
	case K_ERR_ARM_SECURE_LAZY_STATE_PRESERVATION:
		return "Lazy state preservation";
	case K_ERR_ARM_SECURE_LAZY_STATE_ERROR:
		return "Lazy state error";
	case K_ERR_ARM_BUS_STACKING:
		return "Stacking error";
	case K_ERR_ARM_BUS_UNSTACKING:
		return "Unstacking error";
	case K_ERR_ARM_BUS_PRECISE_DATA_BUS:
		return "Precise data bus error";
	case K_ERR_ARM_BUS_IMPRECISE_DATA_BUS:
		return "Imprecise data bus error";
	case K_ERR_ARM_BUS_INSTRUCTION_BUS:
		return "Instruction bus error";
	case K_ERR_ARM_MEM_STACKING:
		return "Stacking error (context area might be not valid)";
	case K_ERR_ARM_MEM_UNSTACKING:
		return "Unstacking error";
	case K_ERR_ARM_MEM_DATA_ACCESS:
		return "Data Access Violation";
	case K_ERR_ARM_MEM_INSTRUCTION_ACCESS:
		return "Instruction Access Violation";
	default:
		return "Unknown error";
	}
}

const char *CrashData::FaultSourceToStr(uint32_t source)
{
	switch (source) {
	case FaultSource::HardFault:
		return "HARD FAULT";
	case FaultSource::MemManageFault:
		return "MEMORY MANAGE FAULT";
	case FaultSource::BusFault:
		return "BUS FAULT";
	case FaultSource::UsageFault:
		return "USAGE FAULT";
	case FaultSource::SecureFault:
		return "SECURE FAULT";
	case FaultSource::DebugMonitor:
		return "DEBUG MONITOR";
	default:
		return "UNKNOWN";
	}
}

bool CrashData::Collect(const char *format, ...)
{
	/* Skip the line until reaching the last known one */
	if (mLastLine >= mLine && mLastLine != 0) {
		mLine++;
		return true;
	}

	memset(mBuffer, 0, sizeof(mBuffer));

	/* Collect new data */
	va_list args;
	va_start(args, format);
	int size = vsnprintk(mBuffer, sizeof(mBuffer), format, args);
	va_end(args);

	/* Skip the log line that does not fit the buffer size */
	if (size < 0) {
		return true;
	}

	/* The data can be written if the buffer is available,
	otherwise only mOffset will be used to calculate the actual size */
	if (mOutBuffer) {
		if (mOffset + size > mOutBufferSize) {
			/* The new data cannot be stored in the out buffer because there is no free space */
			return false;
		}
		memcpy(mOutBuffer + mOffset, mBuffer, size);
	}

	/* Save the processed line and seek to the next one */
	mLastLine = mLine++;
	/* Increase the offset */
	mOffset += size;

	return true;
}

extern "C" {

/* The actual code of the z_fatal_error function assigned as __real_ for linker wrapping purpose.
It required -Wl,--wrap=z_fatal_error linker option added */
void __real_z_fatal_error(unsigned int reason, const z_arch_esf_t *esf);

/* Wrapped z_fatal_error function to implement saving crash data to the retention memory
and then call the _real_ function that contains the actual code of the z_fatal_error.
It required -Wl,--wrap=z_fatal_error linker option added */
void __wrap_z_fatal_error(unsigned int reason, const z_arch_esf_t *esf)
{
	/* Store the crash data into the retained RAM memory */
	if (esf) {
		Nrf::CrashDescription description = { *esf, reason, k_thread_name_get(_current), (uint32_t *)_current,
						      (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) };
		retention_write(DEVICE_DT_GET(DT_NODELABEL(crash_retention)), 0,
				reinterpret_cast<uint8_t *>(&description), sizeof(description));
	}

	/* Call the original z_fatal_error function */
	__real_z_fatal_error(reason, esf);
}
}
