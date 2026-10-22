#ifndef UWB_CONTROL_H
#define UWB_CONTROL_H

#include <stdint.h>

/* Build and send the selected f,<14 parameters>\r configuration command. */
uint8_t uwb_control_send_configuration(void);

#endif /* UWB_CONTROL_H */
