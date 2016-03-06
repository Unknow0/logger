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

#define _putc(l, c) putc(c, logger_out(l))
#define _puts(l, s) fputs(s, logger_out(l))

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

static hashmap_t *loggers;
static pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

static const char *logger_prefix;
static size_t prefix_len;

size_t hash_string(void *e)
	{
	size_t h=7;
	char *c=*((char **)e);
	while(*c!=0)
		h=h*17+*c++;
	return h;
	}

static void logger_destroy(logger_t *l)
	{
	if(l->out!=NULL && l->out!=stdout && l->out!=stderr)
		fclose(l->out);
	if(l!=&_default)
		{
		free((char*)l->name);
		free(l);
		}
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

void _log_parse(logger_t *l, int level, const char *fmt, time_t ct, struct tm *t, char *format, va_list ap)
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
					vfprintf(logger_out(l), format, ap);
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
int logger_level(logger_t *l)
	{
	while(l->level==LOG_LEVEL_INH && l->parent!=NULL)
		l=l->parent;
	return l->level==LOG_LEVEL_INH?DEFAULT_LEVEL:l->level;
	}

FILE *logger_out(logger_t *l)
	{
	while(l->out==NULL && l->parent!=NULL)
		l=l->parent;
	return l->out==NULL?DEFAULT_OUT:l->out;
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
	if(logger_level(l)>level)
		return;
	while(l->fmt==NULL && l->parent!=NULL)
		l=l->parent;
	ct=time(NULL);
	localtime_r(&ct, &t);
	va_list ap;
	va_start(ap, format);
	_log_parse(lorg, level, l->fmt==NULL?DEFAULT_FMT:l->fmt, ct, &t, format, ap);
	va_end(ap);
	// TODO if autoflush ??
	fflush(logger_out(lorg));
	}

void logger_init(char *prefix)
	{
	loggers=hashmap_create(4, 0.66, &hash_string, (void (*)(void*))&logger_destroy);
	hashmap_add(loggers, &_default);
	const char *str=cfg_get_string("logger.default.format");

	logger_prefix=prefix?strdup(prefix):"logger";
	prefix_len=strlen(logger_prefix);
	if(str!=NULL)
		_default.fmt=str;
	if(cfg_has_key("logger.default.level"))
		_default.level=cfg_get_int("logger.default.level");
	if(cfg_has_key("logger._default.str_level"))
		{
		json_object *a=cfg_get("logger.default.str_level");
		if(json_object_get_type(a)==json_type_array && json_object_array_length(a)==5)
			{
			int i;
			for(i=0; i<5; i++)
				_default.str_level[i]=json_object_get_string(json_object_array_get_idx(a, i));
			}
		else
			error(NULL, "'logger.default.str_string' should be an array of 5 string");
		}
	const char *out=cfg_get_string("logger.default.out");
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
	_default.parent=NULL;
	}

logger_t *get_logger(const char *name)
	{
	logger_t *l;
	char *key, *n;
	int s;
	if(name==NULL)
		return &_default;
	if(loggers==NULL)
		{
		error(NULL, "You should call 'logger_init()' first!");
		return &_default;
		}
	s=strlen(name)+prefix_len+2;
	key=malloc(s+12);
	if(key==NULL)
		{
		error(NULL, "Failed to allocate");
		return &_default;
		}
	strcpy(key, logger_prefix);
	strcat(key, ".");
	strcat(key, name);
	pthread_mutex_lock(&mutex);
	l=(logger_t *)hashmap_get(loggers, hash_string((void *)&key));
	if(l!=NULL)
		{
		pthread_mutex_unlock(&mutex);
		return l;
		}
	l=malloc(sizeof(logger_t));
	l->name=strdup(key);
	hashmap_add(loggers, l);
	pthread_mutex_unlock(&mutex);
	strcat(key, ".format");
	l->fmt=cfg_get_string(key);
	key[s]=0;
	strcat(key, "level");
	l->level=LOG_LEVEL_INH;
	if(cfg_has_key(key))
		l->level=cfg_get_int(key);
	key[s]=0;
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
	key[s]=0;
	strcat(key, "out");
	const char *out=cfg_get_string(key);
	l->out=NULL;
	if(out!=NULL)
		{
		if(strcmp(out, "stdout")==0)
			l->out=stdout;
		else if(strcmp(out, "stderr")==0)
			l->out=stderr;
		else if(strncmp(out, "file:", 5)==0)
			l->out=fopen(out+5, "a");
		else
			error(&_default, "'%s' should be 'stdout', 'stderr' or 'file:<path to file>' not '%s'", key, out);
		}

	l->parent=&_default;
	key[s-1]=0;
	n=key+prefix_len+1;
	for(;s>prefix_len;s--)
		{
		if(n[s-prefix_len]=='.')
			{
			n[s-prefix_len]=0;
			l->parent=get_logger(n);
			n[s-prefix_len]='.';
			break;
			}
		}
	free(key);

	return l;
	}

void logger_deinit()
	{
	hashmap_destroy(loggers);
	}
