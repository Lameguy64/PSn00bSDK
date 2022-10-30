/*
 * PSn00bSDK dynamic linker example (main executable)
 * (C) 2021 spicyjpeg - MPL licensed
 *
 * This example shows how to use the psxetc DL_*() APIs to obtain information
 * about the executable's symbols at runtime. This is accomplished by parsing a
 * symbol map file, which is generated at compile time by GCC's nm command and
 * included into the CD image. The symbol map lists all functions/variables in
 * the executable and their type, address and size. Currently only searching
 * for a symbol's address by its name (DL_GetMapSymbol()) is supported, however
 * this may be expanded in the future.
 *
 * Being able to introspect local symbols at runtime, in turn, allows us to use
 * another set of APIs to load, link and execute code from an external file
 * (compiled with the dll.ld linker script). A dynamically-loaded library can
 * reference and access any non-static function or variable within the main
 * executable (and the libraries the main executable has been compiled with);
 * the dynamic linker will automatically patch the DLL's code and resolve these
 * references so that they point to the addresses listed in the map file. DLLs
 * also have their own symbol tables, and any symbol in a DLL is accessible to
 * the main executable through DL_GetDLLSymbol().
 *
 * This example shows how DLLs can be loaded and unloaded at any time. Pressing
 * START will unload the current DLL and load an alternate one on-the-fly. A
 * custom resolver is also employed to tap into the DLL patching process and
 * override the printf() function referenced by the DLLs with a different
 * implementation, so the debug output from the DLLs can be redirected to the
 * on-screen overlay.
 *
 * Dynamic linking has plenty of practical applications. It can be e.g. used to
 * greatly reduce RAM usage by splitting off a large executable into a "common"
 * executable (containing SDK APIs as well as frequently-used symbols such as
 * rendering buffers) and many smaller DLLs, which can then be swapped in and
 * out depending on which functions are needed. It can also be useful to run
 * code that hasn't been compiled at the same time as the main executable, such
 * as plugins/mods/patches stored on a memory card.
 */

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <psxapi.h>
#include <psxetc.h>
#include <psxgte.h>
#include <psxgpu.h>
#include <psxpad.h>
#include <psxcd.h>

#include "library/dll_common.h"

// List all SDK functions used by the DLLs in a dummy array to ensure GCC won't
// strip them. By default the linker removes all functions unused in the
// executable itself, but we (obviously) need them to be present for a DLL to
// call them. Placing this array in the .dummy section (as defined in the
// PSn00bSDK linker script) ensures it won't be stripped away until all
// functions are in place.
void *DO_NOT_STRIP[] __attribute__((section(".dummy"))) = {
	&rand,
	&InitGeom,
	&RotMatrix,
	&TransMatrix,
	&MulMatrix0,
	&GetTimInfo,
	&LoadImage
};

static const char *const DLL_FILENAMES[] = {
	"\\CUBE.DLL;1",
	"\\BALLS.DLL;1"
};

#define DLL_COUNT 2

/* Display/GPU context utilities */

#define SCREEN_XRES 320
#define SCREEN_YRES 240

#define BGCOLOR_R 48
#define BGCOLOR_G 24
#define BGCOLOR_B  0

void init_context(RenderContext *ctx) {
	Framebuffer *db;

	ResetGraph(0);
	ctx->db_active = 0;

	db = &(ctx->db[0]);
	SetDefDispEnv(&(db->disp),           0, 0, SCREEN_XRES, SCREEN_YRES);
	SetDefDrawEnv(&(db->draw), SCREEN_XRES, 0, SCREEN_XRES, SCREEN_YRES);
	setRGB0(&(db->draw), BGCOLOR_R, BGCOLOR_G, BGCOLOR_B);
	db->draw.isbg = 1;
	db->draw.dtd  = 1;

	db = &(ctx->db[1]);
	SetDefDispEnv(&(db->disp), SCREEN_XRES, 0, SCREEN_XRES, SCREEN_YRES);
	SetDefDrawEnv(&(db->draw),           0, 0, SCREEN_XRES, SCREEN_YRES);
	setRGB0(&(db->draw), BGCOLOR_R, BGCOLOR_G, BGCOLOR_B);
	db->draw.isbg = 1;
	db->draw.dtd  = 1;

	// Set up the ordering tables and primitive buffers.
	db = &(ctx->db[0]);
	ctx->db_nextpri = db->p;
	ClearOTagR(db->ot, OT_LEN);

	PutDrawEnv(&(db->draw));
	//PutDispEnv(&(db->disp));

	db = &(ctx->db[1]);
	ClearOTagR(db->ot, OT_LEN);

	// Create a text stream at the top of the screen.
	FntLoad(960, 0);
	FntOpen(4, 12, 312, 32, 2, 256);
}

void display(RenderContext *ctx) {
	Framebuffer *db;

	DrawSync(0);
	VSync(0);
	ctx->db_active ^= 1;

	db = &(ctx->db[ctx->db_active]);
	ctx->db_nextpri = db->p;
	ClearOTagR(db->ot, OT_LEN);

	PutDrawEnv(&(db->draw));
	PutDispEnv(&(db->disp));
	SetDispMask(1);

	db = &(ctx->db[!ctx->db_active]);
	DrawOTag(&(db->ot[OT_LEN - 1]));
}

/* Symbol overriding example */

static volatile int resolve_counter = 0;

// This function will override printf(), i.e. DLLs will use this instead of the
// "real" printf() present in the executable, thanks to the custom resolver
// defined below. We'll use this to redirect the DLL's output to be shown on
// screen.
int dll_printf(const char *format, ...) {
	va_list args;
	va_start(args, format);

	char buffer[256];
	int  return_value = vsprintf(buffer, format, args);
	va_end(args);

	FntPrint(-1, "DLL:  %s", buffer);
	//FntFlush(-1);

	return return_value;
}

// This function will be called by the linker for each undefined symbol
// (function or variable) in the DLL, and should return the address of the
// symbol so the dynamic linker can patch it in. The default resolver tries to
// find them in the currently loaded symbol map using DL_GetMapSymbol().
void *custom_resolver(DLL *dll, const char *name) {
	if (!strcmp(name, "printf")) {
		printf("Resolving printf() -> dll_printf() (#%d)\n", resolve_counter++);
		return &dll_printf;
	}

	printf("Resolving %s() (#%d)\n", name, resolve_counter++);

	// Custom resolvers should always fall back to the default behavior.
	return DL_GetMapSymbol(name);
}

/* Global variables and structs */

// Define a struct to store pointers to a DLL's functions into. This is not
// strictly required, however looking up symbols is a relatively slow operation
// and the pointers returned by DL_GetDLLSymbol() should be saved and reused as
// much as possible.
typedef struct {
	void (*init)(RenderContext *);
	void (*render)(RenderContext *, uint16_t buttons);
} DLL_API;

static DLL dll;
static DLL_API dll_api;

static RenderContext ctx;

/* Main */

#define SHOW_STATUS(...) { FntPrint(-1, __VA_ARGS__); FntFlush(-1); display(&ctx); }
#define SHOW_ERROR(...)  { SHOW_STATUS(__VA_ARGS__); while (1) __asm__("nop"); }

// This is a simple function to read a CD-ROM file into memory (the dynamic
// linker no longer provides APIs that take file paths directly). You might
// want to replace this with code that e.g. loads the DLL from a compressed
// archive or even from the serial port.
size_t load_file(const char *filename, void **ptr) {
	SHOW_STATUS("LOADING %s\n", filename);

	CdlFILE file;
	if (!CdSearchFile(&file, filename))
		SHOW_ERROR("FAILED TO FIND %s\n", filename);

	// Round up the file size so it matches the number of sectors occupied by
	// the file (as CdRead() can only return entire sectors).
	size_t len   = (file.size + 2047) & 0xfffff800;
	void   *_ptr = malloc(len);
	if (!_ptr)
		SHOW_ERROR("FAILED TO ALLOCATE %d BYTES\n", len);

	CdControl(CdlSetloc, &(file.pos), 0);
	CdRead(len / 2048, _ptr, CdlModeSpeed);
	if (CdReadSync(0, 0) < 0)
		SHOW_ERROR("FAILED TO READ %s\n", filename);

	*ptr = _ptr;
	return file.size;
}

void load_dll(const char *filename) {
	// As we're passing DL_FREE_ON_DESTROY to DL_CreateDLL(), calling
	// DL_DestroyDLL() will also deallocate the buffer the DLL was loaded into.
	DL_DestroyDLL(&dll);

	void   *ptr;
	size_t len = load_file(filename, &ptr);

	if (!DL_CreateDLL(&dll, ptr, len, DL_LAZY | DL_FREE_ON_DESTROY))
		SHOW_ERROR("FAILED TO PARSE %s\n", filename);

	dll_api.init   = DL_GetDLLSymbol(&dll, "init");
	dll_api.render = DL_GetDLLSymbol(&dll, "render");

	printf("DLL init() @ %08x, render() @ %08x\n", dll_api.init, dll_api.render);

	// Unfortunately, due to how position-independent code works, function
	// pointers returned by DL_GetDLLSymbol() can't be called without first
	// initializing register $t9. We have to use the DL_PRE_CALL() macro, which
	// sets up $t9 to ensure the function can locate and reference the DLL's
	// relocation table.
	DL_PRE_CALL(dll_api.init);
	dll_api.init(&ctx);
}

int main(int argc, const char* argv[]) {
	// Reference the dummy array to prevent it from being stripped.
	void **dummy = DO_NOT_STRIP;

	init_context(&ctx);

	SHOW_STATUS("INITIALIZING CD\n");
	CdInit();

	// Load the symbol map file, let the dynamic linker parse it and then
	// unload it.
	void   *ptr;
	size_t len = load_file("\\MAIN.MAP;1", &ptr);

	if (!DL_ParseSymbolMap(ptr, len))
		SHOW_ERROR("FAILED TO PARSE SYMBOL MAP\n");

	free(ptr);

	// Try to obtain a reference to a local function.
	void (*_display)() = DL_GetMapSymbol("display");
	if (!_display)
		SHOW_ERROR("FAILED TO LOOK UP LOCAL FUNCTION\n");

	printf("Symbol map test, display() @ %08x\n", _display);

	// Set up controller polling.
	uint8_t pad_buff[2][34];
	InitPAD(pad_buff[0], 34, pad_buff[1], 34);
	StartPAD();
	ChangeClearPAD(0);

	// Set up the custom resolver and load the first DLL.
	DL_SetResolveCallback(&custom_resolver);
	load_dll(DLL_FILENAMES[0]);

	uint32_t dll_active   = 0;
	uint16_t last_buttons = 0xffff;

	while (1) {
		// Use the currently loaded DLL to render a frame.
		DL_PRE_CALL(dll_api.render);
		dll_api.render(&ctx, last_buttons);

		FntPrint(-1, "MAIN: DLL ADDR=%08x SIZE=%d\n", dll.ptr, dll.size);
		FntPrint(-1, "MAIN: %d FUNCTIONS RESOLVED\n", resolve_counter);
		FntPrint(-1, "[START] LOAD NEXT DLL\n");
		FntFlush(-1);
		display(&ctx);

		// Check if a compatible controller is connected and if START has been
		// pressed (i.e. wasn't previously held down, but now is). If so,
		// switch the active DLL.
		PADTYPE *pad = (PADTYPE *) pad_buff[0];
		if (pad->stat)
			continue;
		if ((pad->type != 4) && (pad->type != 5) && (pad->type != 7))
			continue;

		if ((last_buttons & PAD_START) && !(pad->btn & PAD_START)) {
			dll_active++;
			dll_active %= DLL_COUNT;

			load_dll(DLL_FILENAMES[dll_active]);
		}

		last_buttons = pad->btn;
	}

	//DL_DestroyDLL(&dll);
	//DL_UnloadSymbolMap();
	return 0;
}
