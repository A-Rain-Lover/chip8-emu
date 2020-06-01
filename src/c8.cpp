#include "c8.h"

#include <iostream>
#include <fstream>
#include <random>
#include <bitset>

void color(Color c) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    switch (c) {
    case NORMAL:
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        break;
    case ERR:
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        break;
    case WARN:
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
        break;
    case INFO:
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE);
        break;
    case GOOD:
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        break;
    case MAGENTA:
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        break;
    }
}

namespace chip8 {
	Chip8::Chip8() {
        system("cls");
        color(INFO);
        std::cout << "**** CHIP8 ****" << std::endl;
	}
    /* Loads a ROM file into Chip8 memory */
	void Chip8::load_ROM(std::string path) {
        color(NORMAL);
        std::cout << "Loading ROM : "<<path << std::endl;
        std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
        long size = file.tellg();
        file.seekg(0, std::ios::beg);
        char* buffer = new char[size];
        file.read(buffer, size);
        file.close();
        for(short i = 0; i < size; i++)
            *(m_mem + 0x200 + i) = *(buffer + i);
        delete[] buffer;
        color(GOOD);
        std::cout << "Loaded " << size << " bytes" << std::endl;
        for(int i = 0; i < 80; i++)
            m_mem[0x0 + i] = m_characters[i];
	}
    /* Performs 1 CPU cycle */
    void Chip8::cycle() {
        m_opcode = m_mem[m_pc] << 8 | m_mem[m_pc + 1];
        color(NORMAL);
        if(m_log) std::cout << "[ 0x" << std::hex << m_pc << " ] ";
        color(MAGENTA);
        switch (m_opcode) {
        case 0x00E0: /* CLS */
            for(int i = 0; i < 32*64; i++)
                m_scr[i] = 0;
            if(m_log) std::cout << "CLS" << std::endl;
            m_pc += 2;
            break;
        case 0x00EE: /* RET */
            m_pc = m_stack[m_sp--]+2;
            if(m_log) std::cout << "RET" << std::endl;
            
            break;
       
        default: 
            switch (m_opcode & 0xF000) {
            case 0x1000: /* JP addr */
                m_pc = m_opcode & 0x0FFF;
                if(m_log) std::cout << "JP "<<std::hex<< (m_opcode & 0x0FFF) << std::endl;
                break;
            case 0x2000: /* CALL addr */
                m_stack[++m_sp] = m_pc;
                m_pc = m_opcode & 0x0FFF;
                if(m_log) std::cout << "CALL " << std::hex << (m_opcode & 0x0FFF) << std::endl;
                break;
            case 0x3000: /* SE Vx, byte */
                if (m_CPU_regs[(m_opcode & 0x0F00)>>8] == (m_opcode & 0x00FF)) m_pc += 2;
                if(m_log) std::cout << "SE V" << std::hex << ((m_opcode & 0x0F00) >> 8)<<", " << std::hex << (m_opcode & 0x00FF) << std::endl;
                m_pc += 2;
                break;
            case 0x4000: /* SNE Vx, byte */
                if (m_CPU_regs[(m_opcode & 0x0F00) >> 8] != (m_opcode & 0x00FF)) m_pc += 2;
                if(m_log) std::cout << "SNE V" << std::hex << ((m_opcode & 0x0F00) >> 8) << ", " << std::hex << (m_opcode & 0x00FF) << std::endl;
                m_pc += 2;
                break;
            case 0x5000: /* SE Vx, Vy */
                if (!(m_opcode & 0x000F) && m_CPU_regs[(m_opcode & 0x0F00) >> 8] == m_CPU_regs[(m_opcode & 0x00F0) >> 4]) m_pc += 2;
                if(m_log) std::cout << "SE V" << std::hex << ((m_opcode & 0x0F00) >> 8) << ", V" << std::hex << ((m_opcode & 0x00F0) >> 8) << std::endl;
                m_pc += 2;
                break;
            case 0x6000: /* LD Vx, byte */
                m_CPU_regs[(m_opcode & 0x0F00) >> 8] = m_opcode & 0x00FF;
                if(m_log) std::cout << "LD V" << std::hex << ((m_opcode & 0x0F00)>>8) << ", " <<  (m_opcode & 0x00FF) << std::endl;
                m_pc += 2;
                break;
            case 0x7000: /* ADD Vx, byte */
                m_CPU_regs[(m_opcode & 0x0F00) >> 8] += m_opcode & 0x00FF;
                if(m_log) std::cout << "ADD V" << std::hex << ((m_opcode & 0x0F00) >> 8) << ", " << (m_opcode & 0x00FF) << std::endl;
                m_pc += 2;
                break;
            case 0x8000:
                switch (m_opcode & 0x000F) {
                case 0x0: /* LD Vx, Vy */
                    m_CPU_regs[(m_opcode & 0x0F00) >> 8] = m_CPU_regs[(m_opcode & 0x00F0) >> 4];
                    m_pc += 2;
                    if(m_log) std::cout << "LD V" << std::hex << ((m_opcode & 0x0F00) >> 8) << ", V" << std::hex << ((m_opcode & 0x00F0) >> 8) << std::endl;
                    break;
                case 0x1: /* OR Vx, Vy */
                    m_CPU_regs[(m_opcode & 0x0F00) >> 8] |= m_CPU_regs[(m_opcode & 0x00F0) >> 4];
                    m_pc += 2; 
                    if(m_log) std::cout << "OR V" << std::hex << ((m_opcode & 0x0F00) >> 8) << ", V" << std::hex << ((m_opcode & 0x00F0) >> 8) << std::endl;
                    break;
                case 0x2: /* AND Vx, Vy */
                    m_CPU_regs[(m_opcode & 0x0F00) >> 8] &= m_CPU_regs[(m_opcode & 0x00F0) >> 4];
                    m_pc += 2;
                    if(m_log) std::cout << "AND V" << std::hex << ((m_opcode & 0x0F00) >> 8) << ", V" << std::hex << ((m_opcode & 0x00F0) >> 8) << std::endl;
                    break;
                case 0x3: /* XOR Vx, Vy */
                    m_CPU_regs[(m_opcode & 0x0F00) >> 8] ^= m_CPU_regs[(m_opcode & 0x00F0) >> 4];
                    m_pc += 2; 
                    if(m_log) std::cout << "XOR V" << std::hex << ((m_opcode & 0x0F00) >> 8) << ", V" << std::hex << ((m_opcode & 0x00F0) >> 8) << std::endl;
                    break;
                case 0x4:/* ADD Vx, Vy */
                    m_CPU_regs[0xF] =
                        m_CPU_regs[(m_opcode & 0x0F00) >> 8] + m_CPU_regs[(m_opcode & 0x00F0) >> 4] ? 1 : 0;
                    m_CPU_regs[(m_opcode & 0x0F00) >> 8] += m_CPU_regs[(m_opcode & 0x00F0) >> 4];
                    m_pc += 2;
                    if(m_log) std::cout << "ADD V" << std::hex << ((m_opcode & 0x0F00) >> 8) << ", V" << std::hex << ((m_opcode & 0x00F0) >> 8) << std::endl;
                    break;
                case 0x5:/* SUB Vx, Vy */
                    m_CPU_regs[0xF] =
                        m_CPU_regs[(m_opcode & 0x0F00) >> 8] > m_CPU_regs[(m_opcode & 0x00F0) >> 4] ? 1 : 0;
                    m_CPU_regs[(m_opcode & 0x0F00) >> 8] -= m_CPU_regs[(m_opcode & 0x00F0) >> 4];
                    m_pc += 2;
                    if(m_log) std::cout << "SUB V" << std::hex << ((m_opcode & 0x0F00) >> 8) << ", V" << std::hex << ((m_opcode & 0x00F0) >> 8) << std::endl;
                    break;
                case 0x6:/* SHR Vx, Vy */
                    m_CPU_regs[0xF] = m_CPU_regs[(m_opcode & 0x00F0) >> 4] & 1;
                    m_CPU_regs[(m_opcode & 0x0F00) >> 8] = 
                        m_CPU_regs[(m_opcode & 0x00F0) >> 4] >> 1;
                    m_pc += 2;
                    if(m_log) std::cout << "SHR V" << std::hex << ((m_opcode & 0x0F00) >> 8) << ", V" << std::hex << ((m_opcode & 0x00F0) >> 8) << std::endl;
                    break;
                case 0x7:/* SUBN Vx, Vy */
                    m_CPU_regs[0xF] =
                        m_CPU_regs[(m_opcode & 0x00F0) >> 4] > m_CPU_regs[(m_opcode & 0x0F00) >> 8] ? 1 : 0;
                    m_CPU_regs[(m_opcode & 0x00F0) >> 4] -= m_CPU_regs[(m_opcode & 0x0F00) >> 8];
                    m_pc += 2;
                    if(m_log) std::cout << "SUBN V" << std::hex << ((m_opcode & 0x0F00) >> 8) << ", V" << std::hex << ((m_opcode & 0x00F0) >> 8) << std::endl;
                    break;
                case 0xe:/* SHL Vx, Vy */
                    m_CPU_regs[0xF] = m_CPU_regs[(m_opcode & 0x00F0) >> 4] & 0x80;
                    m_CPU_regs[(m_opcode & 0x0F00) >> 8] =
                        m_CPU_regs[(m_opcode & 0x00F0) >> 4] << 1;
                    m_pc += 2;
                    if(m_log) std::cout << "SHL V" << std::hex << ((m_opcode & 0x0F00) >> 8) << ", V" << std::hex << ((m_opcode & 0x00F0) >> 8) << std::endl;
                    break;
                default:
                    color(ERR);
                    if(m_log) std::cout << "[!] ERROR : Invalid opcode 0x" << m_opcode << " ! at 0x" << std::hex << m_pc << std::endl;
                    m_pc += 2;
                    break;
                }
                break;
            case 0x9000: /* SNE Vx, Vy */
                if (!(m_opcode & 0x000F) && m_CPU_regs[(m_opcode & 0x0F00) >> 8] != m_CPU_regs[(m_opcode & 0x00F0) >> 4]) m_pc += 2;
                if(m_log) std::cout << "SNE V" << std::hex << ((m_opcode & 0x0F00) >> 8) << ", V" << std::hex << ((m_opcode & 0x00F0) >> 8) << std::endl;
                m_pc += 2;
                break;
            case 0xA000: /* LD I, addr */
                m_ir = (m_opcode & 0x0FFF);
                if(m_log) std::cout << "LD I," << std::hex << (m_opcode & 0x0FFF) << std::endl;
                m_pc += 2;
                break;
            case 0xB000: /* JP V0, addr */
                m_pc = (m_opcode & 0x0FFF) + m_CPU_regs[0x0];
                if(m_log) std::cout << "LD I, " << std::hex << (m_opcode & 0x0FFF) << std::endl;
                m_pc += 2;
                break;
            case 0xC000: /* RND Vx, byte */
            {    
            m_CPU_regs[(m_opcode & 0x0F00) >> 8] =(rand()%255 & (m_opcode & 0x00FF));
            if(m_log) std::cout << "RND V" << std::hex << ((m_opcode & 0x0F00) >> 8)<<", " << std::hex << (m_opcode & 0x00FF) << std::endl;
            m_pc += 2;
            break;
            }
            case 0xD000: /* DRW Vx, Vy, nibble */
                m_CPU_regs[0xF] = 0x0;
                for (short y = 0; y < (m_opcode & 0x000F); y++) {
                    unsigned short line = m_mem[m_ir + y];
                    for (short x = 0; x < 8; x++) {
                        if (line & (0x80 >> x)) {
                            unsigned short opx = m_CPU_regs[(m_opcode & 0x0F00) >> 8];
                            unsigned short opy = m_CPU_regs[(m_opcode & 0x00F0) >> 4];

                            m_CPU_regs[0xF] = m_scr[64 * (opy + y) + x + opx]?0x1: m_CPU_regs[0xF];
                            m_scr[64 * (opy + y) + x + opx] ^= 1;
                        }
                    }
                }
                if(m_log) std::cout << "DRW V" << std::hex << ((m_opcode & 0x0F00) >> 8) << ", V" << std::hex << ((m_opcode & 0x00F0)>>4)<< ", " << std::hex << (m_opcode & 0x000F) << std::endl;
                m_draw_flag = true;
                m_pc += 2;
                break;
            case 0xE000:
                switch (m_opcode & 0x00FF) {
                case 0x9E: /* SKP Vx */
                    if (m_key[m_CPU_regs[(m_opcode & 0x0F00) >> 8]]) m_pc += 2;
                    m_pc += 2;
                    if(m_log) std::cout << "SKP V" << std::hex << ((m_opcode & 0x0F00) >> 8) << std::endl;
                    break;
                case 0xA1: /* SKNP Vx */
                    if (!m_key[m_CPU_regs[(m_opcode & 0x0F00) >> 8]]) m_pc += 2;
                    m_pc += 2;
                    if(m_log) std::cout << "SKNP V" << std::hex << ((m_opcode & 0x0F00) >> 8) << std::endl;
                    break;
                }
                break;
            case 0xF000:
                switch (m_opcode & 0x00FF) {
                case 0x07: /* LD Vx, DT */
                    m_CPU_regs[(m_opcode & 0xF00) >> 8] = m_timer;
                    m_pc += 2;
                    if(m_log) std::cout << "LD V" << std::hex << ((m_opcode & 0x0F00) >> 8) << ", DT"<< std::endl;
                    break;
                case 0x0A: /* LD Vx, K */
                {   char key = -1;
                    
                    for (auto k : m_key)
                        if (k) key = k;
                    if (key != -1) {
                        m_CPU_regs[(m_opcode & 0xF00) >> 8] = key;
                        m_pc += 2;
                    }
                    if(m_log) std::cout << "LD V" << std::hex << ((m_opcode & 0x0F00) >> 8) << ", K" << std::endl;
                    break;
                }
                case 0x15: /* LD DT, Vx */
                    m_timer = m_CPU_regs[(m_opcode & 0xF00) >> 8];
                    m_pc += 2;
                    if(m_log) std::cout << "LD DT, V" << std::hex << ((m_opcode & 0x0F00) >> 8)<< std::endl;
                    break;
                case 0x18: /* LD ST, Vx */
                    m_snd_timer = m_CPU_regs[(m_opcode & 0xF00) >> 8];
                    m_pc += 2;
                    if(m_log) std::cout << "LD ST, V" << std::hex << ((m_opcode & 0x0F00) >> 8) << std::endl;
                    break;
                case 0x1E: /* ADD I, Vx */
                    m_ir += m_CPU_regs[(m_opcode & 0xF00) >> 8];
                    m_pc += 2;
                    if(m_log) std::cout << "ADD I, V" << std::hex << ((m_opcode & 0x0F00) >> 8) << std::endl;
                    break;
               
                case 0x29: /* LD F, Vx */
                    m_ir = 5 * m_CPU_regs[(m_opcode & 0x0F00) >> 8];
                    m_pc += 2;
                    if (m_log) std::cout << "LD F, V" << std::hex << ((m_opcode & 0x0F00) >> 8) << std::endl;

                    break;
                case 0x33: /* LD B, Vx */
                    m_mem[m_ir + 2] = m_CPU_regs[(m_opcode & 0x0F00) >> 8] % 10;
                    m_mem[m_ir + 1] = (m_CPU_regs[(m_opcode & 0x0F00) >> 8]/10) % 10;
                    m_mem[  m_ir  ] = (m_CPU_regs[(m_opcode & 0x0F00) >> 8]/100) % 10;
                    m_pc += 2;
                    if (m_log) std::cout << "LD B, V" << std::hex << ((m_opcode & 0x0F00) >> 8) << std::endl;

                    break;
                case 0x55: /* LD [I], Vx */
                    for (unsigned short i = 0; i <= ((m_opcode & 0x0F00) >> 8); i++)
                        m_mem[m_ir + i] = m_CPU_regs[i];
                    m_pc += 2;
                    if (m_log) std::cout << "LD [I], V" << std::hex << ((m_opcode & 0x0F00) >> 8) << std::endl;

                    break;
                case 0x65: /* LD Vx, [I] */
                    for (short i = 0; i <= (m_opcode & 0x0F00) >> 8; i++)
                        m_CPU_regs[i] = m_mem[m_ir + i];
                    m_pc += 2;
                    if (m_log) std::cout << "LD V" << std::hex << ((m_opcode & 0x0F00) >> 8) <<", [I] "<< std::endl;

                    break;
                default:
                    color(ERR);
                    std::cout << "[!] ERROR : Invalid opcode 0x" << m_opcode << " ! at 0x" << std::hex << m_pc << std::endl;
                    m_pc += 2;
                    break;

                }
                break;
            default:
                color(WARN);
                std::cout << "[!] Warning : Unknown opcode : 0x" << std::hex << m_opcode << " at 0x" << std::hex << m_pc << std::endl;
                m_pc += 2;

            }
            
            
        }
        color(NORMAL);
        
        if (m_log_registers) {
            if (m_log) std::cout << "**** V REGISTERS ****" << std::endl;

            short reg = 0;
            while (reg < 8) {
                std::bitset<8> bits(m_CPU_regs[reg]);
                color(NORMAL);
                if (m_log) std::cout << "V" << reg << " = ";
                color(INFO);
                if (m_log) std::cout << bits;
                bits = (m_CPU_regs[reg + 8]);
                color(NORMAL);
                if (m_log) std::cout << "\t V" << 8 + reg++ << " = ";
                color(INFO);
                if (m_log) std::cout << bits << std::endl;
            }
            color(NORMAL);
            if (m_log) std::cout << "**** I REGISTER ****" << std::endl;
            std::bitset<16> bits(m_ir);
            color(INFO);
            if (m_log) std::cout << bits << std::endl;
            color(NORMAL);
        }
        /* Update timers */
        m_timer > 0 ? --m_timer : m_timer;
        if (m_snd_timer > 0 && m_snd_timer-- == 1) {
            if (m_log) std::cout << "[.] beep \a" << std::endl;
        }
    }
}
