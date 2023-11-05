library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity LED_ctrl is
port (
	CLK: in std_logic;
	RESETn: in std_logic;   
	PB_SW1: in std_logic;
	PB_SW2: in std_logic;
	LED: out std_logic_vector(5 downto 0)
);
end LED_ctrl;

architecture RTL of LED_ctrl is

-- signal declarations
signal pb1_reg1: std_logic;
signal pb1_reg2: std_logic;
signal pb2_reg1: std_logic;
signal pb2_reg2: std_logic;
signal counter: std_logic_vector(18 downto 0);
signal rot_lft: std_logic_vector(5 downto 0);
signal rot_rgt: std_logic_vector(5 downto 0);
signal sh_clk: std_logic;
signal disp_sel: std_logic_vector(1 downto 0);

begin

-- Register PushButton input
PB_reg: process (CLK, RESETn)
begin
	if (RESETn = '0') then
	  pb1_reg1 <= '1';
	  pb1_reg2 <= '1';
	  pb2_reg1 <= '1';
	  pb2_reg2 <= '1';
	elsif (CLK'event and CLK = '1') then
	  pb1_reg1 <= PB_SW1;
	  pb1_reg2 <= pb1_reg1;
	  pb2_reg1 <= PB_SW2;
	  pb2_reg2 <= pb2_reg1;
	end if;
 end process PB_reg;

-- free running binary counter
CNTR: process (CLK, RESETn)
begin
	if (RESETn = '0') then
	  counter <= (others => '0');
	elsif (CLK'event and CLK = '1') then
	  counter <= counter + 1;
	end if;
end process CNTR;

sh_clk <= counter(17);			-- clock for shift registers
disp_sel <= pb2_reg2 & pb1_reg2;
-- rotate left
ROT_L: process (sh_clk, RESETn)
begin
	if (RESETn = '0') then
	  rot_lft <= "011111";
	elsif (sh_clk'event and sh_clk = '1') then
	  rot_lft <= rot_lft(4 downto 0) & rot_lft(5);
	end if;
end process ROT_L;

-- rotate right
ROT_R: process (sh_clk, RESETn)
begin
	if (RESETn = '0') then
	  rot_rgt <= "111110";
	elsif (sh_clk'event and sh_clk = '1') then
	  rot_rgt <= rot_rgt(0) & rot_rgt(5 downto 1);
	end if;
end process ROT_R;

-- LED display
DISP: process (RESETn, pb1_reg2, pb2_reg2, rot_lft, rot_rgt, counter(18))
begin
	if (RESETn = '0') then
		LED <= (others => '1');									-- LEDs are off
	else
		case (disp_sel) is
			when "11" => LED <= ('1' & '1'& counter(18) & counter(18) & not counter(18) & not counter(18));	         -- blink Toggle Red and orang LEDs (S4, S2 released)			
			when "10" => LED <= rot_lft;						-- rotate left (S4 released, S2 depressed)
			when "01" => LED <= rot_rgt;						-- rotate right(S4 depressed, S2 released)  
			when others => LED <= ( counter(18) & counter(18) & not counter(18) & not counter(18) & '1' & '1' );		-- Toggle Yellow and orange LEDs (S4, S2 depressed)
		end case;
	end if;
end process DISP;

end RTL;
