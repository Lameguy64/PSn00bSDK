const static huff_t table0[]={
	{6,3},{8,5},{10,6},{12,8},{76,9},{66,9},{20,11},{58,13},{48,13},{38,13},{32,13},{52,14},{50,14},{48,14},{46,14},{62,15},{60,15},{58,15},{56,15},{54,15},{52,15},{50,15},{48,15},{46,15},{44,15},{42,15},{40,15},{38,15},{36,15},{34,15},{32,15},{48,16},{46,16},{44,16},{42,16},{40,16},{38,16},{36,16},{34,16},{32,16},
};
const static huff_t table1[]={
	{6,4},{12,7},{74,9},{24,11},{54,13},{44,14},{42,14},{62,16},{60,16},{58,16},{56,16},{54,16},{52,16},{50,16},{38,17},{36,17},{34,17},{32,17},
};
const static huff_t table2[]={
	{10,5},{8,8},{22,11},{40,13},{40,14},
};
const static huff_t table3[]={
	{14,6},{72,9},{56,13},{38,14},
};
const static huff_t table4[]={
	{12,6},{30,11},{36,13},
};
const static huff_t table5[]={
	{14,7},{18,11},{36,14},
};
const static huff_t table6[]={
	{10,7},{60,13},{40,17},
};
const static huff_t table7[]={
	{8,7},{42,13},
};
const static huff_t table8[]={
	{14,8},{34,13},
};
const static huff_t table9[]={
	{10,8},{34,14},
};
const static huff_t table10[]={
	{78,9},{32,14},
};
const static huff_t table11[]={
	{70,9},{52,17},
};
const static huff_t table12[]={
	{68,9},{50,17},
};
const static huff_t table13[]={
	{64,9},{48,17},
};
const static huff_t table14[]={
	{28,11},{46,17},
};
const static huff_t table15[]={
	{26,11},{44,17},
};
const static huff_t table16[]={
	{16,11},{42,17},
};
const static huff_t table17[]={
	{62,13},
};
const static huff_t table18[]={
	{52,13},
};
const static huff_t table19[]={
	{50,13},
};
const static huff_t table20[]={
	{46,13},
};
const static huff_t table21[]={
	{44,13},
};
const static huff_t table22[]={
	{62,14},
};
const static huff_t table23[]={
	{60,14},
};
const static huff_t table24[]={
	{58,14},
};
const static huff_t table25[]={
	{56,14},
};
const static huff_t table26[]={
	{54,14},
};
const static huff_t table27[]={
	{62,17},
};
const static huff_t table28[]={
	{60,17},
};
const static huff_t table29[]={
	{58,17},
};
const static huff_t table30[]={
	{56,17},
};
const static huff_t table31[]={
	{54,17},
};
const static huff_t *huff_table[]={
	table0,table1,table2,table3,table4,table5,table6,table7,table8,table9,table10,table11,table12,table13,table14,table15,table16,table17,table18,table19,table20,table21,table22,table23,table24,table25,table26,table27,table28,table29,table30,table31,
};
const static int maxlevel[]={
	40,18,5,4,3,3,3,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
};
