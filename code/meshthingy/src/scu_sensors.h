#ifndef SCU_SENSORS_H
#define SCU_SENSORS_H

int scu_ccs811_init(void);
double scu_ccs811_read_voc(void);
double scu_ccs811_read_eco2(void);
int scu_hts221_init(void);
double scu_hts221_read_temp(void);
double scu_hts221_read_hum(void);
int scu_lis2dh_init(void);
double scu_lis2dh_read(int axis);
int scu_lps22hb_init(void);
double scu_lps22hb_read(void);
#endif