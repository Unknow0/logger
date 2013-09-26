/*******************************************************************************
 * This file is part of Logger.
 *
 * Logger is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Logger is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along
 * with Logger; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 ******************************************************************************/
#ifndef _LOGGER_H
#define _LOGGER_H

#include <stdio.h>

#define LOG_LEVEL_DBG 0
#define LOG_LEVEL_INF 1
#define LOG_LEVEL_WRN 2
#define LOG_LEVEL_ERR 3
#define LOG_LEVEL_FTL 4
#define LOG_LEVEL_OFF 5

#define log_debug(l, ...)	log(l, LOG_LEVEL_DBG, __VA_ARGS__)
#define log_info(l, ...)	log(l, LOG_LEVEL_INF, __VA_ARGS__)
#define log_warn(l, ...)	log(l, LOG_LEVEL_WRN, __VA_ARGS__)
#define log_error(l, ...)	log(l, LOG_LEVEL_ERR, __VA_ARGS__)
#define log_fatal(l, ...)	log(l, LOG_LEVEL_FTL, __VA_ARGS__)

typedef struct logger
	{
	char *name;
	char *fmt;
	int level;
	char **str_level;
	FILE *out;
	} logger_t;

/** log something with the logger "name" or root if this logger doesn't exist. */
void _log(logger_t *logger, int level, char *fmt, ...);

/** get or create a logger. if name==NULL you will get root logger */
logger_t *get_logger(char *name);

#endif
