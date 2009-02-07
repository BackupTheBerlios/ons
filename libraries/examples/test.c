            #include <ons/ons.h>
            #include <stdio.h>

            ons_mutex_t mutex;
            unsigned int val;
            void *callback(void *arg);

            void *callback(void *arg) {
                unsigned int *begin = arg;

                val = *begin;
                while(1) {
                    /* Do we need to return? */
                    if(!ons_mutex_trylock(&mutex)) {
                        return NULL;
                    }

                    ++val;
                    ons_mutex_unlock(&mutex);
                }
            }

            int main() {
                ons_thread_t thread;
                unsigned int begin = 10;

                /* Start thread an let him count from 10 upwards. */
                ons_mutex_init(&mutex);
                ons_thread_run(&thread, callback, &begin);

                /* Now sleep 1 second and then lock the mutex and wait for the thread to exit. */
                ons_sleep(1);
                ons_mutex_lock(&mutex);
                ons_thread_join(&thread);

                ons_mutex_unlock(&mutex);
                ons_mutex_free(&mutex);

                printf("Thread counted: %u\n", val);

                return 0;
            }
