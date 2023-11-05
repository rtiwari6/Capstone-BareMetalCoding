`timescale 1 ns/100 ps


module LED_ctrl (CLK, RESETn, PB_SW1, PB_SW2, LED);

input CLK;
input RESETn;
input PB_SW1;
input PB_SW2;
output [5:0] LED;

// declarations
reg pb1_reg1;
reg pb1_reg2;
reg pb2_reg1;
reg pb2_reg2;
reg [18:0] counter;
reg [5:0] rot_lft;
reg [5:0] rot_rgt;
reg [5:0] LED;
wire sh_clk;
wire [1:0] disp_sel;

// Register PushButton inputs
always @ (posedge CLK or negedge RESETn)
begin
  if (~RESETn)
    begin
    pb1_reg1 <= 1'b1;
    pb1_reg2 <= 1'b1;
    pb2_reg1 <= 1'b1;
    pb2_reg2 <= 1'b1;
    end
  else
    begin
    pb1_reg1 <= PB_SW1;
    pb1_reg2 <= pb1_reg1;
    pb2_reg1 <= PB_SW2;
    pb2_reg2 <= pb2_reg1;
  end
end

assign disp_sel = {pb2_reg2, pb1_reg2};

// free running binary counter
always @ (posedge CLK or negedge RESETn)
begin
  if (~RESETn)
   counter <= 19'b0;
  else
   counter <= counter + 1'b1;
end

assign sh_clk = counter[18];

// rotate left
always @ (posedge sh_clk or negedge RESETn)
begin
  if (~RESETn)
   rot_lft <= 8'b011111;
  else
   rot_lft <= {rot_lft[4:0], rot_lft[5]};
end

// rotate right
always @ (posedge sh_clk or negedge RESETn)
begin
  if (~RESETn)
   rot_rgt <= 8'b111110;
  else
   rot_rgt <= {rot_rgt[0], rot_rgt[5:1]};
end

// LED display
always @ (*)
begin
  if (~RESETn)
    LED <= 8'b111111;				// LEDs off
  else
    case (disp_sel)
	2'b11:	LED <= {1'b1, 1'b1, counter[18], counter[18], ~counter[18], ~counter[18]};	// toggle Red and Green LEDs (S4, S2 released)
	2'b10:	LED <= rot_lft;		// rotate left (S4 released, S2 depressed)
	2'b01:	LED <= rot_rgt;		// rotate right(S4 depressed, S2 released)
	default:	LED <= {counter[18], counter[18],  ~counter[18], ~counter[18], 1'b1, 1'b1}; // toggle Yellow and Blue LEDs (S4, S2 depressed)
    endcase
end

endmodule
