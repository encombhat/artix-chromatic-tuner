#include "app.h"

#include "xil_cache.h"		// Cache Drivers
#include "xil_printf.h" 	// Used for xil_printf()

//BaseAddr points to the base (byte) address of the DDR2 Memory
u8 * BaseAddr = (u8 *) XPAR_MIG7SERIES_0_BASEADDR;

// Has I ever???
void fast() {
	Xil_ICacheInvalidate();
	Xil_ICacheEnable();
	Xil_DCacheInvalidate();
	Xil_DCacheEnable();
}

int main() {
	fast();

	u32 tick = 100000; // 1ms
//	u32 tick = 10000; // 100us

	struct App app = app_new(tick);
	app_initialize(&app);

	// Dark magic
	while (1) {
		app_main_loop(&app);
	}
}

