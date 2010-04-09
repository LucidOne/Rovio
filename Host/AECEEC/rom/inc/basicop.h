
short abs_s(short var1)      /* Short abs, 1 */
{

    if (var1 == (short)0x8000 ) {
        return (short)(0x7fff);
    }
    else {
        if (var1 < 0) {
            return (-var1);
        }
        else {
            return (var1);
        }
    }

}


short min_s(short var1,short var2)
{
	if(var1<=var2) return var1;
	else		   return var2;

}

short max_s(short var1,short var2)
{
	if(var1>=var2) return var1;
	else		   return var2;

}

short saturate_s(int var32)
{
	if(var32>=(int)MAX16)
		return MAX16;
	else if(var32<(int)MIN16)
		return MIN16;
	else
	    return (short)var32;	
		

}

short sign_s(short var)
{
	if(var>=0) return 1;
	else	   return -1;

}


short saturate16_add(short var1,short var2)
{
	if((int)(var1+var2)>=(int)MAX16)
		return MAX16;
	else if((int)(var1+var2)<(int)MIN16)
		return MIN16;
	else
		return (var1+var2);		

}