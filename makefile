CC=g++
CFLAGS=-Wall -g -O3
LIBS=-pthread

CORE_INC=-I sim_src/core/
DEVICE_INC=-I sim_src/devices/
UTIL_INC=-I sim_src/utils/
TP_INC=-I void_thread_pool/
GOL_INC=-I sim_src/devices/game_of_life/
MD_INC=-I sim_src/magic_devices/
	
CORE_SRC:=$(wildcard sim_src/core/*.cpp)
DEVICES_SRC:=$(wildcard sim_src/devices/*.cpp)
UTILS_SRC:=$(wildcard sim_src/utils/*.cpp)
TP_SRC:=$(wildcard void_thread_pool/*.cpp)
GOL_SRC:=$(wildcard sim_src/devices/game_of_life/*.cpp)
MD_SRC:=$(wildcard sim_src/magic_devices/*.cpp)

naked_jk_ff_demo: naked_jk_ff_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $@.cpp -o $@

jk_ff_demo: jk_ff_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $@.cpp -o $@

jk_ff_aspc_demo: jk_ff_aspc_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $@.cpp -o $@

4_bit_counter_demo: 4_bit_counter_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $@.cpp -o $@

n_bit_counter_demo: n_bit_counter_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $@.cpp -o $@

n_bit_counter_asc_demo: n_bit_counter_asc_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $@.cpp -o $@

n_bit_counter_c_asc_demo: n_bit_counter_c_asc_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $@.cpp -o $@

n_bit_decoder_demo: n_bit_decoder_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $@.cpp -o $@

1_bit_register_demo: 1_bit_register_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $@.cpp -o $@

n_bit_register_demo: n_bit_register_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $@.cpp -o $@

n_x_1_bit_mux_demo: n_x_1_bit_mux_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $@.cpp -o $@

n_x_m_bit_mux_demo: n_x_m_bit_mux_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $@.cpp -o $@

timing_test: timing_test.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $@.cpp -o $@

really_long_inverter_chain: really_long_inverter_chain.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $@.cpp -o $@

simple_rom_demo: simple_rom_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(MD_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(MD_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(MD_SRC) $(UTILS_SRC) $(TP_SRC) $@.cpp -o $@

simple_ram_demo: simple_ram_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(MD_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(MD_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(MD_SRC) $(UTILS_SRC) $(TP_SRC) $@.cpp -o $@

simple_terminal_demo: simple_terminal_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(MD_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(MD_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(MD_SRC) $(UTILS_SRC) $(TP_SRC) $@.cpp -o $@

game_of_life_cell_demo: game_of_life_cell_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $(GOL_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(GOL_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $(GOL_SRC) $@.cpp -o $@

gol_cell_descision_demo: gol_cell_descision_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $(GOL_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(GOL_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $(GOL_SRC) $@.cpp -o $@

gol_cell_sibling_selector_demo: gol_cell_sibling_selector_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $(GOL_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(GOL_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $(GOL_SRC) $@.cpp -o $@

n_x_n_game_of_life_demo: n_x_n_game_of_life_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $(GOL_SRC)
	$(CC) $(LIBS) $(CFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(GOL_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $(GOL_SRC) $@.cpp -o $@
