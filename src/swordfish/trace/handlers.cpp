/*
 * handlers.c
 *
 * Created: 3/09/2021 12:28:27 pm
 *  Author: smohekey
 */ 

#include <exception>

#include <sam.h>
#include <swordfish/types.h>
#include <swordfish/core/Console.h>

#define HALT_IF_DEBUGGING()                              \
do {                                                   \
	if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) { \
		__asm("bkpt 1");                                   \
	}                                                    \
} while (0)

#define HANDLE_INTERRUPT(name) \
	__asm volatile(              \
	"tst lr, #4 \n"              \
	"ite eq \n"                  \
	"mrseq r0, msp \n"           \
	"mrsne r0, psp \n"           \
	"b " #name "InterruptHandler \n"			 \
	)

typedef struct __attribute__((packed)) {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t return_address;
	uint32_t xpsr;
} ContextState;

extern "C" {
	__attribute__((optimize("O0")))
	void hardInterruptHandler(ContextState* frame) {
		HALT_IF_DEBUGGING();
	
		volatile uint32_t *cfsr = (volatile uint32_t *)0xE000ED28;
		const uint32_t usage_fault_mask = 0xffff0000;
		const bool non_usage_fault_occurred = (*cfsr & ~usage_fault_mask) != 0;
		// the bottom 8 bits of the xpsr hold the exception number of the
		// executing exception or 0 if the processor is in Thread mode
		const bool faulted_from_exception = ((frame->xpsr & 0xFF) != 0);
	
		if (faulted_from_exception || non_usage_fault_occurred) {
			// For any fault within an ISR or non-usage faults let's reboot the system
			volatile uint32_t *aircr = (volatile uint32_t *)0xE000ED0C;
			*aircr = (0x05FA << 16) | 0x1 << 2;
			while (1) { } // should be unreachable
		}
	
		// If it's just a usage fault, let's "recover"
		// Clear any faults from the CFSR
		//*cfsr |= *cfsr;
		// the instruction we will return to when we exit from the exception
		//frame->return_address = (uint32_t)throwHardFaultException;
		// the function we are returning to should never branch
		// so set lr to a pattern that would fault if it did
		//frame->lr = 0xdeadbeef;
		// reset the psr state and only leave the
		// "thumb instruction interworking" bit set
		//frame->xpsr = (1 << 24);
	}

	void NMI_Handler(void) {
		HALT_IF_DEBUGGING();
	}

	__attribute__((naked))
	void HardFault_Handler(void) {
		HANDLE_INTERRUPT(hard);
	}

	__attribute__((naked))
	void MemManage_Handler(void) {
		HANDLE_INTERRUPT(hard);
	}

	__attribute__((naked))
	void BusFault_Handler(void) {
		HANDLE_INTERRUPT(hard);
	}
 
	__attribute__((naked))
	void UsageFault_Handler(void) {
		HANDLE_INTERRUPT(hard);
	}

	__attribute__((optimize("O0")))
	void wdtInterruptHandler(ContextState* frame) {
		HALT_IF_DEBUGGING();
	
		auto& out = swordfish::core::Console::out();
	
		out << frame->return_address << '\n';
	}

	__attribute__((naked))
	void WDT_Handler(void) {
		HANDLE_INTERRUPT(wdt);
	}
};

extern "C" void __tzset() noexcept { }
extern "C" void __tz_lock() noexcept { }
extern "C" void __tz_unlock() noexcept { }
extern "C" void _tzset_unlocked() noexcept { }
	
[[noreturn]] void Terminate() noexcept {
	HALT_IF_DEBUGGING();
	
	while(true);
}

namespace __cxxabiv1 {
	std::terminate_handler __terminate_handler = Terminate;
}