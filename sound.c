#include <pulse/simple.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>

#include "sound.h"

////////////////////////////////////////////////////////////////////////////////
void *sound_thread(void *arg){

    //printf("INIT RUN THREAD\n");
    sound_t *p = (sound_t*)arg;

    p->endthread = 0;

    uint8_t *bufplay = malloc(p->bufsize);

    int lowlimit = p->bufsize>>1;

    while ((!p->endthread)||(p->bufqty)){

        if (p->endthread)
            lowlimit = 0;

        if (p->bufqty > lowlimit){

            pthread_mutex_lock(&p->lock);

            int i;
            for (i = 0; i < p->bufqty; i++){

                bufplay[i] = p->buf[p->bufget++];
                if (p->bufget == p->bufsize)
                    p->bufget = 0;
            }

            int qty = p->bufqty;
            p->bufqty = 0;
            pthread_mutex_unlock(&p->lock);

            pa_simple_write(p->pa_driver,bufplay,qty,&p->pa_error);
        }

       usleep(100);
    }

    free (bufplay);

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
void start_sound_thread(sound_t *p){

    //printf("START THREAD\n");

    /*int res =*/ pthread_create(&p->mythread, NULL, sound_thread, p);
}

////////////////////////////////////////////////////////////////////////////////
void wait_thread(sound_t *p){

    //printf("\nWaiting threads...\n");
    pthread_join(p->mythread,NULL);
}

////////////////////////////////////////////////////////////////////////////////
sound_t *sound_init(void){

    sound_t *s = malloc(sizeof(sound_t));
    memset(s,0,sizeof(sound_t));

    pa_buffer_attr pba;
    pba.fragsize = (uint32_t) -1;
    pba.maxlength = 10000;//(uint32_t) -1;
    pba.minreq = (uint32_t) -1;
    pba.prebuf = (uint32_t) -1;
    pba.tlength = (uint32_t) -1;

    pa_sample_spec ss;

    ss.format = PA_SAMPLE_U8;
    ss.channels = 1;
    ss.rate = 125000;

    s->pa_driver = pa_simple_new(NULL,     // Use the default server.
                      "Chiptune",           // Our application's name.
                      PA_STREAM_PLAYBACK,
                      NULL,               // Use the default device.
                      "Music",            // Description of our stream.
                      &ss,                // Our sample format.
                      NULL,               // Use default channel map
                      &pba,               // Use default buffering attributes.
                      NULL               // Ignore error code.
                      );

    s->buf = malloc(PSGBUFSZ);
    s->bufins = 0;
    s->bufget = 0;
    s->bufqty = 0;
    s->bufsize = PSGBUFSZ;

    //printf("PSG INIT 1\n");

    pthread_mutex_init(&s->lock,NULL);

    //printf("PSG INIT 2\n");

    start_sound_thread(s);

    return s;
}

////////////////////////////////////////////////////////////////////////////////
void sound_send_sample (sound_t *s, uint8_t outval){

    while (s->bufqty == s->bufsize){usleep(100);}

    pthread_mutex_lock(&s->lock);
    s->buf[s->bufins++] = outval;
    if (s->bufins == s->bufsize)
        s->bufins = 0;
    s->bufqty++;
    pthread_mutex_unlock(&s->lock);
}

////////////////////////////////////////////////////////////////////////////////
void sound_end(sound_t *s){

    s->endthread = 1;

    wait_thread(s);
    pthread_mutex_destroy(&s->lock);

    pa_simple_drain(s->pa_driver,&s->pa_error);

    //usleep(250000);

    pa_simple_free(s->pa_driver);

    free(s);
}
