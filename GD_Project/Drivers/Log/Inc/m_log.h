#ifndef M_LOG_H
#define M_LOG_H

#include <stdint.h>

void m_log_init(uint32_t baudrate);
void m_log_write(const uint8_t *data, uint16_t length);

#endif /* M_LOG_H */
