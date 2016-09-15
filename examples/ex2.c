
#include <stdio.h>

#include <BearLibTerminal.h>
#include <butterfly.h>

int main(void)
{
	terminal_open();

	while (terminal_read() != TK_CLOSE) {
	}

	terminal_close();
	return 0;
}
