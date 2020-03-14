unsigned char *videobuf = (unsigned char*)0xb8000;
const char *str = "Hello, World !! ";

int kernel_start(void)
{
	int i;
	for (i = 0; str[i]; i++) {
		videobuf[i * 2 + 0] = str[i];
		videobuf[i * 2 + 1] = 0x17;
	}
	for (; i < 80 * 25; i++) {
		videobuf[i * 2 + 0] = ' ';
		videobuf[i * 2 + 1] = 0x17;
	}
	while (1) {}
	return 0;
}