#pragma once
#include <string>
#include <windows.h>

enum Color {
	ERR, WARN, INFO, NORMAL, GOOD, MAGENTA
};


namespace chip8 {
	class Chip8 {
	private:
		
		unsigned char  m_mem[4096]		= { 0 };	/* Memory */
		unsigned char  m_CPU_regs[16]	= { 0 };	/* CPU registers */
		unsigned short m_stack[16]		= { 0 };

		unsigned short m_opcode = 0;				/* The current opcode */
		unsigned short m_pc = 0x200;				/* Program counter */
		unsigned short m_ir = 0;					/* Index register */
		short m_sp = -1;							/* Stack pointer */

		unsigned char  m_scr[32 * 64]	= { 0 };	/* Screen */
		unsigned char  m_timer = 0;					/* Delay timer */
		unsigned char  m_snd_timer = 0;				/* Sound timer */

		unsigned char  m_key[16]		= { 0 };

		unsigned char m_characters[80] =
		{
			0xF0, 0x90, 0x90, 0x90, 0xF0,		/* 0 */
			0x20, 0x60, 0x20, 0x20, 0x70,		/* 1 */
			0xF0, 0x10, 0xF0, 0x80, 0xF0,		/* 2 */
			0xF0, 0x10, 0xF0, 0x10, 0xF0,		/* 3 */
			0x90, 0x90, 0xF0, 0x10, 0x10,		/* 4 */
			0xF0, 0x80, 0xF0, 0x10, 0xF0,		/* 5 */
			0xF0, 0x80, 0xF0, 0x90, 0xF0,		/* 6 */
			0xF0, 0x10, 0x20, 0x40, 0x40,		/* 7 */
			0xF0, 0x90, 0xF0, 0x90, 0xF0,		/* 8 */
			0xF0, 0x90, 0xF0, 0x10, 0xF0,		/* 9 */
			0xF0, 0x90, 0xF0, 0x90, 0x90,		/* A */
			0xE0, 0x90, 0xE0, 0x90, 0xE0,		/* B */
			0xF0, 0x80, 0x80, 0x80, 0xF0,		/* C */
			0xE0, 0x90, 0x90, 0x90, 0xE0,		/* D */
			0xF0, 0x80, 0xF0, 0x80, 0xF0,		/* E */
			0xF0, 0x80, 0xF0, 0x80, 0x80		/* F */
		};

		bool m_draw_flag = false;

	public:
		bool m_log				= false;
		bool m_log_registers	= false;
	public:
		Chip8();

		/* Loads a ROM file into Chip8 memory */
		void load_ROM(std::string path);

		/* Performs 1 CPU cycle */
		void cycle();

		inline unsigned char* getScreen() { return m_scr; };
		inline void setKey(short k, bool value) { m_key[k] = value?0xF:0x0; };
		inline bool shouldDraw() { 
			if (m_draw_flag)
			{
				m_draw_flag = false;
				return true;
			}
			return false;
		};
	};
}
