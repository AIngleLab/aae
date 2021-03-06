/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to you under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 * 
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "aingle/errors.h"

/* 4K should be enough, right? */
#define AINGLE_ERROR_SIZE 4096

/*
 * To support the aingle_prefix_error function, we keep two string buffers
 * around.  The AINGLE_CURRENT_ERROR points at the buffer that's holding
 * the current error message.  aingle_prefix error writes into the other
 * buffer, and then swaps them.
 */

struct aingle_error_data_t {
    char  AINGLE_ERROR1[AINGLE_ERROR_SIZE];
    char  AINGLE_ERROR2[AINGLE_ERROR_SIZE];

    char  *AINGLE_CURRENT_ERROR;
    char  *AINGLE_OTHER_ERROR;
};


#if defined THREADSAFE 
#if ( defined __unix__ || defined __unix )
#include <pthread.h>
static pthread_key_t error_data_key;
static pthread_once_t error_data_key_once = PTHREAD_ONCE_INIT;

static void make_error_data_key()
{
    pthread_key_create(&error_data_key, free);
}
#elif defined _WIN32
#include <Windows.h>

static __declspec( thread ) struct aingle_error_data_t TLS_ERROR_DATA = { "", "", NULL, NULL };

#endif /* unix||_unix||_WIN32 */
#endif /* THREADSAFE */

static struct aingle_error_data_t *
aingle_get_error_data(void)
{
#if defined THREADSAFE  
#if defined __unix__ || defined __unix

    pthread_once(&error_data_key_once, make_error_data_key);

    struct aingle_error_data_t *ERROR_DATA =
        (struct aingle_error_data_t*) pthread_getspecific(error_data_key);

    if (!ERROR_DATA) {
        ERROR_DATA = (struct aingle_error_data_t*) malloc(sizeof(struct aingle_error_data_t));
        pthread_setspecific(error_data_key, ERROR_DATA);

        ERROR_DATA->AINGLE_ERROR1[0] = '\0';
        ERROR_DATA->AINGLE_ERROR2[0] = '\0';
        ERROR_DATA->AINGLE_CURRENT_ERROR = ERROR_DATA->AINGLE_ERROR1;
        ERROR_DATA->AINGLE_OTHER_ERROR = ERROR_DATA->AINGLE_ERROR2;
    }

    return ERROR_DATA;

#elif defined _WIN32
	
	if ( TLS_ERROR_DATA.AINGLE_CURRENT_ERROR == NULL )
	{
		//first usage of the ERROR_DATA, initialize 'current' and 'other' pointers.
		TLS_ERROR_DATA.AINGLE_CURRENT_ERROR = TLS_ERROR_DATA.AINGLE_ERROR1;
		TLS_ERROR_DATA.AINGLE_OTHER_ERROR = TLS_ERROR_DATA.AINGLE_ERROR2;
	}
	return &TLS_ERROR_DATA;

	#endif /* UNIX and WIN32 threadsafe handling */

#else /* not thread-safe */  
    static struct aingle_error_data_t ERROR_DATA = {
      /* .AINGLE_ERROR1 = */ {'\0'},
      /* .AINGLE_ERROR2 = */ {'\0'},
      /* .AINGLE_CURRENT_ERROR = */ ERROR_DATA.AINGLE_ERROR1,
      /* .AINGLE_OTHER_ERROR = */ ERROR_DATA.AINGLE_ERROR2,
    };

    return &ERROR_DATA;
#endif
}


void
aingle_set_error(const char *fmt, ...)
{
	va_list  args;
	va_start(args, fmt);
	vsnprintf(aingle_get_error_data()->AINGLE_CURRENT_ERROR, AINGLE_ERROR_SIZE, fmt, args);
	va_end(args);
	//fprintf(stderr, "--- %s\n", AINGLE_CURRENT_ERROR);
}


void
aingle_prefix_error(const char *fmt, ...)
{
    struct aingle_error_data_t *ERROR_DATA = aingle_get_error_data();

	/*
	 * First render the prefix into OTHER_ERROR.
	 */

	va_list  args;
	va_start(args, fmt);
	int  bytes_written = vsnprintf(ERROR_DATA->AINGLE_OTHER_ERROR, AINGLE_ERROR_SIZE, fmt, args);
	va_end(args);

	/*
	 * Then concatenate the existing error onto the end.
	 */

	if (bytes_written < AINGLE_ERROR_SIZE) {
		strncpy(&ERROR_DATA->AINGLE_OTHER_ERROR[bytes_written], ERROR_DATA->AINGLE_CURRENT_ERROR,
			AINGLE_ERROR_SIZE - bytes_written);
		ERROR_DATA->AINGLE_OTHER_ERROR[AINGLE_ERROR_SIZE-1] = '\0';
	}

	/*
	 * Swap the two error pointers.
	 */

	char  *tmp;
	tmp = ERROR_DATA->AINGLE_OTHER_ERROR;
	ERROR_DATA->AINGLE_OTHER_ERROR = ERROR_DATA->AINGLE_CURRENT_ERROR;
	ERROR_DATA->AINGLE_CURRENT_ERROR = tmp;
	//fprintf(stderr, "+++ %s\n", AINGLE_CURRENT_ERROR);
}


const char *aingle_strerror(void)
{
	return aingle_get_error_data()->AINGLE_CURRENT_ERROR;
}
