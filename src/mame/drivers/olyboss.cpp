// license:BSD-3-Clause
// copyright-holders: Gabriele D'Antona
/*
    Olympia BOSS
    Made in Germany around 1981

    The BOSS series was not a great success, as its members differed too much to be compatible:
    First they were 8085 based, later machines used a Z80A.

    Other distinguishing features were the capacity of the disk drives:

    BOSS A: Two 128K floppy drives
    BOSS B: Two 256K disk drives
    BOSS C: Two 600K disk drives
    BOSS D: One 600K disk drive, one 5 MB harddisk
    BOSS M: M for multipost, up to four BOSS machines linked together for up to 20MB shared harddisk space

    Olympia favoured the French Prologue operating system over CPM (cf. Olympia People PC) and supplied BAL
    as a programming language with it.

    Video is 80x28

    There are no service manuals available (or no documentation in general), so everything is guesswork.

    - Ports 0x80 and 0x81 seem to be related to the graphics chip and cursor position
    The rom outs value 0x81 to port 0x81 and then the sequence <column> <row> (?) to port 0x80

    - The machine boots up and shows "BOSS .." on the screen. Every keystroke is repeated on screen.
    If you press <return>, the machine seems to go into a boot sequence (from the HD, probably)

    The harddisk controller is based on a MSC-9056.

    Links: http://www.old-computers.com/museum/computer.asp?c=95
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/i8085/i8085.h"
#include "machine/keyboard.h"
#include "video/upd3301.h"
#include "machine/i8257.h"
#include "machine/i8255.h"
#include "machine/am9519.h"
#include "machine/upd765.h"
#include "machine/pic8259.h"
#include "machine/i8251.h"
#include "screen.h"

#define UPD3301_TAG     "upd3301"
#define I8257_TAG       "i8257"
#define SCREEN_TAG      "screen"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class olyboss_state : public driver_device
{
public:
	olyboss_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_dma(*this, I8257_TAG),
			m_crtc(*this,UPD3301_TAG),
			m_fdc(*this, "fdc"),
			m_uic(*this, "uic"),
			m_pic(*this, "pic"),
			m_ppi(*this, "ppi"),
			m_fdd0(*this, "fdc:0"),
			m_fdd1(*this, "fdc:1"),
			m_rom(*this, "mainrom"),
			m_lowram(*this, "lowram"),
			m_char_rom(*this, UPD3301_TAG)
		{ }

public:
	void bossa85(machine_config &config);
	void bossb85(machine_config &config);
	void olybossb(machine_config &config);
	void olybossc(machine_config &config);
	void olybossd(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	DECLARE_READ8_MEMBER(keyboard_read);

	UPD3301_DRAW_CHARACTER_MEMBER( olyboss_display_pixels );

	DECLARE_WRITE_LINE_MEMBER( hrq_w );
	DECLARE_WRITE_LINE_MEMBER( tc_w );
	DECLARE_WRITE_LINE_MEMBER( romdis_w );
	DECLARE_READ8_MEMBER( dma_mem_r );
	DECLARE_WRITE8_MEMBER( dma_mem_w );
	DECLARE_READ8_MEMBER( fdcctrl_r );
	DECLARE_WRITE8_MEMBER( fdcctrl_w );
	DECLARE_WRITE8_MEMBER( fdcctrl85_w );
	DECLARE_READ8_MEMBER( fdcdma_r );
	DECLARE_WRITE8_MEMBER( fdcdma_w );
	DECLARE_WRITE8_MEMBER( crtcdma_w );
	DECLARE_READ8_MEMBER( rom_r );
	DECLARE_WRITE8_MEMBER( rom_w );
	DECLARE_WRITE8_MEMBER( vchrmap_w );
	DECLARE_WRITE8_MEMBER( vchrram_w );
	DECLARE_WRITE8_MEMBER( vchrram85_w );
	DECLARE_WRITE8_MEMBER( ppic_w );
	void olyboss_io(address_map &map);
	void olyboss_mem(address_map &map);
	void olyboss85_io(address_map &map);
	IRQ_CALLBACK_MEMBER(irq_cb);

	required_device<cpu_device> m_maincpu;
	required_device<i8257_device> m_dma;
	required_device<upd3301_device> m_crtc;
	required_device<upd765a_device> m_fdc;
	optional_device<am9519_device> m_uic;
	optional_device<pic8259_device> m_pic;
	optional_device<i8255_device> m_ppi;
	required_device<floppy_connector> m_fdd0;
	optional_device<floppy_connector> m_fdd1;
	required_memory_region m_rom;
	required_shared_ptr<u8> m_lowram;
	required_memory_region m_char_rom;

	bool m_keybhit;
	u8 m_keystroke;
	void keyboard_put(u8 data);
	void keyboard85_put(u8 data);
	u8 m_fdcctrl, m_fdctype;
	u8 m_channel, m_vchrmap, m_vchrpage;
	u16 m_vchraddr;
	u8 m_vchrram[0x800];
	bool m_romen, m_timstate;
	emu_timer *m_timer;
};

void olyboss_state::machine_reset()
{
	m_keybhit=false;
	m_romen = true;
	m_timstate = false;

	m_fdcctrl = 0;
	m_vchrmap = 0;
	m_vchrpage = 0;
	m_timer->adjust(attotime::from_hz(30), 0, attotime::from_hz(30)); // unknown timer freq, possibly com2651 BRCLK
}

void olyboss_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_timstate = !m_timstate;
	if(m_pic)
		m_pic->ir0_w(m_timstate);
	else
		m_uic->ireq7_w(m_timstate);
}

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void olyboss_state::olyboss_mem(address_map &map)
{
	map(0x0000, 0x7ff).rw(this, FUNC(olyboss_state::rom_r), FUNC(olyboss_state::rom_w)).share("lowram");
	map(0x800, 0xffff).ram();
}

void olyboss_state::olyboss_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x0, 0x8).rw(m_dma, FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0x10, 0x11).m(m_fdc, FUNC(upd765a_device::map));
	//AM_RANGE(0x20, 0x20) //beeper?
	map(0x30, 0x30).rw(m_uic, FUNC(am9519_device::data_r), FUNC(am9519_device::data_w));
	map(0x31, 0x31).rw(m_uic, FUNC(am9519_device::stat_r), FUNC(am9519_device::cmd_w));
	map(0x40, 0x43).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	//AM_RANGE(0x50, 0x53) COM2651
	map(0x60, 0x60).rw(this, FUNC(olyboss_state::fdcctrl_r), FUNC(olyboss_state::fdcctrl_w));
	map(0x80, 0x81).rw(m_crtc, FUNC(upd3301_device::read), FUNC(upd3301_device::write));
	map(0x82, 0x84).w(this, FUNC(olyboss_state::vchrmap_w));
	map(0x90, 0x9f).w(this, FUNC(olyboss_state::vchrram_w));
}

void olyboss_state::olyboss85_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x0, 0x8).rw(m_dma, FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0x10, 0x11).m(m_fdc, FUNC(upd765a_device::map));
	map(0x20, 0x21).rw(m_crtc, FUNC(upd3301_device::read), FUNC(upd3301_device::write));
	map(0x30, 0x31).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x42, 0x42).r(this, FUNC(olyboss_state::keyboard_read));
	map(0x42, 0x44).w(this, FUNC(olyboss_state::vchrram85_w));
	map(0x45, 0x45).w(this, FUNC(olyboss_state::fdcctrl85_w));
}

static INPUT_PORTS_START( olyboss )
	PORT_START("DSW")
INPUT_PORTS_END

READ8_MEMBER( olyboss_state::rom_r )
{
	return m_romen ?  m_rom->as_u8(offset) : m_lowram[offset];
}

WRITE8_MEMBER( olyboss_state::rom_w )
{
	m_lowram[offset] = data;
}

WRITE8_MEMBER( olyboss_state::vchrram85_w )
{
	switch(offset)
	{
		case 0:
			m_vchraddr = (m_vchraddr & 0x00f) | (data << 4);
			break;
		case 1:
			m_vchraddr = (m_vchraddr & 0xff0) | (data & 0xf);
			break;
		case 2:
			m_vchrram[m_vchraddr] = data;
			break;
	}
}

WRITE8_MEMBER( olyboss_state::vchrmap_w )
{
	switch(offset)
	{
		case 0:
			m_vchrmap = data;
			break;
		case 2:
			m_vchrpage = data & 0x7f;
			break;
	}
}

WRITE8_MEMBER( olyboss_state::vchrram_w )
{
	m_vchrram[(m_vchrpage << 4) + (offset ^ 0xf)] = data;
}

WRITE_LINE_MEMBER( olyboss_state::romdis_w )
{
	m_romen = state ? false : true;
}

IRQ_CALLBACK_MEMBER( olyboss_state::irq_cb )
{
	if(!irqline)
		return m_pic->acknowledge();
	return 0;
}

//**************************************************************************
//  VIDEO
//**************************************************************************

UPD3301_DRAW_CHARACTER_MEMBER( olyboss_state::olyboss_display_pixels )
{
	uint8_t data = cc & 0x7f;
	if(cc & 0x80)
		data = m_vchrram[(data << 4) | lc];
	else
		data = m_char_rom->base()[(data << 4) | lc];
	int i;

	//if (lc >= 8) return;
	if (csr)
	{
		data = 0xff;
	}

	for (i = 0; i < 8; i++)
	{
		int color = BIT(data, 7) ^ rvv;
		bitmap.pix32(y, (sx * 8) + i) = color?0xffffff:0;
		data <<= 1;
	}
}

//**************************************************************************
//  KEYBOARD
//**************************************************************************

READ8_MEMBER( olyboss_state::keyboard_read )
{
	//logerror ("keyboard_read offs [%d]\n",offset);
	if (m_keybhit)
	{
		m_keybhit=false;
		if(m_pic)
			m_pic->ir1_w(CLEAR_LINE);
		return m_keystroke;
	}
	return 0x00;
}

WRITE8_MEMBER( olyboss_state::ppic_w )
{
	m_uic->ireq4_w(BIT(data, 5) ? CLEAR_LINE : ASSERT_LINE);
	m_fdcctrl = (m_fdcctrl & ~0x10) | (BIT(data, 5) ? 0x10 : 0);
}

void olyboss_state::machine_start()
{
	m_timer = timer_alloc();
	const char *type = m_fdd0->get_device()->shortname();
	if(!strncmp(type, "floppy_525_qd", 13))
		m_fdctype = 0xa0;
	else
		m_fdctype = 0x80;
}

void olyboss_state::keyboard_put(u8 data)
{
	if (data)
	{
		//logerror("Keyboard stroke [%2x]\n",data);
		m_keystroke=data ^ 0xff;
		m_keybhit=true;
		m_ppi->pc4_w(ASSERT_LINE);
		m_ppi->pc4_w(CLEAR_LINE);
	}
}

void olyboss_state::keyboard85_put(u8 data)
{
	if(data)
	{
		m_pic->ir1_w(ASSERT_LINE);
		m_keybhit = true;
		m_keystroke = data;
	}
}

/* 8257 Interface */

WRITE_LINE_MEMBER( olyboss_state::hrq_w )
{
	//logerror("hrq_w\n");
	m_maincpu->set_input_line(INPUT_LINE_HALT,state);
	m_dma->hlda_w(state);
}

WRITE_LINE_MEMBER( olyboss_state::tc_w )
{
	if((m_channel == 0) && state)
	{
		m_fdc->tc_w(1);
		m_fdc->tc_w(0);
	}
}

READ8_MEMBER( olyboss_state::dma_mem_r )
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	return program.read_byte(offset);
}

WRITE8_MEMBER( olyboss_state::dma_mem_w )
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.write_byte(offset, data);
}

READ8_MEMBER( olyboss_state::fdcdma_r )
{
	m_channel = 0;
	return m_fdc->dma_r();
}

WRITE8_MEMBER( olyboss_state::fdcdma_w )
{
	m_channel = 0;
	m_fdc->dma_w(data);
}

WRITE8_MEMBER( olyboss_state::crtcdma_w )
{
	m_channel = 2;
	m_crtc->dack_w(space, offset, data, mem_mask);
}

READ8_MEMBER( olyboss_state::fdcctrl_r )
{
	return m_fdcctrl | m_fdctype; // 0xc0 seems to indicate an 8" drive, 0x80 a 5.25" dd drive, 0xa0 a 5.25" qd drive
}

WRITE8_MEMBER( olyboss_state::fdcctrl_w )
{
	m_fdcctrl = data;
	m_romen = (m_fdcctrl & 1) ? false : true;
	m_fdd0->get_device()->mon_w(!(data & 2));
	if(m_fdd1)
		m_fdd1->get_device()->mon_w(!(data & 4));
}

WRITE8_MEMBER( olyboss_state::fdcctrl85_w )
{
	m_fdcctrl = data;
	m_fdd0->get_device()->mon_w(!(data & 0x40));
	if(m_fdd1)
		m_fdd1->get_device()->mon_w(!(data & 0x80));
}

static void bossa_floppies(device_slot_interface &device)
{
	device.option_add("525ssdd", FLOPPY_525_SSDD);
}

static void bossb_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

static void bosscd_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

MACHINE_CONFIG_START( olyboss_state::olybossd )
	MCFG_DEVICE_ADD("maincpu", Z80, 4_MHz_XTAL)
	MCFG_DEVICE_PROGRAM_MAP(olyboss_mem)
	MCFG_DEVICE_IO_MAP(olyboss_io)
	MCFG_DEVICE_IRQ_ACKNOWLEDGE_DEVICE("uic", am9519_device, iack_cb)

	/* video hardware */

	MCFG_SCREEN_ADD_MONOCHROME(SCREEN_TAG, RASTER, rgb_t::green())
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DEVICE(UPD3301_TAG, upd3301_device, screen_update)
	MCFG_SCREEN_SIZE(80*8, 28*11)
	MCFG_SCREEN_VISIBLE_AREA(0, (80*8)-1, 0, (28*11)-1)

	/* devices */

	MCFG_DEVICE_ADD("uic", AM9519, 0)
	MCFG_AM9519_OUT_INT_CB(INPUTLINE("maincpu", 0))

	MCFG_UPD765A_ADD("fdc", true, true)
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE("uic", am9519_device, ireq2_w)) MCFG_DEVCB_INVERT
	MCFG_UPD765_DRQ_CALLBACK(WRITELINE(I8257_TAG, i8257_device, dreq0_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", bosscd_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	MCFG_DEVICE_ADD(I8257_TAG, I8257, XTAL(4'000'000))
	MCFG_I8257_OUT_HRQ_CB(WRITELINE(*this, olyboss_state, hrq_w))
	MCFG_I8257_IN_MEMR_CB(READ8(*this, olyboss_state, dma_mem_r))
	MCFG_I8257_OUT_MEMW_CB(WRITE8(*this, olyboss_state, dma_mem_w))
	MCFG_I8257_IN_IOR_0_CB(READ8(*this, olyboss_state, fdcdma_r))
	MCFG_I8257_OUT_IOW_0_CB(WRITE8(*this, olyboss_state, fdcdma_w))
	MCFG_I8257_OUT_IOW_2_CB(WRITE8(*this, olyboss_state, crtcdma_w))
	MCFG_I8257_OUT_TC_CB(WRITELINE(*this, olyboss_state, tc_w))

	MCFG_DEVICE_ADD(UPD3301_TAG, UPD3301, XTAL(14'318'181))
	MCFG_UPD3301_CHARACTER_WIDTH(8)
	MCFG_UPD3301_DRAW_CHARACTER_CALLBACK_OWNER(olyboss_state, olyboss_display_pixels)
	MCFG_UPD3301_DRQ_CALLBACK(WRITELINE(I8257_TAG, i8257_device, dreq2_w))
	MCFG_UPD3301_INT_CALLBACK(WRITELINE("uic", am9519_device, ireq0_w)) MCFG_DEVCB_INVERT
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)

	MCFG_DEVICE_ADD("ppi", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(*this, olyboss_state, keyboard_read))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(*this, olyboss_state, ppic_w))

	/* keyboard */
	MCFG_DEVICE_ADD("keyboard", GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(PUT(olyboss_state, keyboard_put))
MACHINE_CONFIG_END

MACHINE_CONFIG_START( olyboss_state::olybossb )
	olybossd(config);
	MCFG_DEVICE_REMOVE("fdc:0")
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", bossb_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", bossb_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

MACHINE_CONFIG_START( olyboss_state::olybossc )
	olybossd(config);
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", bosscd_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

MACHINE_CONFIG_START( olyboss_state::bossb85 )
	MCFG_DEVICE_ADD("maincpu", I8085A, 4_MHz_XTAL)
	MCFG_DEVICE_PROGRAM_MAP(olyboss_mem)
	MCFG_DEVICE_IO_MAP(olyboss85_io)
	MCFG_DEVICE_IRQ_ACKNOWLEDGE_DRIVER(olyboss_state, irq_cb)
	MCFG_I8085A_SOD(WRITELINE(*this, olyboss_state, romdis_w))

	/* video hardware */

	MCFG_SCREEN_ADD_MONOCHROME(SCREEN_TAG, RASTER, rgb_t::green())
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DEVICE(UPD3301_TAG, upd3301_device, screen_update)
	MCFG_SCREEN_SIZE(80*8, 28*11)
	MCFG_SCREEN_VISIBLE_AREA(0, (80*8)-1, 0, (28*11)-1)

	/* devices */

	MCFG_DEVICE_ADD("pic", PIC8259, 0)
	MCFG_PIC8259_OUT_INT_CB(INPUTLINE("maincpu", 0))

	MCFG_UPD765A_ADD("fdc", true, true)
	MCFG_UPD765_INTRQ_CALLBACK(INPUTLINE("maincpu", I8085_RST65_LINE))
	MCFG_UPD765_DRQ_CALLBACK(WRITELINE(I8257_TAG, i8257_device, dreq0_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", bossb_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", bossb_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	MCFG_DEVICE_ADD(I8257_TAG, I8257, XTAL(4'000'000))
	MCFG_I8257_OUT_HRQ_CB(WRITELINE(*this, olyboss_state, hrq_w))
	MCFG_I8257_IN_MEMR_CB(READ8(*this, olyboss_state, dma_mem_r))
	MCFG_I8257_OUT_MEMW_CB(WRITE8(*this, olyboss_state, dma_mem_w))
	MCFG_I8257_IN_IOR_0_CB(READ8(*this, olyboss_state, fdcdma_r))
	MCFG_I8257_OUT_IOW_0_CB(WRITE8(*this, olyboss_state, fdcdma_w))
	MCFG_I8257_OUT_IOW_2_CB(WRITE8(*this, olyboss_state, crtcdma_w))
	MCFG_I8257_OUT_TC_CB(WRITELINE(*this, olyboss_state, tc_w))

	MCFG_DEVICE_ADD(UPD3301_TAG, UPD3301, XTAL(14'318'181))
	MCFG_UPD3301_CHARACTER_WIDTH(8)
	MCFG_UPD3301_DRAW_CHARACTER_CALLBACK_OWNER(olyboss_state, olyboss_display_pixels)
	MCFG_UPD3301_DRQ_CALLBACK(WRITELINE(I8257_TAG, i8257_device, dreq2_w))
	MCFG_UPD3301_INT_CALLBACK(INPUTLINE("maincpu", I8085_RST75_LINE))
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)

	/* keyboard */
	MCFG_DEVICE_ADD("keyboard", GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(PUT(olyboss_state, keyboard85_put))
MACHINE_CONFIG_END

MACHINE_CONFIG_START( olyboss_state::bossa85 )
	bossb85(config);
	MCFG_DEVICE_REMOVE("fdc:0")
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", bossa_floppies, "525ssdd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_DEVICE_REMOVE("fdc:1")
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", bossa_floppies, "525ssdd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************
ROM_START( bossa85 )
	ROM_REGION(0x800, "mainrom", ROMREGION_ERASEFF)
	ROM_LOAD( "boss_8085_bios.bin", 0x0000, 0x800, CRC(43030231) SHA1(a1f6546a9dc1066324e93e5eed886f2313678180) )

	ROM_REGION( 0x800, UPD3301_TAG, 0)
	ROM_LOAD( "olympia_boss_graphics_251-461.bin", 0x0000, 0x800, CRC(56149540) SHA1(b2b893bd219308fc98a38528beb7ddae391c7609) )
ROM_END

ROM_START( bossb85 )
	ROM_REGION(0x800, "mainrom", ROMREGION_ERASEFF)
	ROM_LOAD( "boss_8085_bios.bin", 0x0000, 0x800, CRC(43030231) SHA1(a1f6546a9dc1066324e93e5eed886f2313678180) )

	ROM_REGION( 0x800, UPD3301_TAG, 0)
	ROM_LOAD( "olympia_boss_graphics_251-461.bin", 0x0000, 0x800, CRC(56149540) SHA1(b2b893bd219308fc98a38528beb7ddae391c7609) )
ROM_END

ROM_START( olybossb )                                           // verified: BOSS B uses the same ROMs as D, so C is safe to assume as well
	ROM_REGION(0x800, "mainrom", ROMREGION_ERASEFF)
	ROM_LOAD( "olympia_boss_system_251-462.bin", 0x0000, 0x800, CRC(01b99609) SHA1(07b764c36337c12f7b40aa309b0805ceed8b22e2) )

	ROM_REGION( 0x800, UPD3301_TAG, 0)
	ROM_LOAD( "olympia_boss_graphics_251-461.bin", 0x0000, 0x800, CRC(56149540) SHA1(b2b893bd219308fc98a38528beb7ddae391c7609) )
ROM_END

ROM_START( olybossc )
	ROM_REGION(0x800, "mainrom", ROMREGION_ERASEFF)
	ROM_LOAD( "olympia_boss_system_251-462.bin", 0x0000, 0x800, CRC(01b99609) SHA1(07b764c36337c12f7b40aa309b0805ceed8b22e2) )

	ROM_REGION( 0x800, UPD3301_TAG, 0)
	ROM_LOAD( "olympia_boss_graphics_251-461.bin", 0x0000, 0x800, CRC(56149540) SHA1(b2b893bd219308fc98a38528beb7ddae391c7609) )
ROM_END

ROM_START( olybossd )
	ROM_REGION(0x800, "mainrom", ROMREGION_ERASEFF)
	ROM_LOAD( "olympia_boss_system_251-462.bin", 0x0000, 0x800, CRC(01b99609) SHA1(07b764c36337c12f7b40aa309b0805ceed8b22e2) )

	ROM_REGION( 0x800, UPD3301_TAG, 0)
	ROM_LOAD( "olympia_boss_graphics_251-461.bin", 0x0000, 0x800, CRC(56149540) SHA1(b2b893bd219308fc98a38528beb7ddae391c7609) )
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//   YEAR  NAME         PARENT      COMPAT  MACHINE     INPUT       CLASS           INIT  COMPANY                     FULLNAME                FLAGS
COMP(1981, bossa85,     olybossd,   0,      bossa85,    olyboss,    olyboss_state,  0,    "Olympia International",    "Olympia BOSS A 8085",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP(1981, bossb85,     olybossd,   0,      bossb85,    olyboss,    olyboss_state,  0,    "Olympia International",    "Olympia BOSS B 8085",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP(1981, olybossb,    olybossd,   0,      olybossb,   olyboss,    olyboss_state,  0,    "Olympia International",    "Olympia BOSS B",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP(1981, olybossc,    olybossd,   0,      olybossc,   olyboss,    olyboss_state,  0,    "Olympia International",    "Olympia BOSS C",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP(1981, olybossd,    0,          0,      olybossd,   olyboss,    olyboss_state,  0,    "Olympia International",    "Olympia BOSS D",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
