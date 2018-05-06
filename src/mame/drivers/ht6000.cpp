// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Casio HT-6000

    SD ("Spectrum Dynamic") Synthesizer

    Skeleton driver

***************************************************************************/

#include "emu.h"
#include "cpu/upd7810/upd7811.h"
#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ht6000_state : public driver_device
{
public:
	ht6000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_rom2(*this, "rom2"),
		m_switches(*this, "kc%u", 0),
		m_port_a(0),
		m_led_latch(0xff),
		m_ram_card_addr(0)
	{ }

	void ht6000(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_memory_region m_rom2;
	required_ioport_array<16> m_switches;

	void maincpu_map(address_map &map);

	DECLARE_WRITE8_MEMBER(port_a_w);

	DECLARE_WRITE8_MEMBER(music_w);
	DECLARE_WRITE8_MEMBER(pg1_w);
	DECLARE_WRITE8_MEMBER(pg2_w);
	DECLARE_WRITE8_MEMBER(pg3_w);
	DECLARE_WRITE8_MEMBER(led_w);
	DECLARE_WRITE8_MEMBER(led_addr_w);
	DECLARE_WRITE8_MEMBER(led_data_w);
	DECLARE_READ8_MEMBER(switches_r);
	DECLARE_READ8_MEMBER(keys_r);
	DECLARE_WRITE8_MEMBER(ram_card_l_w);
	DECLARE_WRITE8_MEMBER(ram_card_h_w);
	DECLARE_READ8_MEMBER(rom2_r);

	uint8_t m_port_a;
	uint8_t m_led_latch;
	uint16_t m_ram_card_addr;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void ht6000_state::maincpu_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xbfff).ram();
	map(0xc000, 0xcfff).w(this, FUNC(ht6000_state::music_w)); // UPD935G
	map(0xd000, 0xd0ff).w(this, FUNC(ht6000_state::pg1_w)); // MSM6294-07
	map(0xd100, 0xd1ff).w(this, FUNC(ht6000_state::pg2_w)); // MSM6294-08
	map(0xd200, 0xd2ff).w(this, FUNC(ht6000_state::pg3_w)); // MSM6294-09
	map(0xd300, 0xd3ff).w(this, FUNC(ht6000_state::led_w));
	map(0xd400, 0xd4ff).w(this, FUNC(ht6000_state::led_addr_w));
	map(0xd500, 0xd5ff).w(this, FUNC(ht6000_state::led_data_w));
	map(0xd600, 0xd6ff).r(this, FUNC(ht6000_state::switches_r));
	map(0xd700, 0xd7ff).r(this, FUNC(ht6000_state::keys_r));
	map(0xd800, 0xd8ff).w(this, FUNC(ht6000_state::ram_card_l_w));
	map(0xd900, 0xd9ff).w(this, FUNC(ht6000_state::ram_card_h_w));
	map(0xe000, 0xefff).r(this, FUNC(ht6000_state::rom2_r));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( ht6000 )
	PORT_START("kc0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UPPER SYNTH. ENS.")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UPPER COSMIC DANCE")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UPPER STRING ENS.")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UPPER BRASS ENS.")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UPPER PIPE ORGAN")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LINE 1")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LINE 1")

	PORT_START("kc1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UPPER PIANO")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UPPER HARPSICHORD")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UPPER GUITAR")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UPPER TRUMPET")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UPPER VIBRAPHONE")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UPPER TONE SELECT")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LINE 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LINE 2")

	PORT_START("kc2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOWER SYNTH. ENS.")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOWER BRASS ENS.")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOWER SYNTH. BRASS")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOWER COSMIC DANCE")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOWER JAZZ ORGAN")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOWER BASS/OBBLI.")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LINE 3")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LINE 3")

	PORT_START("kc3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOWER PIANO")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOWER HARPSICHORD")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOWER HARP")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOWER GUITAR")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOWER SYNTH. GUITAR")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ACCOMP. VARIATION")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("kc4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ROCK")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 BEAT")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("16 BEAT")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DISCO")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("POPS")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RHYTHM VARIATION")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LINE 4")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LINE 4")

	PORT_START("kc5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SWING")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SLOW ROCK")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SAMBA")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BOSSA NOVA")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("WALTZ")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RHYTHM PRESET A")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DETUNE")

	PORT_START("kc6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOWER PRESET")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOWER INTERNAL")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOWER CARD")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RHYTHM PRESET B")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("VELOCITY")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("AMPLITUDE LEVEL")

	PORT_START("kc7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UPPER PRESET")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UPPER INTERNAL")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UPPER CARD")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("AUTO HARMONIZE")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UPPER EDIT")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOWER EDIT")

	PORT_START("kc8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RHYTHM INTERNAL")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RHYTHM CARD")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PATTERN/MIDI")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("WRITE")

	PORT_START("kc9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INTRO/ENDING")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CHORD MEM. RECORD/DELETE")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("OP. MEM. RECORD")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CHORD/OP. MEM. SELECT")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SYNCHRO FILL-IN")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("START/STOP")

	PORT_START("kc10")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NORMAL")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SPLIT")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("FING'D")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CASIO CHORD")

	PORT_START("kc11")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("kc12")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("kc13")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("kc14")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("kc15")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

WRITE8_MEMBER( ht6000_state::port_a_w )
{
	logerror("port_a_w: %02x\n", data);
	m_port_a = data;
}

WRITE8_MEMBER( ht6000_state::music_w )
{
	// a8-a11 selects the chip (there are 4), a6-a7 selects command or data
	logerror("music_w: offset = %02x, %02x = %02x\n", offset >> 8, (offset >> 6 & 0x03), data);
}

WRITE8_MEMBER( ht6000_state::pg1_w )
{
	logerror("pg1_w: %02x\n", data);
}

WRITE8_MEMBER( ht6000_state::pg2_w )
{
	logerror("pg2_w: %02x\n", data);
}

WRITE8_MEMBER( ht6000_state::pg3_w )
{
	logerror("pg3_w: %02x\n", data);
}

WRITE8_MEMBER( ht6000_state::led_w )
{
	logerror("led_w: %02x\n", data);
}

WRITE8_MEMBER( ht6000_state::led_addr_w )
{
	if (data != 0x00)
	{
		logerror("led matrix %02x = %02x\n", data, m_led_latch);
	}
}

WRITE8_MEMBER( ht6000_state::led_data_w )
{
	m_led_latch = data;
}

READ8_MEMBER( ht6000_state::switches_r )
{
	return m_switches[m_port_a & 0x0f]->read();
}

READ8_MEMBER( ht6000_state::keys_r )
{
	return 0;
}

WRITE8_MEMBER( ht6000_state::ram_card_l_w )
{
	// a0-a7
	m_ram_card_addr = (m_ram_card_addr & 0xff00) | (data << 0);
}

WRITE8_MEMBER( ht6000_state::ram_card_h_w )
{
	data &= 0x1f; // a8-a12
	m_ram_card_addr = (m_ram_card_addr & 0x00ff) | (data << 8);
}

READ8_MEMBER( ht6000_state::rom2_r )
{
	// a12-14 from port a
	offs_t addr = ((m_port_a >> 4) & 0x07) << 12;
	return m_rom2->base()[addr | offset];
}

void ht6000_state::machine_start()
{

}

void ht6000_state::machine_reset()
{

}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

MACHINE_CONFIG_START( ht6000_state::ht6000 )
	MCFG_DEVICE_ADD("maincpu", UPD7810, 12_MHz_XTAL)
	MCFG_DEVICE_PROGRAM_MAP(maincpu_map)
	MCFG_UPD7810_PORTA_WRITE_CB(WRITE8(*this, ht6000_state, port_a_w))

	MCFG_DEVICE_ADD("keycpu", I8049, 10_MHz_XTAL)
MACHINE_CONFIG_END


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( ht6000 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("eac-067.bin", 0x0000, 0x8000, CRC(c3063c07) SHA1(f012add068d7d765bcb701ad372c0bab3302a776))

	ROM_REGION(0x8000, "rom2", 0)
	ROM_LOAD("eac-068.bin", 0x0000, 0x8000, CRC(bc28b60d) SHA1(6f4be2861adea57352f0d52c61e004a5c022854a))

	ROM_REGION(0x800, "keycpu", 0)
	ROM_LOAD("187_8734h7.bin", 0x000, 0x800, CRC(47b47af7) SHA1(8f0515f95dcc6e224a8a59e0c2cd7ddb4796e34e))
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT   CLASS         INIT  COMPANY  FULLNAME   FLAGS
CONS( 1987, ht6000, 0,      0,       ht6000,  ht6000, ht6000_state, 0,    "Casio", "HT-6000", MACHINE_IS_SKELETON )
