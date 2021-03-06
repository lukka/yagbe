#pragma once
#include <array>
#include "registers.hpp"
#include "interrupts.hpp"
#include "memory.hpp"
#include "vm/gpu/gpu.hpp"
#include "vm/apu/apu.hpp"
#include "key_handler.hpp"
#include "timer.hpp"
#include "rom/rom_info.hpp"
#include "rom/mbc_handler.hpp"

namespace yagbe
{
	class context;
	using instruction_pointer = int(*)(context &c);
	using instructions_array = std::array<instruction_pointer, 256>;

	class context
	{
	protected:


	public:
		bool is_cbg()
		{
			return false;
		}

		context() : flags(registers.f)
		{
			reset();
		}

		void reset();
		rom_info* load_rom(const std::string& path);
		rom_info* load_rom(uint8_t* data, int size);
		rom_info* load_rom(std::vector<uint8_t>&& data);

		yagbe::registers   registers;
		yagbe::flags       flags;
		yagbe::memory      memory{ *this };
		yagbe::interrupts  interrupt{ *this };
		yagbe::gpu         gpu{ memory, interrupt };
		yagbe::apu         apu{ memory };
		yagbe::timer       timer{ memory, interrupt };

		yagbe::key_handler key_handler{ memory.io_register.P1, interrupt };



		uint32_t cycles_elapsed;
		bool     halted;


		const instructions_array& instructions();
		const instructions_array& cb_instructions();
		void cpu_step();

		//reads a byte at PC, and increments PC
		uint8_t read_byte()
		{
			return memory.read_at(registers.pc++);
		}
		
		//peeks byte at PC
		uint8_t peek_byte()
		{
			return memory.read_at(registers.pc);
		}

		//reads a word at PC, and increments PC
		uint16_t read_word()
		{
			memory::ByteWord w;
			w.byte[0] = read_byte();
			w.byte[1] = read_byte();
			return w.word;
		}

		void push(uint16_t word)
		{
			memory::ByteWord w = { word };

			memory.write_byte_at(--registers.sp, w.byte[1]);
			memory.write_byte_at(--registers.sp, w.byte[0]);
		}

		uint16_t pop()
		{
			memory::ByteWord w;

			w.byte[0] = memory.read_at(registers.sp++);
			w.byte[1] = memory.read_at(registers.sp++);

			return w.word;
		}

		const mbc_handler::pointer& current_mbc_handler() { return _mbc_handler; }




		template<class Archive>
		void save(Archive & ar) const
		{
			ar(cycles_elapsed, halted,
				registers, memory, interrupt,
				timer, gpu
				);

			if (_mbc_handler)
				_mbc_handler->save(ar);
		}

		template<class Archive>
		void load(Archive & ar)
		{
			ar(cycles_elapsed, halted,
				registers, memory, interrupt,
				timer, gpu
				);

			if (_mbc_handler)
				_mbc_handler->load(ar);
		}

		auto current_rom_info()
		{
			return (rom_info*)memory.raw_pointer_at(rom_info::address());
		}
	protected:
		mbc_handler::pointer _mbc_handler;
	};
};