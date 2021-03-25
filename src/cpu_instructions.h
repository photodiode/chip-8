

typedef void(*opcode_fp)();


static void nop() {
	printf("/!\\ nop (0x%x)\n", cpu.ci.raw);
}

// 00E0
static void clear_screen() {
	SDL_LockMutex(mem_lock);
	memset(GFX, 0x00, GFX_SIZE);
	SDL_UnlockMutex(mem_lock);
}

// 00EE
static void ret() {
	cpu.sp -= 1;
	cpu.pc = STACK(cpu.sp);
}

// 1nnn
static void jump() {
	cpu.pc = cpu.ci.addr;
}

// 2nnn
static void call() {
	STACK(cpu.sp) = cpu.pc;
	cpu.sp += 1;
	cpu.pc = cpu.ci.addr;
}

// 3xkk
static void skip_if_x_eq_k() {
	if (cpu.v[cpu.ci.x] == cpu.ci.k) {
		cpu.pc += 2;
	}
}

// 4xkk
static void skip_if_x_neq_k() {
	if (cpu.v[cpu.ci.x] != cpu.ci.k) {
		cpu.pc += 2;
	}
}

// 5xy0
static void skip_if_x_eq_y() {
	if (cpu.v[cpu.ci.x] == cpu.v[cpu.ci.y]) {
		cpu.pc += 2;
	}
}

// 6xkk
static void load_x_imm() {
	cpu.v[cpu.ci.x] = cpu.ci.k;
}

// 7xkk
static void add_x_imm() {
	cpu.v[cpu.ci.x] += cpu.ci.k;
}

// 8xy0
static void load_x_y() {
	cpu.v[cpu.ci.x] = cpu.v[cpu.ci.y];
}

// 8xy1
static void or_x_y() {
	cpu.v[cpu.ci.x] |= cpu.v[cpu.ci.y];
}

// 8xy2
static void and_x_y() {
	cpu.v[cpu.ci.x] &= cpu.v[cpu.ci.y];
}

// 8xy3
static void xor_x_y() {
	cpu.v[cpu.ci.x] ^= cpu.v[cpu.ci.y];
}

// 8xy4
static void add_x_y() {
	uint16_t result = (uint16_t)cpu.v[cpu.ci.x] + (uint16_t)cpu.v[cpu.ci.y];
	cpu.v[0xF] = result > 0xFF;
	cpu.v[cpu.ci.x] = result;
}

// 8xy5
static void sub_x_y() {
	cpu.v[0xF] = cpu.v[cpu.ci.x] > cpu.v[cpu.ci.y];
	cpu.v[cpu.ci.x] -= cpu.v[cpu.ci.y];
}

// 8xy6
static void shift_right_x() {
	cpu.v[0xF] = cpu.v[cpu.ci.x] & 0x1;
	cpu.v[cpu.ci.x] >>= 1;
}

// 8xy7
static void sub_y_x() {
	cpu.v[0xF] = cpu.v[cpu.ci.y] >= cpu.v[cpu.ci.x];
	cpu.v[cpu.ci.x] = cpu.v[cpu.ci.y] - cpu.v[cpu.ci.x];
}

// 8xyE
static void shift_left_x() {
	cpu.v[0xF] = cpu.v[cpu.ci.x] >> 7;
	cpu.v[cpu.ci.x] <<= 1;
}

// 9xy0
static void skip_if_x_neq_y() {
	if (cpu.v[cpu.ci.x] != cpu.v[cpu.ci.y]) {
		cpu.pc += 2;
	}
}

// Annn
static void load_i_imm() {
	cpu.i = cpu.ci.addr;
}

// Bnnn
static void jump_imm_v0() {
	cpu.pc = cpu.ci.addr + cpu.v[0];
}

// Cxkk
static void load_rnd() {
	cpu.v[cpu.ci.x] = rand() & cpu.ci.k;
}

// Dxyn
static void draw_sprite() {

	uint8_t vx = cpu.v[cpu.ci.x];
	uint8_t vy = cpu.v[cpu.ci.y];

	uint8_t x_byte = vx >> 3;
	uint8_t x_bit  = vx - (x_byte << 3);

	cpu.v[0xF] = 0;

	SDL_LockMutex(mem_lock);

	for(uint8_t y = 0; y < cpu.ci.n; y++) {

		uint8_t byte_a =   x_byte       + ((vy+y)%HEIGHT) * 8;
		uint8_t byte_b = ((x_byte+1)%8) + ((vy+y)%HEIGHT) * 8;
		
		// check for erased bit
		uint8_t cb = (GFX[byte_a] << x_bit) | (GFX[byte_b] >> (8 - x_bit));
		uint8_t nb = mem[cpu.i+y] ^ cb;

		for (uint8_t i = 0; i < 8; i++) {
			if (((cb >> i) & 0x1) == 1 &&
			    ((nb >> i) & 0x1) == 0) {
				cpu.v[0xF] = 1;
			}
		}
		// ----

		GFX[byte_a] ^= mem[cpu.i+y] >> x_bit;
		GFX[byte_b] ^= mem[cpu.i+y] << (8 - x_bit);
	}

	SDL_UnlockMutex(mem_lock);
}

// Ex9E
static void skip_if_xkey() {
	SDL_LockMutex(keypad_lock);
	if (keypad[cpu.v[cpu.ci.x]]) {
		cpu.pc += 2;
	}
	SDL_UnlockMutex(keypad_lock);
}

// ExA1
static void skip_if_not_xkey() {
	SDL_LockMutex(keypad_lock);
	if (!keypad[cpu.v[cpu.ci.x]]) {
		cpu.pc += 2;
	}
	SDL_UnlockMutex(keypad_lock);
}

// Fx07
static void load_x_dt() {
	SDL_LockMutex(timer_lock);
	cpu.v[cpu.ci.x] = timer_delay;
	SDL_UnlockMutex(timer_lock);
}

// Fx0A
static void load_key_wait() {

	SDL_LockMutex(keypad_cond_lock);
	while(!keypad_pressed) SDL_CondWait(keypad_cond, keypad_cond_lock);
	SDL_UnlockMutex(keypad_cond_lock);
	
	SDL_LockMutex(keypad_lock);
	cpu.v[cpu.ci.x] = keypad_pressed_key;
	SDL_UnlockMutex(keypad_lock);
}

// Fx15
static void load_dt_x() {
	SDL_LockMutex(timer_lock);
	timer_delay = cpu.v[cpu.ci.x];
	SDL_UnlockMutex(timer_lock);
}

// Fx18
static void load_st_x() {
	SDL_LockMutex(timer_lock);
	timer_sound = cpu.v[cpu.ci.x];
	SDL_UnlockMutex(timer_lock);
}

// Fx1E
static void add_i_x() {
	cpu.i += cpu.v[cpu.ci.x];
	cpu.v[0xF] = cpu.i > 0x0FFF;
}

// Fx29
static void load_font() {
	cpu.i = (uint16_t)cpu.v[cpu.ci.x] * 5;
}

// Fx33
static void load_i_bcd() {
	SDL_LockMutex(mem_lock);
	mem[cpu.i]     =  cpu.v[cpu.ci.x] / 100;
	mem[cpu.i + 1] = (cpu.v[cpu.ci.x] / 10) % 10;
	mem[cpu.i + 2] =  cpu.v[cpu.ci.x] % 10;
	SDL_UnlockMutex(mem_lock);
}

// Fx55
static void save_regs() {
	SDL_LockMutex(mem_lock);
	memcpy(&mem[cpu.i], cpu.v, cpu.ci.x+1);
	SDL_UnlockMutex(mem_lock);
}

// Fx65
static void load_regs() {
	SDL_LockMutex(mem_lock);
	memcpy(cpu.v, &mem[cpu.i], cpu.ci.x+1);
	SDL_UnlockMutex(mem_lock);
}








const opcode_fp op_0_list[] = {
	clear_screen,
	nop,
	nop,
	nop,
	nop,
	nop,
	nop,
	nop,
	nop,
	nop,
	nop,
	nop,
	nop,
	nop,
	ret,
	nop
};

static void op_0() {
	op_0_list[cpu.ci.n](cpu);
}


const opcode_fp op_8_list[] = {
	load_x_y,
	or_x_y,
	and_x_y,
	xor_x_y,
	add_x_y,
	sub_x_y,
	shift_right_x,
	sub_y_x,
	nop,
	nop,
	nop,
	nop,
	nop,
	nop,
	shift_left_x,
	nop
};

static void op_8() {
	op_8_list[cpu.ci.n](cpu);
}


const opcode_fp op_e_list[] = {
	skip_if_xkey,
	nop,
	nop,
	skip_if_not_xkey
};

static void op_e() {
	op_e_list[cpu.ci.k - 0x9E](cpu);
}


opcode_fp op_f_list[0x100] = {nop};

static void op_f() {

	op_f_list[0x07] = load_x_dt;
	op_f_list[0x0A] = load_key_wait;
	op_f_list[0x15] = load_dt_x;
	op_f_list[0x18] = load_st_x;
	op_f_list[0x1E] = add_i_x;
	op_f_list[0x29] = load_font;
	op_f_list[0x33] = load_i_bcd;
	op_f_list[0x55] = save_regs;
	op_f_list[0x65] = load_regs;
	
	op_f_list[cpu.ci.k](cpu);
}


const opcode_fp op_base_list[] = {
	op_0,
	jump,
	call,
	skip_if_x_eq_k,
	skip_if_x_neq_k,
	skip_if_x_eq_y,
	load_x_imm,
	add_x_imm,
	op_8,
	skip_if_x_neq_y,
	load_i_imm,
	jump_imm_v0,
	load_rnd,
	draw_sprite,
	op_e,
	op_f
};

static void execute_instruction() {

	// get instruction
	SDL_LockMutex(mem_lock);
	cpu.ci.upper = mem[cpu.pc];
	cpu.ci.lower = mem[cpu.pc+1];
	SDL_UnlockMutex(mem_lock);

	cpu.pc += 2;
	// ----
	
	op_base_list[cpu.ci.op](cpu);
}
