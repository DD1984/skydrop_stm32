void dump_line(void *addr, int len, int line_len)
{
	if (len <= 0)
		return;
	DEBUG(" %04x  ", addr);
	char *ptr = (char *)addr;
	while (ptr - (char *)addr < len) {
		DEBUG("%02x ", (unsigned char)*ptr);
		ptr++;
	}

	int i;
	for (i = 0; i < line_len - len; i++)
		DEBUG("   ");

	ptr = (char *)addr;
	while (ptr - (char *)addr < len) {
		DEBUG("%c", ((unsigned char)*ptr < 0x20 || (unsigned char)*ptr > 0x7e) ? '.' : (unsigned char)*ptr);
		ptr++;
	}

	DEBUG("\n");
}

void _hex_dump(void *addr, int len, int line_len)
{
	char *ptr = (char *)addr;
	while (ptr - (char *)addr < len) {
		dump_line(ptr, ((char *)addr + len - ptr > line_len) ? line_len : (char *)addr + len - ptr, line_len);
		ptr += line_len;
	}
}
