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

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <langinfo.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include <cfg.h>
#include <container/hashmap.h>

#include "logger.h"

#define LEAPYEAR(year) ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

#define _putc(l, c) putc(c, l->out==NULL?DEFAULT_OUT:l->out)
#define _puts(l, s) fputs(s, l->out==NULL?DEFAULT_OUT:l->out)

logger_t _default={
	.name="default",
	.fmt=DEFAULT_FMT,
	.level=DEFAULT_LEVEL,
	.str_level[0]=NULL,
	.str_level[1]=NULL,
	.str_level[2]=NULL,
	.str_level[3]=NULL,
	.str_level[4]=NULL,
	.out=NULL};

static char *_default_str_level[5]={"DBG", "INF", "WRN", "ERR", "FTL"};

hashmap_t *loggers;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

size_t hash_string(void *e)
	{
	size_t h=0;
	char *c=*((char **)e);
	while(*c!=0)
		h=(h<<2)+*c++;
	return h;
	}

void _fmt(logger_t *l, int nbr, int nb, int padding)
	{
	int i;
	for(i=1; nb>1; i*=10, nb--);
	while(nbr<i)
		{
		_putc(l, padding);
		i/=10;
		}
	while(i>0)
		{
		_putc(l, nbr/i+'0');
		nbr%=i;
		i/=10;
		}
	}

void _log_parse(logger_t *l, int level, char *fmt, time_t ct, struct tm *t, char *format, va_list ap)
	{
	for(; *fmt; ++fmt)
		{
		if(*fmt=='%')
			{
			switch(*++fmt)
				{
				case '\0':
					--fmt;
					break;
				case 'A':
					if(t->tm_wday<0 || t->tm_wday>6)
						_putc(l, '?');
					else
						_puts(l, nl_langinfo(DAY_1+t->tm_wday));
					continue;
				case 'a':
					if(t->tm_wday<0 || t->tm_wday>6)
						_putc(l, '?');
					else
						_puts(l, nl_langinfo(ABDAY_1+t->tm_wday));
					continue;
				case 'B':
					if(t->tm_mon<0 || t->tm_mon>11)
						_putc(l, '?');
					else
						_puts(l, nl_langinfo(MON_1+t->tm_mon));
					continue;
				case 'b':
				case 'h':
					if(t->tm_mon<0 || t->tm_mon>11)
						_putc(l, '?');
					else
						_puts(l, nl_langinfo(ABMON_1+t->tm_mon));
					continue;
				case 'C':
					{
					int i=t->tm_year+1900/100;
					_fmt(l, i, 2, '0');
					continue;
					}
				case 'c':
					_log_parse(l, level, nl_langinfo(D_T_FMT), ct, t, format, ap);
					continue;
				case 'D':
					_log_parse(l, level, "%m/%d/%y", ct, t, format, ap);
					continue;
				case 'd':
				case 'e':
					_fmt(l, t->tm_mday, 2, *fmt=='d'?'0':' ');
					continue;
				case 'F':
					_log_parse(l, level, "%Y-%m-%d", ct, t, format, ap);
					continue;
				case 'H':
				case 'k':
					_fmt(l, t->tm_hour, 2, *fmt=='H'?'0':' ');
					continue;
				case 'I':
				case 'l':
					_fmt(l, t->tm_hour % 12 ? t->tm_hour % 12 : 12, 2, *fmt=='I'?'0':' ');
					continue;
				case 'j':
					_fmt(l, t->tm_yday+1, 3, '0');
					continue;
				case 'M':
					_fmt(l, t->tm_min, 2, '0');
					continue;
				case 'm':
					_fmt(l, t->tm_mon+1, 2, '0');
					continue;
				case 'P':
				case 'p':
					if(t->tm_hour >= 12)
						_puts(l, *fmt=='P'?"pm":"PM");
					else
						_puts(l, *fmt=='P'?"am":"AM");
					continue;
				case 'R':
					_log_parse(l, level, "%H:%M", ct, t, format, ap);
					continue;
				case 'r':
					_log_parse(l, level, "%I:%M:%S %p", ct, t, format, ap);
					continue;
				case 'S':
					_fmt(l, t->tm_sec, 2, '0');
					continue;
				case 's':
					{
					time_t mkt=mktime(t);
					unsigned long int i;
					for(i=1; i*10<mkt; i*=10);
					while(mkt>0)
						{
						_putc(l, mkt/i+'0');
						mkt/=10;
						i/=10;
						}
					continue;
					}
				case 'T':
					_log_parse(l, level, "%H:%M:%S", ct, t, format, ap);
					continue;
				case 't':
					_putc(l, '\t');
					continue;
				case 'U':
					_fmt(l, (t->tm_yday + 7 - t->tm_wday) / 7, 2, '0');
					continue;
				case 'u':
					_fmt(l, t->tm_wday == 0 ? 7 : t->tm_wday, 1, ' ');
					continue;
				case 'V':   // ISO 8601 week number
				case 'G':   // ISO 8601 year (four digits)
				case 'g': { // ISO 8601 year (two digits)
					int  year;
					int	yday;
					int	wday;
					int	w;

					year = t->tm_year + 1900;
					yday = t->tm_yday;
					wday = t->tm_wday;
					while (1)
						{
						int	len;
						int	bot;
						int	top;

						len = LEAPYEAR(year) ? 366 : 365;
						bot = ((yday + 11 - wday) % 7) - 3;
						top = bot - (len % 7);
						if (top < -3) top += 4;
						top += len;
						if (yday >= top)
							{
							++year;
							w = 1;
							break;
							}
						if (yday >= bot)
							{
							w = 1 + ((yday - bot) / 7);
							break;
							}
						--year;
						yday += LEAPYEAR(year) ? 366 : 365;
						}
					if (*fmt == 'V')
						_fmt(l, w, 2, '0');
					else if (*fmt == 'g')
						_fmt(l, year % 100, 2, '0');
					else
						_fmt(l, year, 4, '0');
					continue;
					}
				case 'v':
					_log_parse(l, level, "%e-%b-%Y", ct, t, format, ap);
					continue;
				case 'W':
					_fmt(l, (t->tm_yday + 7 - (t->tm_wday ? (t->tm_wday - 1) : 6)) / 7, 2, '0');
					continue;
				case 'w':
					_fmt(l, t->tm_wday, 1, '0');
					continue;
				case 'X':
					_log_parse(l, level, nl_langinfo(T_FMT), ct, t, format, ap);
					continue;
				case 'x':
					_log_parse(l, level, nl_langinfo(D_FMT), ct, t, format, ap);
					continue;
				case 'y':
					_fmt(l, (t->tm_year+1900) % 100, 2, '0');
					continue;
				case 'Y':
					_fmt(l, t->tm_year+1900, 4, '0');
					continue;
				case 'z':
					{
					int diff=t->tm_gmtoff;
					if(diff<0)
						{
						diff=-diff;
						_putc(l, '-');
						}
					else 
						_putc(l, '+');
					_fmt(l, diff/3600, 2, '0');
					_fmt(l, (diff%3600)/60, 2, '0');
					continue;
					}
				case 'Z':
					_puts(l, tzname[t->tm_isdst>0?1:0]);
					continue;
				case '+':
					_log_parse(l, level, "%a, %d %b %Y %H:%M:%S %z", ct, t, format, ap);
					continue;
				case '_':
					vfprintf(l->out==NULL?DEFAULT_OUT:l->out, format, ap);
					continue;
				case 'L':
					_puts(l, l->str_level[level]==NULL?_default_str_level[level]:l->str_level[level]);
					continue;
				case 'n':
					_puts(l, l->name);
					continue;
				case '%':
				default:
					break;
				}
			}
		_putc(l, *fmt);
		}
	}

void _l(logger_t *lorg, int level, char *format, ...)
	{
	int i;
	time_t ct;
	struct tm t;
	char *fmt;
	logger_t *l=lorg;
	if(l==NULL)
		l=&_default;
	if(l->level>level)
		return;
	while(l->fmt==NULL && l->parent!=NULL)
		l=l->parent;
	ct=time(NULL);
	localtime_r(&ct, &t);
	va_list ap;
	va_start(ap, format);
	_log_parse(lorg, level, l->fmt==NULL?DEFAULT_FMT:l->fmt, ct, &t, format, ap);
	va_end(ap);
	fflush(l->out==NULL?DEFAULT_OUT:l->out);
	}

void logger_init()
	{
	cfg_init();
	loggers=hashmap_create(4, 0.66, &hash_string);
	hashmap_add(loggers, &_default);
	char *str=cfg_get_string("logger.default.format");
	if(str!=NULL)
		_default.fmt=str;
	if(cfg_has_key("logger.default.level"))
		_default.level=cfg_get_int("logger.default.level");
	if(cfg_has_key("logger._default.str_level"))
		{
		json_object *a=cfg_get("logger.default.str_level");
		if(json_object_get_type(a)==json_type_array && json_object_array_length(a)<6)
			{
			int i;
			for(i=0; i<5; i++)
				_default.str_level[i]=json_object_get_string(json_object_array_get_idx(a, i));
			}
		else
			error(NULL, "'logger.default.str_string' should be an array of 5 string");
		}
	char *out=cfg_get_string("logger.default.out");
	if(out!=NULL)
		{
		if(strcmp(out, "stdout")==0)
			_default.out=stdout;
		else if(strcmp(out, "stderr")==0)
			_default.out=stderr;
		else if(strncmp(out, "file:", 5)==0)
			_default.out=fopen(out+5, "a");
		else
			error(NULL, "'logger.default.out' should be 'stdout', 'stderr' or 'file:<path to file>' not '%s'", out);
		}
	_default->parent=NULL;
	}

logger_t *get_logger(const char *name)
	{
	logger_t *l;
	char *key;
	int s;
	if(name==NULL)
		return &_default;
	if(loggers==NULL)
		{
		error(NULL, "You should call 'logger_init()' first!");
		return &_default;
		}
	pthread_mutex_lock(&mutex);
	l=(logger_t *)hashmap_get(loggers, hash_string((void *)&name));
	if(l!=NULL)
		{
		pthread_mutex_unlock(&mutex);
		size_t h1=hash_string(&name), h2=hash_string(&l->name);
		_l(&_default, 0, "From cache %s->%s\n\t%x->%d\n\t%x->%d", name, l->name, h1, h1%loggers->map_size, h2, h2%loggers->map_size);
		return l;
		}
	l=malloc(sizeof(logger_t));
	l->name=strdup(name);
	hashmap_add(loggers, l);
	pthread_mutex_unlock(&mutex);
	s=strlen(name);
	key=malloc(s+18);
	strcpy(key, "logger.");
	strcat(key, name);
	strcat(key, ".format");
	l->fmt=cfg_get_string(key);
	key[s+8]=0;
	strcat(key, "level");
	l->level=DEFAULT_LEVEL;
	if(cfg_has_key(key))
		l->level=cfg_get_int(key);
	key[s+8]=0;
	strcat(key, "str_level");
	memset(l->str_level, 0, sizeof(char*)*5);
	if(cfg_has_key(key))
		{
		json_object *a=cfg_get(key);
		if(json_object_get_type(a)==json_type_array && json_object_array_length(a)<6)
			{
			int i;
			for(i=0; i<5; i++)
				l->str_level[i]=json_object_get_string(json_object_array_get_idx(a, i));
			}
		else
			error(NULL, "'%s' should be an array of 5 string", key);
		}
	key[s+8]=0;
	strcat(key, "out");
	char *out=cfg_get_string(key);
	l->out=DEFAULT_OUT;
	if(out!=NULL)
		{
		if(strcmp(out, "stdout")==0)
			l->out=stdout;
		else if(strcmp(out, "stderr")==0)
			l->out=stderr;
		else if(strncmp(out, "file:", 5)==0)
			l->out=fopen(out+5, "a");
		else
			error(NULL, "'%s' should be 'stdout', 'stderr' or 'file:<path to file>' not '%s'", key, out);
		}

	return l;
	}
