void encode(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        str[i] = (str[i] + 3) % 128;
    }
}

void decode(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        str[i] = (str[i] + 125) % 128;
    }
}
