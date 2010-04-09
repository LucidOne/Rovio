#ifndef INC__PPOT_H__
#define INC__PPOT_H__


void ppot_disconnect();
bool ppot_connect(const char * const*servers, const int *ports, int num, const char *username, const char *password);
bool ppot_is_connecting();
bool ppot_is_online();


#endif
