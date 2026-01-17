#ifndef __SOUND_H__
#define __SOUND_H__

#define PSGBUFSZ 4000

typedef struct {

    pa_simple *pa_driver;
    int pa_error;

    ////////
    pthread_t mythread;
    int endthread;
    uint8_t *buf;
    int bufins;
    int bufget;
    int bufqty;
    int bufsize;
    pthread_mutex_t lock;

} sound_t;

sound_t *sound_init(void);
void sound_send_sample (sound_t *s, uint8_t outval);
void sound_end(sound_t *s);

#endif

