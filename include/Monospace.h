#ifndef MONOSPACE_H
#define MONOSPACE_H

void Monospace_setup(void);
void Monospace_cleanup(void);
float Monospace_getStringWidth(const char *str, float height);
void Monospace_renderCenteredString(const char *str, float height);

#endif /* MONOSPACE_H */
