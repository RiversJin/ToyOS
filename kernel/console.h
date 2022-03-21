#ifndef CONSOLE_H
#define CONSOLE_H

void consputc(int c);
int consolewrite(char* src, int n);
int consoleread(char* dst, int n);
void consoleintr(int c);
void consoleinit(void);

#endif /* CONSOLE_H */