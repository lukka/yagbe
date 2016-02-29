#pragma once

#include "registers.hpp"
#include "memory.hpp"

namespace yagbe
{
	class context
	{
	public:
		context() : flags(registers.f)
		{

		}

		yagbe::registers registers;
		yagbe::flags flags;
		yagbe::memory memory;

		//reads a byte at PC, and increments PC
		uint8_t read_byte()
		{
			return memory.at(registers.pc++);
		}

		//reads a word at PC, and increments PC
		uint16_t read_word()
		{
			union
			{
				uint8_t bytes[2];
				uint16_t word;
			};

			//TODO endianess
			bytes[0] = read_byte();
			bytes[1] = read_byte();
			return word;
		}

		void push(uint16_t w)
		{
			union
			{
				uint8_t bytes[2];
				uint16_t word;
			};

			word = w;

			//TODO endianess
			c.memory.at(c.registers.sp--) = bytes[0];
			c.memory.at(c.registers.sp--) = bytes[1];
		}

		uint16_t pop()
		{
			union
			{
				uint8_t bytes[2];
				uint16_t word;
			};

			//TODO endianess
			bytes[1] = c.memory.at(c.registers.sp++);
			bytes[0] = c.memory.at(c.registers.sp++);

			return word;
		}

	};
};