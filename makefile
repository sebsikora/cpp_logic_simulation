CPPC=g++
#CPPFLAGS=-Wall -g -O3 -DVERBOSE_SOLVE
CPPFLAGS=-Wall -g -O3
CPPLIBS=-pthread

CC=gcc
CLIBS=-lncurses

CORE_INC=-I sim_src/core/
DEVICE_INC=-I sim_src/devices/
UTIL_INC=-I sim_src/utils/
TP_INC=-I void_thread_pool/
GOL_INC=-I sim_src/devices/game_of_life/
MD_INC=-I sim_src/magic_devices/
SD_INC=-I sim_src/special_devices/

CORE_SRC:=$(wildcard sim_src/core/*.cpp)
DEVICES_SRC:=$(wildcard sim_src/devices/*.cpp)
UTILS_SRC:=$(wildcard sim_src/utils/*.cpp)
TP_SRC:=$(wildcard void_thread_pool/*.cpp)
GOL_SRC:=$(wildcard sim_src/devices/game_of_life/*.cpp)
MD_SRC:=$(wildcard sim_src/magic_devices/*.cpp)
SD_SRC:=$(wildcard sim_src/special_devices/*.cpp)

sr_latch_demo: demos_src/sr_latch_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

sr_latch_demo_2: demos_src/sr_latch_demo_2.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

quad_sr_latch_demo: demos_src/quad_sr_latch_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

n_bit_sr_latch_demo: demos_src/n_bit_sr_latch_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

naked_jk_ff_demo: demos_src/naked_jk_ff_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

jk_ff_demo: demos_src/jk_ff_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

jk_ff_aspc_demo: demos_src/jk_ff_aspc_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

4_bit_counter_demo: demos_src/4_bit_counter_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

n_bit_counter_demo: demos_src/n_bit_counter_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

n_bit_counter_aio_demo: demos_src/n_bit_counter_aio_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

n_bit_counter_asc_demo: demos_src/n_bit_counter_asc_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

n_bit_counter_c_asc_demo: demos_src/n_bit_counter_c_asc_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

n_bit_decoder_demo: demos_src/n_bit_decoder_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

1_bit_register_demo: demos_src/1_bit_register_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

n_bit_register_demo: demos_src/n_bit_register_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

n_bit_register_asc_aio_demo: demos_src/n_bit_register_asc_aio_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

n_x_1_bit_mux_demo: demos_src/n_x_1_bit_mux_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

n_x_m_bit_mux_demo: demos_src/n_x_m_bit_mux_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

timing_test: demos_src/timing_test.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

timing_test_3: demos_src/timing_test_3.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

really_long_inverter_chain: demos_src/really_long_inverter_chain.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

ram_demo: demos_src/ram_demo.cpp $(CORE_SRC) $(SD_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(SD_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(SD_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

rom_demo: demos_src/rom_demo.cpp $(CORE_SRC) $(SD_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(SD_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(SD_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

ram_speed_test: demos_src/ram_speed_test.cpp $(CORE_SRC) $(SD_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(SD_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(SD_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

simple_terminal_demo: demos_src/simple_terminal_demo.cpp sim_src/simple_terminal_client/terminal_client.c $(CORE_SRC) $(MD_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CC) $(CLIBS) sim_src/simple_terminal_client/terminal_client.c -o sim_src/simple_terminal_client/terminal_client
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(MD_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(MD_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@

game_of_life_cell_demo: demos_src/game_of_life_cell_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $(GOL_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(GOL_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $(GOL_SRC) demos_src/$@.cpp -o $@

gol_cell_descision_demo: demos_src/gol_cell_descision_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $(GOL_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(GOL_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $(GOL_SRC) demos_src/$@.cpp -o $@

gol_cell_sibling_selector_demo: demos_src/gol_cell_sibling_selector_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $(GOL_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(GOL_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $(GOL_SRC) demos_src/$@.cpp -o $@

n_x_n_game_of_life_demo: demos_src/n_x_n_game_of_life_demo.cpp $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $(GOL_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(DEVICE_INC) $(UTIL_INC) $(TP_INC) $(GOL_INC) $(CORE_SRC) $(DEVICES_SRC) $(UTILS_SRC) $(TP_SRC) $(GOL_SRC) demos_src/$@.cpp -o $@

control_panel_demo: demos_src/control_panel_demo.cpp $(CORE_SRC) $(MD_SRC) $(UTILS_SRC) $(TP_SRC)
	$(CPPC) $(CPPLIBS) $(CPPFLAGS) $(CORE_INC) $(MD_INC) $(UTIL_INC) $(TP_INC) $(CORE_SRC) $(MD_SRC) $(UTILS_SRC) $(TP_SRC) demos_src/$@.cpp -o $@
