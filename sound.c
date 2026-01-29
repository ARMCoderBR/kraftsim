#include <pthread.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>

#include "sound.h"

////////////////////////////////////////////////////////////////////////////////
void *sound_thread(void *arg){

    sound_t *p = (sound_t*)arg;

    p->endthread = 0;

    uint8_t *bufplay = malloc(p->bufsize);

    int lowlimit = p->bufsize>>2;

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

            SDL_QueueAudio(p->sdl_audio,  //SDL_AudioDeviceID dev,
                           bufplay,       //const void *data,
                           qty            //Uint32 len
                           );

            while (SDL_GetQueuedAudioSize(p->sdl_audio) > (SOUND_SAMPLES-30)) usleep(50);
        }

       usleep(10);
    }

    free (bufplay);

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
void start_sound_thread(sound_t *p){

    pthread_create(&p->mythread, NULL, sound_thread, p);
}

////////////////////////////////////////////////////////////////////////////////
void wait_thread(sound_t *p){

    pthread_join(p->mythread,NULL);
}

////////////////////////////////////////////////////////////////////////////////
sound_t *sound_init(void){

    sound_t *s = malloc(sizeof(sound_t));
    memset(s,0,sizeof(sound_t));

    SDL_AudioSpec sa_desired;
    SDL_zero(sa_desired);
    sa_desired.freq = 125000;
    sa_desired.format = AUDIO_U8;
    sa_desired.channels = 1;
    sa_desired.samples = SOUND_SAMPLES;
    sa_desired.callback = NULL;
    sa_desired.userdata = NULL;

    SDL_AudioSpec sa_obtained;
    SDL_zero(sa_obtained);

    s->sdl_audio = SDL_OpenAudioDevice(
                              NULL, //const char *device,
                              0,    //int iscapture,
                              &sa_desired,  //const SDL_AudioSpec *desired,
                              &sa_obtained, //SDL_AudioSpec *obtained,
                              0     //int allowed_changes
                              );

    if (s->sdl_audio < 0){

        printf("AUDIO OPEN ERROR!\n");
        exit(0);
    }

    SDL_PauseAudioDevice(s->sdl_audio, 0);

    s->buf = malloc(PSGBUFSZ);
    s->bufins = 0;
    s->bufget = 0;
    s->bufqty = 0;
    s->bufsize = PSGBUFSZ;

    pthread_mutex_init(&s->lock,NULL);

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

    SDL_CloseAudioDevice(s->sdl_audio);

    free(s);
}
