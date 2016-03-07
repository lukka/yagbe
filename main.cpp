#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <map>

#include "vm/context.hpp"
#include "renderer/sdl2_renderer.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
using namespace yagbe;

context ctx;
sdl2_renderer renderer(gpu::screen_size());
bool frame_drawn = false;
bool loaded_rom = false;

void one_iter()
{
	if (!loaded_rom)
		return;
	frame_drawn = false;
	while (renderer.step() && !frame_drawn)
	{
		ctx.cpu_step();
	}
}

int main(int argc, char * argv[])
{

	std::map<SDL_Keycode, key_handler::key> keys = {
		{ SDLK_LEFT, key_handler::key::Left },
		{ SDLK_RIGHT, key_handler::key::Right },
		{ SDLK_UP, key_handler::key::Up },
		{ SDLK_DOWN, key_handler::key::Down },

		{ SDLK_a, key_handler::key::Select },
		{ SDLK_s, key_handler::key::Start },
		{ SDLK_z, key_handler::key::A },
		{ SDLK_x, key_handler::key::B },
	};

	std::string path = YAGBE_ROMS;

	//path += "../test_roms/individual/01-special.gb"; //PASSED
	path += "../test_roms/individual/02-interrupts.gb"; //failing EI, test address 0xC2B5
	//fail jump is C1B9, fails at c2c9
	// cp C0
	// jp nz, 0xC1B9

	// it goes - load sp-2 into hl, ldi (hl) into a, a should equal C0
	// sooo.... a should be equal to old stack value.. I think he means "interrupt should occur at C2C0

	//timer interrupt should only increment a.

	//path += "../test_roms/individual/03-op sp,hl.gb"; //PASSED

	//path += "../test_roms/individual/04-op r,imm.gb"; //PASSED
	//path += "../test_roms/individual/05-op rp.gb"; //PASSED
	//path += "../test_roms/individual/06-ld r,r.gb"; //PASSED

	//path += "../test_roms/individual/07-jr,jp,call,ret,rst.gb"; //PASSED

	//path += "../test_roms/individual/08-misc instrs.gb"; //PASSED

	//path += "../test_roms/individual/09-op r,r.gb"; //PASSED

	//path += "../test_roms/individual/10-bit ops.gb"; //PASSED
	
	//path += "../test_roms/individual/11-op a,(hl).gb"; //PASSED

	//path += "adjtris.gb";
	//path += "opus5.gb";




	ctx.gpu.onFrameReady = [&](auto &frame)
	{
		renderer.accept_image(frame);
		frame_drawn = true;
	};

	renderer.onKeyChanged = [&](SDL_Keycode c, bool v)
	{
		auto it = keys.find(c);
		if (it != keys.end())
			ctx.key_handler.set_key(it->second, v);
	};

	
#ifdef __EMSCRIPTEN__
	std::cout << "Starting emulator..." << std::endl;
	if (argc > 1)
	{
		std::cout << "Getting rom from: " << argv[1] << std::endl;

		auto onLoad = [](void*, void* b, int s)
		{
			std::cout << "Done." << std::endl;

			if (ctx.load_rom((uint8_t*)b, s))
			{
				loaded_rom = true;
				std::cout << "Loaded OK." << std::endl;
				return;
			}
			std::cout << "Loading failed" << std::endl;
		};

		auto onError = [](void*)
		{
			std::cout << "Error." << std::endl;
		};


		emscripten_async_wget_data(argv[1], nullptr, onLoad, onError);
	}
	else
	{
		if (!ctx.load_rom(path))
			return -1;
		loaded_rom = ctx.load_rom(path);
	}
		


	emscripten_set_main_loop(one_iter, 60, 1);
#else
	if (!ctx.load_rom(path))
		return -1;
	loaded_rom = true;

	while (renderer.running())
		one_iter();
#endif

	return 0;
}


/*
Left to do:
- probably interrupts are implemented slightly wrong - if two would happen in exactly same time, IF shoud represent this
- 16h sprites

-sound
*/