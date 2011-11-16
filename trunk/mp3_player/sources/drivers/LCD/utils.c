void NumToString(unsigned char num, char * str, unsigned char length)
{
    unsigned char tmp = num;
    //str[length] = 0;
    length;
    while(length)
    {
        if(tmp)
            str[length-1]=tmp%10+48;
        else
            str[length-1]=' ';//space
        tmp = tmp/10;
        length--;
    }
} 